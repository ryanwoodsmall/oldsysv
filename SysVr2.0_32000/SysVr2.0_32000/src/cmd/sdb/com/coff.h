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
	/* @(#) coff.h: 1.8 7/18/83 */

/*		common object file format #include's */

/*	requires a.out.h (which is #included by head.h) */
#include "sgs.h"
#if u3b || u3b5
#define OMAGIC	AOUT1MAGIC	/* combined text and data segments */
#define NMAGIC	AOUT2MAGIC	/* separate text and data segments */
#define	PMAGIC	AOUT3MAGIC	/* Paging aout header magic number */
#endif

#if vax || ns32000
#define OMAGIC	AOUT2MAGIC	/* combined text and data segments */
#define NMAGIC	AOUT1MAGIC	/* separate text and data segments */
#define	PMAGIC	AOUT3MAGIC	/* Paging aout header magic number */
#endif

/*	 added for convenience */
#define ISTELT(c)	((c==C_MOS) || (c==C_MOU) || (c==C_MOE) || (c==C_FIELD))
#define ISREGV(c)	((c==C_REG) || (c==C_REGPARM))
#define ISARGV(c)	((c==C_ARG) || (c==C_REGPARM))
#define ISTRTYP(c)	((c==T_STRUCT) || (c==T_UNION) || (c==T_ENUM))
#define ISSTTAG(c)	((c==C_STRTAG) || (c==C_UNTAG) || (c==C_ENTAG))
#define ISSTACK(c)	((c==C_REG) || (c==C_REGPARM) || (c==C_ARG) || (c==C_AUTO))

#define MAXAUXENT	1	/* max number auxilliary entries */
