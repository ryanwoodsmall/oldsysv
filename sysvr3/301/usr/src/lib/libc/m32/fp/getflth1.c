/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-m32:fp/getflth1.c	1.5.1.5"

/*
 * Copyright (c) 1984 by AT&T.  All rights reserved.
 */

/*************************************************************
 * WARNING WARNING WARNING WARNING
 *
 * This file contains code specific to the function-call CCS,
 * and will need to be modified for the in-line CCS.
 * See the comments toward the end of this file.
 *
 * WARNING WARNING WARNING WARNING
 *************************************************************/

/* _GETFLTHW( r0ptr, handler )
 * This is the FULL version of the SIGFPE "glue routine".
 * It is responsible for determining what caused SIGFPE to be raised,
 * and setting the global state used by the user handler.
 * If the user handler returns, the saved PC may need to be advanced
 * past the faulting instruction.
 *
 * ENTRY CONDITIONS:
 *	a complete in-order copy of the user registers is at r0ptr.
 *	SIGFPE was raised
 *	if the signal was caused by a true floating-point fault,
 *	    the fault type in the saved PSW is ON_NORMAL (3),
 *	    and the internal state code is XMEMFLT (5).
 *	if the signal was caused by an integer fault,
 *	    the fault type in the saved PSW is ON_NORMAL (3),
 *	    and the internal state code is IZDFLT (0) or IOVFLT (9).
 *	if the signal was raised during a system call,
 *	    the fault type is ON_STACK (2), and
 *	    the internal state code is STKFLT (1).
 *	if the signal was raised while the process was time-sliced,
 *	    the fault type is ON_PROCESS (1), and
 *	    the internal state code is POPCBFLT (0).
 */

#include	<sys/signal.h>	/* for SIGFPE			*/
#include	<sys/psw.h>	/* for PSW fault-type defines	*/
#define	P754_NOFAULT	1	/* avoid generating extra code	*/
#include	<ieeefp.h>	/* floating-point data-types	*/

extern int	_fp_hw;		/* to know what HW is equipped	*/

#define	R_PSW	(11)		/* register number for the PSW	*/
#define	R_AP	(10)		/* register number for the AP	*/
#define	R_FP	(9)		/* register number for the FP	*/
#define	R_PC	(15)		/* register number for the PC	*/

/* the hardware access routines are passed a flag saying whether to
 * read or write the MAU registers.
 */
#define	HW_READ		(0)
#define	HW_WRITE	(1)

/* the following structure collects in one place everything that
 * is known about the MAU operation from disassembling the
 * WE32100 instruction stream and the WE32106 operation word.
 */
struct mau_ins {
    unsigned int op_width;	/* number of words in operands	*/
    int *	 rd_op;		/* address of read operand	*/
    int *	 wr_op;		/* address of write operand	*/
    unsigned int opcode;	/* opcode part of SPOP word	*/
    struct mau_op {
	char	type;		/* IS_NULL, IS_MEM, or IS_REG	*/
#define IS_NULL	(0)
#define IS_MEM	(1)
#define IS_REG	(2)
	char	regno;		/* in [0,3] */
	char	format;		/* in [1,3] */
	char	rounding;	/* in [0,3] (0 if rounding is unknown*/
	}	op[3];		/* decoded operand descriptors	*/
    };

/* HARDWARE REGISTER LAYOUTS
 * the following structures define the bit meanings in the MAU DR
 * and ASR.
 */

/* the DR is reformatted from a 17-bit exponent to a 15-bit exponent */
typedef struct dr_type {
    union {
	struct pre_format {
	    unsigned int	:14;
	    unsigned int sign	: 1;
	    unsigned int exp	:17;
	    } preword0;
	struct post_format {
	    unsigned int	:16;
	    unsigned int sign	: 1;
	    unsigned int exp	:15;
	    } postword0;
	} word0;
    unsigned long	word1;
    unsigned long	word2;
    } dr_t;

/* the ASR is defined at the bit-level */
typedef struct asr_type {
    unsigned ra		: 1;	/* 31:result available  (ind)	*/
    unsigned		: 5;	/* <unused>			*/
    unsigned ecp	: 1;	/* 25: exception present(ind)	*/
    unsigned ntnc	: 1;	/* 24: non-trapping NaN (cntl)	*/
    unsigned rc		: 2;	/* 22-23: rounding mode (cntl)	*/
    unsigned n		: 1;	/* 21: negative result  (ind)	*/
    unsigned z		: 1;	/* 20: zero result	(ind)	*/
    unsigned io		: 1;	/* 19: integer ovflw	(ind)	*/
    unsigned ps		: 1;	/* 18: inexact result	(sticky)*/
    unsigned csc	: 1;	/* 17: context switch	(ind)	*/
    unsigned uo		: 1;	/* 16: unordered result (ind)	*/
    unsigned		: 1;	/* 15: <unused>			*/
    unsigned im		: 1;	/* 14: invalid op	(mask)	*/
    unsigned om		: 1;	/* 13: overflow		(mask)	*/
    unsigned um		: 1;	/* 12: underflow	(mask)	*/
    unsigned qm		: 1;	/* 11: zerodivide	(mask)	*/
    unsigned pm		: 1;	/* 10: loss of Precision(mask)	*/
    unsigned is		: 1;	/* 09: invalid op	(sticky)*/
    unsigned os		: 1;	/* 08: overflow		(sticky)*/
    unsigned us		: 1;	/* 07: underflow	(sticky)*/
    unsigned qs		: 1;	/* 06: zerodivide	(sticky)*/
    unsigned pr		: 1;	/* 05: partial remainder(ind)	*/
    unsigned		: 5;	/* <unused>			*/
    } asr_t;

/* TABLE OF CONTENTS and CALLING GRAPH:
 * this file contains twenty functions in eight classes:
 * 1) the top-level driver (visible to the outside world), and an
 *    interface routine to the user handler
 *	_getflthw()
 *	call_user()
 * 2) four functions to understand the WE32100 instruction stream
 *	nextins()
 *	nextop()
 *	cpu_opcode()
 *	cpu_operand()
 * 3) two functions to understand the WE32106 (MAU) operation word
 *	mau_opcode()
 *	mau_operand()
 * 4) two functions to map between floating-point type and size
 *	size_to_type()	[functions which use this are flagged [4]]
 *	type_to_size()	[functions which use this are flagged [4]]
 * 5) three functions for WE32106 (MAU) hardware access
 *	mau_asr()	[functions which use this are flagged [5]]
 *	mau_dr()	[functions which use this are flagged [5]]
 *	mau_reg()	[functions which use this are flagged [5]]
 * 6) three functions built on the hardware access routines
 *	mau_except()
 *	reformat()
 *	spop_fetch()
 * 7) two functions to reformat the Data Register 
 *	dr_format()
 *	qnan_gen()
 * 8) two intermediate-level, CCS dependent, routines
 *	format_data()
 *	spop_store()
 * NOTE: to unclutter the calling graph, most uses of the mapping
 *	(class 4) and hardware access (class 5) functions are indicated
 *	only by a bracketed number, as defined above.
 *
 * 			_getflthw()
 * 			     |
 *   ________________________________________________________________
 *   |        |          |       |    |          |       |          |
 *   |        |          |       |    |          |       |          |
 * nextins cpu_opcode mau_opcode | mau_dr   format_data  |    mau_except
 *   |       /|          |       |            /  |       |          |
 *   |      / |          |       |           /   |     spop_store  [5]
 *   |     /  |          |    call_user     [4]  |       |
 *   |    /   |          |       |               |      [5]
 *   |   /    |          |      [5]    ____________________________ 
 *   |  /     |          |             |            |             |
 * nextop cpu_operand  mau_operand     |            |        dr_format
 *                                     |            |        /   | \
 *                                     |            |       /    |  \
 *                                     |            |      /     | [4,5]
 *                                 spop_fetch       |     /      |
 *                                     |            |    /       |
 *                                    [5]        reformat     qnan_gen
 *                                                  |
 *                                                 [5]
 */
	void		_getflthw();
static	void		call_user();
static	int		cpu_opcode();
static	int *		cpu_operand();
static	void		dr_format();
static	int		format_data();
static	void		mau_asr();
static	void		mau_dr();
static	fp_ftype	mau_except();
static	int		mau_opcode();
static	struct mau_op	mau_operand();
static	int		mau_reg();
static	unsigned char *	nextins();
static	unsigned char *	nextop();
static	void		qnan_gen();
static	void		reformat();
static	fp_type		size_to_type();
static	void		spop_fetch();
static	void		spop_store();
static	int		type_to_size();

 void
