/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/ppc_lla.h	10.4"
/*
	This file contains PPC-specific codes for low-level access.
*/

/*
	Opcodes of Common I/O queue entries

	These codes are used as opcodes in request queue entries.
	They are also used in the "status" field in completion queue
	entries, so that the association between the type of request
	and the type of completion is maintained.
*/

#define PPC_OPTIONS	32		/* GEN, COMP queues: set PPC options */
#define PPC_XMIT	33		/* GEN, COMP queues: transmit a buffer */
#define PPC_CONN	34		/* GEN, COMP queues: connect a device */
#define PPC_DISC	35		/* GEN, COMP queues: disconnect a device */
#define PPC_BRK		36		/* GEN, COMP queues: ioctl break */
#define PPC_DEVICE	40		/* EXP, ECOMP entries: device control command */
#define PPC_CLR		41		/* EXP, ECOMP entries: board clear */
#define PPC_RECV	50		/* RECV, COMP queues: receive request */

#define CFW_CONFIG	70		/* GEN, COMP queues: set PPC port 0 hardware options */
#define CFW_IREAD	71		/* GEN, COMP queues: read immediate one to four bytes */
#define CFW_IWRITE	72		/* GEN, COMP queues: write immediate one to four bytes */
#define CFW_WRITE	73		/* GEN, COMP queues: write  */


/*
	The following is a status field value, used only on asynchronous
	outputs from the PPC.
*/

#define PPC_ASYNC	60		/* COMP queue: asynchronous report */

/*
	Sub-field values for the PPC_DEVICE request entry;
	these are placed in appl.pc[0] in the PPC_DEVICE application field.
	The prefix DR indicates that this is a code for use
	in "device" request entries only.
*/

#define DR_ENA		1		/* enable a device */
#define DR_DIS		2		/* disable a device */
#define DR_ABR		3		/* abort reception on a device */
#define DR_ABX		4		/* abort transmission on a device */
#define DR_BRK		5		/* transmit "break" on a device */
#define DR_SUS		6		/* suspend xmit on a device */
#define DR_RES		7		/* resume xmit on a device */
#define DR_BLK		8		/* transmit STOP character */
#define DR_UNB		9		/* transmit START character */

/*
	Sub-field values for the PPC_DEVICE completion entry;
	these appear in appl.pc[0] in the PPC_DEVICE application field.
	These are mutually exclusive and cannot be combined.
	The prefix DC indicates that this is a code for use
	in "device" completion entries only.
*/

#define DC_NORM	0x00	/* command executed as requested */
#define DC_DEV	0x01	/* bad device number */
#define DC_NON	0x02	/* bad sub-code on request */
#define DC_FAIL	0x03	/* failed to read express entry */

/*
	Sub-field values for the PPC_RECV completion entry;
	these appear in appl.pc[0] in the PPC_RECV application field.
	These are NOT mutually exclusive and may appear in combination.
	The prefix RC indicates that this is a code for use
	in "read" completion entries only.
*/

#define RC_DSR	0x01	/* disruption of service */
#define RC_FLU	0x02	/* buffer flushed */
#define RC_TMR	0x04	/* inter-character timer expired */
#define RC_BQO	0x08	/* PPC buffer queue overflow */
#define RC_UAO	0x10	/* uart overrun */
#define RC_PAR	0x20	/* parity error */
#define RC_FRA	0x40	/* framing error */
#define RC_BRK	0x80	/* break received */

/*
	The following codes are included on the DISC (disconnect) command.
	They are "or"ed into the appl.pc[1] application field in a request.
	These codes are NOT mutually exclusive and can be used in any combination.
*/

#define GR_DTR		0x01
#define GR_CREAD	0x02

/*
	Sub-field values for the PPC_XMIT and PPC_OPTIONS completion entries;
	these appear in appl.pc[0] in the application fields.
	These are NOT mutually exclusive and may appear in combination.
	The prefix GC indicates that this is a code for use
	in "general" completion entries only.
*/

#define GC_DSR	0x01	/* disruption of service */
#define GC_FLU	0x02	/* buffer flushed */

/*
	Sub-field values for the PPC_ASYNC completion entry;
	these appear in appl.pc[0] in the PPC_ASYNC application field.
	These are mutually exclusive and cannot be combined.
	The prefix AC indicates that this is a code for use
	in "asynchronous" completion entries only.
*/

#define AC_CON	0x01	/* connection detected */
#define AC_DIS	0x02	/* disconnection detected */
#define AC_BRK	0x03	/* asynchronous "break" */
#define AC_FLU	0x04	/* xmit flush complete */

/*
	"options" structure
*/

typedef struct
{
	short		line;	/* line discipline: key for union below */
	short		pad1;	/* needed for 4-byte alignment on iapx86 */

	union
	{
		struct
		{
			unsigned short	iflag;	/* input options word */
			unsigned short	oflag;	/* output options word */
			unsigned short	cflag;	/* hardware options */
			unsigned short	lflag;	/* line discipline options */
			unsigned char	cerase;	/* "erase" character */
			unsigned char	ckill;	/* "kill" character */
			unsigned char	cinter;	/* ?? */
			unsigned char	cquit;	/* "quit" character */
			unsigned char	ceof;	/* "end of file" character */
			unsigned char	ceol;	/* "end of line" character */
			unsigned char	itime;	/* inter character timer multiplier */
			unsigned char	vtime;	/* user-specified inter char timer */
			unsigned char	vcount;	/* user-specified maximum buffer char count */
		} zero;
	} ld;
} Options;

/*
	Option flag bit masks and shift values,
	with corresponding prefixes M_ and S_ respectively.
*/

/* input */

#define M_IGNBRK	0x0001
#define M_BRKINT	0x0002
#define M_IGNPAR	0x0004
#define M_PARMRK	0x0008
#define M_INPCK		0x0010
#define M_ISTRIP	0x0020
#define M_INLCR		0x0040
#define M_IGNCR		0x0080
#define M_ICRNL		0x0100
#define M_IUCLC		0x0200
#define M_IXON		0x0400
#define M_IXANY		0x0800

/* output */

#define M_OPOST		0x0001
#define M_OLCUC		0x0002
#define M_ONLCR		0x0004
#define M_OCRNL		0x0008
#define M_ONOCR		0x0010
#define M_ONLRET	0x0020
#define M_OFILL		0x0040
#define M_OFDEL		0x0080
#define M_ONLDLY	0x0100
#define M_OCRDLY	0x0600
#define M_OTABDLY	0x1800
#define M_OBSDLY	0x2000
#define M_OVTDLY	0x4000
#define M_OFFDLY	0x8000

#define S_OCRDLY	9
#define S_OTABDLY	11

/* hardware options */

#define M_CBAUD		0x000f
#define M_CSIZE		0x0030
#define M_CSTOP		0x0040
#define M_CREAD		0x0080
#define M_PARENB	0x0100
#define M_PARODD	0x0200
#define M_CLOCAL	0x0800

#define S_CBAUD		0
#define S_CSIZE		4

/* line discipline options */

#define M_ITIME		0x0004

#define	DR_FLU		10		/* flush request entry */
#define	AC_CLS		0x05		/* request entry flush complete */

/* version */

#define VERS		80		/* EXP, ECOMP entries: prom version */
