#include <errno.h>
#include <stdio.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/csi.h>
#include <sys/vpmt.h>

#define TRACE	"/dev/trace"
#define VPMDEV	"/dev/vpm"

static char Sccsid[] = "@(#)snoop.c	1.9";

struct event {
	short	seq;
	char	typ;
	char	dev;
	short	wd1;
	short	wd2;
};
char ebuf[512];

int chan;
char buf[64];

main(argc,argv)
int argc;
char **argv;
{
	register int k, n, dev, fd, fd1;
	char *lastchar, vpmdev[10];

	while(*++*argv);
	lastchar = (--*argv);
	dev = (int) (*lastchar - '0');
	strcpy(vpmdev,"/dev/vpm");
	strcat(vpmdev,lastchar);
	if((fd1 = open(vpmdev,O_NDELAY)) < 0) {
		fprintf(stderr,"Can't open %s\n",vpmdev);
		exit(1);
	}
	if(ioctl(fd1,VPMSTAT,&chan) < 0) {
		fprintf(stderr,"Ioctl to %s failed (vpm device not attached to physical device) --\n",vpmdev);
		fprintf(stderr,"rjeload must be performed before snoop\n");
		exit(1);
	}
	close(fd1);
	setbuf(stdout,NULL);
	if((fd = open(TRACE,0)) < 0) {
		fprintf(stderr,"Can't open %s\n",TRACE);
		exit(1);
	}
	if(ioctl(fd,VPMSETC,01<<(chan % 16)) < 0) {
		fprintf(stderr,"Ioctl to trace driver failed\n");
		exit(1);
	}
	fprintf(stdout,"Tracing vpm%o\n",dev);
	for(;;) {
		sleep(1);
		n = read(fd,ebuf,512);
		for(k = 6; k < n; k+=14)
			prinfo(&ebuf[k]);
	}
}

prinfo(st)
register struct event *st;
{

	if(((int)st->dev & 07) != chan)
		return;
	sprintf(buf,"%d\t",(st->seq) % 100);
	cvttyp(st->typ);
	getwd1(st);
	getwd2(st);
	fprintf(stdout,"%s\n",buf);
}

cvttyp(c)
register char c;
{
	char ch;

	switch(c) {
	case 's':
		strcat(buf,"ST\tStart");
		break;
	case 'T':
		strcat(buf,"TR\t");
		break;
	case 'o':
		strcat(buf,"OP\tOpened");
		break;
	case 'p':
		strcat(buf,"OP\tFailed");
		break;
	case 'w':
		strcat(buf,"WR\t");
		break;
	case 'r':
		strcat(buf,"RD\t");
		break;
	case 'c':
		strcat(buf,"CL\tClosed");
		break;
	case 'E':
		strcat(buf,"SC\tExit");
		break;
	case 'C':
		strcat(buf,"CL\tClean");
		break;
	case 'B':
		strcat(buf,"ST\tStartack");
		break;
	case 'Z':
		strcat(buf,"ST\tStopped");
		break;
	case 'Y':
		strcat(buf,"ST\tStopchk");
		break;
	case 'H':
		strcat(buf,"ST\tStopack");
		break;
	default:
		ch = c;
		strncat(buf,&ch,1);
		strcat(buf,"\t");
		return;
	}
}

getwd1(s)
register struct event *s;
{
	char tbuf[12];

	switch(s->typ) {
	case 'r':
	case 'w':
		sprintf(tbuf,"%d bytes",s->wd1);
		strcat(buf,tbuf);
		break;
	case 'E':
		sprintf(tbuf,"(%d)",s->wd1);
		strcat(buf,tbuf);
		break;
	default:
		break;
	}
}

getwd2(s)
register struct event *s;
{
	char tbuf[8];

	switch(s->typ) {
	case 'p':
		strcat(buf,"(start)");
		break;
	case 'H':
		sprintf(tbuf,"(%d)",s->wd2);
		strcat(buf,tbuf);
		break;
	case 'T':
		switch((s->wd2)&0xFF) {
		case 0:
			strcat(buf,"Started");
			break;
		case 1:
			strcat(buf,"R-ACK");
			break;
		case 2:
			strcat(buf,"R-WAIT");
			break;
		case 3:
			strcat(buf,"R-ENQ");
			break;
		case 4:
			strcat(buf,"S-EOT");
			break;
		case 5:
			strcat(buf,"R-ERRBLK");
			break;
		case 6:
			strcat(buf,"R-NAK");
			break;
		case 7:
			strcat(buf,"R-OKBLK");
			break;
		case 8:
			strcat(buf,"R-SEQERR");
			break;
		case 9:
			strcat(buf,"R-JUNK");
			break;
		case 10:
			strcat(buf,"TIMEOUT");
			break;
		case 11:
			strcat(buf,"S-ENQ");
			break;
		case 12:
			strcat(buf,"S-NAK");
			break;
		case 13:
			strcat(buf,"S-ACK");
			break;
		case 14:
			strcat(buf,"S-BLK");
			break;
		case 15:
			strcat(buf,"S-BADBLK");
			break;
		default:
			sprintf(tbuf,"%-7x",s->wd2&0xFF);
			strcat(buf,tbuf);
		}
		break;
	case 'r':
	case 'w':
	case 's':
	case 'o':
	case 't':
	case 'c':
	case 'C':
	case 'B':
	case 'Z':
	case 'Y':
	case 'E':
		break;
	default:
		sprintf(tbuf,"%-5o",s->wd2);
		strcat(buf,tbuf);
	}
}
