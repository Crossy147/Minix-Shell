#define SSTRING 257
#define OAPPREDIR 258
#define COMMENT 259
#ifdef YYSTYPE
#undef  YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
#endif
#ifndef YYSTYPE_IS_DECLARED
#define YYSTYPE_IS_DECLARED 1
typedef union{
	int flags;
	char *name;
	char **argv;
	redirection *redir;
	redirection **redirseq;
	command *comm;
	pipeline pipeln;
	pipelineseq pipelnsq;
	line* parsedln;
} YYSTYPE;
#endif /* !YYSTYPE_IS_DECLARED */
extern YYSTYPE yylval;