_getflthw( r0ptr, handler )
    int	*	r0ptr;		/* pointer to array of 16 words	*/
    int	( *	handler)();	/* user-defined handler		*/
    {
    extern void		   call_user();
    extern int		   cpu_opcode();
    extern int		   format_data();
    extern void            mau_dr();
    extern fp_ftype        mau_except();
    extern int		   mau_opcode();
    extern unsigned char * nextins();
    extern void            spop_store();

    int			i;
    struct fp_fault	localfault;
    fp_ftype		localftype;
    struct mau_ins	mau_ins;
    dr_t		t_dr;		/* temp for DR		*/

    /* get the easy cases out of the way first:
     *	UNKNOWN and INT_OVFLW
     */
    if ( ( (psw_t *)&(r0ptr[R_PSW]) )->FT != ON_NORMAL ) {
	/* the signal must have been generated explicitly,
	 * so we don't know what caused it.
	 */
	_fpftype = UNKNOWN;
	_fpfault = (struct fp_fault *)0;
	call_user( handler );
	return;
	}

    switch ( ( (psw_t *)&(r0ptr[R_PSW]) )->ISC ) {

    default:		/* should not happen!	*/
	_fpftype = UNKNOWN;
	_fpfault = (struct fp_fault *)0;
	call_user( handler );
	return;

    case IOVFLT:	/* integer overflow	*/
	_fpftype = INT_OVFLW;
	_fpfault = (struct fp_fault *)0;
	call_user( handler );

	/* since integer overflow is a trap (rather than an exception),
	 * the PC alreadly points at the next instruction,
	 * so there is nothing else to do.
	 */
	return;

    case IZDFLT:	/* integer zero-divide	*/
	_fpftype = INT_DIVZ;
	_fpfault = (struct fp_fault *)0;
	call_user( handler );

	/* integer zero-divide is an exception (rather than a trap),
	 * so the PC has been backed up to the beginning of the
	 * faulting instruction.  We must advance it to the
	 * beginning of the next instruction.
	 */
	r0ptr[R_PC] = (int)nextins( (unsigned char *)(r0ptr[R_PC]) );
	return;

    case XMEMFLT:	/* a hardware fault		*/
	/* here's the hard task! - fall out of switch	*/
	break;
	}

    /* determine which kind of hardware is equipped	*/
    switch ( _fp_hw ) {

    default:	/* no hardware -- SHOULD NOT HAPPEN!	*/
	break;

    case 1:	/* MAU hardware				*/
	/************************************************************
	 * Grab the MAU DR before doing any other accesses
	 ***********************************************************/
	mau_dr( &t_dr );

	/************************************************************
	 * Find out (from the MAU ASR) what the exception was
	 ***********************************************************/
	localftype = mau_except();
	if ( localftype == UNKNOWN ) /* this should not happen! */
	    break;

	/************************************************************
	 * Zero out localfault
	 ***********************************************************/
	for ( i=0; i < sizeof(localfault)/sizeof(int); i++ )
	    ((int *)&localfault)[i] = 0;

	/************************************************************
	 * Establish basic sanity of the instruction stream,
	 * and fill in mau_ins.
	 ***********************************************************/
	if ( cpu_opcode( r0ptr, &mau_ins ) == 0 )
	    break;
	if ( mau_opcode(
		(unsigned char *)((unsigned long)(r0ptr[R_PC]) + 1),
		&mau_ins ) == 0 )
	    break;

	/************************************************************
	 * Use mau_ins to fill in localfault
	 ***********************************************************/
	if ( format_data( &localfault, &mau_ins, &t_dr, localftype,
		r0ptr ) == 0 )
	    break;

	/************************************************************
	 * Establish default result (a 0 of the right type)
	 * Since we zeroed out localfault above, only need to change
	 * if result is packed decimal, and then only the sign byte
	 ***********************************************************/
	if ( localfault.result.type == FP_P )
	    localfault.result.value.p.w[2] = 0x0000000CL;

	/************************************************************
	 * set the global data items
	 ***********************************************************/
	_fpfault = &(localfault);
	_fpftype = localftype;

	/************************************************************
	 * and off we go ... to the user handler
	 ***********************************************************/
	call_user( handler );

	/**********************************************************
	 * Copy localfault.result to wherever it belongs
	 *********************************************************/
	spop_store( &(localfault), &(mau_ins), r0ptr );

	/**********************************************************
	 * Advance the faulted PC to the next instruction
	 *********************************************************/
	r0ptr[R_PC] = (int)nextins( (unsigned char *)(r0ptr[R_PC]) );

	return;
	}

    /* error exit if no hardware, or inconsistancies in instruction */
    _fpftype = UNKNOWN;
    _fpfault = (struct fp_fault *)0;
    call_user( handler );
    return;
    }

/* CALL_USER( handler ) -- call the user handler.  Since the handler
 *	can be an arbitrary address, there is no guarantee that a
 *	register SAVE/RESTORE will be done, so the GP registers
 *	must be saved/restored here.  In addition, the signal() code
 *	does NOT preserve the MAU registers, so they also must be
 *	saved/restored here.  The ASR and the DR need not be
 *	saved, since the ASR is explicitly declared global in the
 *	floating-point exception document, and the DR value will
 *	already have been captured (in _getflthw()).
 */
 static void
call_user( handler )
    int (* handler)();
    {
    extern int	_fp_hw;		/* do MAU registers exist?	 */
    extern int	mau_reg();	/* hardware access routine	 */

    extended	regs[4];	/* stack space to save registers */

    if ( _fp_hw == 1 ) {
	(void)mau_reg( (int *)&regs[0], HW_READ, 0, 3 );
	(void)mau_reg( (int *)&regs[1], HW_READ, 1, 3 );
	(void)mau_reg( (int *)&regs[2], HW_READ, 2, 3 );
	(void)mau_reg( (int *)&regs[3], HW_READ, 3, 3 );
	}
    /* since the C compiler doesn't expect r0,r1,r2 to be preserved
     * across the subroutine call, only need to save r3-r8,fp.
     */
    asm("	pushw	%r3	");
    asm("	pushw	%r4	");
    asm("	pushw	%r5	");
    asm("	pushw	%r6	");
    asm("	pushw	%r7	");
    asm("	pushw	%r8	");
    asm("	pushw	%fp	");

    (* handler)( SIGFPE );

    asm("	POPW	%fp	");
    asm("	POPW	%r8	");
    asm("	POPW	%r7	");
    asm("	POPW	%r6	");
    asm("	POPW	%r5	");
    asm("	POPW	%r4	");
    asm("	POPW	%r3	");

    if ( _fp_hw == 1 ) {
	(void)mau_reg( (int *)&regs[0], HW_WRITE, 0, 3 );
	(void)mau_reg( (int *)&regs[1], HW_WRITE, 1, 3 );
	(void)mau_reg( (int *)&regs[2], HW_WRITE, 2, 3 );
	(void)mau_reg( (int *)&regs[3], HW_WRITE, 3, 3 );
	}
    }

/* NEXTINS( pc ) -- given a PC, return the address of the beginning
 *	of the next instruction. Return the present PC if the
 *	instruction cannot be disassembled.
 * NEXTOP( pc )  -- given a pointer to the first byte of an operand
 *	descriptor, return the address of the first byte beyond
 *	the descriptor (the next descriptor or the next opcode).
 * NOTE: these routines assume a sane instruction stream.
 */

/* the following macros pick apart the opcode table definitions:
 *	1) the high nibble is the size of the opcode:
 *	    0 for invalid opcodes
 *	    1 for nearly everything
 *	    2 for macro-rom opcodes
 *	    5 for support processor opcodes
 *	2) the low nibble is the number of operands
 */
extern unsigned char op_table[];
#define	opsize(x)	(op_table[(unsigned char)(x)] >> 4)
#define	opcount(x)	(op_table[(unsigned char)(x)] & 0x0f)

 static unsigned char *
nextins( pc )
    unsigned char * pc;
    {
    extern unsigned char * nextop();

    int    i;	/* loop counter for number of operands */

    /* in the following loop, must initialize i BEFORE changing pc */

    for ( i = opcount(*pc), pc += opsize(*pc); i>0; i-- )
	pc = nextop( pc );

    return ( pc );
    }

 static unsigned char *
nextop( pc )
    unsigned char * pc;
    {
    register int	lonib = *pc & 0x0f;
    register int	hinib = *pc >> 4;

    /* two arrays giving operand descriptor size based on the high
     * nibble's value -- two, since if the low nibble is 15 (PC)
     * things change.
     */
    static char	op_size[16] = {
	1,1,1,1, /* 0-literal, 1-literal, 2-literal, 3-literal	*/
	1,1,1,1, /* 4-register,5-reg.def, 6-FP off,  7-AP off	*/
	5,5,3,3, /* 8-word dis,9-w.d.def, a-half dis,b-h.d. def */
	2,2,1,1, /* c-byte dis,d-b.d.def, e-expando, f-literal	*/
	};
    static char  pc_op_size[16] = {
	1,1,1,1, /* 0-literal, 1-literal, 2-literal, 3-literal	*/
	5,3,2,5, /* 4-word im, 5-half im, 6-byte im, 7-absolute	*/
	5,5,3,3, /* 8-word dis,9-w.d.def, a-half dis,b-h.d. def	*/
	2,2,5,1, /* c-byte dis,d-b.d.def, e-abs.def, f-literal	*/
	};

    if ( lonib == R_PC )			/* register == PC*/
	return ( pc + pc_op_size[ hinib ] );
    if ( hinib == 0xe )				/* expand-o-mode */
	return ( nextop( pc + 1 ) );
    else					/* everything else */
	return ( pc + op_size[ hinib ] );
    }

