#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

int 
main(int argc, char** argv)
{
	int n=0;

	if (argc!=2) {
		printf("Syntax: %s Time_to_wait\n", argv[0]);
		exit(-1);
	}

	n= atoi(argv[1]);

	sleep(n);
	exit(0);
}
