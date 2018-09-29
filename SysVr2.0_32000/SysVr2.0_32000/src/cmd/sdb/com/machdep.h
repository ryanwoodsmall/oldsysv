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
	/* @(#) machdep.h: 1.8 7/18/83 */

#include	<sys/param.h>
#include	<sys/types.h>
#include	<signal.h>

#if u3b
#include	<sys/macro.h>	/* has stob, sys/seg.h below (also has ptob) */
#else
#if vax || ns32000 || u3b5
#include	<sys/sysmacros.h>
#endif
#endif

#include	<sys/dir.h>

#if u3b
#include	<sys/seg.h>	/* needed for USRSTACK */
#include	<sys/istk.h>	/* needed for istk structure in struct user */
#else
#if u3b5
#define USRSTACK	0xf00000
#endif
#endif

#include	<sys/user.h>
#include	<sys/errno.h>
#if vax || ns32000 || u3b
#include	<a.out.h>
#else
#if u3b5
/* CHANGE THIS WHEN NEW HEADERS ARE IN 6.0 (makefile as well) */
#include	"a.myout.h"
#endif
#endif
#include	<sys/reg.h>
#include	<sys/stat.h>

#if u3b || u3b5
#define SUBCALL		(isubcall(dot,ISP))	/* subroutine call instruction*/
#if u3b
#define SUBRET		get(SDBREG(SP)-8, DSP)	/* subroutine ret instruction */
#else
extern int regvals[];

#define SUBRET		get(regvals[12]-2*WORDSIZE,DSP)
#endif
#define RETREG		0			/* register with ret value    */

/* Given the current fp (frame), these are the next frame pc, ap, fp */
#if u3b
#define XTOB(x) (ptob(x))

#define NEXTCALLPC (frame - (13*WORDSIZE))
#define NEXTARGP   (frame - (12*WORDSIZE))
#define NEXTFRAME  (frame - (11*WORDSIZE))
#ifndef	USEG
#define	USEG	0x5
#endif

#define TXTRNDSIZ	0x20000L	/* = 128K = (1L<<17)	*/

/* Address of UBLOCK (absolute) is beginning of segment 5 */
#define ADDR_U ((unsigned) (USEG * TXTRNDSIZ))

#define BKOFFSET   10	/* offset from beginning of proc to bkpt (no -O) */

#define APNO	9	/* argument pointer register number */
#define FPNO	10	/* frame pointer register number */
#else
#define XTOB(x) (ctob(x))

#define NEXTCALLPC	(frame-(9*WORDSIZE))
#define NEXTARGP	(frame-(8*WORDSIZE))
#define NEXTFRAME	(frame-(7*WORDSIZE))

#define TXTRNDSIZ	0x80000L

#define BKOFFSET	9	/* offset from start of proc to bkpt */

#define APNO		10
#define FPNO		9
#endif
#define	PROCOFFSET	0	/* offset from label to first instruction */
#define ALTBKOFFSET 0	/* offset from beginning of proc to bkpt */

#define NUMREGLS 16	/* number of "registers" in reglist[] */

#define ISREGVAR(regno)	(3<=(regno) && (regno) <= 8)

#else
#if vax
#define XTOB(x) (ctob(x))
#define SUBCALL		((get(dot,ISP) & 0xff) == 0xfb) /* subroutine call */
#define SUBRET		get(SDBREG(SP)+16, DSP)	/* subroutine ret instruction */

#define RETREG		0			/* register with ret value */

/* Given the current fp (frame), these are the next frame pc, ap, fp */
#define NEXTCALLPC (frame + (4*WORDSIZE))
#define NEXTARGP   (frame + (2*WORDSIZE))
#define NEXTFRAME  (frame + (3*WORDSIZE))

#define TXTRNDSIZ 512L

#define ADDR_U ((unsigned) 0x7ffff800)	/* absolute address of UBLOCK */

#define	PROCOFFSET	2	/* offset from label to first instruction */
#define BKOFFSET    0	/* offset from beginning of proc to bkpt */
#define ALTBKOFFSET 0	/* offset from beginning of proc to bkpt */

