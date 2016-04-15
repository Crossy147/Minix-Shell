#ifndef _SIPARSE_H_
#define _SIPARSE_H_

typedef struct redirection{
	char *filename;
	int flags;
} redirection;

/*
 * redirection flags
 */
#define RIN 	1
#define ROUT 	(1<<1)
#define RAPPEND	(1<<2)

#define RTMASK (RIN|ROUT|RAPPEND)
#define IS_RIN(x)		 (((x)&RTMASK) == RIN )
#define IS_ROUT(x)		 (((x)&RTMASK) == ROUT )
#define IS_RAPPEND(x)	 (((x)&RTMASK) == (ROUT|RAPPEND) )


typedef struct command {
	char** argv; 			/* NULL ended array of arguments */
	redirection** redirs;	/* NULL ended array of pointers to redirections */
} command;  

/* NULL ended array of pointers to commands */
typedef command** pipeline;

/* NULL ended array of pipelines */
typedef pipeline* pipelineseq;

typedef struct line {
	pipelineseq pipelines;
	int flags;
} line;

/*
 * flags for parsed line
 */
#define LINBACKGROUND 	1
#define IS_BACKGROUND(x) ((x)&LINBACKGROUND) 

/*
 * Parses given string containing sequence of pipelines separated by ';'. 
 * Each pipeline is a sequence of commands separated by '|'.
 * Function returns a pointer to the static structure line or NULL if meets a parse error.
 * All structures referenced from the result of the function are statically allocated and shall not be freed.
 * Passing a string longer than MAX_LINE_LENGHT may result in an unspecified behaviour.
 * Consecutive calls to the function destroy the content of previously returned structures.
 */
line * parseline(char *);

void execute_command(const command* com, int* lfd, int* rfd, int is_background);

#endif /* !_SIPARSE_H_ */
