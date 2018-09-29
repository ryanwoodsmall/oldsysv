/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/stat.d/prime.c	1.3"
/* <:t-5 d:> */
#include "stdio.h"
#include "s.h"
char *nodename;
#define MAXINT 32767

struct options {
	int col; /* columns per output line */
	doub low,high; /* bounds for primes */
	int num; /* number of primes to output */
} op = { DFLTCOL, 2, UNDEF, 10 };

main(argc,argv) /* generate primes */
int argc;
char *argv[];
{
	int ac;
	char **av, *cp;

	long k;
	int count=0;

	nodename = *argv;
	for( ac=argc-1,av=argv+1; ac>0; ac--,av++ )
		if( *(cp=av[0])=='-' && *++cp!='\0' ) {
			while( *cp!='\0' )
				switch(*cp++) {
				case 'c': SETINT(op.col,cp); break;
				case 'h': SETDOUB(op.high,cp); break;
				case 'l': SETDOUB(op.low,cp); break;
				case 'n': SETINT(op.num,cp); break;
				case ' ': case ',': break;
				default: ERRPR1(%c?,*(cp-1));
				}
			--argc;
		}

	if(DEF(op.high)) op.num=MAXINT;
	else op.high = POSINFIN;

	if( 2>=op.low ) PRINT(2,++count,op.col);
	for( k=3;  count<op.num && k<=op.high; k+=2 )
		if(prime(k))
			if( k>=op.low ) PRINT(k,++count,op.col);
	if( count%op.col!=0 ) putchar('\n');
	exit(0);
}
prime(i) /* determine if i is prime */
long i;
{
	static long primes[MAXELEM] = {
		2,3,5,7,11,13,17,19,23,29,
		31,37,41,43,47,53,59,61,67,71,
		73,79,83,89,97,101,103,107,109,113,
		127,131,137,139,149,151,157,163,167,173,
		179,181,191,193,197,199,211,223,227,229,
		233,239,241,251,257,263,269,271,277,281,
		283,293,307,311,313,317,331,337,347,349,
		353,359,367,373,379,383,389,397,401,409,
		419,421,431,433,439,443,449,457,461,463,
		467,479,487,491,499,503,509,521,523,541
	};
	static int ep=100;
	int j;
	long root;
	doub pow(), ceil();

	root = ceil(pow((doub)i,.5));
	for( j=0; primes[j]<=root; j++ )
		if( i%primes[j]==0 ) return(0);

	if( i>primes[ep] ) /* add prime to list */
		if( ep<MAXELEM ) primes[ep++]=i;
		else if( i>primes[ep-1]*primes[ep-1] )
			ERRPR1(primes greater that %D suspect,i);
	return(1);
}
