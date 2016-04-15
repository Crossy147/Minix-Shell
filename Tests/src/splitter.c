#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>

#define BUF_SIZE 100
#define PACKAGE_LEN 30

int 
main(int argc, char* argv[]){
	char buf[BUF_SIZE];
	int n,k,l,d;
	int sl;

	sl=0;
	if (argc==2) sl=atoi(argv[1]);
	if (sl<0) sl=0;
	if (sl>10) sl=10;

	srand(1);

	while ((n=read(0,buf,BUF_SIZE-1))!=0){
		if (n<0){
			if (errno == EINTR) continue;
			else exit(1);
		}
		k=0;
		while (k<n){
			l = rand()%PACKAGE_LEN +1;
			if (l>n-k) l=n-k;
			d=write(1,buf+k,l);
			if (d<0){
				if (errno != EINTR) exit(1);
				else d=0;
			}
			k+=d;
			sleep(sl);
		}
		
	}
}
