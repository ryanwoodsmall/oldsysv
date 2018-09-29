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
/* @(#)init.h	6.4 */
extern int clkstart(),cinit(),binit(),errinit(),iinit(),inoinit();
extern int finit();
extern int flckinit();
#ifdef X25_0
extern x25init();
#endif
#ifdef BX25S_0
extern bxncinit();
extern sessinit();
#endif
#ifdef ST_0
extern	stinit();
#endif
#ifdef	VPM_0
extern	vpminit();
#endif
#ifdef EM_0
extern  eminit();
#endif

/*	Array containing the addresses of the various initializing	*/
/*	routines executed by "main" at boot time.			*/

int (*init_tbl[])() = {
	inoinit,
	clkstart,
	cinit,
	binit,
	errinit,
	finit,
	iinit,
	flckinit,
#ifdef	VPM_0
	vpminit,
#endif
#ifdef X25_0
	x25init,
#endif
#ifdef ST_0
	stinit,
#endif
#ifdef BX25S_0
        bxncinit,
        sessinit,
#endif
#ifdef EM_0
	eminit,
#endif
	0
};
