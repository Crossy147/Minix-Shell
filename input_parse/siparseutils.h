
#define MAX_COMMANDS (MAX_LINE_LENGTH/2 +1)
#define MAX_PIPELINES MAX_COMMANDS
#define MAX_ARGS MAX_COMMANDS
#define MAX_REDIRS MAX_COMMANDS

void resetutils(void);

/*
 * buffer for string from the parsed line
 */

char * copytobuffer(const char *, const short);
void resetbuffer(void);

/* 
 * buffer for args
 * each argv is NULL terminated substring of the buffer
 */

char ** appendtoargv(char*);
char ** closeargv(void);
void resetargvs(void);

/*
 * buffer for commands
 */
command * nextcommand(void);
void resetcommands(void);

/*
 * buffer for redirections
 */
redirection * nextredir(void);
void resetredirs(void);
redirection ** appendtoredirseq(redirection * );

redirection ** closeredirseq(void);
void resetredirseqs(void);

pipeline appendtopipeline(command *);
pipeline closepipeline(void);
void resetpipelines(void);


pipelineseq appendtopipelineseq(command **);
pipelineseq closepipelineseq(void);
void resetpipelineseqs(void);
	
