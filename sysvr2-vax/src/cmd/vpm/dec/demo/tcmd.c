/*	tcmd.c 1.4 of 3/3/82
	@(#)tcmd.c	1.4
 */



#include <stdio.h>
#include <sys/types.h>
#include <sys/csi.h>
#include <sys/vpmt.h>

static char Sccsid[] ="@(#)tcmd.c	1.4";

int fd,k,i;
char cmd[8];
short errs[8];
main(argc,argv)
char	*argv[];
{

	if(argc != 2){
		fprintf(stderr,"Arg count\n");
		exit(1);
	}
	if((fd = open(argv[1],2)) < 0){
		fprintf(stderr,"Can't open %s\n",argv[1]);
		perror("main");
		exit(2);
	}
	for (i=0; i<8; i++)
		errs[i] = 077777;
	if (ioctl(fd,VPMERRSET,errs) < 0) {
		fprintf(stderr,"Ioctl failed\n");
		exit(3);
	}
	cmd[0] = 1;
	cmd[1] = 2;
	cmd[2] = 3;
	cmd[3] = 4;
	if (ioctl(fd,VPMCMD,cmd) < 0){
		fprintf(stderr,"Ioctl failed\n");
		exit(4);
	}
	cmd[0] = 0;
	cmd[1] = 0;
	cmd[2] = 0;
	cmd[3] = 0;
	while((k = ioctl(fd,VPMRPT,cmd)) == 0);
	if (k < 0){
		fprintf(stderr,"Ioctl failed\n");
		exit(5);
	}
	printf("k=%d\n",k);
	printf("%d %d %d %d\n",cmd[0],cmd[1],cmd[2],cmd[3]);
	if (k = ioctl(fd,VPMERRS,cmd) < 0){
		fprintf( stderr, "Ioctl failed\n");
		exit(6);
	}
	printf("%d %d %d %d\n",077777-errs[0],077777-errs[1],077777-errs[2],077777-errs[3]);
	printf("%d %d %d %d\n",077777-errs[4],077777-errs[5],077777-errs[6],077777-errs[7]);
	sleep(12);
	exit(0);
}
