#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <dirent.h>
#include <sys/param.h>
#include <signal.h>

#include "builtins.h"
#include "config.h"

int builtinEcho(char*[]);
int builtinExit(char*[]);
int builtinCd(char*[]);
int builtinLs(char*[]);
int builtinKill(char*[]);
int undefined(char*[]);


builtin_pair builtins_table[]={
	{"exit",	&builtinExit},
	{"lecho",	&builtinEcho},
	{"lcd",		&builtinCd},
	{"lkill", 	&builtinKill},
	{"lls",		&builtinLs},
	{NULL,NULL}
};

builtin_ptr getBuiltinPtr(char* program) {
	for (int index = 0; builtins_table[index].name != NULL; ++index) {
		if (strcmp(program, builtins_table[index].name) == 0) {
			return builtins_table[index].fun;
		}
	}
	return NULL;
}

int builtinExit(char * argv[]) {
	exit(0);
}
int builtinEcho(char * argv[]) {
	int i = 1;
	if (argv[i]) printf("%s", argv[i++]);
	while  (argv[i])
		printf(" %s", argv[i++]);

	printf("\n");
	fflush(stdout);
	return 0;
}

int builtinKill(char * argv[]) {
	if (argv[1] == NULL) {
		return BUILTIN_ERROR;
	}

	char* pidNumber;
	if (argv[2] == NULL) {
		pidNumber = argv[1];
	} else {
		if (argv[3] != NULL) {
			return BUILTIN_ERROR;
		}
		pidNumber = argv[2];
	}

	int sig;
	if (argv[1] != NULL) {
		sig = getSignal(argv[1]);
		if (sig < 0) {
			return BUILTIN_ERROR;
		}
	} else {
		sig = SIGTERM;
	}

	int pid = atoi(pidNumber);
	char str[strlen(pidNumber)];
	sprintf(str, "%d", pid);
	if (strcmp(str, pidNumber) != 0) {
		return BUILTIN_ERROR;	
	}

	if (kill(pid, sig) == -1) {
		return BUILTIN_ERROR;
	}

	return 0;
}

int builtinCd(char * argv[]) {
    int c = -1;
    if (argv[1] == NULL) {
        c = chdir(getenv(HOME));
        if(c == -1) {
            return BUILTIN_ERROR;
        }
        return 0;
    }

    if (argv[2] != NULL) {
        return BUILTIN_ERROR;
    }

    c = chdir(argv[1]);
    if (c == -1) {
        return BUILTIN_ERROR;
    }
    return 0;
}

int getSignal(char * string) {
	char num[strlen(string)];
	if (string[0] != '-') {
		return -1;
	}
	int sig = atoi(string + 1);
	sprintf(num, "%d", sig);
	if (strcmp(num, string + 1) != 0) {
		return -1;
	}
	return sig;
}

void printWithoutDotStarted(struct dirent * entry) {
	if (strncmp(entry->d_name, ".", 1) != 0) {
		fprintf(stdout, "%s\n", entry->d_name);
		fflush(stdout);		
	}
}

int builtinLs(char * argv[]) {
	char* additionalArgument = argv[1];
	if (additionalArgument == NULL) {
		char path[PATH_MAX];
		if (getcwd(path, PATH_MAX) == NULL) {
			return BUILTIN_ERROR;
		}
		DIR* files = opendir(path);
		if (files == NULL) {
			return BUILTIN_ERROR;
		}
		struct dirent* entry;
		while ((entry = readdir(files)) != NULL) {
			printWithoutDotStarted(entry);
		}
		closedir(files);

		return 0;
	}
	return BUILTIN_ERROR;
}

int undefined(char * argv[]) {
	fprintf(stderr, "Command %s undefined.\n", argv[0]);
	return BUILTIN_ERROR;
}
