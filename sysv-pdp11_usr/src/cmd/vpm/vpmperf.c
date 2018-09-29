#include <stdio.h>
#include <sys/vpm.h>
#define	MASK16 0177777
#define	SSTOL(x,y)	((((long)x)<<16)|(((long)y)&MASK16))

static char Sccsid[] = "@(#)vpmperf.c	1.1";
struct pstat {
	char	chno;
	char	ctn;
	char	lines;
	char	byte1;
	unsigned short	icnt0;
	unsigned short	icnt1;
	short	lcnt[9];
	short	lbolt1;
	short	lbolt2;
}ev[16];
#define RSIZE	sizeof(struct pstat)
main(argc,argv)
char *argv[];
{
	int fd,k,n,chbits;
	long	time0,time1,lbolt,icnt;

	if((fd = open("/dev/pstat",2)) < 0){
		perror("/dev/pstat");
		exit();
	}
	setbuf(stdout,NULL);
	printf("fd = %d\n",fd);
	sscanf(argv[1], "%6o", &chbits);
	if((k = ioctl(fd, VPMSETC, chbits)) < 0){;
		perror("ioctl");
		exit();
	}
	while((n = read(fd,ev,RSIZE)) != RSIZE);
	time0 = SSTOL(ev[0].lbolt2,ev[0].lbolt1);
	time1 = SSTOL(ev[0].lbolt2,ev[0].lbolt1) - time0;
	printf("%o %o ",ev[0].lines,ev[0].byte1);
	icnt = SSTOL(ev[0].icnt1,ev[0].icnt0);
	printf("%7lu",icnt);
	printf("%6u %6u %6u %6u ",ev[0].lcnt[0],ev[0].lcnt[1],ev[0].lcnt[2],ev[0].lcnt[3]);
	printf("%6u %6u %6u %6u %u",ev[0].lcnt[4],ev[0].lcnt[5],ev[0].lcnt[6],ev[0].lcnt[7],ev[0].lcnt[8]);
	printf("%6u ",time1);
	printf("\n");
	for(;;){
		n = read(fd,ev,512);
		for(k = 0;k < n/RSIZE;k++){
			time1 = SSTOL(ev[k].lbolt2,ev[k].lbolt1) - time0;
			printf("%o %o ",ev[k].lines,ev[k].byte1);
			icnt = SSTOL(ev[k].icnt1,ev[k].icnt0);
			printf("%7lu",icnt);
			printf("%6u %6u %6u %6u ",ev[k].lcnt[0],ev[k].lcnt[1],ev[k].lcnt[2],ev[k].lcnt[3]);
			printf("%6u %6u %6u %6u %u",ev[k].lcnt[4],ev[k].lcnt[5],ev[k].lcnt[6],ev[k].lcnt[7],ev[k].lcnt[8]);
			printf("%6u ",time1);
			printf("\n");
		}
	}
}