#define	OP1	0x10	/* 1 byte opcodes			  */
#define	OP2	0x20	/* 2 byte opcodes: macro-rom, NOP2, EXTOP */
#define	OP3	0x30	/* 3 byte opcodes: NOP3			  */
#define	OP5	0x50	/* 5 byte opcodes: support processor	  */
static unsigned char op_table[ 256 ] = {
    0,		0,		OP5|1,		OP5|2,	/* 0x00-0x03 */
    OP1|2,	0,		OP5|1,		OP5|2,	/* 0x04-0x07 */
    OP1|0,	0,		0,		0,	/* 0x08-0x0b */
    OP1|2,	0,		0,		0,	/* 0x0c-0x0f */
    OP1|1,	0,		0,		OP5|1,	/* 0x10-0x13 */
    OP2|0,	0,		0,		OP5|1,	/* 0x14-0x17 */
    OP1|1,	0,		0,		0,	/* 0x18-0x1b */
    OP1|1,	0,		OP1|1,		OP1|1,	/* 0x1c-0x1f */

    OP1|1,	0,		OP5|1,		OP5|2,	/* 0x20-0x23 */
    OP1|1,	0,		0,		OP1|0,	/* 0x24-0x27 */
    OP1|1,	0,		OP1|1,		OP1|1,	/* 0x28-0x2b */
    OP1|2,	0,		OP1|0,		OP1|0,	/* 0x2c-0x2f */
    OP2|0,	0,		OP5|0,		OP5|1,	/* 0x30-0x33 */
    OP1|1,	0,		OP1|1,		OP1|1,	/* 0x34-0x37 */
    OP1|2,	0,		OP1|2,		OP1|2,	/* 0x38-0x3b */
    OP1|2,	0,		OP1|2,		OP1|2,	/* 0x3c-0x3f */

    OP1|0,	0,		OP1|1,		OP1|1,	/* 0x40-0x43 */
    OP1|0,	0,		OP1|1,		OP1|1,	/* 0x44-0x47 */
    OP1|0,	0,		OP1|1,		OP1|1,	/* 0x48-0x4b */
    OP1|0,	0,		OP1|1,		OP1|1,	/* 0x4c-0x4f */
    OP1|0,	0,		OP1|1,		OP1|1,	/* 0x50-0x53 */
    OP1|0,	0,		OP1|1,		OP1|1,	/* 0x54-0x57 */
    OP1|0,	0,		OP1|1,		OP1|1,	/* 0x58-0x5b */
    OP1|0,	0,		OP1|1,		OP1|1,	/* 0x5c-0x5f */

    OP1|0,	0,		OP1|1,		OP1|1,	/* 0x60-0x63 */
    OP1|0,	0,		OP1|1,		OP1|1,	/* 0x64-0x67 */
    OP1|0,	0,		OP1|1,		OP1|1,	/* 0x68-0x6b */
    OP1|0,	0,		OP1|1,		OP1|1,	/* 0x6c-0x6f */
    OP1|0,	0,		OP3|0,		OP2|0,	/* 0x70-0x73 */
    OP1|0,	0,		OP1|1,		OP1|1,	/* 0x74-0x77 */
    OP1|0,	0,		OP1|1,		OP1|1,	/* 0x78-0x7b */
    OP1|0,	0,		OP1|1,		OP1|1,	/* 0x7c-0x7f */

    OP1|1,	0,		OP1|1,		OP1|1,	/* 0x80-0x83 */
    OP1|2,	0,		OP1|2,		OP1|2,	/* 0x84-0x87 */
    OP1|2,	0,		OP1|2,		OP1|2,	/* 0x88-0x8b */
    OP1|2,	0,		OP1|2,		OP1|2,	/* 0x8c-0x8f */
    OP1|1,	0,		OP1|1,		OP1|1,	/* 0x90-0x93 */
    OP1|1,	0,		OP1|1,		OP1|1,	/* 0x94-0x97 */
    0,		0,		0,		0,	/* 0x98-0x9b */
    OP1|2,	0,		OP1|2,		OP1|2,	/* 0x9c-0x9f */

    OP1|1,	0,		0,		0,	/* 0xa0-0xa3 */
    OP1|2,	0,		OP1|2,		OP1|2,	/* 0xa4-0xa7 */
    OP1|2,	0,		OP1|2,		OP1|2,	/* 0xa8-0xab */
    OP1|2,	0,		OP1|2,		OP1|2,	/* 0xac-0xaf */
    OP1|2,	0,		OP1|2,		OP1|2,	/* 0xb0-0xb3 */
    OP1|2,	0,		OP1|2,		OP1|2,	/* 0xb4-0xb7 */
    OP1|2,	0,		OP1|2,		OP1|2,	/* 0xb8-0xbb */
    OP1|2,	0,		OP1|2,		OP1|2,	/* 0xbc-0xbf */

    OP1|3,	0,		0,		0,	/* 0xc0-0xc3 */
    OP1|3,	0,		OP1|3,		OP1|3,	/* 0xc4-0xc7 */
    OP1|4,	0,		OP1|4,		OP1|4,	/* 0xc8-0xcb */
    OP1|4,	0,		OP1|4,		OP1|4,	/* 0xcc-0xcf */
    OP1|3,	0,		OP1|3,		OP1|3,	/* 0xd0-0xd3 */
    OP1|3,	0,		0,		0,	/* 0xd4-0xd7 */
    OP1|3,	0,		0,		0,	/* 0xd8-0xdb */
    OP1|3,	0,		OP1|3,		OP1|3,	/* 0xdc-0xdf */

    OP1|1,	0,		0,		0,	/* 0xe0-0xe3 */
    OP1|3,	0,		OP1|3,		OP1|3,	/* 0xe4-0xe7 */
    OP1|3,	0,		OP1|3,		OP1|3,	/* 0xe8-0xeb */
    OP1|3,	0,		OP1|3,		OP1|3,	/* 0xec-0xef */
    OP1|3,	0,		OP1|3,		OP1|3,	/* 0xf0-0xf3 */
    OP1|3,	0,		OP1|3,		OP1|3,	/* 0xf4-0xf7 */
    OP1|3,	0,		OP1|3,		OP1|3,	/* 0xf8-0xfb */
    OP1|3,	0,		OP1|3,		OP1|3,	/* 0xfc-0xff */
    };

/* CPU_OPCODE( r0ptr, mau_ins )
 *	disassemble the instruction starting at *(r0ptr[R_PC]),
 *	looking ONLY at those parts of the instruction stream
 *	understood by the WE32100 (i.e. ignoring the support-processor
 *	operation word), and setting:
 *	    mau_ins->op_width = data width for instruction (or 0)
 *	    mau_ins->rd_op    = address of read operand (or 0)
 *	    mau_ins->wr_op    = address of write operand (or 0)
 * RETURN:
 *	0 -- failure: not an SPOP instruction, or has an invalid
 *		operand descriptor of some kind.
 *	1 -- success, and mau_ins has been updated.
 */
 static int
cpu_opcode( r0ptr, mau_ins )
    int *			r0ptr;
    register struct mau_ins *	mau_ins;
    {
    extern int  *		cpu_operand();
    extern unsigned char *	nextop();

    unsigned char *	     pc  = (unsigned char *)(r0ptr[ R_PC ]);
    register unsigned char * op1 = pc + 5; /* address of 1st op desc.*/

    mau_ins->rd_op = (int *)0;
    mau_ins->wr_op = (int *)0;

    switch ( *pc ) {

    default:
	return ( 0 );

    case 0x32:	/* SPOP niladic			*/
	mau_ins->op_width = 0;	/* unknown */
	break;

    case 0x22:	/* SPOP Single Read		*/
	mau_ins->op_width = 1;
	mau_ins->rd_op	  = cpu_operand( op1, r0ptr );
	if ( mau_ins->rd_op == (int *)0 )
	    return ( 0 );
	break;

    case 0x02:	/* SPOP Double Read		*/
	mau_ins->op_width = 2;
	mau_ins->rd_op	  = cpu_operand( op1, r0ptr );
	if ( mau_ins->rd_op == (int *)0 )
	    return ( 0 );
	break;

    case 0x06:	/* SPOP Triple Read		*/
	mau_ins->op_width = 3;
	mau_ins->rd_op	  = cpu_operand( op1, r0ptr );
	if ( mau_ins->rd_op == (int *)0 )
	    return ( 0 );
	break;

    case 0x23:	/* SPOP Single Read/Write	*/
	mau_ins->op_width = 1;
	mau_ins->rd_op	  = cpu_operand( op1, r0ptr );
	mau_ins->wr_op	  = cpu_operand( nextop( op1 ), r0ptr );
	if ( (mau_ins->rd_op == 0)||(mau_ins->wr_op == 0) )
	    return ( 0 );
	break;

    case 0x03:	/* SPOP Double Read/Write	*/
	mau_ins->op_width = 2;
	mau_ins->rd_op	  = cpu_operand( op1, r0ptr );
	mau_ins->wr_op	  = cpu_operand( nextop( op1 ), r0ptr );
	if ( (mau_ins->rd_op == 0)||(mau_ins->wr_op == 0) )
	    return ( 0 );
	break;

    case 0x07:	/* SPOP Triple Read/Write	*/
	mau_ins->op_width = 3;
	mau_ins->rd_op	  = cpu_operand( op1, r0ptr );
	mau_ins->wr_op	  = cpu_operand( nextop( op1 ), r0ptr );
	if ( (mau_ins->rd_op == 0)||(mau_ins->wr_op == 0) )
	    return ( 0 );
	break;

    case 0x33:	/* SPOP Single Write		*/
	mau_ins->op_width = 1;
	mau_ins->wr_op	  = cpu_operand( op1, r0ptr );
	if ( mau_ins->wr_op == (int *)0 )
	    return ( 0 );
	break;

    case 0x13:	/* SPOP Double Write		*/
	mau_ins->op_width = 2;
	mau_ins->wr_op	  = cpu_operand( op1, r0ptr );
	if ( mau_ins->wr_op == (int *)0 )
	    return ( 0 );
	break;

    case 0x17:	/* SPOP Triple Write		*/
	mau_ins->op_width = 3;
	mau_ins->wr_op	  = cpu_operand( op1, r0ptr );
	if ( mau_ins->wr_op == (int *)0 )
	    return ( 0 );
	break;

	} /* end switch */
    return ( 1 );
    }

