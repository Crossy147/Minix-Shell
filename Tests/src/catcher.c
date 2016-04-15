#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

void handler(int sig, siginfo_t * si, void * ctx)
{
	char buf[100];
	int l;

	l = snprintf(buf,20,"signalled by %d \n",sig);
	if (l>0) write(1,buf,l);
	exit(0);
}

int 
main(int argc, char* argv[]){
	struct sigaction sa;

	sa.sa_sigaction = handler;
	sa.sa_flags = SA_SIGINFO ;
    sigemptyset(&sa.sa_mask);
	sigaction(SIGINT, &sa, NULL);
	sigaction(SIGUSR1, &sa, NULL);
	sigaction(SIGUSR2, &sa, NULL);


	while (1) pause();
}
