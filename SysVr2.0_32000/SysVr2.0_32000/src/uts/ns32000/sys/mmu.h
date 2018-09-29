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
/* bits of the mmu status register */

#define	MS_ERC	0x7		/* error class flag */
#define	MS_TET	0x38		/* translation error trace flag */
#define	MS_BN	0x40		/* breakpoint number bit */
#define MS_ED	0x100		/* error data direction bit */
#define MS_BD	0x200		/* breakpoint direction bit */
#define MS_EST	0x1c00		/* error status flag */
#define MS_BST	0xe000		/* breakpoint status flag */
#define MS_TU	0x10000		/* translate user bit */
#define MS_TS	0x20000		/* translate supv bit */
#define MS_DS	0x40000		/* dual space bit */
#define MS_AO	0x80000		/* access override bit */
#define MS_BEN	0x100000	/* breakpoint enable bit */
#define MS_UB	0x200000	/* user break bit */
#define MS_AI	0x400000	/* abort or interrupt bit */
#define MS_FT	0x800000	/* flow trace bit */
#define MS_UT	0x1000000	/* user trace bit */
#define MS_NT	0x2000000	/* nonsequential trace bit */

/* bits in the EIA register */

#define	EIA_U	0x80000000	/* bit for PTB1 addr */

/* encoding of the registers for lmr() */

#define EIA	0
#define MSR	1
#define PTB0	2
#define PTB1	3
#define BPR0	4
#define BPR1	5
#define BCNT	6
