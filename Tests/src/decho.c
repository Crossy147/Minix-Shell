#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int 
main(int argc, char* argv[])
{
	int delay,n;

	if (argc<3) {
		printf("Syntax: %s delay_time arg1 arg2 ...\n", argv[0]);
		return 1;
	}

	delay = atoi(argv[1]);
	sleep(delay);

	for (n=2; n< argc; n++){
		printf(" %s", argv[n]);
	}

	printf("\n");
	return 0;
}
