#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <errno.h>
#include <sys/wait.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <fcntl.h>

#include "errors.h"
#include "config.h"
#include "siparse.h"
#include "builtins.h"

line* parsedLine;
int end;

char lineBuffer[2 * MAX_LINE_LENGTH];
char helperBuffer[2 * MAX_LINE_LENGTH];
int bytesInBuffer;


typedef struct {
	pid_t process;
	int status;
} BackgroundProcess;

BackgroundProcess backgroundProcesses[MAX_CHILDREN_SIZE];

volatile int background_counter;

pid_t foregroundProcesses[MAX_CHILDREN_SIZE];
volatile int foregroundCounter;
volatile int foregroundProcessesSize;

sigset_t waitMask;
sigset_t chld_block_mask;

struct sigaction defaultSigint;
struct sigaction defaultSigChild;
struct sigaction shellSigint;
struct sigaction shellSigChild;

void unblockChildSignal() {
	sigprocmask(SIG_UNBLOCK, &chld_block_mask, NULL);
}

void blockChildSignal() { 
	sigemptyset(&chld_block_mask); 
	sigaddset(&chld_block_mask, SIGCHLD); 
	sigprocmask(SIG_BLOCK, &chld_block_mask, NULL); 
}

void closePipe(int* fileDescriptor) {
	if (fileDescriptor != NULL) {
		close(fileDescriptor[0]);
		close(fileDescriptor[1]);
		free(fileDescriptor);
	}
}

int isPipelineInvalid(pipeline* p) {
	for (command** com = *p + 1; *com != NULL; com++) {
		if ((*com)->argv[0] == NULL) {
			return true;
		}
	}
	return false;
}

int isNextCommandPresent(command** com) {
	return  *(com + 1) != NULL;
}

void findInputRedirection(redirection * redirs[], redirection ** findInputRedirection) {
	for (int i = 0; redirs[i] != NULL; i++) {
		if (IS_RIN(redirs[i]->flags)) {
			*findInputRedirection = redirs[i];
		}
	}
}

void findOutputRedirection(redirection * redirs[], redirection** outputRedirection) {
	for (int i = 0; redirs[i] != NULL; i++) {
		if (IS_ROUT(redirs[i]->flags) || IS_RAPPEND(redirs[i]->flags)) {
			*outputRedirection = redirs[i];
		}
	}
}

void redirectWritePipe(int * writePipe) {
	if (writePipe != NULL) {
		dup2(writePipe[1], STDOUT_FILENO);
		close(writePipe[0]);
		close(writePipe[1]); 
	}
}

void redirectReadPipe(int * readPipe) {
	if (readPipe != NULL) {
		dup2(readPipe[0], STDIN_FILENO); 
		close(readPipe[0]);
	}
}

void CheckAndHandleOpenError(int fd, const char* filename) {
	if (fd != -1) {
		return;
	}
	switch (errno) {
		case EACCES:
			message_error(filename, PERMISSION_DENIED);
			break;
		case ENOENT:
			message_error(filename, NO_SUCH_FILE_OR_DIR);
			break;
		default:
			break;
	}
	exit(EXEC_FAILURE);
}

void redirectInput(struct redirection **redirs) {
	struct redirection *inputRedirection = NULL;
	findInputRedirection(redirs, &inputRedirection);
	if (inputRedirection != NULL) {
		int inputFileDescriptor = open(inputRedirection->filename, O_RDONLY); 
																		
		CheckAndHandleOpenError(inputFileDescriptor, inputRedirection->filename);
		dup2(inputFileDescriptor, STDIN_FILENO);
		close(inputFileDescriptor);
	}
}

void redirectOutput(struct redirection **redirs) {
	struct redirection *outputRedirection = NULL;
	findOutputRedirection(redirs, &outputRedirection);
	if (outputRedirection != NULL) {
		int outputFlag = O_WRONLY | O_CREAT; 
		 
		if (IS_RAPPEND(outputRedirection->flags)) {
			outputFlag |= O_APPEND;
		}
		else {
			outputFlag |= O_TRUNC;
		}
		int outputFileDescriptor = open(outputRedirection->filename, outputFlag, S_IRUSR | S_IWUSR); 

		CheckAndHandleOpenError(outputFileDescriptor, outputRedirection->filename);
		dup2(outputFileDescriptor, STDOUT_FILENO);
		close(outputFileDescriptor);
	}
}

