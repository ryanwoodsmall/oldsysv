/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*
 * static char ID_mcroh[] = "@(#) sgsmacros.h: 1.1 1/7/82";
 */

#define MAXLDIRS	8
#define COMF77		"(common)"

#define max(a,b)		(a<b ? b: a)
#define min(a,b)		(a>b ? b: a)

#define size(str)		(strlen(str) + 1)

#define equal(a,b,l)		(strncmp(a,b,l) == 0)

#define copy(a,b,l)		strncpy(a,b,l)

/*
 * Macros used for 32 bit address manipulation for M32
 */
#if M32
#define AGREE(a,b)	((a & 0x80000000) == (b & 0x80000000))
#define GT32(a,b)	(AGREE(a,b) ? a > b : a < b)
#define LT32(a,b)	(AGREE(a,b) ? a < b : a > b)
#else
#define GT32(a,b)	(a>b)
#define LT32(a,b)	(a<b)
#endif
