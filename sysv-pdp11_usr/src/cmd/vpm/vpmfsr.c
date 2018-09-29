/*	vpmfsr.c 1.5 of 5/11/82
	@(#)vpmfsr.c	1.5
 */


#include <stdio.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/signal.h>
#include <sys/map.h>
#include "sys/csi.h"
#include "sys/csihdw.h"
#include "sys/vpmt.h"
#define WCNT	512

static char Sccsid[] = "@(#)vpmfsr.c	1.1";

long k,cnt;
extern long atol();
int fd,kk,status;
int f;
unsigned char sbuf[] = {"012345678901234567890123456789012345678901234567890123456789012\
aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa\
bbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbbb\
cccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccccc\
dddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddddd\
eeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeeee\
ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff\
gggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggggg"};
main(argc,argv)
char	*argv[];
{

	short cmd[8];
	int stop9();
	int i;
	if(argc != 3){
		fprintf(stderr,"Arg count\n");
		fprintf(stderr, "argc = %d\n", argc);
		exit(9);
	}
	cnt = atol(argv[2]);
	fprintf(stderr,"cnt = %ld\n",cnt);
	signal(14,stop9);
	alarm(30);
	if((fd = open(argv[1],O_RDWR)) < 0){
		fprintf(stderr,"Can't open %s\n",argv[1]);
		perror("main");
		exit(8);
	}
	signal(14, SIG_IGN);
	fprintf(stderr, "Open succeeded.\n");
	for(i=0; i<8; i++)
		cmd[i] = 077777;
	ioctl(fd,VPMERRSET,cmd);
	setpgrp();
	if(f = fork()){
		if(f < 0){
			fprintf(stderr,"Cannot fork\n");
			exit(7);
		}
		signal(SIGINT,SIG_IGN);
		sleep(10);
		send();
		wait(&status);
		exit(0);
	}else{
		sleep(10);
		rec();
		exit(3);
	}
}
send()
{
long n;
long t1,t2,t3,time();
int seq;
int stop8();


	time(&t1);
	seq = 0;
	for(n = 0; n < cnt; n += WCNT){
		sbuf[0] = seq++;
		if(seq == 8)
			seq = 0;
		signal(14,stop8);
		alarm(WCNT/150 + 10);
		kk = write(fd,sbuf,WCNT);
		alarm(0);
		if(kk < 0){
			perror("send");
			if ( f > 0 ) {
				kill(0,SIGINT);
				wait(&status);
			}
			exit(6);
		}
		k += kk;
	}
	sbuf[0] = 'x';
	kk = write(fd,sbuf,WCNT);
	if(kk < 0){
		perror("send");
		return;
	}
	k += kk;
	time(&t2);
	fprintf(stderr,"BYTES SENT = %ld\n",k);
	t3 = t2 - t1;
	if(t3 <= 0)
		t3 = 1;
	t1 = k/t3;
	fprintf(stderr,"TIME = %ld BYTES/SEC = %ld\n",t3,t1);
}
rec()
{
unsigned char buf[513];
int n,rseq;
int stop2(), stop();


	rseq = 0;
	for(;;){
		signal(14,stop2);
		alarm(WCNT/150 + 10);
		if((n = read(fd,buf,512)) < 0){
			perror("read");
			stop();
		}
		k += n;
		if(n != 512)
			fprintf(stdout,"n = %d\n",n);
		if(buf[0]=='x')
			stop();
		if(buf[0] != rseq){
			fprintf(stdout,"rseq = %d nseq = %d\n",rseq,buf[0]);
			buf[n] =0;
			fprintf(stdout,"%s\n",buf);
			stop();
			rseq = buf[0];
			rseq++;
		}else
			if(++rseq == 8)
				rseq = 0;
	}
}
stop()
{
	int nk;
	short cmd[8];
	nk = ioctl(fd,VPMERRS,cmd);
	if(nk < 0)
		fprintf(stderr,"nk = %d\n",nk);
	else
		fprintf(stderr," errs : %d %d %d %d %d %d %d %d\n",
		077777-cmd[0],077777-cmd[1],077777-cmd[2],077777-cmd[3],
		077777-cmd[4],077777-cmd[5],077777-cmd[6],077777-cmd[7]);
	fprintf(stderr,"BYTES REC'ED %ld\n",k);
	if (f > 0) {
		kill(0,SIGINT);
		wait(&status);
	}
	exit(0);
}
stop2()
{
	int nk,code;
	short cmd[8];
	nk = ioctl(fd,VPMERRS,cmd);
	if(nk < 0)
		fprintf(stderr,"nk = %d\n",nk);
	else
		fprintf(stderr," errs : %d %d %d %d %d %d %d %d\n",
		077777-cmd[0],077777-cmd[1],077777-cmd[2],077777-cmd[3],
		077777-cmd[4],077777-cmd[5],077777-cmd[6],077777-cmd[7]);
	fprintf(stderr,"BYTES REC'ED %ld\n",k);
	code = 14;
	fprintf(stderr,"code = %d\n",code);
	if( f > 0 ) {
		kill(0,SIGINT);
		wait(&status);
	}
	exit(code);
}
stop8()
{
	fprintf(stderr,"WRITE TIMEOUT\n");
	if ( f > 0 ) {
		kill(0,SIGINT);
		wait(&status);
	}
	exit(2);
}
stop9()
{
	fprintf(stderr,"OPEN TIMEOUT\n");
	if ( f > 0 ) {
		kill(0,SIGINT);
		wait(&status);
	}
	exit(2);
}
