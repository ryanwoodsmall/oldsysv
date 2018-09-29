/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:m32/system.h	1.9"
/*
 */

#define	USEREGIONS	0	/* allows allocation by regions */
#define	TRVEC	0		/* allows transfer vectors	*/
#define	UNIX	1		/* ld used as standard UNIX tool*/
#define	DMERT	0		/* ld used to generate DMERT obj*/
#define	B16		0		/* ld generates basic-16 object	*/
#define X86		0		/* ld generates extended 8086	*/
#define	N3B		0		/* ld generates 3B-20 object	*/
#define	M32		1		/* ld generates BELLMAC-32 obj	*/
#define U3B 		0		/* ld generates 3B-20Simplex obj*/
#define	VAX		0		/* ld generates VAX 11/780 obj	*/
#define	PDP		0		/* ld generates PDP 11/70  obj	*/
#define	DEC		0		/* ld generates DEC object	*/
#define	IANDD	0		/* allows separate i and d	*/
#define SWABFMT	0		/* text in reverse order 	*/
#define PAGING		1	/* ld uses UNIX 6.0 paging	*/
#define	COMMON	1		/* handle .comm's		*/
#define NATIVE		1	/* ld creates a.out in native mode	*/

#ifndef TS
#define	TS		1		/* ld executed under UNIX-TS	*/
#endif

#ifndef RT
#define	RT		0		/* ld executed under UNIX-RT	*/
#endif

#ifndef PORT5AR
#define PORT5AR		0		/* 5.0 archive format 	  */
#endif

#ifndef PORTAR
#define PORTAR		0		/* defines archive format */
#endif
