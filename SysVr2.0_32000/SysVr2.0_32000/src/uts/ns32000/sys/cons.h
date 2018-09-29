/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*
 *	SYS16 Console (Signetics 2651)
 */

struct console {
	unsigned char data;
	char fill0;
	unsigned char status;
	char fill1;
	unsigned char mode;
	char fill2;
	unsigned char cmd;
};

/* Command register */
#define TxEN	0x1		/* transmitter enable */
#define DTR	0x2		/* assert data terminal ready */
#define RxEN	0x4		/* receiver enable */
#define BRK	0x8		/* force a "break" */
#define ERESET	0x10		/* reset error flags */
#define RTS	0x20		/* assert request to send */

/* Status register */
#define TxRDY	0x1		/* transmitter ready */
#define RxRDY	0x2		/* receiver ready */
#define TxEMT	0x4		/* transmitter empty/dschng */
#define PE	0x8		/* parity error */
#define OE	0x10		/* overrun error */
#define FE	0x20		/* framing error */
#define DCD	0x40		/* data carrier detect */
#define DSR	0x80		/* data set ready */

#define CONADDR	((struct console *) 0xfffa00)

#define C_RINT	0x200	/* bit in mask for rcv int */
#define C_XINT	0x400	/* bit in mask for xmt int */
#define C_DATA	0xff	/* data byte */
