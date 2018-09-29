/*	sr.c 1.3 of 3/23/82
 */
#include <stdio.h>
static char Sccsid[] = "@(#)sr.c	1.3";
int fd;
long k;
main(argc,argv)
char	*argv[];
{

	if(argc != 2){
		fprintf(stderr,"Arg count\n");
		exit();
	}
	if((fd = open(argv[1],2)) < 0){
		fprintf(stderr,"Can't open %s\n",argv[1]);
		perror("main");
		exit();
	}
	fprintf(stderr,"fd = %d\n",fd);
	sleep(10);
	if(fork()){
		fprintf(stderr,"send fork\n");
		send();
		exit();
	}else{
		fprintf(stderr,"rec fork\n");
		rec();
		exit();
	}
}
send()
{
char buf[512];
long k;
int kk,n;
long t1,t2,t3,time();


	time(&t1);
	while((n = read(0,buf,512))>0){
		kk = write(fd,buf,n);
		if(kk < 0){
			perror("send");
			exit();
		}
		fprintf(stderr,"s = %d\n",kk);
		k += kk;
	}
	time(&t2);
	fprintf(stderr,"BYTES SENT = %ld\n",k);
	t3 = t2 - t1;
	if(t3 <= 0)
		t3 = 1;
	t1 = k/t3;
	sleep(3);
	fprintf(stderr,"TIME = %ld BYTES/SEC = %ld\n",t3,t1);
}
rec()
{
char buf[512];
int n;
int stop();


	while(1){
		signal(14,stop);
		alarm(30);
		n = read(fd,buf,512);
		if(n<=0){
			fprintf(stderr,"n = %d\n",n);
			perror("rec");
			stop();
		}
		alarm(0);
		fprintf(stderr,"r = %d\n",n);
		k += write(1,buf,n);
	}
}
stop()
{
	fprintf(stderr,"BYTES REC'ED %ld\n",k);
	exit();
}
