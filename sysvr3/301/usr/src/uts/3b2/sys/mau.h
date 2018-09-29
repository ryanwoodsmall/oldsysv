/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/mau.h	10.8"
/*
 *	WE 32106 MAU
 */

/*
 * MAU ASR (Auxiliary Status Register)
 */

typedef union {
	int		word;
	struct {
		unsigned ra	:1;	/* 31:result available		*/
		unsigned	:5;	/* <unused>			*/
		unsigned ecp	:1;	/* 25:exception present		*/
		unsigned ntnc	:1;	/* 24:non-trapping NAN cntl	*/
		unsigned rc	:2;	/* 22-23:rounding control	*/
		unsigned n	:1;	/* 21:negative result(indicator)*/
		unsigned z	:1;	/* 20:zero result    (indicator)*/
		unsigned io	:1;	/* 19:integer overflw(indicator)*/
		unsigned pss	:1;	/* 18:inexact	     (sticky)	*/
		unsigned csc	:1;	/* 17:context switch cntl	*/
		unsigned uo	:1;	/* 16:unordered	     (indicator)*/
		unsigned	:1;	/* <unused>			*/
		unsigned im	:1;	/* 14:invalid op     (mask)	*/
		unsigned om	:1;	/* 13:overflow	     (mask)	*/
		unsigned um	:1;	/* 12:underflow	     (mask)	*/
		unsigned qm	:1;	/* 11:zerodivide     (mask)	*/
		unsigned pm	:1;	/* 10:inexact	     (mask)	*/
		unsigned is	:1;	/* 09:invalid op     (sticky)	*/
		unsigned os	:1;	/* 08:overflow	     (sticky)	*/
		unsigned us	:1;	/* 07:underflow	     (sticky)	*/
		unsigned qs	:1;	/* 06:zerodivide     (sticky)	*/
		unsigned pr	:1;	/* 05:partial rem.   (indicator)*/
		unsigned	:5;	/* <unused>			*/
	} bits;
} asr_t;

/*
 * Misc. MAU defines
 */
#define	MAU_ID	0		/* support processor id for MAU */
#define	JBIT	0x80000000	/* J-bit in double extended format */
#define	MAU_SPECIAL	2	/* denotes opcode that requires
				 * special page fault handling */
#define	SPOPD2	0x03		/* WE32100 SPOPD2 opcode */
#define	SPOPT2	0x07		/* WE32100 SPOPT2 opcode */
				/* the following are passed as the first
				 * arg to mau_pfault */
#define	MAU_NOPROBE	0	/* disassemble inst and complete op */
#define	MAU_PROBESF	1	/* probe inst on stack fault */
#define	MAU_PROBESB	2	/* probe inst on stack bounds fault */
#define MAU_WRONLY	3	/* write result only */

/*
 * Round Control (RC) values
 */

#define MRC_RN	0	/* Round to Nearest */
#define MRC_RP	1	/* Round towards Plus infinity */
#define MRC_RM	2	/* Round towards Minus infinity */
#define MRC_RZ	3	/* Round towards Zero (trunctation) */

/*
 *  MAU CR (Command Register)
 */

typedef union {
	int	word;			/* for easy manipulation */
	struct {
		unsigned id	:8;	/* MAU processor id == 0 */
		unsigned	:9;	/* <unused>		 */
		unsigned opcode	:5;	/* MAU operation	 */
		unsigned op1	:3;	/* operand 1		 */
		unsigned op2	:3;	/* operand 2		 */
		unsigned op3	:4;	/* operand 3 (result)	 */
	} mau;
    } cr_t;


/*
 * Source Operand specifiers
 */

#define MSO_REG0	0	/* MAU register F0 */
#define MSO_REG1	1	/* MAU register F1 */
#define MSO_REG2	2	/* MAU register F2 */
#define MSO_REG3	3	/* MAU register F3 */
#define MSO_MEM1	4	/* One word memory */
#define MSO_MEM2	5	/* Two word memory */
#define MSO_MEM3	6	/* Three word memory */
#define MSO_NONE	7	/* No operand */

/*
 * Destination Operand specifiers
 */

#define MDO_SR0		0	/* MAU register F0 (Single-precision) */
#define MDO_SR1		1	/* MAU register F1 (Single-precision) */
#define MDO_SR2		2	/* MAU register F2 (Single-precision) */
#define MDO_SR3		3	/* MAU register F3 (Single-precision) */
#define MDO_DR0		4	/* MAU register F0 (Double-precision) */
#define MDO_DR1		5	/* MAU register F1 (Double-precision) */
#define MDO_DR2		6	/* MAU register F2 (Double-precision) */
#define MDO_DR3		7	/* MAU register F3 (Double-precision) */
#define MDO_TR0		8	/* MAU register F0 (Triple-precision) */
#define MDO_TR1		9	/* MAU register F1 (Triple-precision) */
#define MDO_TR2		0xa	/* MAU register F2 (Triple-precision) */
#define MDO_TR3		0xb	/* MAU register F3 (Triple-precision) */
#define MDO_SMEM	0xc	/* Memory-based (Single-precision) */
#define MDO_DMEM	0xd	/* Memory-based (Double-precision) */
#define MDO_TMEM	0xe	/* Memory-based (Triple-precision) */
#define MDO_NONE	0xf	/* No Operand */

/*
 * ASR bit field defines.
 */

#define ASR_RA		0x80000000
#define ASR_ECP		0x2000000
#define ASR_NTNC	0x1000000
#define ASR_RC		0xC00000
#define ASR_N		0x200000
#define ASR_Z		0x100000
#define ASR_IO		0x80000
#define ASR_PSS		0x40000
#define ASR_CSC		0x20000
#define ASR_UO		0x10000
#define ASR_IM		0x4000
#define ASR_OM		0x2000
#define ASR_UM		0x1000
#define ASR_QM		0x800
#define ASR_PM		0x400
#define ASR_IS		0x200
#define ASR_OS		0x100
#define ASR_US		0x80
#define ASR_QS		0x40
#define ASR_PR		0x20

/*
 * in-line asm functions for MAU instructions
 */

asm	void
movta(p)
{
%	mem	p;
	mmovta	p
}

asm	void
movfa(p)
{
%	mem	p;
	mmovfa	p;
}

asm	void
movtd(p)
{
%	mem	p;
	mmovtd	p
}

asm	void
movt0(p)
{
%	mem	p;
	mmovxx	p,%x0
}

asm	void
movt1(p)
{
%	mem	p;
	mmovxx	p,%x1
}

asm	void
movt2(p)
{
%	mem	p;
	mmovxx	p,%x2
}

asm	void
movt3(p)
{
%	mem	p;
	mmovxx	p,%x3
}

asm	void
movf0(p)
{
%	mem	p;
	mmovxx	%x0,p
}

asm	void
movf1(p)
{
%	mem	p;
	mmovxx	%x1,p
}

asm	void
movf2(p)
{
%	mem	p;
	mmovxx	%x2,p
}

asm	void
movf3(p)
{
%	mem	p;
	mmovxx	%x3,p
}

asm	void
movf32(p)
{
%	mem	p;
	mmovxd	%x3,p
}

asm	void
movfd(p)
{
%	mem	p;
	mmovfd	p
}
