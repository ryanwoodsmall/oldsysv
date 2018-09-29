/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-m32:fp/getflth0.c	1.7"

/*
 * Copyright (c) 1984 by AT&T. All rights reserved.
 */

/* _GETFLTHW( r0ptr, handler )
 * This is the non-floating-point version of the SIGFPE "glue routine".
 * It is responsible for determining what caused SIGFPE to be raised,
 * and setting the global state used by the user handler.
 * This version of_getflthw() can be included in a floating-point
 * program if no module included ieeefp.h.
 *
 * ENTRY CONDITIONS:
 *	both the OS and signal() think SIGFPE is reset to SIG_DFL.
 *	if the signal was caused by a true hardware fault,
 *	    the fault-type in the saved PSW is ON_NORMAL (3), and
 *	    the internal state code is IZDFLT (0), IOVFLT (9), or
 *	    XMEMFLT (5).
 *	if the signal was raised during a system call,
 *	    the fault-type in the saved PSW is ON_STACK (2), and
 *	    the internal state code is STKFLT (1).
 *	if the signal was raised while the process was time-sliced,
 *	    the fault-type in the saved PSW is ON_PROCESS (1), and
 *	    the internal state code is POPCBFLT (0).
 *	a complete copy of the user registers is saved at r0ptr.
 */

#include	<sys/signal.h>	/* for SIG_DFL, SIG_IGN, SIGFPE	*/
#include	<sys/psw.h>	/* for PSW fault-type defines	*/
#define	P754_NOFAULT	1	/* avoid generating extra code	*/
#include	<ieeefp.h>	/* for floating-point datatypes	*/

#define R_PSW	(11)		/* register number of the PSW	*/
#define	ASR_CSC	(0x20000)	/* the <CSC> bit in the ASR	*/

 void
_getflthw( r0ptr, handler )
    int *   r0ptr;	/* pointer to array of 16 words	*/
    int ( * handler)();	/* user-defined handler		*/
    {
    extern int	_fp_hw;	/* hardware present flag	*/

    /* WARNING: the following variables are used in asm() escapes
     * as offsets off the frame pointer.
     */
    int		o_asr;	/* saved user's ASR		*/
    int		n_asr;	/* new ASR with all exc. masked	*/
    extended	regs[4];/* %x0-%x3, no need to save %dr */

    /* either this is a non-floating-point program or no module
     * included ieeefp.h.  Either way, we never have
     * to format an IEEE P754 fault structure, and can call the
     * user handler directly.  We reset _fpfault to NULL just in case
     * some user code has assigned a value to it.
     */

    _fpfault = (struct fp_fault *)0;

    if ( ( (psw_t *)&(r0ptr[R_PSW]) )->FT != ON_NORMAL ) {
	_fpftype = UNKNOWN;
	}
    else {
	switch ( ( (psw_t *)&(r0ptr[R_PSW]) )->ISC ) {

	case IZDFLT:	/* integer zero divide	*/
	    _fpftype = INT_DIVZ;
	    break;

	case IOVFLT:	/* integer overflow	*/
	    _fpftype = INT_OVFLW;
	    break;

	/*   XMEMFLT:	/* floating-point hardware fault	*/
	default:
	    _fpftype = UNKNOWN;
	    break;
	    }
	}

    /* now, call the user handler, with the appropriate signal
     * NOTE: this may destroy registers 3-8, but we only need
     * %fp (to RESTORE/RET) after this call.
     */

    /* since signal() does NOT save the floating-point registers,
     * we must do it here. The ASR<UM> bit must be turned OFF
     * to avoid taking an underflow exception when reading the
     * registers.  The ASR is not saved since it contains global
     * state; the DR is not saved since it is only defined during
     * a floating-point exception.
     */
    if ( _fp_hw == 1 ) {
	n_asr = ASR_CSC;		/* ASR with <CSC> bit set */
	asm("	mmovfa	0(%fp)	");	/* o_asr = ASR		  */
	asm("	mmovta	4(%fp)	");	/* ASR   = n_asr	  */
	asm("	mmovxx	%x0, 2*4(%fp) ");
	asm("	mmovxx	%x1, 5*4(%fp) ");
	asm("	mmovxx	%x2, 8*4(%fp) ");
	asm("	mmovxx	%x3,11*4(%fp) ");
	o_asr |= ASR_CSC;
	asm("	mmovta	0(%fp)	");	/* ASR = o_asr | ASR_CSC  */
	}

    (*handler)( SIGFPE );

    if ( _fp_hw == 1 ) {
	asm("	mmovfa	0(%fp)	");	/* o_asr = ASR		  */
	asm("	mmovta	4(%fp)	");	/* ASR   = n_asr	  */
	asm("	mmovxx	 2*4(%fp),%x0 ");
	asm("	mmovxx	 5*4(%fp),%x1 ");
	asm("	mmovxx	 8*4(%fp),%x2 ");
	asm("	mmovxx	11*4(%fp),%x3 ");
	o_asr |= ASR_CSC;
	asm("	mmovta	0(%fp)	");	/* ASR = o_asr | ASR_CSC  */
	}
    }
