/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm:xx/btoa.c	1.1"

/*
 *  ``btoa'' - Convert an binary file to ASCII "btoa" format.
 *
 *  Usage:
 *	btoa < binary > ascii-btoa-file
 *
 *  Stream filter to change 8 bit bytes into printable ascii.  Computes
 *  the number of bytes, and three kinds of simple checksums.  Assumes
 *  that long is 32 bits.  Incoming bytes are collected into 32-bit longs,
 *  then printed in base 85, exp(85,5) > exp(2,32).  The characters used
 *  are between ' ' and 't'.  'z' encodes 32-bit zero; 'x' is used to
 *  mark the end of encoded data.
 *
 *  Author:
 *	Paul Rutter	(philabs!per)
 *	Received from netnews (net.sources) 3-May-84 15:47:06 EDT
 *  Adopted for uOMS:
 *	R. P. Eby	(pegasus!eby)
 *	Modified to work on 16-bit machines 20-Jul-84
 */

#include <stdio.h>

#ifdef	ap16
#define	ulong	long
#else	ap16
#define	ulong	unsigned long
#endif	ap16

#define MAXPERLINE 78
#define EN(c) (int)((c) + (ulong) ' ')

long	Ceor = 0L;
long	Csum = 0L;
long	Crot = 0L;
long	ccount = 0L;
long	bcount = 0L;
long	word = 0L;
extern	void	exit();

#ifndef	USE_BTOA_FUNCTION
#define	infp	stdin
#define	outfp	stdout

main(argc, argv)
char **argv;

#else	USE_BTOA_FUNCTION

FILE *infp, *outfp;

btoa(infile, outfile)
char *infile, *outfile;

#endif	USE_BTOA_FUNCTION
{
	register long c, n;

#ifndef	USE_BTOA_FUNCTION
	if (argc != 1) {
		(void) fprintf(stderr,"%s: bad args\n", argv[0]);
		exit(2);
	}
#else	USE_BTOA_FUNCTION
	Ceor = Csum = Crot = ccount = bcount = word = 0L;
	infp = fopen(infile, "r");
	outfp = fopen(outfile, "w");
#endif	USE_BTOA_FUNCTION
	(void) fprintf(outfp, "xbtoa Begin\n");
	n = 0L;
	while ((c = getc(infp)) != EOF) {
		encode(c);
		n += 1;
	}
	while (bcount != 0) {
		encode(0L);
	}
	/* n is written twice as crude cross check */
	(void) fprintf(outfp, "\nxbtoa End N %ld %lx E %lx S %lx R %lx\n", n, n, Ceor, Csum, Crot);
#ifndef	USE_BTOA_FUNCTION
	exit(0);
#else	USE_BTOA_FUNCTION
	fclose(infp);
	fclose(outfp);
#endif	USE_BTOA_FUNCTION
}

encode(c)
register long c;
{
	Ceor ^= c;
	Csum += c;
	Csum += 1;
	if (Crot & 0x80000000L) {
		Crot <<= 1;
		Crot += 1;
	} else {
		Crot <<= 1;
	}
	Crot += c;
	word <<= 8;
	word |= c;
	if (bcount == 3) {
		wordout(word);
		bcount = 0;
	} else {
		bcount += 1;
	}
}

wordout(wd)
#ifdef	ap16
long wd;
#else	ap16
register long wd;
#endif	ap16
{
	if (wd == 0) {
		charout((int)'z');
	} else {
		/* first division must be unsigned */
#ifdef	ap16
		long val;
		dh(wd, &val, &wd);
		charout(EN(val));
#else	ap16
		charout(EN((ulong) wd / (ulong)(85L * 85L * 85L * 85L)));
		wd = (ulong) wd % (ulong)(85L * 85L * 85L * 85L);
#endif	ap16
		charout(EN(wd / (85L * 85L * 85L)));
		wd %= (85L * 85L * 85L);
		charout(EN(wd / (85L * 85L)));
		wd %= (85L * 85L);
		charout(EN(wd / 85L));
		wd %= 85L;
		charout(EN(wd));
	}
}

charout(c)
{
	(void) putc(c, outfp);
	ccount += 1;
	if (ccount == MAXPERLINE) {
		(void) putc('\n', outfp);
		ccount = 0;
	}
}

#ifdef	ap16
#define	D	(85L * 85L * 85L * 85L)

/*
 * dh -- disgusting hack to make it work on machines w/o unsigned longs.
 *	Look at the top 2 bits of the dividend, and adjust the division
 *	accordingly.  Assumes 32-bit longs.
 */
dh(n, val, rem)
long n, *val, *rem;
{
	long hi2bits = (n>>30) & 03;
	
	*val = 0L;
	switch (hi2bits) {
	case 0L:
	case 1L:
		break;
	case 2L:
		*val = 41L;
		n &= 0x3FFFFFFFL;
		n += 0x6EBFA7L;
		break;
	case 3L:
		*val = 61L;
		n &= 0x3FFFFFFFL;
		n += 0x23461D3L;
		break;
	}
	*val += n / D;
	*rem = n % D;
}
#endif	ap16
