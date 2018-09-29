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
#define SIO_MAX	8	/* max sio's handled by driver - sio_cnt is actual number */

/*
 * Command code definitions:
 
#define DONE	0x00 	/* command chain end */
#define DMA_NO	0x01 	/* stop dma activity */
#define DMA_GO	0x02 	/* (re)start dma activity */
#define MDM_CLR	0x03 	/* clear modem/line control bits */
#define MDM_SET	0x04 	/* set modem/line control bits */
#define LINE_OFF 0x05 	/* disable line */
#define LINE_ON	0x06 	/* enable line */
#define INT_ONE	0x07 	/* interrupt on every char */
#define INT_MANY 0x08 	/* interrupt on 3/4 full */
#define CONFIG	0x0a	/* reconfigure a line */

/*
 * Line status bit definitions (modem control information):
 */
#define M_DTR	0x02	/* Data Terminal Ready	- command */
#define M_BRK	0x08	/* Break		- command */
#define M_RTS	0x20	/* Ready To Send	- command */
#define M_DCD	0x40	/* Data Carrier Detect	- status */
#define M_DSR	0x80	/* Data Set Ready	- status */

/*
 * SIO board DMA parameter layout:
 */
struct sio_dma {
	caddr_t	dma_addr;	/* Start addr (only 1st 3 bytes used)	*/
	ushort	dma_count;
	ushort	dma_resid;
};

/*
 * SIO board command structure:
 */
struct sio_cmd {
	char cmd;
	char fill;
	char parm1, parm2;
};

/*
 * Overall SIO board memory layout:
 */
struct device {
	char	m_prom[0x800];
	char	m_fmode,	/* fifo interrupt mode (don't touch) */
		m_done,		/* bit mask of lines with dma complete */
		m_done_pend,	/* bit mask of lines completing since */
				/* m_done was last set by SIO board */
		m_mdm_lines,	/* bit mask of lines with changed modem */
				/* status */
		m_mdm_pend,	/* bit mask of lines with modem status */
				/* changed since m_mdm_lines was last */
				/* set by SIO board */
		m_fill0[3],
		m_modem[8];	/* modem state info for each line */
	struct sio_dma m_dma[8]; /* a dma parameter area for each line */
	struct sio_cmd m_cmd[0x6c]; /* the command area */
	char	m_fill1[0xe00]; /* spacing to start of hw regs */
	char	m_usart[0x40];
	char	m_mask,		/* interrupt mask register */
		m_status,	/* interrupt status register */
		m_fill2,
		m_go,		/* initiate command by writing here */
		m_done_int,	/* force done interrupt */
		m_modem_int,	/* force modem interrupt */
		m_diag_int;	/* force diagnostic interrupt */
	char	m_fill3[0x3b9];
	short	m_fifo;		/* fifo read location */
};

/*
 * Interrupt mask and status register bit layouts:
 */
#define I_DONE		0x01	/* command and/or dma complete; must be	*/
#define I_MODEM	0x02	/* modem interrupt			*/
#define I_DIAG		0x04	/* interrupt from board diagnostics	*/
#define I_FIFO		0x08	/* char received or 3/4 full		*/
#define I_ERROR	0x10	/* MESA <-> SIO board parity error	*/

/*
 * Fifo bit definitions:
 */
#define FI_PAR		0x0100	/* parity error?	*/
#define FI_OVER	0x0200	/* overrun error?	*/
#define FI_FRAME	0x0400	/* framing error?	*/
#define FI_VALID	0x8000	/* fifo nonempty	*/
