/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:m32/maugen.h	1.4"

#define	FSTRLN	40

/* 
 * Opcodes
 */

#define OP1MEM(size) (size | 0x04)
#define OP1REG(reg) reg
#define OP1NOP 0x7
#define OP2MEM(size) (size | 0x04)
#define OP2REG(reg) reg
#define OP2NOP 0x7
#define OP3MEM(size) (0x0c | size)
#define OP3REG(reg,size) ((size << 2) | reg)
#define OP3NOP 0xf
					/* OPCODES */
#define M_ADD	0x02
#define M_SUB	0x03
#define M_MUL	0x06
#define M_DIV	0x04
#define M_REM	0x05
#define M_MOVE	0x07
#define WR_ASR	0x09
#define RD_ASR	0x08
#define M_CMP	0x0a
#define M_CMPE	0x0b
#define M_NEG	0x17
#define M_ABS	0x0c
#define M_SQRT	0x0d
#define M_RTOI	0x0e
#define M_ITOF	0x10
#define M_FTOI	0x0f
#define M_DTOF	0x11
#define M_FTOD	0x12


#define	OP1	1
#define	OP2	2
#define	OP3	3

#define	TMEM	1
#define	TREG	2
#define	TNOP	3

#define	MAU_RD	0x01
#define	MAU_WR	0x02
#define	max(a,b)	((a<b) ? b : a)
