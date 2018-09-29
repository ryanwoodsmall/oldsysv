/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:ml/gate.c	10.6"

#include	"sys/types.h"
#include	"sys/psw.h"
#include	"sys/gate.h"
#include	"sys/pcb.h"


/*
 *	3B2 low memory - Kernel virtual address space
 */


/*
 *	1st level gate table
 */

extern struct gate_l2 gatex[], gates[], gaten;

gate_l1 gate1[32] ={ gatex, gates, &gaten, &gaten, &gaten, &gaten,
		     &gaten, &gaten, &gaten, &gaten, &gaten, &gaten,
		     &gaten, &gaten, &gaten, &gaten, &gaten, &gaten,
		     &gaten, &gaten, &gaten, &gaten, &gaten, &gaten,
		     &gaten, &gaten, &gaten, &gaten, &gaten, &gaten,
		     &gaten, &gaten };

/*
 *	Place holder for reset vector (physical address only)
 */
extern struct kpcb kpcb_null;

struct kpcb* Xreset = &kpcb_null;

/*
 *	Process and Stack exception vectors
 */
extern struct kpcb kpcb_px, kpcb_sx;

struct kpcb* Xproc = &kpcb_px;
struct kpcb* Xstack = &kpcb_sx;

/*
 *	Interrupt vectors
 */
extern struct kpcb kpcb_L8, kpcb_L9, kpcb_L15;

struct kpcb *Ivect[256] = {
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_L8,	/* iswtch and stream scheduler. */
		&kpcb_L9,	/* timein & iuint */
		&kpcb_null,
		&kpcb_null,	/* idint & ifint */
		&kpcb_null,
		&kpcb_null,	/* iuart */
		&kpcb_null,
		&kpcb_L15,	/* clock & syserr */
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,	/* ppc */
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,	/* ppc */
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,	/* ppc */
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,	/* ni */
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		&kpcb_null,
		};

/*
 *  Second level gate tables
 */

extern int Xsyscall();

struct gate_l2 gates[88] ={ { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			    { GPSW, Xsyscall },
			  };

/* normal exception entry points -- defined in ttrap.s */
extern int nrmx_XX(), nrmx_iop(), nrmx_ilc();

/* normal exception gate table, indexed by the internal state code
 * field in %psw.  See Appendix 1.C of CPU Requirements.
 */
struct gate_l2 gatex[16] ={ { GPSW, nrmx_XX },	/* integer zerodivide*/
			    { GPSW, nrmx_XX },	/* trace trap        */
			    { GPSW, nrmx_iop },	/* illegal opcode    */
			    { GPSW, nrmx_XX },	/* reserved opcode   */
			    { GPSW, nrmx_XX },	/* invalid descriptor*/
			    { GPSW, nrmx_XX },	/* external mem fault*/
			    { GPSW, nrmx_XX },	/* gate vector fault */
			    { GPSW, nrmx_ilc },	/* illegal level chg */
			    { GPSW, nrmx_XX },	/* reserved data type*/
			    { GPSW, nrmx_XX },	/* integer overflow  */
			    { GPSW, nrmx_XX },	/* privileged opcode */
			    { GPSW, nrmx_XX },	/* ISC==11, unused   */
			    { GPSW, nrmx_XX },	/* ISC==12, unused   */
			    { GPSW, nrmx_XX },	/* ISC==13, unused   */
			    { GPSW, nrmx_XX },	/* breakpoint trap   */
			    { GPSW, nrmx_XX }	/* privileged reg    */
			  };


extern int nrmx_YY();

/*
 *	Dummy gate vector to catch user code that does a GATE with
 *	%r0 anything but 0 or 1.  The normal exception code will
 *	fault the user process.
 */
struct gate_l2 gaten ={ GPSW, nrmx_YY };

/*
 *	Allocate enough gate vectors so gate.o fills one page.
 *	A user trying to do a GATE system call entry with a huge
 *	value in %r1 will either hit here, or go past the end of gate.o,
 *	which will be unmapped memory, caught by the mmu hardware.
 *	The structure is static, since no one else needs to
 *	know it is here.
 */
static struct gate_l2 gatefiller[5] = { {GPSW, nrmx_YY},
					{GPSW, nrmx_YY},
					{GPSW, nrmx_YY},
					{GPSW, nrmx_YY},
					{GPSW, nrmx_YY},
				};
