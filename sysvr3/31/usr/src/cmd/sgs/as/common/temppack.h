/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/temppack.h	1.2.1.2"
/*
 */

/********************************************************************************/
/*			FOR MACHINES WITH 32 BIT LONGS				*/
/* all these machinations have the effect of reducing the size of the		*/
/* temporary files created during the first pass.  the intent is to write	*/
/* out a sequence of "codebuf" structures and read them back in.		*/
/* unfortunately this results in very large and sparse temporary files.		*/
/* hence the following packing scheme is used to make a series of records of	*/
/* 1, 2, or 3 words in length depending upon the size of cindex and cvalue.	*/
/********************************************************************************/

/* The highest 2 bits assert the size of the symbol index:
 * 00 = not present, 01 = 16-bit index, 11 = 32-bit index.
 * The next 2 bits (going high to low) assert the size of the value:
 * 00 = not present, 01 = 16-bit value, 11 = 32-bit value.
 * These six conditions determine the contents of the low order 16
 * bits of the first word of the record, whether there is a second
 * or third word, and the contents of these words:
 *
 * symindx	value	low 16 bits	2nd word	3rd word
 * -------	-----	-----------	--------	--------
 * 0		0	0		none		none
 * 0		16bits	value		none		none
 * 0		32bits	none		value		none
 * 16bits	0	symindx		none		none
 * 16bits	16bits	symindx		value		none
 * 16bits	32bits	symindx		value		none
 * 32bits	0	none		symindx		none
 * 32bits	16bits	value		symindx		none
 * 32bits	32bits	0		symindx		value
 */
#define SYMISABSENT(pckword)		((0x3 & ((pckword)>>30)) == 0)
#define MKSYMABSENT(pckword)	{pckword &= 0x3fffffff;}
#define SYMIS16BITS(pckword)		((0x3 & ((pckword)>>30)) == 0x1)
#define MKSYM16BITS(pckword)	{pckword &= 0x3fffffff;pckword |= 0x40000000;}
#define SYMIS32BITS(pckword)		((0x3 & ((pckword)>>30)) == 0x3)
#define MKSYM32BITS(pckword)	{pckword &= 0x3fffffff;pckword |= 0xc0000000;}
#define VALISABSENT(pckword)		((0x3 & ((pckword)>>28)) == 0)
#define MKVALABSENT(pckword)	{pckword &= 0xcfffffff;}
#define VALIS16BITS(pckword)		((0x3 & ((pckword)>>28)) == 0x1)
#define MKVAL16BITS(pckword)	{pckword &= 0xcfffffff;pckword |= 0x10000000;}
#define VALIS32BITS(pckword)		((0x3 & ((pckword)>>28)) == 0x3)
#define MKVAL32BITS(pckword)	{pckword &= 0xcfffffff;pckword |= 0x30000000;}

/* the next 6 bits contain the action number.	*/
/* NOTE: THE MAXIMUM ACTION NUMBER 63!		*/
#	define ACTNUMMASK 0x0fc00000
#	define ACTNUM(X) ((ACTNUMMASK & (X)) >> 22)
#	define MKACTNUM(X) (((unsigned long)(X) << 22) & ACTNUMMASK)

/* the next 6 bits tell the number of bits of the value which		*/
/* are meant to be data. NOTE: THE MAXIMUM NUMBER OF BITS IS 63!	*/
#	define NUMBITMASK 0x003f0000
#	define NUMBITS(X) ((NUMBITMASK & (X)) >> 16)
#	define MKNUMBITS(X) (((unsigned long)(X) << 16) & NUMBITMASK)

/* the low order 16 bits are either a symbol	*/
/* index or a 16 bit representable value	*/
#	define UPRBITS 0xffff0000
#	define LWRBITS ~UPRBITS
#	define SYMORVAL(X) (LWRBITS & (unsigned long)(X))