/* CPU_OPERAND( pc, r0ptr )
 *	decode the operand descriptor starting at *pc,
 *	and return the address of the operand.
 * RETURN:
 *	0 if the descriptor is not valid for an SPOP instruction
 *	0 if the address is not word aligned
 *	operand address, otherwise.
 */

/* a Macro to return 0 (FALSE) if its argument is not word aligned,
 * otherwise, it returns its argument.
 */
#define AL(x)	( (((int)(x) & 0x03) != 0) ? (int *)0 : (int *)(x) )

 static int *
cpu_operand( pc, r0ptr )
    unsigned char *	pc;    /* pointer to 1st byte of descriptor */
    int *		r0ptr; /* pointer to user's register area   */
    {
    register int	lonib = *pc & 0x0f;
    register int	hinib = *pc >> 4;
    int *		retval;
    union {
	int	w;
	char	c[4];
	}		tmp_word;
    union {
	short	s;
	char	c[2];
	}		tmp_short;

    switch ( hinib ) { /* high nibble contains address mode */

    default:	/* SHOULD NOT HAPPEN!	*/
    case 0x00:	/* literal mode		*/
    case 0x01:	/* literal mode		*/
    case 0x02:	/* literal mode		*/
    case 0x03:	/* literal mode		*/
    case 0x04:	/* register mode	*/
	/* 0x4f is word immediate but is still illegal */
    case 0x0f:	/* literal mode		*/
	return ( (int *)0 );

    case 0x05:	/* register deferred	*/
	if ( lonib == 0x0f ) /* becomes halfword immediate mode */
	    return ( (int *)0 );
	retval = (int *)(r0ptr[lonib]);
	break;

    case 0x06:	/* FP short offset	*/
	if ( lonib == 0x0f ) /* becomes byte immediate mode	*/
	    return ( (int *)0 );
	retval = (int *)(r0ptr[ R_FP ] + lonib);
	break;

    case 0x07:	/* AP short offset	*/
	if ( lonib == 0x0f ) { /* becomes absolute mode		*/
	    tmp_word.c[3] = *(pc + 1);
	    tmp_word.c[2] = *(pc + 2);
	    tmp_word.c[1] = *(pc + 3);
	    tmp_word.c[0] = *(pc + 4);
	    retval = (int *)(tmp_word.w);
	    }
	else {
	    retval = (int *)(r0ptr[ R_AP ] + lonib);
	    }
	break;

    case 0x08:	/* word displacement	*/
    case 0x09:	/* word displacement deferred */
	tmp_word.c[3] = *(pc + 1);
	tmp_word.c[2] = *(pc + 2);
	tmp_word.c[1] = *(pc + 3);
	tmp_word.c[0] = *(pc + 4);
	retval = (int *)(r0ptr[ lonib ] + tmp_word.w);
	if ( hinib == 0x09 ) {
	    if ( !AL( retval ) )
		return ( (int *)0 );
	    retval = (int *)(*retval);
	    }
	break;
    
    case 0x0a:	/* halfword displacement */
    case 0x0b:	/* halfword displacement deferred */
	tmp_short.c[1] = *(pc + 1);
	tmp_short.c[0] = *(pc + 2);
	/* since short integers are signed, we get automatic
	 * sign extension (as the WE32100 does) of tmp_short.s,
	 * when using it in an expression.
	 */
	retval = (int *)(r0ptr[ lonib ] + tmp_short.s);
	if ( hinib == 0x0b ) {
	    if ( !AL( retval ) )
		return ( (int *)0 );
	    retval = (int *)(*retval);
	    }
	break;

    case 0x0c:	/* byte displacement */
    case 0x0d:	/* byte displacement deferred */
	/* need to sign extend the next byte, since the WE32100
	 * sign-extends the byte when doing address calculations,
	 * but C (on the 3B2/5) assumes unsigned char variables
	 */
	if ( *(pc + 1) & 0x80 )
	    retval = (int *)( *(pc+1) | 0xffffff00 );
	else
	    retval = (int *)(*(pc + 1));
	retval = (int *)(r0ptr[lonib] + (int)retval);
	if ( hinib == 0x0d ) {
	    if ( !AL( retval ) )
		return ( (int *)0 );
	    retval = (int *)(*retval);
	    }
	break;

    case 0x0e:	/* expanded operand */
	if ( lonib != 0x0f ) /* expanded mode is illegal */
	    return ( (int *)0 );
	/* but if the register is the PC this is absolute deferred */
	tmp_word.c[3] = *(pc + 1);
	tmp_word.c[2] = *(pc + 2);
	tmp_word.c[1] = *(pc + 3);
	tmp_word.c[0] = *(pc + 4);
	if ( !AL(tmp_word.w) )
	    return ( (int *)0 );
	retval = (int *)(*(int *)(tmp_word.w) );
	break;
	} /* end switch */

    return ( AL( retval ) );
    }

/* MAU_OPCODE( pc, mau_ins )
 *	pick apart the mau operation word, starting at *pc,
 *	setting:
 *		mau_ins->opcode
 *		mau_ins->op[0], op[1], op[2]
 *	check the operand descriptors for agreement with the number
 *	and sizes of the CPU operand descriptors, check the opcode
 *	against the list of valid ones, validate the operand descriptors
 *	against the opcode.
 * RETURN:
 *	0 -- failure, unknown opcode, disagreement with CPU instruction
 *	1 -- success
 */
 static int
mau_opcode( pc, mau_ins )
    unsigned char *	pc;
    register struct mau_ins *	mau_ins;
    {
    extern struct mau_op mau_operand();

    int			num;		/* operand count */
    union {
	unsigned int	w;
	unsigned char	c[4];
	}		mau_word;	/* MAU operation word */

    /* re-assemble the operation word into normal byte order */
    mau_word.c[3] = *(pc + 0);
    mau_word.c[2] = *(pc + 1);
    mau_word.c[1] = *(pc + 2);
    mau_word.c[0] = *(pc + 3);

    /* pull apart the various parts of the operation word */
    if ( (mau_word.w & 0xff000000) != 0x0 ) /* processor ID */
	return ( 0 );
    /* unused:         mau_word.w & 0x00ff8000 */
    mau_ins->opcode = (mau_word.w & 0x00007c00) >> 10;
    mau_ins->op[0]  = mau_operand( (mau_word.w & 0x00000380) >> 7, 0);
    mau_ins->op[1]  = mau_operand( (mau_word.w & 0x00000070) >> 4, 0);
    mau_ins->op[2]  = mau_operand( (mau_word.w & 0x0000000f) >> 0, 1);

    /* check that the numbers of memory-based operands
     * agree with the CPU opcode, by calculating the difference
     * (# of memory operands in MAU word) - (# of memory operands in CPU
     * opcode).
     */

    /* operands 1 and 2 are always read (if used); mau_ins->rd_op is
     * non-NULL iff the CPU opcode specified a memory source.
     */
    num = 0;
    if ( mau_ins->op[0].type == IS_MEM )  num++;
    if ( mau_ins->op[1].type == IS_MEM )  num++;
    if ( mau_ins->rd_op != (int *)0 )     num--;
    if ( num != 0 ) /* disagreement between CPU and MAU */
	return ( 0 );

    /* operand 3 is always written (if used); mau_ins->wr_op is non-NULL
     * iff the CPU opcode specified a memory destination.
     */
    if ( mau_ins->op[2].type == IS_MEM )   num++;
    if ( mau_ins->wr_op != (int *)0 )      num--;
    if ( num != 0 ) /* disagreement between CPU and MAU */
	return ( 0 );

    /* check that the formats of memory-based operands
     * agree with the CPU opcode.
     */
    if ( mau_ins->rd_op != (int *)0 ) { /* i.e. there IS a memory op */
	if ( mau_ins->op[0].type == IS_MEM ) {
	    if ( mau_ins->op[0].format != mau_ins->op_width )
		return ( 0 );
	    }
	else { /* op[1] MUST be memory based */
	    if ( mau_ins->op[1].format != mau_ins->op_width )
		return ( 0 );
	    }
	}
    if ( mau_ins->wr_op != (int *)0 ) { /* there IS a memory op */
	if ( mau_ins->op[2].format != mau_ins->op_width )
	    return ( 0 );
	}

    /* check that the opcode is one we know how to handle,
     * and that the operand descriptors are sane.
     */
    switch ( mau_ins->opcode ) {

    case 0x02:	/* ADD	*** operations with two source, one dest */
    case 0x03:	/* SUB	*/
    case 0x04:	/* DIV	*/
    case 0x05:	/* REM	*/
    case 0x06:	/* MUL	*/
	if ( (mau_ins->op[0].type == IS_NULL)||
	     (mau_ins->op[1].type == IS_NULL)||
	     (mau_ins->op[2].type == IS_NULL)  )
	    return ( 0 );
	break;

    case 0x0a:	/* CMP	*** operations with two source, no dest  */
    case 0x0b:	/* CMPE	*/
    case 0x1a:	/* CMPS	*/
    case 0x1b:	/* CMPES*/
	if ( (mau_ins->op[0].type == IS_NULL)||
	     (mau_ins->op[1].type == IS_NULL)||
	     (mau_ins->op[2].type != IS_NULL)  )
	    return ( 0 );
	break;

    case 0x07:	/* MOV	*** operations with one source, one dest */
    case 0x0c:	/* ABS	*/
    case 0x0d:	/* SQRT	*/
    case 0x0e:	/* RTOI	*/
    case 0x17:	/* NEG	*/
	if ( (mau_ins->op[0].type == IS_NULL)||
	     (mau_ins->op[1].type != IS_NULL)||
	     (mau_ins->op[2].type == IS_NULL)  )
	    return ( 0 );
	break;

    case 0x0f:	/* FTOI *** conversions: one source, one dest    */
	if ( (mau_ins->op[0].type   == IS_NULL)||
	     (mau_ins->op[1].type   != IS_NULL)||
	     (mau_ins->op[2].type   != IS_MEM )||
	     (mau_ins->op[2].format != 1    )  )
	    return ( 0 );
	break;
    case 0x10:	/* ITOF	*/
	if ( (mau_ins->op[0].type   != IS_MEM )||
	     (mau_ins->op[0].format != 1      )||
	     (mau_ins->op[1].type   != IS_NULL)||
	     (mau_ins->op[2].type   == IS_NULL)  )
	    return ( 0 );
	break;
    case 0x11:	/* DTOF */
	if ( (mau_ins->op[0].type   != IS_MEM )||
	     (mau_ins->op[0].format != 3      )||
	     (mau_ins->op[1].type   != IS_NULL)||
	     (mau_ins->op[2].type   == IS_NULL)  )
	    return ( 0 );
	break;
    case 0x12:	/* FTOD */
	if ( (mau_ins->op[0].type   == IS_NULL)||
	     (mau_ins->op[1].type   != IS_NULL)||
	     (mau_ins->op[2].type   != IS_MEM )||
	     (mau_ins->op[2].format != 3      )  )
	    return ( 0 );
	break;

    case 0x08:	/* RDASR*** operations which should not fault   */
    case 0x09:	/* WRASR*/
    case 0x13:	/* NOP	*/
    case 0x14:	/* EROF	*/
    case 0x18:	/* LDR	*/
    default:	/* ???	*/
	return ( 0 );

	}	/* end switch */

    return ( 1 );
    }

