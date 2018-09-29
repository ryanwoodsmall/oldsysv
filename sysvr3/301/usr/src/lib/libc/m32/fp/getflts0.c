/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-m32:fp/getflts0.c	1.6"

/*
 * Copyright (c) 1984 by AT&T. All rights reserved.
 */

/* This is the "null" version of the floating-point fault
 * routine called directly from the function-call emulation.
 * It will be loaded only if NO modules in the final a.out
 * have #included ieeefp.h (to request full fault handling).
 *
 * This version does nothing but attempt to cause a core-dump
 * by sending SIGFPE to itself.
 *
 * The user code may have set SIGFPE to SIG_IGN, or the user handler
 * may return.  This routine must be prepared for that,
 * and return a sane value to the emulation.
 */

#include	<signal.h>	/* for SIGFPE	*/
#define	P754_NOFAULT	1	/* do NOT force full fault env.	*/
#include	<ieeefp.h>	/* for fp_union	*/

fp_union
_getfltsw( ftype, optype, format, op1, op2, rres )
    fp_ftype	ftype;	/* not used	*/
    fp_op	optype;	/* not used	*/
    char *	format;
    fp_union *	op1;	/* not used	*/
    fp_union *	op2;	/* not used	*/
    fp_union *	rres;	/* not used	*/
    {
    fp_union	res;	/* in case of no core-dump	*/

    extern int	kill();		/* system call, see kill(2)	*/
    extern int	getpid();	/* system call, see getpid(2)	*/

    /* send SIGFPE to this process */
    (void)kill( getpid(), SIGFPE );

    /* if we get here, return a zero to the emulation.
     * A number with zero in all three words is a zero in all
     * formats except packed decimal.
     */
    res.x.w[0] = 0;
    res.x.w[1] = 0;
    res.x.w[2] = 0;
    if ( format[3] == FP_P ) /* generate a packed decimal zero */
	res.p.w[2] = 0x0000000CL;
    return ( res );
    }
