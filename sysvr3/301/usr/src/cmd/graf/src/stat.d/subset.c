/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/subset.c	1.3"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;

struct options {
	int col;	/* columns per output line */
	FILE *fda;	/* file that pick or leave value looks at */
			/* in order to match fdi file's data */
	int interv;	/* fda interval to match pick or leave value */
	double above;	/* pick elements above above */
	double below;	/* pick elements below below */
	double leave;	/* leave elements matching leave */
	double pick;	/* pick elements matching leave */
	int nflg;	/* nfile contains the positions of fdi to be picked */
	int start;	/* starting position in fda */
	int term;	/* terminating position in fda */
} op = { DFLTCOL, NULL, 1, UNDEF, UNDEF, UNDEF, UNDEF, 0,1,32767 };

main(argc,argv) /*set-up for subset*/
int argc;
char *argv[];
{
	FILE *fdi;
	int ac, argf, i;
	char **av, *cp, label[MAXLABEL];

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'a': SETDOUB(op.above,cp); break;
				case 'b': SETDOUB(op.below,cp); break;
				case 'c': SETINT(op.col,cp); break;
				case 'F': SETFILE(op.fda,cp); break;
				case 'i': SETINT(op.interv,cp); break;
				case 'l': SETDOUB(op.leave,cp); break;
				case 'p': SETDOUB(op.pick,cp); break;
				case 'n':
				 switch(*cp++) {
				 case 'p': op.nflg = 1; break;
				 case 'l': op.nflg = -1; break;
				 default: ERRPR1(%c?,*(cp-1));
				 }; break;
				case 's': SETINT(op.start,cp); break;
				case 't': SETINT(op.term,cp); break;
				case ' ': case ',': break;
				default: ERRPR1(%c?,*(cp-1));
				}
			--argc;
		}

	if(argc <= 1)  argc=2, argf=0;  /* no args, use stdin */
	else argf=1;

	while(argc-- > 1) {
		if( strcmp(*++argv,"-")==0 || !argf ) fdi=stdin;
		else if( **argv!='-' ) {
			if( (fdi=fopen(*argv,"r"))==NULL ) {
				ERRPR1(cannot open %s, *argv);
				continue;
			}
		} else { argc++; continue; }

		TITOUT1(fdi,label,MAXLABEL,subset);
		if(op.nflg!=0) subsetn(fdi);
		else if(op.fda) subsetf(fdi);
		else subset(fdi);
		fclose(fdi);
	}
	exit(0);
}

subset(fdi) /*subset elements:  begin at start
			end at term, output every interv
			leave values of leave, pick vals of pick */
FILE *fdi;
{
	int i, j, k;
	double x;

	for(i=0; getdoub(fdi,&x)!=EOF && ++i<op.start; ) ;
	for(k=1; i<=op.term && !feof(fdi); ) {
		if(print(x)) PRINT(x,k++,op.col);
		for(j=i+op.interv; getdoub(fdi,&x)!=EOF && ++i<j; ) ;
	}
	if(k%op.col != 1) putchar('\n');
	return;
}

subsetf(fdi) /*subset elements using *fda as key*/
FILE *fdi;
{
	int i, k, j;
	double x, y;
	for(i=1; i<op.start && !feof(fdi) && !feof(op.fda); i++) {
		getdoub(fdi,&x); getdoub(op.fda,&x);
	}
	for(k=1; i<op.term && getdoub(fdi,&x)!=EOF && getdoub(op.fda,&y)!=EOF; i++) {
		if(print(y)) PRINT(x,k++,op.col);
		for(j=1; j<op.interv && !feof(fdi) && !feof(op.fda); j++) {
			getdoub(fdi,&x); getdoub(op.fda,&x);
		}
	}
	if(k%op.col != 1) putchar('\n');
	return;
}

subsetn(fdi) /* subset named elements */
FILE *fdi;
{
	int i, k, n, getint();
	double buf[MAXELEM], *bp, *filbuf1();

	bp = filbuf1(fdi,buf,MAXELEM);
	if(op.nflg > 0) { /* output elements specified */
		for(i=1;getint((op.fda?op.fda:stdin),&n)!=EOF;i++)
			if(n>0 && n<=bp-buf+1) {
				if(print(buf[n-1])) PRINT(buf[n-1],i,op.col);
			} else ERRPR1(%d out of range,n);
		if(i%op.col != 1) putchar('\n');
		return;
	}
	else { /* output all but elements specified */
		while(getint(op.fda?op.fda:stdin,&n)!=EOF)
			if(n>0 && n<= bp-buf+1) buf[n-1]=DONEFLG;
			else ERRPR1(%d out of range,n);
		for(i=0,k=1;i<=bp-buf;i++)
			if(buf[i] > DONEFLG)
				if(print(buf[i])) PRINT(buf[i],k++,op.col);
		if(k%op.col != 1) putchar('\n');
		return;
	}
}

print(val) /* returns true if val should be printed */
double val;
{
	int state=0, def=0;
	if(DEF(op.pick)) { def++; if(val==op.pick) state++; }
	if(DEF(op.above)) { def++; if(val>op.above) state++; }
	if(DEF(op.below)) { def++; if(val<op.below) state++; }
	if(!def) state++; /* default is to select every element */
	if( DEF(op.leave) && val==op.leave ) state=0;
	return(state);
}