/* MAU_OPERAND( descriptor, flag)
 *	translate the descriptor into a struct mau_op,
 *	assuming source if flag==0, destination otherwise.
 * WARNING: does not check range of descriptor.
 */
 static struct mau_op
mau_operand( desc, flag )
    unsigned int	desc;
    unsigned int	flag;
    {
    static struct mau_op src[8] = {
	{ IS_REG,	0,	3,	0 },
	{ IS_REG,	1,	3,	0 },
	{ IS_REG,	2,	3,	0 },
	{ IS_REG,	3,	3,	0 },
	{ IS_MEM,	0,	1,	1 },
	{ IS_MEM,	0,	2,	2 },
	{ IS_MEM,	0,	3,	3 },
	{ IS_NULL,	0,	0,	0 } };
    static struct mau_op dst[16] = {
	{ IS_REG,	0,	3,	1 },
	{ IS_REG,	1,	3,	1 },
	{ IS_REG,	2,	3,	1 },
	{ IS_REG,	3,	3,	1 },
	{ IS_REG,	0,	3,	2 },
	{ IS_REG,	1,	3,	2 },
	{ IS_REG,	2,	3,	2 },
	{ IS_REG,	3,	3,	2 },
	{ IS_REG,	0,	3,	3 },
	{ IS_REG,	1,	3,	3 },
	{ IS_REG,	2,	3,	3 },
	{ IS_REG,	3,	3,	3 },
	{ IS_MEM,	0,	1,	1 },
	{ IS_MEM,	0,	2,	2 },
	{ IS_MEM,	0,	3,	3 },
	{ IS_NULL,	0,	0,	0 } };
    
    if ( flag == 0 )
	return ( src[ desc ] );
    else
	return ( dst[ desc ] );
    }

/* the following two routines map from 'enum fp_type' to size in
 * words.  This could be done with macros, but lint complains,
 * and if ieeefp.h ever changed, the macros would break in obscure
 * fashion.
 * TYPE_TO_SIZE( type ) returns the size in words of an FP_F, FP_D,
 *	or FP_X. Otherwise, return 0.
 * SIZE_TO_TYPE( size ) returns the floating-point type for size 1,
 *	2, or 3.  Otherwise, return FP_NULL.
 */
 static int
type_to_size( type )
    fp_type	type;
    {
    switch ( type ) {
    case FP_F:
	return ( 1 );
    case FP_D:
	return ( 2 );
    case FP_X:
	return ( 3 );
	}
    return ( 0 );
    }

 static fp_type
size_to_type( size )
    int	size;
    {
    switch ( size ) {
    case 1:
	return ( FP_F );
    case 2:
	return ( FP_D );
    case 3:
	return ( FP_X );
	}
    return ( FP_NULL );
    }

/* MAU hardware access routines:
 * MAU_ASR( &buf, rw ) -- read/write ASR
 *	if ( rw == 0 ) read, else write ASR
 * NOTE: whenever the ASR is written, the routine will force
 * the <CSC> bit ON to interface properly with the kernel context-switch
 * routines.
 *
 * MAU_DR( &buf ) -- read DR, store at buffer [three words]
 *
 * MAU_REG( &buf, rw, desc, width ) -- read/write flt.pt. registers
 *	if ( rw == 0 ) read, else write
 *	MAU register specified in descriptor 'desc'
 *	in format 'width' (in [1,3])
 *	into buffer 'buf'.
 *	RETURN: 0 on argument error, 1 on success
 * NOTE: this routine guards against overflow when reading
 * a register by writing the ASR to mask all exceptions.
 *
 */

 static void
mau_asr( bufp, rw )
    int *	bufp;
    int		rw;
    {
    if ( rw == HW_READ )
	asm("	mmovfa	*0(%ap)	");
    else {
	((asr_t *)bufp)->csc = 1;
	asm("	mmovta	*0(%ap)	");
	}
    }

 static void
mau_dr( bufp )
    dr_t *	bufp;	/* address of a three-word buffer */
    {
    asm("	mmovfd	*0(%ap)	");
    }

 static int
mau_reg( bufp, rw, reg, width )
    int *	bufp;	/* address of a 'width'-word buffer	*/
    int		rw;	/* == 0 to read register		*/
    int		reg;	/* register number in set [0,3]		*/
    int		width;	/* buffer width in set [1,3]		*/
    {
    asr_t	o_asr, n_asr;

    /* argument sanity checking */
    if ( (reg<0) || (reg>3) || (width<1) || (width>3) )
	 return ( 0 );
    if ( rw != HW_READ )
	rw = HW_WRITE;

    /* before reading the register, mask all exceptions		*/
    mau_asr( (int *)&(o_asr), HW_READ );
    n_asr = o_asr;
    n_asr.im = 0;
    n_asr.om = 0;
    n_asr.um = 0;
    n_asr.qm = 0;
    n_asr.pm = 0;
    mau_asr( (int *)&(n_asr), HW_WRITE );

    /* pack the three arguments into a single integer:
     *	reg is [0,3] and occupies bits [1,0],
     *	rw  is [0,1] and occupies bit  [2],
     *	(width-1) is [0,2] and occupies bits [4,3].
     */
    switch ( ((width-1)<<3) + (rw<<2) + reg ) {

    case 000:	/* 00 0 00 -- single read %f0	*/
	asm("	mmovxs	%f0,*0(%ap)	"); break;

    case 001:	/* 00 0 01 -- single read %f1	*/
	asm("	mmovxs	%f1,*0(%ap)	"); break;

    case 002:	/* 00 0 10 -- single read %f2	*/
	asm("	mmovxs	%f2,*0(%ap)	"); break;

    case 003:	/* 00 0 11 -- single read %f3	*/
	asm("	mmovxs	%f3,*0(%ap)	"); break;

    case 004:	/* 00 1 00 -- single write %f0	*/
	asm("	mmovsx	*0(%ap),%f0	"); break;

    case 005:	/* 00 1 01 -- single write %f1	*/
	asm("	mmovsx	*0(%ap),%f1	"); break;

    case 006:	/* 00 1 10 -- single write %f2	*/
	asm("	mmovsx	*0(%ap),%f2	"); break;

    case 007:	/* 00 1 11 -- single write %f3	*/
	asm("	mmovsx	*0(%ap),%f3	"); break;

    case 010:	/* 01 0 00 -- double read %f0	*/
	asm("	mmovxd	%f0,*0(%ap)	"); break;

    case 011:	/* 01 0 01 -- double read %f1	*/
	asm("	mmovxd	%f1,*0(%ap)	"); break;

    case 012:	/* 01 0 10 -- double read %f2	*/
	asm("	mmovxd	%f2,*0(%ap)	"); break;

    case 013:	/* 01 0 11 -- double read %f3	*/
	asm("	mmovxd	%f3,*0(%ap)	"); break;

    case 014:	/* 01 1 00 -- double write %f0	*/
	asm("	mmovdx	*0(%ap),%f0	"); break;

    case 015:	/* 01 1 01 -- double write %f1	*/
	asm("	mmovdx	*0(%ap),%f1	"); break;

    case 016:	/* 01 1 10 -- double write %f2	*/
	asm("	mmovdx	*0(%ap),%f2	"); break;

    case 017:	/* 01 1 11 -- double write %f3	*/
	asm("	mmovdx	*0(%ap),%f3	"); break;

    case 020:	/* 10 0 00 -- extended read %f0	*/
	asm("	mmovxx	%f0,*0(%ap)	"); break;

    case 021:	/* 10 0 01 -- extended read %f1	*/
	asm("	mmovxx	%f1,*0(%ap)	"); break;

    case 022:	/* 10 0 10 -- extended read %f2	*/
	asm("	mmovxx	%f2,*0(%ap)	"); break;

    case 023:	/* 10 0 11 -- extended read %f3	*/
	asm("	mmovxx	%f3,*0(%ap)	"); break;

    case 024:	/* 10 1 00 -- extended write %f0*/
	asm("	mmovxx	*0(%ap),%f0	"); break;

    case 025:	/* 10 1 01 -- extended write %f1*/
	asm("	mmovxx	*0(%ap),%f1	"); break;

    case 026:	/* 10 1 10 -- extended write %f2*/
	asm("	mmovxx	*0(%ap),%f2	"); break;

    case 027:	/* 10 1 11 -- extended write %f3*/
	asm("	mmovxx	*0(%ap),%f3	"); break;

	}	/* end switch */

    mau_asr( (int *)&(o_asr), HW_WRITE );

    return ( 1 );
    }

