/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/rand.c	1.3"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;
#define DFLTLOW 0
#define DFLTHIGH 1
#define DFLTNUM 10
#define DFLTSEED 1

struct options{
	int col; /* num scalars per line */
 	int num; /* size of output vector */
 	int seed;	/* seed of random number generator */
 	double low; /* bottom limit of output range */
 	double high;	/* top limit of output range */
 	double mul; /* nultiplier */
} op = { DFLTCOL,DFLTNUM,DFLTSEED,DFLTLOW,DFLTHIGH,UNDEF };

main(argc,argv) /*generate random numbers*/
int argc;
char *argv[];
{
	int ac, argf, i;
	char **av, *cp;

	double x;

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'c': SETINT(op.col,cp); break;
				case 'n': SETINT(op.num,cp); break;
				case 'h': SETDOUB(op.high,cp); break;
				case 'l': SETDOUB(op.low,cp);
					if(op.high<=op.low) op.high+=op.low; break;
				case 'm': SETDOUB(op.mul,cp); break;
				case 's': SETINT(op.seed,cp); break;
				case ' ': case ',': break;
				default: ERRPR1(%c?,*(cp-1));
				}
				--argc;
		}

	srand(op.seed);
	if(!DEF(op.mul))
		 op.mul = (op.high-op.low)/32767; /* convert rand output to 0,1 */
	else op.mul /= 32767;

	for(i=0; i<op.num;) {
		x = op.mul * rand() + op.low;
		PRINT(x,++i,op.col);
	}
	if(i%op.col != 0) putchar('\n');
	exit(0);
}
