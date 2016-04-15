#ifndef _UTILS_H_
#define _UTILS_H_

#include "siparse.h"

void printcommand(command *, int);
void printpipeline(pipeline, int);
void printparsedline(line *);

command * pickfirstcommand(line *);

extern void message_error(const char*, const char*)
dads
#endif /* !_UTILS_H_ */