/* MAU_EXCEPT() -- MAU ASR interpreter -- return the primary fault
 * which caused the exception.  If the fault type can not be
 * determined, return UNKNOWN.
 *
 * This code does not check for the existance of multiple unmasked
 * exceptions.
 *
 * WARNING WARNING WARNING WARNING WARNING *******************
 * When "shadow sticky" bits are implemented, this routine will
 * have to change to clear/copy the sticky bits.
 * WARNING WARNING WARNING WARNING WARNING *******************
 */
 static fp_ftype
mau_except()
    {
    extern void	mau_asr();
    asr_t	temp;

    mau_asr( (int *)&temp, HW_READ );

    /*	 MASK BITS	 STICKY BITS	*/
    if ( (temp.im == 1) && (temp.is == 1) )
	return( FP_INVLD );

    if ( (temp.qm == 1) && (temp.qs == 1) )
	return ( FP_DIVZ );

    if ( (temp.om == 1) && (temp.os == 1) )
	return ( FP_OVFLW );

    if ( (temp.um == 1) && (temp.us == 1) )
	return ( FP_UFLW );

    /* the four tests above can be in any order, assuming that
     * the user has been faithfully clearing the sticky bits
     * on previous exceptions.  We must test for "imprecise" last,
     * since it can occur along with overflow and underflow,
     * and the IEEE standard says overflow and underflow take priority
     * over loss of precision.
     */

     if ( (temp.pm == 1) && (temp.ps == 1) )
	return ( FP_INXACT );

    /* if we get this far something is wrong -- no unmasked exception
     * exists.  Return "UNKNOWN" to the caller.
     */
    return ( UNKNOWN );
    }

/* REFORMAT( buf, s_width, d_width ) -- reformat a floating-point
 *	number in buffer 'buf' from width 's_width' to width 'd_width'.
 *
 * Rather than do a lot of bit-field manipulations, this routine
 * uses the MAU hardware to reformat the number. By changing the ASR
 * to mask off all exceptions, we avoid any possible faults.
 *
 * WARNING: uses (and restores) register %f3;
 *	does not validate s_width or d_width.
 */
 static void
reformat( buf, s_width, d_width )
    int *	buf;
    int 	s_width;
    int		d_width;
    {
    extern void	mau_asr();
    extern int	mau_reg();

    extended	temp;
    asr_t	o_asr, n_asr;

    /* save %f3 before using it */
    (void)mau_reg( (int *)&temp, HW_READ, 3, 3 );

    /* save the ASR before writing one with all exceptions masked */
    mau_asr( (int *)&o_asr, HW_READ );
    n_asr = o_asr;
    n_asr.im = 0;
    n_asr.om = 0;
    n_asr.um = 0;
    n_asr.qm = 0;
    n_asr.pm = 0;
    mau_asr( &n_asr, HW_WRITE );

    /* run the buffer through %f3 */
    (void)mau_reg( buf, HW_WRITE, 3, s_width );
    (void)mau_reg( buf, HW_READ,  3, d_width );

    /* restore %f3 */
    (void)mau_reg( (int *)&temp, HW_WRITE, 3, 3 );

    /* restore the ASR */
    mau_asr( (int *)&o_asr, HW_WRITE );
    }

/* SPOP_FETCH( fpdp, insp, num ) -- fetch the operand described by
 *	insp->op[num] into fpdp->value.  Set fpdp->type accordingly.
 *	(The type will be incorrect for non floating-point types,
 *	but the bits in fpdp->value will be correct.)
 *
 * NOTE: since registers ALWAYS have format == 3, we only check
 * op[].type for that format.
 */
 static void
spop_fetch( fpdp, insp, num )
    fp_dunion *		fpdp;	/* place to store the operand    */
    struct mau_ins *	insp;	/* disassembled instruction data */
    unsigned int	num;	/* operand number [0,1]          */
    {
    extern int		mau_reg();

    int *	 faddr = insp->rd_op;		/* source address     */
    int *	 taddr = (int *)&(fpdp->value);	/* destination address*/

    switch ( insp->op[num].format ) {

    default:	/* SHOULD NOT HAPPEN */
    case 0:	/* a no-op descriptor, i.e., no operand	*/
	fpdp->type = FP_NULL;
	return;

    case 1:	/* single precision */
	fpdp->type = FP_F;
	* taddr = * faddr;
	return;

    case 2:	/* double precision */
	fpdp->type = FP_D;
	* taddr++ = * faddr++;
	* taddr   = * faddr;
	return;

    case 3:	/* double extended precision */
	fpdp->type = FP_X;
	if ( insp->op[num].type == IS_MEM ) {
	    * taddr++ = * faddr++;
	    * taddr++ = * faddr++;
	    * taddr   = * faddr;
	    }
	else {
	    (void)mau_reg( taddr, HW_READ, insp->op[num].regno, 3 );
	    }
	return;
	} /* end switch */
    }

/*********************************
 * WARNING WARNING WARNING WARNING
 *
 * THE FOLLOWING FOUR ROUTINES MAY NEED TO BE RECODED FOR THE in-line
 * CCS. (All the code above this warning should be CCS independent)
 *
 * WARNING WARNING WARNING WARNING
 *********************************/

/* FORMAT_DATA( faultp, insp, drp, ftype ) -- use the instruction stream
 *	information collected in insp to fill in the IEEE fault struct.
 *
 * WARNING: This function makes assumptions about the CCS:
 *	no mixed-mode math (except conversions)
 */
 static int
