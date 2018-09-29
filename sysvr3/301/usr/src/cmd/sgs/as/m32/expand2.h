/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:m32/expand2.h	1.4"

/*
 */

#ifndef	CALLPCREL
#define	NITYPES 6	/* number of sde types */

#define UBRB 		0
#define CBRB 		1
#define BSBB 		2
#define UBRH 		3
#define CBRH 		4
#define BSBH 		5

#else
#define NITYPES	8	/* number of sdi types */

#define UBRB 		0
#define CBRB 		1
#define BSBB 		2
#define	PCBRELCALL	3
#define PCHRELCALL	4
#define UBRH 		5
#define CBRH 		6
#define BSBH 		7

#endif
