/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/temppack.h	1.1"
/*
 */

/********************************************************************************/
/*			FOR MACHINES WITH 32 BIT LONGS				*/
/* all these machinations have the effect of reducing the size of the		*/
/* temporary files created during the first pass.  the intent is to write	*/
/* out a sequence of "codebuf" structures and read them back in.		*/
/* unfortunately this results in very large and sparse temporary files.		*/
/* hence the following packing scheme is used to make a series of records of	*/
/* one or two words in length depending upon the size of cindex and cvalue.	*/
/********************************************************************************/

/* the low order 16 bits are either a symbol	*/
/* index or a 16 bit representable value	*/
#	define UPRBITS 0xffff0000
#	define LWRBITS 0x0000ffff
#	define SYMORVAL(X) (LWRBITS & (X))	

/* the next 6 bits tell the number of bits of the value which		*/
/* are meant to be data. NOTE: THE MAXIMUM NUMBER OF BITS IS 63!	*/
#	define NUMBITMASK 0x003f0000
#	define NUMBITS(X) ((NUMBITMASK & (X)) >> 16)
#	define MKNUMBITS(X) (((X) << 16) & NUMBITMASK)

/* the next 6 bits contain the action number.	*/
/* NOTE: THE MAXIMUM ACTION NUMBER 63!		*/
#	define ACTNUMMASK 0x0fc00000
#	define ACTNUM(X) ((ACTNUMMASK & (X)) >> 22)
#	define MKACTNUM(X) (((X) << 22) & ACTNUMMASK)

/* flag asserts "symbol index present (in the low order 16 bits)"	*/
#	define SYMINDEX 0x10000000

/* flag asserts "value is zero (not present)"	*/
#	define VAL0 0x20000000

/* flag asserts "value is in low order 16 bits of word"	*/
#	define VAL16 0x40000000

/* in the remaining case, the value is contained in the following word	*/
