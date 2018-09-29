/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/pcb.h	10.1"
/*
 * WE32000 process control block
 */

#define MOVEDATA	2	/* size of map moves */

struct moveblk
	{
	int  movesize;		/* size of move */
	int *moveaddr;		/* address of target */
	int  movedata[MOVEDATA];/* data to be moved */
	};

/*
 * Process Control Block (PCB) 
 */

#define MAPINFO		2	/* number of map blocks in PCB */

typedef struct ipcb
	{
	psw_t psw;		/* initial PSW */
	int (*pc)();		/* initial program counter (PC) */
	int *sp;		/* initial stack pointer (SP) */
} ipcb_t;

typedef struct pcb
	{
	psw_t psw;		/* PSW */
	int (*pc)();		/* PC */
	int *sp;		/* SP */
	int *slb;		/* stack lower bound */
	int *sub;		/* stack upper bound */
	int regsave[11];	/* registers AP,FP,R0-R8 save area */
	struct moveblk mapinfo[MAPINFO];	/* map information */
} pcb_t;

typedef struct kpcb			/* Interrupt pcb */
	{
	struct ipcb ipcb;	/* initial PCB */
	psw_t psw;		/* PSW */
	int (*pc)();		/* PC */
	int *sp;		/* SP */
	int *slb;		/* stack lower bound */
	int *sub;		/* stack upper bound */
	int regsave[11];	/* registers AP,FP,R0-R8 save area */
	int movesize;		/* no map information */
} kpcb_t;

/*
 * Symbolic locations of registers in pcb relative to regsave[0]
 * Usage: u.u_pcb.regsave[K_XX] == u.u_ar0[XX]
 */

#define	K_R0	2
#define	K_R1	3
#define	K_R2	4
#define	K_R3	5
#define	K_R4	6
#define	K_R5	7
#define	K_R6	8
#define	K_R7	9
#define	K_R8	10
#define	K_FP	1	/* FP == R9  */
#define	K_AP	0	/* AP == R10 */
#define	K_PS	-5	/* PS == R11 */
#define	K_SP	-3	/* SP == R12 */
#define	K_PC	-4	/* PC == R15 */
