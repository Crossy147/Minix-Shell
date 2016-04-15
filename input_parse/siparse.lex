%{
	#include "siparse.h"
    #include "y.tab.h"
    #include <stdlib.h>


    void yyerror(char *);
	YY_BUFFER_STATE stringinputbuf;
%}

%%

[^|;<>\n \t&#]+	{
		yylval.name = yytext;	
		return SSTRING;
		}


[|;><&\n]	return *yytext;

">>"		return OAPPREDIR;	

[ \t]       ; /* skip whitespace */

#[^\n]*		return COMMENT;

%%

int yywrap(void) {
    return 1;
}

void switchinputbuftostring(const char * str){
	stringinputbuf=yy_scan_string(str);
}

void freestringinputbuf(void){
	yy_delete_buffer(stringinputbuf);
}