format_data( faultp, insp, drp, ftype, r0ptr )
    struct fp_fault *	faultp;
    struct mau_ins  *	insp;
    dr_t *		drp;
    fp_ftype		ftype;
    int *		r0ptr;
    {
    extern void		dr_format();
    extern void		reformat();
    extern fp_type	size_to_type();
    extern void		spop_fetch();
    extern int		type_to_size();

    /* This array gives the mapping from MAU opcode to fp_op.
     * Opcodes that should not get to format_data() are 'FP_ADD',
     * since they will have been caught by mau_opcode().
     */
    static fp_op	op_map[32] = {
	FP_ADD,  FP_ADD,  FP_ADD,  FP_SUB,		/*  0- 3 */
	FP_DIV,  FP_REM,  FP_MULT, FP_CONV,		/*  4- 7 */
	FP_ADD,  FP_ADD,  FP_CMP,  FP_CMPT,		/*  8- b */
	FP_ABS,  FP_SQRT, FP_RNDI, FP_CONV,		/*  c- f */
	FP_CONV, FP_CONV, FP_CONV, FP_ADD,		/* 10-13 */
	FP_ADD,  FP_ADD,  FP_ADD,  FP_NEG,		/* 14-17 */
	FP_ADD,  FP_ADD,  FP_CMP,  FP_CMPT,		/* 18-1b */
	FP_ADD,  FP_ADD,  FP_ADD,  FP_ADD,		/* 1c-1f */
	};

    /* since spop_fetch() handles no-op cases sanely, we can
     * always fetch operands 0 and 1. We may have to change the
     * type of operand 0 for conversions.
     */
    spop_fetch( &(faultp->operand[0]), insp, 0 );
    spop_fetch( &(faultp->operand[1]), insp, 1 );

    /* since we have already validated the op_code (in mau_opcode()),
     * we can just map it to the fp_op.
     */
    faultp->operation = op_map[ insp->opcode ];

    /* assume the result is floating-point and set result.type.
     * we may change it later for some conversions.
     *
     * NB: if result is in memory, op_width == rounding == format,
     * if result is in register, we want rounding precision, NOT format
     */
     faultp->result.type =  size_to_type( insp->op[2].rounding );

    switch ( insp->opcode ) {

    case 0x0a:	/* COMPARISONS	*/
    case 0x1a:
    case 0x0b:
    case 0x1b:
	faultp->result.type = FP_C;
	/* The same arguments about mixed-mode operations apply
	 * as in the dyadic arithmetic operations.  Fall through
	 * to that code.
	 */

    case 0x02:	/* DYADIC ARITHMETIC OPERATIONS */
    case 0x03:
    case 0x04:
    case 0x05:
    case 0x06:
	/* Because the SPOP operations do not have a general dyadic
	 * format, one of the arguments will have had to be moved
	 * into a register.  Since we "know" that no one does mixed-
	 * precision math, if the two operands are not the same size,
	 * we should shorten the wider form.
	 * Because the instruction stream has been checked, we know that
	 * all operands coming through here are true floating-point.
	 */
	if ( faultp->operand[0].type != faultp->operand[1].type ) {
	    if ( (int)(faultp->operand[0].type) >
		 (int)(faultp->operand[1].type) ) {
		/* operand[0] is wider */
		reformat( (int *)&(faultp->operand[0].value),
		    type_to_size(faultp->operand[0].type),
		    type_to_size(faultp->operand[1].type)
		    );
		faultp->operand[0].type = faultp->operand[1].type;
		}
	    else {
		/* operand[1] is wider */
		reformat( (int *)&(faultp->operand[1].value),
		    type_to_size(faultp->operand[1].type),
		    type_to_size(faultp->operand[0].type)
		    );
		faultp->operand[1].type = faultp->operand[0].type;
		}
	    }
	/* The MAU has the interesting feature that for some arithmetic
	 * operations the first operand is Op2 and the second is Op1.
	 * This code swaps the operands for these cases.
	 */
	if ( (faultp->operation == FP_SUB)||
	     (faultp->operation == FP_DIV)||
	     (faultp->operation == FP_REM) ) {
 	    fp_dunion	tmp;
	    tmp		       = faultp->operand[0];
	    faultp->operand[0] = faultp->operand[1];
	    faultp->operand[1] = tmp;
	    }
	break;

    case 0x0c:	/* MONADIC ARITHMETIC OPERATIONS */
    case 0x0d:
    case 0x0e:
    case 0x17:
	/* For the function call CCS, there is no need for the source
	 * to be in a register, since we can do a memory->memory op.
	 * Later on, there may even be cases where we go reg->reg,
	 * and we'll have to decide about them then.
	 */
	break;

    case 0x0f:	/* CONVERSIONS: floating to/from something else	*/
	/* In all these conversions, if the number is changing size,
	 * it will have to have been moved into a register first.
	 * We have *NO* information about its original type,
	 * so it will show up as "double-extended".
	 */
	faultp->result.type = FP_L;
	break;
    case 0x10:
	faultp->operand[0].type = FP_L;
	break;
    case 0x11:
	faultp->operand[0].type = FP_P;
	break;
    case 0x12:
	faultp->result.type = FP_P;
	break;

    case 0x07:	/* CONVERSION: floating to floating */
	break;
	}

    /* whether we put any information in faultp->t_value depends on
     * what kind of exception we are handling, and what kind of
     * operation was underway. The contents of the DR also depend
     * on both exception and operation. The following tables summarize
     * this:
     *
     * The following table defines the contents of the DR
     * for each operation type/fault type pair.
     *	** -- means that combination cannot occur
     *	UU -- means the DR value is not defined
     *	dX -- means the DR contains the result in double-extended
     *		with 17-bit exponent
     *	dXr -- means the DR contains the rounded result " "
     *		with 17-bit exponent
     *
     *			OVFLW	UFLW	IMP	INV	DIVZ
     * Arithmetic	dXr	dXr	dXr	dX	dX
     *
     * Comparisons	**	**	**	dX	**
     *
     * Flt->flt conv.	dXr 	dXr	dXr	dX	**
     *
     * Flt->dec conv.	**	**	UU	dX	**
     *
     * Flt->int conv.	**	**	UU	dX	**
     *
     * Dec->flt conv.	**	**	dXr	dX	**
     *
     * Int->flt conv.	**	**	dXr	**	**
     *
     * The following table defines what the Family Standard wants
     * to have for the trapped result for each operation/fault pair.
     *	**  -- means no trapped result is required
     *	df  -- means the trapped result should be in destination format
     *	dfb -- means the trapped result should be in destination format
     *		with the exponent biased
     *	dxr -- means the trapped result should be in double-extended "
     *		but rounded to the destination's precision.  For dec
     *		and int destinations this means "rounded to integer".
     *
     *			OVFLW	UFLW	IMP	INV	DIVZ
     * Arithmetic	dfb	dfb	df	**	**
     *
     * Comparisons	**	**	**	**	**
     *
     * Flt->flt conv.	dxr	dxr	dxr	**	**
     *
     * Flt->dec conv.	**	**	dxr	**	**
     *
     * Flt->int conv.	**	**	dxr	**	**
     *
     * Dec->flt conv.	**	**	dxr	**	**
     *
     * Int->flt conv.	**	**	dxr	**	**
     */

    /* since localfault is zeroed in _getflthw(), unless we do something
     * to it, t_value.type == FP_NULL.
     */
    switch ( ftype ) {

    default:		/* should not happen! */
    case INT_DIVZ:
    case INT_OVFLW:
	return ( 0 );

    case FP_DIVZ:	/* can happen, but no more information */
    case FP_INVLD:
	break;

    case FP_OVFLW:	/* may need to reformat the DR */
    case FP_UFLW:
    case FP_INXACT:
	if ( faultp->operation == FP_CMP ) {
	    /* no additional information on comparisons */
	    break;
	    }

	/* before calling dr_format(), check, just in case,
	 * to see if we have one of the exceptions that
	 * "cannot happen".  If so, we have a hardware failure,
	 * or a serious misunderstanding of the requirements.
	 * dr_format() is not set up to cope with these.
	 */
	if ( ((ftype == FP_OVFLW)||(ftype == FP_UFLW)) &&
	     ((insp->opcode == 0x10) ||	/* ITOF */
	      (insp->opcode == 0x0f) ||	/* FTOI	*/
	      (insp->opcode == 0x11) ||	/* DTOF	*/
	      (insp->opcode == 0x12) ))	/* FTOD	*/
	    return ( 0 );

	/* reformat the DR and put into faultp->t_value */
	dr_format( drp, ftype, insp, faultp, r0ptr );

	break;
	}
    return ( 1 );
    }

/* SPOP_STORE( faultp, insp, r0ptr )
 *	store the result in faultp->result wherever it belongs
 * ASSUMPTIONS:
 *	1) the instruction stream sanity has been checked, so that
 *		we do know where to store the result, we know what
 *		size the result has, and we will not be trying to store
 *		a non-floating result into a MAU register.
 *	2) this code only has to deal with code from the function-call
 *		CCS, so the condition-code bits only need to be updated
 *		after a comparison.
 * With the in-line CCS, the ASR and PSW will need to be updated
 * after every operation.
 */
 static void
spop_store( faultp, insp, r0ptr )
    struct fp_fault *	faultp;
    struct mau_ins  *	insp;
    int *		r0ptr;
    {
    extern void	mau_asr();	/* read/write ASR register */
    extern int	mau_reg();	/* read/write flt.pt. regs */

    switch ( faultp->result.type ) {

    default:		/* should not happen! */
    case FP_NULL:	/* should not happen! */
	break;

    case FP_C:		/* compare */
	/* check for the MAU "compare with flags swapped" opcodes */
	if ( (insp->opcode == 0x1a)||(insp->opcode == 0x1b) ) {

	    if ( faultp->result.value.c == FP_LT )
		faultp->result.value.c = FP_EQ;

	    else if ( faultp->result.value.c == FP_EQ )
		faultp->result.value.c = FP_LT;

	    /* "flags swapped" leaves FP_GT and FP_UO the same */
	    }

	/* read the ASR and psw, mask off the condition code bits,
	 * switch on result.value.c to know what to OR into ASR and psw,
	 * write the ASR and psw back again
	 */
	    {
	    asr_t	t_asr;
	    psw_t	t_psw;

	    mau_asr( (int *)&t_asr, HW_READ );
	    t_psw = *(psw_t *)&r0ptr[ R_PSW ];

	    t_psw.NZVC= 0;		/* mask off NZVC bits	*/
	    t_asr.n   = 0;		/* mask "", plus pr	*/
	    t_asr.z   = 0;
	    t_asr.io  = 0;
	    t_asr.ps  = 0;
	    t_asr.pr  = 0;

	    switch ( faultp->result.value.c ) {
	    default:
		/* the user must have stored a bogus value! */
		break;
	    case FP_LT:
		t_asr.n = 1;		/* set N bit */
		t_psw.NZVC |= PS_N;
		break;
	    case FP_EQ:
		t_asr.z = 1;		/* set Z bit */
		t_psw.NZVC |= PS_Z;
		break;
	    case FP_GT:
		break;	/* leave all condition codes == 0 */
	    case FP_UO:
		t_asr.uo = 1;		/* set unorderd (not in PSW) */
		break;
		}
	    
	    mau_asr( (int *)&t_asr, HW_WRITE );
	    r0ptr[ R_PSW ] = *(int *)&t_psw;
	    }
	break;

    case FP_F:		/* single precision */
	if ( insp->wr_op == (int *)0 ) { /* destination not memory */
	    (void)mau_reg(
		(int *)&(faultp->result.value.f),
		HW_WRITE,
		insp->op[2].regno,
		1			/* size == one word	     */
		);
	    break;
	    }
	/* otherwise, fall through to store one word in memory */

    case FP_L:		/* long integer		*/
    case FP_U:		/* unsigned long integer*/
	(insp->wr_op)[0] = faultp->result.value.l;
	break;

    case FP_D:	/* double precision (may be in register) */
	if ( insp->wr_op == (int *)0 ) { /* is in register */
	    (void)mau_reg(
		(int *)&(faultp->result.value.d),
		HW_WRITE,
		insp->op[2].regno,
		2
		);
	    }
	else {
	    (insp->wr_op)[0] = ((int *)&(faultp->result.value.d))[0];
	    (insp->wr_op)[1] = ((int *)&(faultp->result.value.d))[1];
	    }
	break;

    case FP_X:	/* extended precision (may be in register) */
	if ( insp->wr_op == (int *)0 ) {
	    (void)mau_reg(
		(int *)&(faultp->result.value.x),
		HW_WRITE,
		insp->op[2].regno,
		3
		);
	    break;
	    }
	/* otherwise, fall through to store three words in memory */

    case FP_P:	/* packed decimal, must be in memory */
	(insp->wr_op)[0] = ((int *)&(faultp->result.value.p))[0];
	(insp->wr_op)[1] = ((int *)&(faultp->result.value.p))[1];
	(insp->wr_op)[2] = ((int *)&(faultp->result.value.p))[2];
	break;

	}	/* end switch */
    }