#define APNO	12	/* argument pointer register number */
#define FPNO	13	/* frame pointer register number */
#define NUMREGLS 17	/* number of "registers" in reglist[] */

#define ISREGVAR(regno) (6 <= (regno) && (regno) < 12)
#else
#if ns32000
#define XTOB(x) (ctob(x))
#define SUBCALL		((get(dot,ISP) & 0x7ff) == 0x67f)  /* subroutine call */
#define SUBRET		get(SDBREG(SP), DSP)  /* subroutine ret instruction */

#define RETREG		0			/* register with ret value */

/* Given the current fp (frame), these are the next frame pc and fp */
#define NEXTCALLPC (frame + (WORDSIZE))
#define NEXTFRAME  (frame)

#define TXTRNDSIZ 512L

#define ADDR_U ((unsigned) 0x810000)	/* absolute address of UBLOCK */

#define	PROCOFFSET  0	/* offset from label to first instruction */
#define BKOFFSET    0	/* offset from beginning of proc to bkpt */
#define ALTBKOFFSET 0	/* offset from beginning of proc to bkpt */

#define FPNO	  8	/* frame pointer register number */
#define NUMREGLS 12	/* number of "registers" in reglist[] */

#define ISREGVAR(regno) (4 <= (regno) && (regno) < 8)
#endif
#endif
#endif

extern char uu[XTOB(USIZE)];

#ifndef ns32000
extern ADDR	callpc, frame, argp;	/* current stack frame */
#else
extern ADDR	callpc, frame;		/* current stack frame */
#endif

#define WORDSIZE (sizeof(int))	/* wordsize in bytes on this machine */
#define REGSIZE WORDSIZE	/* register size in bytes on this machine */

#define NOBACKUP 0		/* set to 1 if machine does not back up */
			        /* to previous instruction at exception */
#define ADDRTYPE	"d"	/* type of address for getval */

#ifndef ns32000
#define MAXPOS	0x7ffffff		/* maximum address */
#else
#define MAXPOS	0xffffff		/* maximum address */
#endif

/*  two extra numbers to be printed with regs; in optab.c */
/*  removed because these are not offsets from R0; can't use SDBREG */
/*
#define VAL1	((unsigned)&(((struct user *) 0)->u_rval1)
#define VAL2	((unsigned)&(((struct user *) 0)->u_rval2)
*/

/* ptracew modes */
#define	SETTRC	0
#define	RDUSER	2
#define	RIUSER	1
#define	WDUSER	5
#define WIUSER	4
#define	RUREGS	3
#define	WUREGS	6
#define	CONTIN	7
#define	EXIT	8
#define SINGLE	9

extern REGLIST reglist [];

#if vax || ns32000 || u3b
#define SDBREG(A) (((struct user *)uu)->u_ar0[A])
#define SYSREG(A) ((int) (((char *) (&SDBREG(A)) - ((int) uu))))
#endif

#if ns32000
#define NUMREGS 8	/* number of general purpose registers */
#else
#define NUMREGS 16	/* number of general purpose registers */
#endif

#if vax || ns32000 || u3b
#define ISREGN(reg)	(0<= (reg) && (reg) < NUMREGS)
#else
#if u3b5
#define ISREGN(reg)	(0 <= (reg) && (reg) < NUMREGS && (reg) != 13 && (reg) != 14)
#endif
#endif

#if vax || ns32000 || u3b
#define USERPC  SDBREG(PC)
#else
#if u3b5
#define USERPC	regvals[15]
#endif
#endif

union word {
	char c[WORDSIZE]; 
	unsigned char cu[WORDSIZE];
	short s[WORDSIZE/2];
	unsigned short su[WORDSIZE/2];
	int w;
	unsigned int iu;
	long l;
	float f;
};
union dbl {
	struct {
		int w1, w2;
	} ww;
	double dd;
	int i;
	float f;
	char cc[WORDSIZE*2];
};
