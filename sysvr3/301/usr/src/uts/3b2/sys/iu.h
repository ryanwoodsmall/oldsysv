/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/iu.h	10.1"
/*
 *	Integral DUART
 * (Signetics 2681 Dual UART)
 *                                    READ         WRITE
 *                                  7      0     7      0
 *                                 +--------+   +--------+
 *  (struct duart *) ->mr1_2a      |        |   |        |
 *                                 +--------+   +--------+
 *
 *                                 +--------+   +--------+
 *                   ->a_sr_csr    |  SRA   |   |  CSRA  |
 *                                 +--------+   +--------+
 *
 *                                              +--------+
 *		     ->a_cmnd       RESERVED    |   CRA  |
 *                                              +--------+
 *
 *                                 +--------+   +--------+
 *                   ->a_data      |  RHRA  |   |  TXRA  |
 *                                 +--------+   +--------+
 *
 *                                 +--------+   +--------+
 *                   ->ipc_acr     |  IPCR  |   |   ACR  |
 *                                 +--------+   +--------+
 *
 *                                 +--------+   +--------+
 *                   ->is_imr      |   ISR  |   |   IMR  |
 *                                 +--------+   +--------+
 *
 *                                 +--------+   +--------+
 *                   ->ctur        |   CTU  |   |  CTUR  |
 *                                 +--------+   +--------+
 *
 *                                 +--------+   +--------+
 *                   ->ctlr        |   CTL  |   |  CTLR  |
 *                                 +--------+   +--------+
 *
 *                                 +--------+   +--------+
 *                   ->mr1_2b      |        |   |        |
 *                                 +--------+   +--------+
 *
 *                                 +--------+   +--------+
 *                   ->b_sr_csr    |   SRB  |   |  CSRB  |
 *                                 +--------+   +--------+
 *
 *                                              +--------+
 *                   ->b_cmnd       RESERVED    |   CRB  |
 *                                              +--------+
 *
 *                                 +--------+   +--------+
 *                   ->b_data      |  RHRB  |   |  THRB  |
 *                                 +--------+   +--------+
 *
 *                                 +--------+   +--------+
 *                   ->ip_opcr     | In.Prt |   |  OPCR  |
 *                                 +--------+   +--------+
 *
 *                                 +--------+   +--------+
 *                   ->scc_sopbc   |  SCC   |   |  SOPBC |
 *                                 +--------+   +--------+
 *
 *                                 +--------+   +--------+
 *                   ->scc_ropbc   |  SCC   |   |  ROPBC |
 *                                 +--------+   +--------+
 */

struct duart
{
	unsigned char mr1_2a;	/* Mode Register A Channels 1 & 2 */
	unsigned char a_sr_csr;	/* Status Reg A/Clock Select Reg A */
	unsigned char a_cmnd;	/* Command Reg A */
	unsigned char a_data;	/* RX Holding/TX Holding Reg A */

	unsigned char ipc_acr;	/* Input Port Change/Aux Cntrl Reg A */
	unsigned char is_imr;	/* Intrupt Status/Intrupt Mask Reg A */
	unsigned char ctur;	/* Counter/Timer Upper Reg */
	unsigned char ctlr;	/* Counter/Timer Lower Reg */

	unsigned char mr1_2b;	/* Mode Reg B Channels 1 & 2 */
	unsigned char b_sr_csr;	/* Status Reg B/Clock Select Reg B */
	unsigned char b_cmnd;	/* Command Reg B */
	unsigned char b_data;	/* RX Holding/TX Holding Reg B */

	int :8;			/* RESERVED */

	unsigned char ip_opcr;	/* Input Port/Output Port Conf Reg */
	unsigned char scc_sopbc;/* Start Counter Command/Set Output */
				/* Port Bits Command */
	unsigned char scc_ropbc;/* Stop Counter Command/Reset Output */
				/* Port Bits Command */
	unsigned char pad;
	unsigned char c_uartdma;/* location read to clear DMAC intr in UART */
};

/* Register Bit Format Defines	*/

/*
 *	Mode Register 1 Channel A and B Defines
 */

