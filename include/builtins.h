#ifndef _BUILTINS_H_
#define _BUILTINS_H_

#define BUILTIN_ERROR 2

typedef struct {
	char* name;
	int (*fun)(char**); 
} builtin_pair;

extern builtin_pair builtins_table[];

typedef int (*builtin_ptr)(char**);

builtin_ptr get_builtin_ptr(char*);

#endif /* !_BUILTINS_H_ */
