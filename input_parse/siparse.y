%{
	#include <siparse.h>
	#include "siparseutils.h"
    #include <stdio.h>
    
	extern int yyleng;

	int yylex(void);
	void yyerror(char *);
  
	void switchinputbuftostring(const char *);
	void freestringinputbuf(void);

	static line parsed_line;
%}


%union{
	int flags;
	char *name;
	char **argv;
	redirection *redir;
	redirection **redirseq;
	command *comm;
	pipeline pipeln;
	pipelineseq pipelnsq;
	line* parsedln;
}

%token SSTRING
%token OAPPREDIR
%token COMMENT
%%

line:
	pipelineseq mamp mcomment mendl {
			parsed_line.pipelines = closepipelineseq(); 
			parsed_line.flags= $2.flags;
			$$.parsedln = &parsed_line;
		}
	;

mamp:
	'&' { $$.flags = LINBACKGROUND; } 
	|	{ $$.flags = 0; }
	;

mcomment:
	COMMENT
	|
	;

mendl:
	'\n'
	|
	;

pipelineseq:
	pipelineseq ';' prepipeline{
			$$.pipelnsq = appendtopipelineseq($3.pipeln);
		}
	| prepipeline{
			$$.pipelnsq = appendtopipelineseq($1.pipeln);
		}
	;

prepipeline:
	pipeline {
			closepipeline();
		}
	;

pipeline:
	pipeline '|' single {
			$$.pipeln = appendtopipeline($3.comm);
		}
	| single {
			$$.pipeln = appendtopipeline($1.comm);
		}
	;

single:
	allnames allredirs {
			if ($1.argv==NULL) {
				$$.comm= NULL;	
			} else {
				command *com= nextcommand();
				com->argv = $1.argv;
				com->redirs = $2.redirseq;
				$$.comm = com;
			}
		}
	;

allnames:
		names { $$.argv = closeargv(); }


allredirs:
		 redirs { $$.redirseq = closeredirseq(); }

names:
	names name {
			$$.argv = appendtoargv($2.name);
		} 
	|	 
	;

name:	SSTRING {
			$$.name= copytobuffer(yyval.name, yyleng+1);
		};

redirs:
	redirs redir {
			$$.redirseq = appendtoredirseq($2.redir);
		}
	|	{	$$.redirseq = NULL; };
	;

redir:
	redirIn
	| redirOut
	;

redirIn:
	'<' rname { $2.redir->flags = RIN; $$=$2; }
	;

redirOut:
	OAPPREDIR rname 	{ $2.redir->flags = ROUT | RAPPEND ; $$=$2; }
	| '>' rname	{ $2.redir->flags = ROUT; $$=$2; }
	;

rname:
	 name {
			redirection * red;

			red=nextredir();
			red->filename = $1.name;
			$$.redir= red;
		}

%%

void yyerror(char *s) {
}


line * parseline(char *str){
	int parseresult;

	resetutils();
	switchinputbuftostring(str);
	parseresult = yyparse();
	freestringinputbuf();

	if (parseresult) return NULL;
	return &parsed_line;
}

