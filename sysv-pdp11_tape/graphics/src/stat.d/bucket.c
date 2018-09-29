static char SCCSID[]="@(#)bucket.c	1.3";
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;

struct options{
	int col;	/* columnation */
	FILE *fda;	/* file contains limits */
	double intv;	/* interval between limits */
	int avsz;	/* avg count per bucket */
	int n;		/* no. of buckets */
	double low;	/* lowest limit */
	double high;	/* highest limit */
} op = { DFLTCOL,NULL,0.,0,0,NEGINFIN,POSINFIN };
 
main(argc,argv) /*set-up for bucket*/
int argc;
char *argv[];
{
	FILE *fdi;
	int ac, argf, i;
	char **av, *cp;

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'c': SETINT(op.col,cp); break;
				case 'F': SETFILE(op.fda,cp); break;
				case 'i': SETDOUB(op.intv,cp); break;
				case 'a': SETINT(op.avsz,cp); break;
				case 'n': SETINT(op.n,cp); break;
				case 'l': SETDOUB(op.low,cp); break;
				case 'h': SETDOUB(op.high,cp); break;
				case ' ': case ',': break;
				default: ERRPR1(%c?,*(cp-1));
				}
			--argc;
		}
	if(argc <= 1) { argc=2; argf=0; } /* no args, use stdin */
	else argf=1;

	while(argc-- > 1) {
		if( strcmp(*++argv,"-")==0 || !argf ) fdi=stdin;
		else if( **argv!='-' ) {
			if( (fdi=fopen(*argv,"r"))==NULL ) {
				ERRPR1(cannot open %s, *argv);
				continue;
			}
		} else { argc++; continue; }

			if(op.fda) bucketf(fdi);
			else bucket(fdi);
		fclose(fdi);
	}
	exit(0);
}

double sortbuf[MAXELEM];
bucket(fdi)
FILE *fdi;
{
	double *p=sortbuf, *sep, level, log(), *filbuf1(), ceil();
	int i, count, cnt;
	char label[MAXLABEL];

	TITOUT0(fdi,label,MAXLABEL);
	sep = filbuf1(fdi,sortbuf,MAXELEM);

/* generate break points */
	if(op.low==NEGINFIN) op.low = *sortbuf;
	if(op.high==POSINFIN) op.high = *sep;
	if(op.n > 0) {
		if(op.intv <= 0.) op.intv = (op.high - op.low) / op.n;
	} else /* n not set */
		if(op.intv > 0.) op.n = ceil((op.high - op.low) /op.intv);
		else { /* intv not set */
			if(op.avsz > 0) op.n = (sep-sortbuf+1)/op.avsz;
			else  op.n = 1 + (int)LOG(2,sep-sortbuf+1);
			op.intv = (op.high-op.low)/(op.n=(op.n?op.n:1));
		}
	i=1;
	PRINTP(op.low,i++,op.col);
	while(*p < op.low) p++; /*ignore elements below lowest limit*/
	for(cnt=1; cnt<=op.n; cnt++) {
		if((level=op.low+op.intv*cnt) > op.high) level = op.high;
		for(count=0; p<=sep && *p<=level; p++,count++) ;
		printf("%d%c",count,(i++%op.col ? '\t':'\n'));
		PRINTP(level,i++,op.col);
	}
	if(i%op.col != 1) putchar('\n');
	return;
}

bucketf(fdi) /*get limits from *fda*/
FILE *fdi;
{
	int cnt, i=1;
	double lim, x;
	char label[MAXLABEL];

	TITOUT0(fdi,label,MAXLABEL);
	getdoub(op.fda,&lim); PRINTP(lim,i++,op.col);
	while(getdoub(fdi,&x)!=EOF && x<lim);
	while( getdoub(op.fda,&lim)!=EOF ) {
		for(cnt=0; x<lim; cnt++)
			if(!feof(fdi)) getdoub(fdi,&x);
			else { x = POSINFIN; break;}
		PRINT((double)cnt,i++,op.col); PRINTP(lim,i++,op.col);
	}
	if(i%op.col != 1) putchar('\n');
	return;
}
