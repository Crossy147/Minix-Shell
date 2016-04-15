#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int 
main(int argc, char* argv[])
{
	int n, fds;

	fds=0;

	for (n=0; n< OPEN_MAX; n++){
		if ((fcntl(n, F_GETFD) != -1) || (errno != EBADF) ) fds++;
	}

	printf("%d file descriptors used.\n", fds);
}
