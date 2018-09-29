/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/psw.h	10.2"

/*	PSW structure.
*/

typedef struct psw {		/* program status word */

		unsigned int	rsvd	:6;	/* Unused bits            */
 		unsigned int	CSH_F_D	:1;	/* Cache Flush Disable    */
		unsigned int	QIE	:1;	/* Quick Interrupt Enable */
		unsigned int	CSH_D	:1;	/* Cache Disable	  */
		unsigned int	OE	:1;	/* Overflow enable.	  */
		unsigned int	NZVC	:4;	/* Neg - Zero - Ovr - Ca  */
		unsigned int	TE	:1;	/* Trace enable.	  */
		unsigned int	IPL	:4;	/* Interrupt priority.	  */
		unsigned int	CM	:2;	/* Current mode.	  */
		unsigned int	PM	:2;	/* Previous mode.	  */
		unsigned int	R	:1;	/* Register save/restore. */
		unsigned int	I	:1;	/* Initial psw.		  */
		unsigned int	ISC	:4;	/* Internal state code.	  */
		unsigned int	TM	:1;	/* Trace mask.		  */
		unsigned int	FT	:2;	/* Fault type.		  */
} psw_t;


/*
 * MAC32A internal state code values for normal exceptions
 */

#define IZDFLT		0	/* integer zero divide fault */
#define TRCTRAP		1	/* TRACE TRAP	*/
#define	ILLOPFLT	2	/* illegal opcode fault */
#define RESOPFLT	3	/* reserved opcode fault */
#define IVDESFLT	4	/* invalid descriptor fault */
#define XMEMFLT		5	/* external memory fault	*/
#define GVFLT		6	/* gate vector fault	*/
#define ILCFLT		7 	/* illegal level change fault */
#define	RDTPFLT		8	/* reserved data type fault */
#define IOVFLT		9	/* integer overflow fault */
#define PRVOPFLT	10	/* priviliged opcode fault */
#define BPTRAP		14	/* break point trap */
#define PRVREGFLT	15	/* privileged register fault */

/*
 * Stack exception internal state codes
 */

#define STKBOUND	0	/* stack bounds exception */
#define STKFLT		1	/* stack fault	*/
#define INTIDFTCH	3	/* interrupt id fetch fault */

/* 
 * process exception internal state codes
 */

#define POPCBFLT	0	/* old pcb fault */
#define PGPCBFLT	1	/* gate pcb fault */
#define PNPCBFLT	4	/* new pcb fault */

/*
 * on reset internal state codes
 */

#define ROPCBFLT	0	/* reset, old pcb fault */
#define RSYSDATA	1	/* reset, system data fault */
#define RISTKFLT	2	/* reset, interrupt stack fault */
#define REXTRNL		3	/* reset, external */
#define RNPCBFLT	4	/* reset, new pcb fault */
#define RGVFLT		6 	/* reset, gate vector fault */

/*
 * MAC32A Fault Type Codes
 */

#define ON_RESET	0
#define ON_PROCESS	1
#define ON_STACK	2
#define ON_NORMAL	3

/*
 * MAC32A NZVC codes
 */

#define PS_C		1
#define PS_V		2
#define PS_Z		4
#define PS_N		8

/*
 * MAC-32 mode fields
 */

#define PS_USER		3
#define PS_KERNEL	0

/*	The following is the initial psw for process 0.
*/

#define	IPSW	{0,0,0,0,0,0,0,15,0,0,1,0,0,0,0}

/*
 *  Kernel interrupt psw's
 */

#define ZPSW	{0,0,0,0,0,0,0, 0,0,0,1,0,0,0,0}

#define KPSW0	{0,0,0,0,0,0,0, 0,0,0,1,1,0,0,0}
#define KPSW1	{0,0,0,0,0,0,0, 1,0,0,1,1,0,0,0}
#define KPSW2	{0,0,0,0,0,0,0, 2,0,0,1,1,0,0,0}
#define KPSW3	{0,0,0,0,0,0,0, 3,0,0,1,1,0,0,0}
#define KPSW4	{0,0,0,0,0,0,0, 4,0,0,1,1,0,0,0}
#define KPSW5	{0,0,0,0,0,0,0, 5,0,0,1,1,0,0,0}
#define KPSW6	{0,0,0,0,0,0,0, 6,0,0,1,1,0,0,0}
#define KPSW7	{0,0,0,0,0,0,0, 7,0,0,1,1,0,0,0}
#define KPSW8	{0,0,0,0,0,0,0, 8,0,0,1,1,0,0,0}
#define KPSW9	{0,0,0,0,0,0,0, 9,0,0,1,1,0,0,0}
#define KPSW10	{0,0,0,0,0,0,0,10,0,0,1,1,0,0,0}
#define KPSW11	{0,0,0,0,0,0,0,11,0,0,1,1,0,0,0}
#define KPSW12	{0,0,0,0,0,0,0,12,0,0,1,1,0,0,0}
#define KPSW13	{0,0,0,0,0,0,0,13,0,0,1,1,0,0,0}
#define KPSW14	{0,0,0,0,0,0,0,14,0,0,1,1,0,0,0}
#define KPSW15	{0,0,0,0,0,0,0,15,0,0,1,1,0,0,0}

/*	The following psw is used in the gate tables
**	in ml/gate.c
*/

#define GPSW	{0,0,0,0,0,0,0, 0,0,0,1,0,0,0,0}

/*	The following psw is used by machdep.c/sendsig.
*/

#define	SIGPSW	{0,0,0,0,0,0,0, 0,3,3,1,0,0,0,0}

/*	The following defines the cache inhibit and cache
**	flush inhibit bits which must be turned on for
**	a 3b2/300 with a 32100 chip.
*/

#define	ICACHEPSW	0x02800000
