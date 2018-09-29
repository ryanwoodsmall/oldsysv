/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/point.c	1.3"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;
#define DFLTPER .5
#define MAXEL 2501

struct options {
	int num;	/* index of element */
	doub per;	/* percent point of element */
	short sort;	/* if TRUE, sort elements */
} op = { 0, DFLTPER, TRUE };

main(argc,argv) /*set-up for point*/
int argc;
char *argv[];
{
	FILE *fdi;
	int ac, argf, i;
	char **av, *cp;

	double x;

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'f': SETDOUB(x,cp);
					op.per = x>0 ? 1/x : (x<0 ? 1+1/x : 1);
					break;
				case 'n': SETSINT(op.num,cp); break;
				case 'p': SETDOUB(x,cp);
					op.per = (x<0 ? 1+x : x); break;
				case 's': op.sort=FALSE; break;
				case ' ': case ',': break;
				default: ERRPR1(%c?,*(cp-1));
				}
			--argc;
		}

	if( argc<=1 ) { argc=2; argf=0; } /* no args, use stdin */
	else argf=1;

	while( argc-- > 1 ) {
		if( strcmp(*++argv,"-")==0 || !argf ) fdi=stdin;
		else if( **argv!='-' ) {
			if( (fdi=fopen(*argv,"r"))==NULL ) {
				ERRPR1(cannot open %s, *argv);
				continue;
			}
		} else { argc++; continue; }

		if( point(fdi,&x) ) {
			PRINT(x,1,1);
			fclose(fdi);
		}
	}
	exit(0);
}

point(fdi,val) /*get point at num or per*/
FILE *fdi;
double *val;
{
	int i, doubcmp();
	double x, buf[MAXEL], *ep, *filbuf1(), floor();

	ep = filbuf1(fdi,buf,MAXEL);
	if( ep>=buf ) {
		if(op.sort) qsort(buf,ep+1-buf,sizeof(double),doubcmp);
		if( (i=op.num)!=0 ) {
			if( ABS(i)<(ep-buf+1) ) {
				*val = i>0 ? *(buf+i-1) : *(ep+i+1);
				return(TRUE);
			} else ERRPR1(%d out of range,i);
		} else if( op.per>=0 && op.per<=1 ) {
			x = op.per*(ep-buf);
			i = floor(x);
			*val = ((buf[i+1]-buf[i]) * (x-i)) + buf[i];
			return(TRUE);
		} else ERRPR1(%g out of range, op.per);
	}
	return(FALSE);
}
