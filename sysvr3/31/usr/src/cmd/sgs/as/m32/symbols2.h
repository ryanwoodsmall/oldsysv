/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:m32/symbols2.h	1.5.1.1"
/*
*/

/*
 *	NINSTRS	= number of legal assembler instructions (from ops.out)
 *	NSYMS	= the number of symbols allowed in a user program
 */

#if	FLOAT
#define	NINSTRS	394
#else
#define	NINSTRS	358
#endif

#if	pdp11
#define NSYMS	1693
#else
#define	NSYMS	1009
#endif