int isBackgroundChild(pid_t pid) {
	for (int i = 0; i < foregroundProcessesSize; ++i) {
		if (foregroundProcesses[i] == pid) {
			return false;
		}
	}
	return true;
}


void shellChildHandler(int sig_nb) {
	pid_t childPid;
	int childStatus;

	while ((childPid = waitpid(-1, &childStatus, WNOHANG)) > 0) {
		if (isBackgroundChild(childPid)) {
			backgroundProcesses[background_counter].process = childPid;
			backgroundProcesses[background_counter].status = childStatus;
			background_counter++;
		}
		else {
			--foregroundCounter;
		}
	}
}

void handleTerminationInfo(int i) {
	if (WIFSIGNALED(backgroundProcesses[i].status)) {
		fprintf(stdout, "Background process %d terminated. (%s, %d)\n", backgroundProcesses[i].process, KILLED, WTERMSIG(backgroundProcesses[i].status));
	} else {
		fprintf(stdout, "Background process %d terminated. (%s, %d)\n", backgroundProcesses[i].process, TERMINATED, backgroundProcesses[i].status);		
	}
}

void printBackgroundInfo() {
	for (int i = 0; i < background_counter; ++i) {
		handleTerminationInfo(i);
	}
	background_counter = 0;
}

void messageSingleError(char * error) {
	fprintf(stderr, "%s\n", error);
	fflush(stderr);
}

void cleanStandardInput() {
	char x = -1;
	int bytesRead = -1;
	while (x != LINE_FEED && bytesRead) {
		bytesRead = read(STDIN_FILENO, &x, 1);
	}
}


line* readAndParseCommand() {
	int totalBytesRead = bytesInBuffer;
	int bytesRead = 0;
	int lastReadBytePosition = 0;

	while (true) {

		int EOFPosition = -1;
		for (int i = lastReadBytePosition; i < totalBytesRead; i++) {
			if (helperBuffer[i] == LINE_FEED) {
				EOFPosition = i;
				break;
			}
		}

		if (EOFPosition == -1) {
			if (totalBytesRead > MAX_LINE_LENGTH) {
				bytesInBuffer = 0;
				if (helperBuffer[bytesRead - 1]) {
					cleanStandardInput();
				}
				messageSingleError(SYNTAX_ERROR_STR);
				return NULL;
			}
		} else {
			bytesInBuffer = totalBytesRead - EOFPosition - 1;
			if (EOFPosition > MAX_LINE_LENGTH) {
				messageSingleError(SYNTAX_ERROR_STR);
				bytesRead = 0;
			}
			else {
				memcpy(lineBuffer, helperBuffer, sizeof(char) * EOFPosition);
				lineBuffer[EOFPosition] = 0;

				bytesRead = EOFPosition;
			}
			memmove(helperBuffer, helperBuffer + EOFPosition + 1, sizeof(char) * bytesInBuffer);
			break;
		}

		bytesRead = read(STDIN_FILENO, helperBuffer + totalBytesRead, MAX_LINE_LENGTH + 1);
		switch (bytesRead) {
			case 0:
				end = true;
				return NULL;
			case -1:
				bytesRead = 0;
				break;
			default:
				lastReadBytePosition = totalBytesRead;
				totalBytesRead += bytesRead;
		}
	}

	if (bytesRead == 0) {
		return NULL;
	}

	line* ln = parseline(lineBuffer);
	if (ln == NULL) {
		messageSingleError(SYNTAX_ERROR_STR);
		return NULL;
	}
	else {
		return ln;
	}
}