/* DR_FORMAT( drp, ftype, insp, faultp, r0ptr )
 *	using (where appropriate) the DR register's value,
 *	generate the correct trapped result and assign it to t_value.
 * There are four cases to consider:
 * 1) over/under flow on an arithmetic operation, where the DR exponent
 *	must be biased according to the destination's precision,
 *	with a quiet NaN generated if after biasing the number is
 *	still out of range. The number must then be reformatted to
 *	the destination's precision.
 * 2) imprecise on an arithmetic operation, where the DR exponent
 *	must be reformatted to 15 bits, but not biased. The number
 *	must then be reformatted to the destination's precision.
 * 3) any exception on a conversion to floating-point, where the DR
 *	exponent must be reformatted.
 * 4) an imprecise exception on a conversion to int or dec, where
 *	the DR is undefined.  The source operand must be rounded
 *	to an integer, and left in double-extended precision.
 */
 static void
dr_format( drp, ftype, insp, faultp, r0ptr )
    dr_t *		drp;
    fp_ftype		ftype;
    struct mau_ins *	insp;
    struct fp_fault *	faultp;
    int *		r0ptr;
    {
    extern void	mau_asr();
    extern void	qnan_gen();
    extern void	reformat();
    extern int	type_to_size();

    /* WARNING: the following definition is used for assembly-level
     * access and MUST be the first local declaration.
     */
    extended *	ptr;

    int		exp;		/* sign-extended exponent in DR	*/
    asr_t	n_asr, o_asr;	/* ASR buffers for case 4	*/
    int		sign;		/* sign bit extracted in DR	*/
    int		tindex;		/* type index [1,3]		*/

    /* The following array defines the possible exponent ranges
     * for the three precisions, WHEN EXPRESSED IN DR FORMAT.
     * Exponents are unsigned integers in the range [0, 2**n - 1],
     * each carries a bias of (2**(n-1) -1), but is offset by
     * the double-extended bias (2**(15-1) - 1) == (16K -1).
     */
#define	K	(1024)
    static struct expval {
	int	minexp;
	int	maxexp;
	int	bias;
	} expval[4] = {
	    { 0,		0,		0 }, /* ignore [0]   */
	    { 0 +              (16*K - 1) - (128 - 1),
		(256 - 2) +    (16*K - 1) - (128 - 1),
		3*(1<<6) },				/* single */
	    { 0 +              (16*K - 1) - (1*K - 1),
		(2*K - 2) +    (16*K - 1) - (1*K - 1),
		3*(1<<9) },				/* double */
	    { 0 +              (16*K - 1) - (16*K - 1),
		(32*K - 2) +   (16*K - 1) - (16*K - 1),
		3*(1<<13)}				/* d-x    */
	    };
#define INFEXP	(32*K - 1)	/* exponent for infinity */

    /* pick out sign and exponent from DR, extending exponent	*/
    sign = drp->word0.preword0.sign;
    exp  = drp->word0.preword0.exp;
    if ( exp & 0x00010000 )
	exp |= 0xfffe0000;

    /* CASES 1 and 2: ARITHMETIC OPERATIONS *********************/
    if ( faultp->operation != FP_CONV ) {

	/* map the result type to an index into expval[] array	*/
	tindex = type_to_size( faultp->result.type );

	switch ( ftype ) {
	default:	/* should not happen!			*/
	    return;
	case FP_OVFLW:
	    if ( exp <= expval[tindex].maxexp )
		/* OVERFLOW with a small exponent?	*/
		return;

	    if ( exp == INFEXP ) {
		/* this may be either a NaN, an infinity, or
		 * an overflowed number.  We should not be getting
		 * a NaN on a arithmetic overflow trap, and
		 * we can detect an infinity by checking for
		 * the bottom two words being 0.
		 */
		if ( (drp->word1 == 0) && ( drp->word2 == 0) ) {
		    /* AHA! it IS an infinity, so move the
		     * sign bit over to make a double-extended.
		     */
		    drp->word0.postword0.sign = sign;
		    break;
		    }
		}

	    exp -= expval[tindex].bias;

	    if ( exp > expval[tindex].maxexp )	/* STILL out of range*/
		qnan_gen( drp, r0ptr );
	    else {
		drp->word0.postword0.sign = sign;
		drp->word0.postword0.exp  = exp & 0x7fff;
		}
	    break;

	case FP_UFLW:
	    if ( exp > expval[tindex].minexp )
		/* UNDERFLOW with a large exponent?	*/
		return;

	    exp += expval[tindex].bias;

	    if ( exp < expval[tindex].minexp ) /* STILL out of range*/
		qnan_gen( drp, r0ptr );
	    else {
		drp->word0.postword0.sign = sign;
		drp->word0.postword0.exp  = exp & 0x7fff;
		}
	    break;

	/* CASE 2: imprecise, no biasing will be done *********/
	case FP_INXACT:
	    /* since we are not biasing, the exponent must be
	     * either in range or an infinity.
	     */
	    if ( (exp < expval[tindex].minexp)||
		 ( (exp > expval[tindex].maxexp)&&
		   (exp != INFEXP) ) )
		/* must have had a masked overflow or underflow
		 * in addition to the loss of precision.
		 */
		qnan_gen( drp, r0ptr );
	    else {
		drp->word0.postword0.sign = sign;
		drp->word0.postword0.exp = exp & 0x7fff;
		}
	    break;
	    } /* end switch ( ftype ) */

	/* the trapped value is the same size as the desired result */
	faultp->t_value.type    = faultp->result.type;
	faultp->t_value.value.x = *(extended *)drp;

	/* if the result is not "double extended", reformat the DR
	 * value (now in t_value.value) to the right width.
	 */
	if ( tindex != 3 )
	    reformat( &(faultp->t_value.value), 3, tindex );
	return;
	}

    /* CASE 3: EXCEPTION ON CONVERSION TO FLOATING FORMAT *****/
    if ( (insp->opcode != 0x0f) && (insp->opcode != 0x12) ) {
	/* we want to leave the t_value in double-extended, so
	 * just move the sign bit over.
	 */
	drp->word0.postword0.sign = sign;

	faultp->t_value.type = FP_X;
	faultp->t_value.value.x = *(extended *)drp;
	return;
	}

    /* CASE 4: (by elimination) CONVERSION TO int OR dec *********
     * the DR value is undefined, so start with the input operand,
     * reformat it to double-extended, and then round it to integer.
     */
    faultp->t_value.value = faultp->operand[0].value;

    /* find the size of the input operand */
    tindex = type_to_size( faultp->operand[0].type );

    if ( tindex != 3 )
	reformat( &(faultp->t_value.value), tindex, 3 );

    /* round to integer, but first, mask off all faults! */
    mau_asr( (int *)&o_asr, HW_READ );
    n_asr     = o_asr;
    n_asr.im  = 0;
    n_asr.om  = 0;
    n_asr.um  = 0;
    n_asr.qm  = 0;
    n_asr.pm  = 0;
    mau_asr( (int *)&n_asr, HW_WRITE );

    /* set the pointer 0(%fp) for assembly access */
    ptr = &(faultp->t_value.value.x);

    /* round to integer, in place */
    asm("	mfrndx1	*0(%fp)		");

    /* restore the original ASR */
    mau_asr( (int *)&o_asr, HW_WRITE );

    /* SUCCESS! change the type field from FP_NULL to FP_X */
    faultp->t_value.type = FP_X;
    }

/* QNAN_GEN( drp, r0ptr )
 *	generate a double-extended-precision quiet NaN
 *
 * WARNING: for this release, all quiet NaNs become the default
 * hardware quiet NaN, so r0ptr is unused.
 */
 static void
qnan_gen( drp, r0ptr )
    dr_t *	drp;
    int *	r0ptr;
    {
    drp->word0.postword0.sign = 0;
    drp->word0.postword0.exp  = 0x7fff;

    drp->word1 = 0xffffffff;
    drp->word2 = 0xffffffff;
    }

/* Deo sola gloria */
