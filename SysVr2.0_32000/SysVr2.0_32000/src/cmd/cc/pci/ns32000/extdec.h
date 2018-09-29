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
/*	@(#)extdec.h	1.3	*/
extern	boolean	wwchanged;	/* has a window altered anything? */
extern	boolean	isreg();	/* is this operand a register? */
extern	boolean	isstatic();	/* is this operand in static memory? */
extern	boolean	isdead();	/* isreg() && is it dead after node? */
extern	boolean	isconst();	/* is this operand an int constant? */
extern	int	constval;	/* value of last constant */
extern	boolean	isfconst();	/* is this operand a float constant? */
extern	double	fconstval;	/* value of last constant */
extern	boolean	noside();	/* has this operand no side effects? */
extern	char	*destarg();	/* last dest operand of an instruction node */

/* Peephole flags used to enable/disable certain improvements */

extern	boolean	loopflag;	/* enables use of loop instrs. */
extern	boolean	bitbflag;	/* enables use of branch-on-bit instrs. */

/* Externals for window statistics */

extern	int	w1size;		/* # of elements in each of following arrays */
extern	int	w1stats[];	/* counters; one per improvement */
extern	char	*w1opts[];	/* English description; one per improvement */

extern	int	w2size;		/* # of elements in each of following arrays */
extern	int	w2stats[];	/* counters; one per improvement */
extern	char	*w2opts[];	/* English description; one per improvement */

extern	int	w3size;		/* # of elements in each of following arrays */
extern	int	w3stats[];	/* counters; one per improvement */
extern	char	*w3opts[];	/* English description; one per improvement */