char* handleError() {
	switch (errno) {
		case EACCES:
			return PERMISSION_DENIED;
		case EPERM:
			return PERMISSION_DENIED;
		case ENOENT:
			return NO_SUCH_FILE_OR_DIR;
		default:
			return EXEC_ERROR;
	}
}

void runBuiltinCommand(char* program, builtin_ptr builtin, char** arguments) {
	int returnCode = builtin(arguments);
	if (returnCode == BUILTIN_ERROR) {
		fprintf(stderr, "Builtin %s error.\n", program);
		fflush(stderr);
	}
}

void addChildProcess(pid_t childPid) {
	foregroundProcesses[foregroundCounter++] = childPid;
	++foregroundProcessesSize;
}

void runCommand(const command* com, int* leftFileDescriptor, int* rightFileDescriptor, int isBackground) {
	if (com == NULL || *(com->argv) == NULL) {
		return;
	}

	char* program = *(com->argv);
	char **arguments = com->argv;
	struct redirection **redirs = com->redirs;

	builtin_ptr builtin = getBuiltinPtr(program);
	if (builtin) {
		runBuiltinCommand(program, builtin, arguments);
	} else {	
		pid_t childPid = fork();

		switch (childPid) {
			case -1:
				exit(EXEC_FAILURE);

			case 0:
				if (isBackground) {
					setsid(); 
				}


				sigaction(SIGINT, NULL, NULL);
				sigaction(SIGCHLD, NULL, NULL);
				unblockChildSignal();
				redirectReadPipe(leftFileDescriptor);
				redirectWritePipe(rightFileDescriptor);
				redirectInput(redirs);
				redirectOutput(redirs);

				if (execvp(program, arguments) < 0) {
					message_error(program, handleError());
					exit(EXEC_FAILURE);		
				}
				break;

			default:
				addChildProcess(childPid);
				break;
		}
	}
}


void handlePipelineSequence() {
	if (parsedLine == NULL) {
		return;
	}
	int isBackground = parsedLine->flags;
	for (pipeline* p = parsedLine->pipelines; *p != NULL; ++p) {
		if (isPipelineInvalid(p)) {
			fprintf(stderr, "%s\n", SYNTAX_ERROR_STR);
			fflush(stderr);
		} else {
			int* leftPipe = NULL;
			int* rightPipe = NULL;
			foregroundCounter = 0;

			blockChildSignal(); 
			for (command** com = *p; *com != NULL; com++) {
				closePipe(leftPipe);
				leftPipe = rightPipe;
				if (isNextCommandPresent(com)) {
						rightPipe = (int*) malloc(2);
						pipe(rightPipe);
				} else {
					rightPipe = NULL;
				}
				runCommand(*com, leftPipe, rightPipe, isBackground);
			}
			closePipe(leftPipe); 
			if (!isBackground) {
				while (foregroundCounter) { 
					sigsuspend(&waitMask); 
				}
			}
			foregroundProcessesSize = 0;
			unblockChildSignal();
		}
	}
}


void writePrompt() {
	struct stat stdout_stat;
	if (fstat(STDOUT_FILENO, &stdout_stat) >= 0 && S_ISCHR(stdout_stat.st_mode)) {
		blockChildSignal();

		printBackgroundInfo();
	    
	    fprintf(stdout, "%s", PROMPT);
    	fflush(stdout);

		unblockChildSignal();
	}
}

int main(int argc, char *argv[]) {

	shellSigint.sa_handler = SIG_IGN;
	shellSigChild.sa_handler = shellChildHandler;
	shellSigChild.sa_flags = SA_RESTART | SA_NOCLDSTOP;

	sigprocmask(SIG_BLOCK, NULL, &waitMask);
	sigaction(SIGINT, &shellSigint, &defaultSigint); 
	sigfillset(&shellSigChild.sa_mask);
	sigaction(SIGCHLD, &shellSigChild, &defaultSigChild);

	parsedLine = NULL;
	end = false;
	bytesInBuffer = 0;

	while (!end) {
		parsedLine = NULL;
		end = false;
		writePrompt();
		parsedLine = readAndParseCommand();
		handlePipelineSequence();
	}
	return 0;
}
