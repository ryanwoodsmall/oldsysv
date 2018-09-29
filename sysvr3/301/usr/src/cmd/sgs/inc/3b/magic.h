/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sgs-inc:3b/magic.h	1.3"
/*
 */

/* normal unix user, text and data in separate segments = 410		*/
#define	NMAGIC		0410
/* text and data squashed together = 407				*/
#define	OMAGIC		0407
/* public library created by ldp					*/
#define	PLIBMAGIC	0370
/* data library created by ldp						*/
#define	DLIBMAGIC	0371
/* ldp created kernel process						*/
#define	KMAGIC		0401
/* ldp created supervisor process					*/
#define	SMAGIC		0402
/* ldp created user process						*/
#define	USRMAGIC	0403
/* sgen created boot process						*/
#define	BOOTMAGIC	0400
