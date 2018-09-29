/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-m32:fp/getflts1.c	1.4"

/*
 * Copyright (c) 1984 by AT&T. All rights reserved.
 */

/* define the FULL _getfltsw(), the "glue" routine between the software
 * emulation and a user fault handler.
 *
 * NOTE: as called from assembly level,
 * the arguments op1, op2, and rres, are not really fp_union's.
 * Doing an assignment "trueunion = op1;" will do a structure
 * copy that may copy additional garbage, but the needed bits
 * will have been copied.
 */
#include	<signal.h>	/* for SIGFPE definition	*/
#define	P754_NOFAULT	1	/* avoid forcing FULL fault env.*/
#include	<ieeefp.h>	/* for floating-point datatypes	*/


 fp_union
_getfltsw( ftype, optype, format, op1, op2, rres )
    fp_ftype	ftype;	/* fault type		*/
    fp_op	optype;	/* operation type	*/
    char *	format;	/* vector of formats	*/
    fp_union *	op1;	/* input operand 1	*/
    fp_union *	op2;	/* input operand 2	*/
    fp_union *	rres;	/* rounded result	*/
    {
    struct fp_fault fault; /* _fpfault == &fault*/
    fp_union	retval;	/* value back to emul.	*/
    int (*	hand)();/* user's signal handler*/

    /* we can simplify code later if we initialize retval.
     * Setting all three words to zero is a zero in every format
     * except comparisons (where it is FP_EQ, which is what is needed)
     * and packed decimal.
     */
    retval.x.w[0] = 0;
    retval.x.w[1] = 0;
    retval.x.w[2] = 0;
    if (format[3] == FP_P )		/* packed decimal	*/
	retval.p.w[2] = 0x0000000CL;	/* sign byte == +	*/

    /* find out the user's disposition for SIGFPE */
    hand = signal( SIGFPE, SIG_DFL);

    switch( (int)hand ) {

    case (int)SIG_DFL:	/* user wants a core dump */
	/* the call to signal() above has already
	 * reset the handling of SIGFPE to SIG_DFL,
	 * artificially causing a SIGFPE will dump
	 * core.
	 */
	(void)kill( getpid(), SIGFPE );
	/*NOTREACHED*/
    
    case (int)SIG_IGN:	/* user doesn't care	*/
	/* we must re-establish SIG_IGN for SIGFPE
	 * and return a zero of the right type.
	 */
	(void)signal( SIGFPE, SIG_IGN );
	return ( retval );
	/*NOTREACHED*/

    default:		/* user has a handler! */
	/* the call to signal() above has already
	 * reset the handling of SIGFPE to SIG_DFL
	 * (simulating OS processing),
	 * so we just format the fault structure and
	 * call the user handler.
	 */

	/* format the local fault structure */
	fault.operation = optype;

	fault.operand[0].type  = format[0];
	fault.operand[0].value = *op1;

	fault.operand[1].type  = format[1];
	if ( format[1] != FP_NULL )
	    fault.operand[1].value = *op2;

	fault.t_value.type = format[2];
	if ( format[2] != FP_NULL )
	    fault.t_value.value = *rres;

	fault.result.type = format[3];
	fault.result.value = retval;

	/* set the global variables */
	_fpftype = ftype;
	_fpfault = & fault;

	/* call the user handler 
	 * since the handler may not do a SAVE/RESTORE, do it here
	 */
	asm("	PUSHW	%r3	");
	asm("	PUSHW	%r4	");
	asm("	PUSHW	%r5	");
	asm("	PUSHW	%r6	");
	asm("	PUSHW	%r7	");
	asm("	PUSHW	%r8	");
	asm("	PUSHW	%fp	");
	(*hand)( SIGFPE );
	asm("	POPW	%fp	");
	asm("	POPW	%r8	");
	asm("	POPW	%r7	");
	asm("	POPW	%r6	");
	asm("	POPW	%r5	");
	asm("	POPW	%r4	");
	asm("	POPW	%r3	");

	/* NULL out _fpfault to avoid leaving a dangling pointer */
	_fpfault = (struct fp_fault *)0;

	return ( fault.result.value );
	/*NOTREACHED*/
	}
    }
