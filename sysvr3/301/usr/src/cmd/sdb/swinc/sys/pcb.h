/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:swinc/sys/pcb.h	1.2"
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

struct ipcb
	{
	PSW psw;		/* initial PSW */
	int (*pc)();		/* initial program counter (PC) */
	int *sp;		/* initial stack pointer (SP) */
	};

struct pcb
	{
	PSW psw;		/* PSW */
	int (*pc)();		/* PC */
	int *sp;		/* SP */
	int *slb;		/* stack lower bound */
	int *sub;		/* stack upper bound */
	int regsave[11];	/* registers AP,FP,R0-R8 save area */
	struct moveblk mapinfo[MAPINFO];	/* map information */
	};

struct kpcb			/* Interrupt pcb */
	{
	struct ipcb ipcb;	/* initial PCB */
	PSW psw;		/* PSW */
	int (*pc)();		/* PC */
	int *sp;		/* SP */
	int *slb;		/* stack lower bound */
	int *sub;		/* stack upper bound */
	int regsave[11];	/* registers AP,FP,R0-R8 save area */
	int movesize;		/* no map information */
	};

/*
 *  Kernel interrupt psw's
 */

#define ZPSW	{0,0,0,0,0,0,0,0,0,0,1,0,0,0,0}

#define KPSW0	{0,0,0,0,0,0,0,0,0,0,1,1,0,0,0}
#define KPSW1	{0,0,0,0,0,0,0,1,0,0,1,1,0,0,0}
#define KPSW2	{0,0,0,0,0,0,0,2,0,0,1,1,0,0,0}
#define KPSW3	{0,0,0,0,0,0,0,3,0,0,1,1,0,0,0}
#define KPSW4	{0,0,0,0,0,0,0,4,0,0,1,1,0,0,0}
#define KPSW5	{0,0,0,0,0,0,0,5,0,0,1,1,0,0,0}
#define KPSW6	{0,0,0,0,0,0,0,6,0,0,1,1,0,0,0}
#define KPSW7	{0,0,0,0,0,0,0,7,0,0,1,1,0,0,0}
#define KPSW8	{0,0,0,0,0,0,0,8,0,0,1,1,0,0,0}
#define KPSW9	{0,0,0,0,0,0,0,9,0,0,1,1,0,0,0}
#define KPSW10	{0,0,0,0,0,0,0,10,0,0,1,1,0,0,0}
#define KPSW11	{0,0,0,0,0,0,0,11,0,0,1,1,0,0,0}
#define KPSW12	{0,0,0,0,0,0,0,12,0,0,1,1,0,0,0}
#define KPSW13	{0,0,0,0,0,0,0,13,0,0,1,1,0,0,0}
#define KPSW14	{0,0,0,0,0,0,0,14,0,0,1,1,0,0,0}
#define KPSW15	{0,0,0,0,0,0,0,15,0,0,1,1,0,0,0}