#define	ENB_RX_RTS	0x80
#define ENB_RXINT	0x40
#define BLCK_ERR	0x20
#define CHAR_ERR	0x00
#define PENABLE		0x00
#define	FRC_PAR		0x08
#define	NO_PAR		0x10
#define	SPECIAL		0x18
#define EPAR		0x00
#define OPAR		0x04
#define BITS5		0x00
#define	BITS6		0x01
#define	BITS7		0x02
#define	BITS8		0x03

/*
 *	Mode Register 2 Channel A and B Defines
 */
 
#define	NRML_MOD	0x00
#define	ENBECHO		0x40
#define	LCL_LOOP	0x80
#define	REM_LOOP	0xc0

#define	ENB_TXRTS	0x20
#define	ENB_CTSTX	0x10

#define	ZEROP563SB	0x00
#define	ZEROP625SB	0x01
#define	ZEROP688SB	0x02
#define	ZEROP750SB	0x03
#define	ZEROP813SB	0x04
#define	ZEROP875SB	0x05
#define	ZEROP938SB	0x06

#define	ONESB		0x07
#define	ONEP563SB	0x08
#define	ONEP625SB	0x09
#define	ONEP688SB	0x0a
#define	ONEP750SB	0x0b
#define	ONEP813SB	0x0c
#define	ONEP875SB	0x0d
#define	ONEP938SB	0x0e

#define	TWOSB		0x0f

/*
 *	Baud Rate Select Register Defines
 */

#define	B50BPS		0x00
#define	B75BPS		0x00
#define	B110BPS		0x11
#define	B134BPS		0x22
#define	B150BPS		0x33
#define	B200BPS		0x33
#define	B300BPS		0x44
#define	B600BPS		0x55
#define	B1200BPS	0x66
#define	B1050BPS	0x77
#define	B1800BPS	0xaa
#define	B2000BPS	0x77
#define	B2400BPS	0x88
#define	B4800BPS	0x99
#define	B7200BPS	0xaa
#define	B9600BPS	0xbb
#define	B19200BPS	0xcc
#define	B38400BPS	0xcc

/*
 *	Auxillary Command Register Defines
 */

#define	B50ACR		0x00
#define	B75ACR		0x80
#define	B110ACR		0x00
#define	B134ACR		0x00
#define	B150ACR		0x80
#define	B200ACR		0x00
#define	B300ACR		0x00
#define	B600ACR		0x00
#define	B1200ACR	0x00
#define	B1050ACR	0x00
#define	B1800ACR	0x80
#define	B2000ACR	0x80
#define	B2400ACR	0x00
#define	B4800ACR	0x00
#define	B7200ACR	0x00
#define	B9600ACR	0x00
#define	B19200ACR	0x80
#define	B38400ACR	0x00

/*
 *	Miscellaneous Commands for CRA/CRB
 */

#define	NO_OP		0x00
#define	RESET_MR	0x10
#define	RESET_RECV	0x20
#define	RESET_TRANS	0x30
#define	RESET_ERR	0x40
#define	RST_A_BCI	0x50
#define	STRT_BRK	0x60
#define	STOP_BRK	0x70
#define	DIS_TX		0x08
#define	ENB_TX		0x04
#define	DIS_RX		0x02
#define	ENB_RX		0x01

/*
 *	Status Register Defines for SRA/SRB
 */

#define	RCVD_BRK	0x80
#define	FE		0x40
#define	PARERR		0x20
#define	OVRRUN		0x10
#define	XMTEMT		0x08
#define	XMTRDY		0x04
#define	FIFOFULL	0x02
#define	RCVRDY		0x01

/*
 *	Register Defines for IPCR/OPCR
 */

/* Inputs */
#define	DCDA		0x01
#define	DCDB		0x02
#define SFLOP		0x04

/* Outputs */
#define	DTRA		0x01
#define	DTRB		0x02
#define KILLPWR		0x04
#define PWRLED		0x08
#define F_LOCK		0x10
#define F_SEL		0x20

#define	OIDUART		0x00049000L
#define	IDUART		((struct duart *)OIDUART)
