/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:m32/systems.h	1.13"
/*
 */

/*
 *	SYSTEMS.H FOR M32
 */

#define DEBUG	0
#define TRANVEC 0
#define DCODGEN 0
#define M4ON	1
#define FLOAT	1
#define MACRO	0

#if ABWRMAC

#define ER21WAR 1
#define ER16FIX 1
#define ER10ERR 1
#define ER13WAR 1
#define M32RSTFIX 1
#define CHIPFIX 1
#define	ER40FIX 1
#define	ER43ERR 1
#define ER76FIX 1

#endif

#if ABWORMAC

#define ER21WAR 1
#define ER16FIX 1
#define ER10ERR 1
#define ER13WAR 1
#define CHIPFIX 1
#define	ER40FIX 1
#define	ER43ERR 1
#define ER76FIX 1

#endif

#if BMAC

#define ER10ERR 1
#define ER13WAR 1
#define	ER40FIX 1
#define	ER43ERR 1
#define ER76FIX 1

#endif

#define ALLWAR	0
#define	NO_AWAR	1
