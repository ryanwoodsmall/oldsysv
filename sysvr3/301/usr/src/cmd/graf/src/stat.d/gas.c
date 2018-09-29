/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/gas.c	1.5"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;
#define DFLTTO POSINFIN
#define DFLTFROM 1
#define DFLTNUM 10
#define DFLTINTV 1

struct options {
	int col;	/* columns per output line */
	long num;	/* # of numbers to be generated */
	double intv;	/* interval between numbers */
	double high;	/* termination value */
	double low;	/* start value */
} op = { DFLTCOL,0,UNDEF,DFLTTO,DFLTFROM };

main(argc,argv) /*generate arithmetic series*/
int argc;
char *argv[];
{
	int ac, argf, i;
	char **av, *cp;
	double x, fmod();
	int  num=0, setnum=0, j;

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'c': SETINT(op.col,cp); break;
				case 'i': SETDOUB(op.intv,cp); break;
				case 't': SETDOUB(op.high,cp); setnum=1; break;
				case 's': SETDOUB(op.low,cp); break;
				case 'n': SETINT(op.num,cp); break;
				case ' ': case ',': break;
				default: ERRPR1(%c?,*(cp-1));
				}
			--argc;

		}

	if( !DEF(op.intv) ) op.intv = op.low<op.high ? DFLTINTV : -DFLTINTV;
		else if( !setnum && op.intv < 0) op.high = NEGINFIN;
	if(!op.num)
		if( setnum && op.intv!=0 )
			op.num = abs((int)((op.high-op.low)/op.intv)) + 1;
		else op.num = DFLTNUM;

	for(i=0,j=0; i<op.num; j+=1) {
		x = op.low + j*op.intv;
		if( (op.intv>0) ? x>op.high : x<op.high ) x=op.low, j=0;
		PRINT(x,++i,op.col);
	}
	if(i%op.col != 0) putchar('\n');
	exit(0);
}
