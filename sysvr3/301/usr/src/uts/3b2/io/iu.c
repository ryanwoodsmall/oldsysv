/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/iu.c	10.11"
/*
 *	3B2 Integral UART Driver
 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/iu.h"
#include "sys/csr.h"
#include "sys/dma.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/conf.h"
#include "sys/sysinfo.h"
#include "sys/firmware.h"
#include "sys/nvram.h"
#include "sys/sysmacros.h"
#include "sys/inline.h"
#include "sys/cmn_err.h"

#define	IUCONSOLE   0	/* minor device number for system board console uart */
#define	IUCONTTY    1	/* minor device number for system board second uart */
#define	IU_CNT      2	/* number of iuarts on the system board (one duart) */

#define ONRCVQ	0x1	/* flag to indicate that there are chars to receive */
#define ONXMTQ	0x2	/* flag to indicate that there are chars to transmit */

/* This structure is used by the integral UART interrupt handlers
 * to keep track of the Receive buffer pool of characters, and to
 * determine whether rcv or xmt PIR requests are outstanding.
 */
struct ttint 
{
	struct clist rcvq;
	short rcvflag;
	short xmtflag;
};

/*  Values used to designate whether characters should be queued or
 *  processed depending upon the rate characters are being received.
 */
#define IU_IDLE		0	/* no characters have been rcv for 30ms */
#define IU_TIMEOUT	1	/* state was active but no chars rcv for 30ms */
#define IU_ACTIVE	2	/* character was rcv within last 30ms */
#define	IU_SCAN		3	/* clock ticks for the iuart scan timeout */

#define ISSUE_PIR9	0x01	/* sets the CSR bit for UART lo-IPL handler */

/*
 *  Register defines for the interrupt mask register an the 
 *  output port configuration register used during DMA jobs.
 */

#define INIT_IMR	0x00	/*  initializes interrupt mask register */
#define DCD_INT0	0x01	/*  interrupt on dcd change for console */
#define DCD_INT1	0x02	/*  interrupt on dcd change for contty */
#define BAUD_SEL	0x80	/*  sets baud rate generator to 1  */
#define	COUNTER		0x30	/*  selects crystal counter (CLK/16)  */
#define DCD_CHG		0x80	/*  allovs interrupt on input port change in state  */
#define INIT_OPCR	0xc0	/*  initializes output port config register */

#define RXINT0		0x03	/*  iuart 0 xmt/rcv interrupt enab/dis  */
#define RXINT1		0x30	/*  iuart 1 xmt/rcv interrupt enab/dis  */
#define	CNTRINT		0x08	/*  counter/timer interrupt enable  */
#define  XINT0		0x01	/*  iuart 0 transmitter interrupt enab/dis */
#define  XINT1		0x10	/*  iuart 1 transmitter interrupt enab/dis */

#define IU0TC		0x04	/*  iuart 0 terminal count in dmac  */
#define IU1TC		0x08	/*  iuart 1 terminal count in dmac  */
#define DIS_DMAC	0x04	/*  disables dmac chan for requested chan */
#define	IUCTRHZ		230525	/*  counter/timer frequency (CLK/16)  */
#define	IUCTRMAX	0xffff	/*  maximum counter/timer value (16 bits) */


extern struct duart duart;	/* system board duart structure */
extern struct dma dmac;		/* system board dma controller structure */
extern int sbdrcsr;		/* used to access the sbd CSR for reading */
extern int sbdwcsr;		/* used to access the sbd CSR for writing */
extern char uartflag;		/* used to generate pir-9 interrupt */
extern struct tty iu_tty[];	/* tty data structures */
extern rnvram(), wnvram();	/* functions called to read and write NVRAM */
extern int	Demon_cc;	/* Control character for entry to demon. */

/* NVRAM subfield from physical mode */
unsigned short nvramcons = CS8 | B9600 | CREAD;	/* console characteristics  */

struct ttint iu_ttint[2];	/* tty interrupt structures */
char iu_work[2];		/* call list for line discipline */
unsigned char	imaskreg;	/* local copy of intr mask reg */
unsigned char	opcrmask;	/* local copy of output port conf reg */
unsigned char	acrmask;	/* local copy of aux config reg */

char	iu_speeds[16] =
{
	0, 0, 0, B110BPS, B134BPS, 0,
	0, B300BPS, B600BPS, B1200BPS, B1800BPS,
	B2400BPS, B4800BPS, B9600BPS, B19200BPS, 0
};

/*
 *  structure used to reference uart 0/1 registers indirectly
 *  without checking which device is being referenced
 */

struct {
	unsigned char	*uart_csr;	/*  ptr to duart.[ab]_sr_csr  */
	unsigned char	*uart_cmnd;	/*  ptr to duart.[ab]_cmnd    */
	unsigned char	*uart_data;	/*  ptr to duart.[ab]_data    */
	char		dmajob;	
	char		dmac_chan;	/*  DMAC channel (CH2IUA or CH3IUB)  */
	char		INT;		/*  XINT1 or XINT0  */
} dual[IU_CNT] = {
		&duart.a_sr_csr, &duart.a_cmnd, &duart.a_data, 0, CH2IUA, XINT0,
		&duart.b_sr_csr, &duart.b_cmnd, &duart.b_data, 0, CH3IUB, XINT1
	         };

/* MACROS used to access the specific uart DCD and DTR signals */
#define DCD(dev)	(dev ? DCDB : DCDA)
#define DTR(dev)	(dev ? DTRB : DTRA)
int	(*iutimefn)();		/* Function to call when timer goes off */
int	iutimepir;		/* Timer pir9 pending */
int	iuspurint;		/* Spurious CNTRINT count */


iuinit()
{
	register dev;

	duart.a_cmnd = (DIS_TX | DIS_RX);

	duart.b_cmnd = (DIS_TX | DIS_RX);

	imaskreg = INIT_IMR | DCD_CHG | CNTRINT;
	duart.is_imr = imaskreg;

	opcrmask |= INIT_OPCR;
	duart.ip_opcr = INIT_OPCR;
	acrmask = DCD_INT0 | DCD_INT1 | BAUD_SEL | COUNTER;
	duart.ipc_acr = acrmask;

	for (dev = 0; dev < IU_CNT; dev++) 
	{
		dual[dev].dmajob =0;
		iu_work[dev] = IU_IDLE;
		iumodem(dev,ON) ;
	}
	iuscan();
	
}  /* iuinit */


iuopen(dev, flag)
register dev, flag;
{
	register struct tty *tp;
	register int retval;
	register char *conflag, *conbaud;
	unsigned short consbaud;
	int		oldpri;
	extern iuproc();

	dev = minor(dev);

	if (dev >= IU_CNT) 
	{
		u.u_error = ENXIO;
		return;
	}

	tp = &iu_tty[dev];
	if ((tp->t_state & (ISOPEN | WOPEN)) == 0) 
	{
		ttinit(tp);
		if (dev == IUCONSOLE) 
		{
			/* ttinit leaves flags 0, for the console we want sane values */
			tp->t_iflag |= ICRNL | IXON | IXANY | BRKINT | IGNPAR | ISTRIP;
			tp->t_oflag |= OPOST | ONLCR | TAB3;
			tp->t_lflag |= ISIG | ICANON | ECHO | ECHOK;

			/* get control modes from the nvram */
			conflag = (char *)(&UNX_NVR->consflg);
			conbaud = (char *)(&consbaud);
			retval = rnvram(conflag, conbaud, sizeof(UNX_NVR->consflg));
			if (retval != 1)
				tp->t_cflag = nvramcons;
			else
			{
				tp->t_cflag = consbaud;
			}
		}
		tp->t_proc = iuproc;
		iuparam(dev);
	}

	oldpri = spltty();
	if (tp->t_cflag & CLOCAL || iumodem(dev, ON))
		tp->t_state |= CARR_ON;
	else
		tp->t_state &= ~CARR_ON;

	if (!(flag & FNDELAY))
		while ((tp->t_state & CARR_ON) == 0) 
		{
			tp->t_state |= WOPEN;
			sleep((caddr_t) & tp->t_canq, TTIPRI);
		}

	(*linesw[tp->t_line].l_open)(tp);
	splx(oldpri);
}  /* iuopen */


iuclose(dev)
register dev;
{
	register struct tty *tp;
	register int	oldpri;

	dev = minor(dev);

	tp = &iu_tty[dev];
	(*linesw[tp->t_line].l_close)(tp);

	if (tp->t_cflag & HUPCL) 
	{
		oldpri = spltty();
		iumodem(dev, OFF);
		splx(oldpri);
	}

	/* reinitialize the duart registers for the closing
	   device and turn off xmt and rcv interrupts */
	if (dev == IUCONSOLE) 
	{
		duart.a_cmnd = (DIS_TX | DIS_RX);
		imaskreg &= ~RXINT0;
	} 
	else 
	{
		duart.b_cmnd = (DIS_TX | DIS_RX);
		imaskreg &= ~RXINT1;
	}
	duart.is_imr = imaskreg;
}  /* iuclose */


iuread(dev)
register dev;
{
	register struct tty *tp;

	dev = minor(dev);

	tp = &iu_tty[dev];
	(*linesw[tp->t_line].l_read)(tp);
}  /* iuread */


iuwrite(dev)
register dev;
{
	register struct tty *tp;

	dev = minor(dev);

	tp = &iu_tty[dev];
	(*linesw[tp->t_line].l_write)(tp);
}  /* iuwrite */


iuioctl(dev, cmd, arg, mode)
register dev, cmd, arg, mode;
{

	register char	*conflag, *conbaud;
	unsigned short consbaud;
	int retval;

	dev = minor(dev);

	/* if dev is the console, save control modes in nvram */
/* 	if (dev == IUCONSOLE)
		switch (cmd) 
		{
			case TCSETAF:
			case TCSETA:
			case TCSETAW:
			{
				consbaud = iu_tty[dev].t_cflag;
				switch (consbaud & CBAUD)
				{
					case B110:
					case B300:
					case B600:
					case B1200:
					case B2400:
					case B4800:
					case B9600:
					{
						conflag = (char *)(&UNX_NVR->consflg);
						conbaud = (char *)(&consbaud);
						retval = wnvram(conbaud, conflag, sizeof(UNX_NVR->consflg));
						break;
					}
					default:
						break;
				}  /* switch (consbaud & CBAUD)  
			}
			default:
				break;
			} /* switch (cmd)  
		*/

	if (ttiocom(&iu_tty[dev], cmd, arg, mode))
		iuparam(dev);
}  /* iuioctl */


iuparam(dev)
register dev;
{
	register struct tty *tp;
	register flags, mr1, mr2 ,cr;
	int s;


	s = spltty();
	tp = &iu_tty[dev];
	flags = tp->t_cflag;

	if ((flags & CBAUD) == 0) 
	{
		/* hang up modem */
		iumodem(dev,OFF);
		return;
	}

	mr1 = CHAR_ERR;
	if ((flags & CSIZE) == CS8)
		mr1 |= BITS8;
	if ((flags & CSIZE) == CS7)
		mr1 |= BITS7;
	if ((flags & CSIZE) == CS6)
		mr1 |= BITS6;
	if ((flags & PARENB) == 0)  
		mr1 |= NO_PAR;
	if ((flags & PARODD) != 0)
		mr1 |= OPAR;	/* if not odd, then evenp assumed */

	/* construct mode register 2 */
	mr2 = NRML_MOD;
	if (flags & CSTOPB)
		mr2 |= TWOSB;
	else
		mr2 |= ONESB;

	(*tp->t_proc)(tp,T_SUSPEND) ;

	/* write the command register to reset selected options including
	 * pointer to mr1 */

	*(dual[dev].uart_cmnd) = (STOP_BRK | DIS_TX | DIS_RX);
	*(dual[dev].uart_cmnd) = RESET_MR; 
	*(dual[dev].uart_cmnd) = RESET_ERR; 

	if (dev == IUCONSOLE)
		imaskreg |= RXINT0;
	else
		imaskreg |= RXINT1;
	duart.is_imr = imaskreg;

	if (dev == IUCONSOLE) 
	{
		duart.mr1_2a = mr1;
		duart.mr1_2a = mr2;
		duart.a_sr_csr = iu_speeds[flags & CBAUD];
	} 
	else 
	{
		duart.mr1_2b = mr1;
		duart.mr1_2b = mr2;
		duart.b_sr_csr = iu_speeds[flags & CBAUD];
	}

	cr = (ENB_TX | RESET_MR);
	if (flags & CREAD) 
		cr |= ENB_RX;
	else
		cr &= ~ENB_RX;

	*(dual[dev].uart_cmnd) = cr;

	(*tp->t_proc)(tp,T_RESUME) ;

	splx(s);
}  /* iuparam */


iuint()
{
	register struct tty *tp;
	register struct ttint *ip;
	register unsigned char sr, clrdmac;
	register dev;
	register int	ignore;

	/* check CSR to determine whether DMAC or DUART caused interrupt */

	if (Rcsr & CSRUART) 
	{
		if (duart.is_imr & CNTRINT) {
			ignore = duart.scc_ropbc;	/* Stop counter */
			if (iutimefn) {
				iutimepir = 1;
				uartflag = 1;
				Wcsr->s_pir9 = ISSUE_PIR9;
			} else {
				++iuspurint;
				cmn_err(CE_NOTE, "spurious iu counter interrupt");
			}
		}
		for (dev = 0; dev < IU_CNT; dev++) 
		{
			tp = &iu_tty[dev];
			ip = &iu_ttint[dev];

			/* check for data set change;
			 * reverse logic used to check DCD bit in input port */
			if (duart.ipc_acr & (DCD_INT0 | DCD_INT1))
				;
			if (tp->t_cflag & CLOCAL || !(duart.ip_opcr & DCD(dev)))
			{
				if ((tp->t_state & CARR_ON) == 0) 
				{
					wakeup(&tp->t_canq);
					tp->t_state |= CARR_ON;
				}
			} 
			else 
			{
				if (tp->t_state & CARR_ON) 
				{
					if (tp->t_state & ISOPEN) 
					{
						register unsigned char bit;

						signal(tp->t_pgrp, SIGHUP);
						bit = DTR(dev);
						duart.scc_ropbc = bit;
						ttyflush(tp, (FREAD | FWRITE));
						iuflush(ip);	/* flush rcvq */
					}
					tp->t_state &= ~CARR_ON;
				}
			}

			/* check status register */
			sr = *(dual[dev].uart_csr);

			if (sr & RCVRDY)
				iurint(dev);
			if (sr & XMTRDY)
				iuxint(dev);
		}
	}

	if (Rcsr & CSRDMA) 
	{
		sr = dmac.RSR_CMD;
		clrdmac = duart.c_uartdma;

		if (sr & IU0TC)
		{
			*(dual[IUCONSOLE].uart_cmnd) = DIS_TX;
			dmac.WMKR = (CH2IUA | DIS_DMAC);
			dual[IUCONSOLE].dmajob = 0;
			imaskreg |= XINT0;
			duart.is_imr = imaskreg;
			*(dual[IUCONSOLE].uart_cmnd) = ENB_TX;
		}

		if (sr & IU1TC) 
		{
			*(dual[IUCONTTY].uart_cmnd) = DIS_TX;
			dmac.WMKR = (CH3IUB | DIS_DMAC);
			dual[IUCONTTY].dmajob = 0;
			imaskreg |= XINT1;
			duart.is_imr = imaskreg;
			*(dual[IUCONTTY].uart_cmnd) = ENB_TX;
		}
	}
}  /* iuint */


iurint(dev)
register dev;
{
	register struct tty *tp;
	register struct ttint *ip;
	register char c, stat;
	register unsigned char *sr;

	sysinfo.rcvint++;
	if (dev >= IU_CNT)
		return;

	tp = &iu_tty[dev];
	ip = &iu_ttint[dev];

	sr = dual[dev].uart_csr;

	while ((stat = *sr) & RCVRDY) 
	{
		*(dual[dev].uart_cmnd) = RESET_ERR;
		c = *(dual[dev].uart_data);

		/*	Check for entry to demon.	*/

		if (dev == IUCONSOLE) {
			register int	ctmp;

			ctmp = c & 0177;
			if (Demon_cc  &&  Demon_cc == ctmp) {
				if (call_demon())
					continue;
			}
		}

		/* check for CSTART/CSTOP */

		if (tp->t_iflag & IXON) 
		{
			register char ctmp;
			if (tp->t_state & ISTRIP)
				ctmp = c & 0177;
			else
				ctmp = c;
			if (tp->t_state & TTSTOP) 
			{
				if (ctmp == CSTART || tp->t_iflag & IXANY)
					(*tp->t_proc)(tp, T_RESUME);
			} 
			else 
			{
				if (ctmp == CSTOP)
					(*tp->t_proc)(tp, T_SUSPEND);
			}
			if (ctmp == CSTART || ctmp == CSTOP)
				continue;
		}

	        /* Check for errors */
	        {
			register int flg;
			char lbuf[3];		/* local character buffer */
			short lcnt;		/* count of chars in lbuf */

			lcnt = 1;
			flg = tp->t_iflag;
			if (stat & (FE | PARERR | OVRRUN)) 
				*(dual[dev].uart_cmnd) = RESET_ERR;

			if (stat & PARERR && !(flg & INPCK))
				stat &= ~PARERR;

			if (stat & (RCVD_BRK | FE | PARERR | OVRRUN)) 
			{
				if ((c & 0377) == 0) 
				{
					if (flg & IGNBRK)
						continue;

					if (flg & BRKINT) 
					{
						(*linesw[tp->t_line].l_input)(tp,L_BREAK) ;
						iuflush(ip);	/* flush rcvq */
						continue;
					}
				} 
				else 
				{
					if (flg & IGNPAR)
						continue;
				}

				if (flg & PARMRK) 
				{
					lbuf[2] = 0377;
					lbuf[1] = 0;
					lcnt = 3;
					sysinfo.rawch += 2;
				} 
				else
					c = 0;
			} 
			else 
			{
				if (flg & ISTRIP)
					c &= 0177;
				else 
				{
					c &= 0377;
					if (c == 0377 && flg & PARMRK) 
					{
						lbuf[1] = 0377;
						lcnt = 2;
					}
				}
			}

			/* put input character on rcvq */
			lbuf[0] = c;
			while (lcnt) 
			{
				if ((putc(lbuf[--lcnt], &ip->rcvq)) == -1)
					return;
				iu_work[dev] = IU_ACTIVE;
			}

			if (ip->rcvq.c_cc >= CLSIZE) 
			{
				iurcvq(dev);
				if (ip->rcvq.c_cc > TTXOHI)
				{
					if ((flg & IXOFF) && !(tp->t_state & TBLOCK))
						(*tp->t_proc)(tp, T_BLOCK); 	/* note this gets cleared by ttin */
					if (ip->rcvq.c_cc > TTYHOG) 
					{
						iuflush(ip);		/* flush rcvq */
						return;
					}
				}
			}
	    	}
	}
}  /* iurint */


iurcvq(dev)
register dev;
{
	register struct ttint *ip;

	ip = &iu_ttint[dev];

	/* if flag on, already have work for this device */
	if (ip->rcvflag & ONRCVQ)
		return;

	/* set rcv flag and issue PIR 9 */
	ip->rcvflag |= ONRCVQ;
	uartflag = 1;
	Wcsr->s_pir9 = ISSUE_PIR9;
}  /* iurcvq */


iupirint()
{
	register struct tty *tp;
	register struct ttint *ip;
	register char sr;
	register s, dev;
	register struct cblock *cb;
	register int	(*fn)();

	if (iutimepir) {
		if ((fn = iutimefn) == 0)
			cmn_err(CE_PANIC, "iutimefn");
		iutimepir = 0;
		iutimefn = 0;
		(*fn)();
	}

	for (dev = 0; dev < IU_CNT; dev++) 
	{
		tp = &iu_tty[dev];
		ip = &iu_ttint[dev];
		if (ip->rcvflag & ONRCVQ) 
		{
			/* link cblock from rcvq to rbuf and initialize rbuf */
			s = spltty();
			if (ip->rcvq.c_cf == NULL)
				break;

			if (tp->t_rbuf.c_ptr != NULL)
				putcf(CMATCH((struct cblock *)(tp->t_rbuf.c_ptr)));

			tp->t_rbuf.c_ptr = (ip->rcvq.c_cf)->c_data;
			tp->t_rbuf.c_count = CLSIZE - ((ip->rcvq.c_cf)->c_last - (ip->rcvq.c_cf)->c_first);
			tp->t_rbuf.c_size = CLSIZE;

			cb = getcb(&ip->rcvq);
			splx(s);

			(*linesw[tp->t_line].l_input)(tp,L_BUF);

			ip->rcvflag &= ~ONRCVQ;
		}
	}

	for (dev = 0; dev < IU_CNT; dev++) 
	{
		tp = &iu_tty[dev];
		ip = &iu_ttint[dev];
		if (ip->xmtflag & ONXMTQ) 
		{
			s = spltty();
			if (tp->t_state & TTSTOP) {
				splx(s);
				continue;
			}
			if (tp->t_tbuf.c_ptr) 
				tp->t_tbuf.c_ptr -= tp->t_tbuf.c_size - tp->t_tbuf.c_count;

			if (CPRES & (*linesw[tp->t_line].l_output)(tp))
				iuproc(tp, T_OUTPUT);
			else 
			{

				sr = *(dual[dev].uart_csr);

			  	if (sr & XMTEMT) 
				{
					*(dual[dev].uart_cmnd) = DIS_TX;
				}
				ip->xmtflag &= ~ONXMTQ;
			}
			splx(s);
		}
	}
}  /* iupirint */


iuxint(dev)
register dev;
{
	register struct tty *tp;
	register unsigned char *sr;

	sysinfo.xmtint++;
	tp = &iu_tty[dev];

	/* check for TTXON/TTXOFF */
	sr =  dual[dev].uart_csr;

	while (*sr & XMTRDY)	/* TX rdy */ 
	{
		*(dual[dev].uart_cmnd) = DIS_TX;

		if (tp->t_state & TTXON) 
		{
			if (dual[dev].dmajob == 1)
				dmac.WMKR = (dual[dev].dmac_chan | DIS_DMAC);
			*(dual[dev].uart_cmnd) = ENB_TX;
			*(dual[dev].uart_data) = CSTART;
			if (dual[dev].dmajob == 1)
				dmac.WMKR = dual[dev].dmac_chan;  /* enable DMAC */

			tp->t_state &= ~TTXON;
			continue;
		}

		if (tp->t_state & TTXOFF) 
		{
			if (dual[dev].dmajob == 1)
				dmac.WMKR = (dual[dev].dmac_chan | DIS_DMAC);
			*(dual[dev].uart_cmnd) = ENB_TX;
			*(dual[dev].uart_data) = CSTOP;
			if (dual[dev].dmajob == 1)
				dmac.WMKR = dual[dev].dmac_chan;  /* enable DMAC */

			tp->t_state &= ~TTXOFF;
			continue;
		}

		if (dual[dev].dmajob == 1)
			*(dual[dev].uart_cmnd) = ENB_TX;
		else
		{
			if (tp->t_state & BUSY) 
			{
				tp->t_state &= ~BUSY;
				iuproc(tp, T_OUTPUT);
				continue;
			}
		}
		return;
	}
}  /* iuxint */


iuproc(tp, cmd)
register struct tty *tp;
register cmd;
{
	register dev;
	unsigned char chan;
	unsigned int addr;
	int s;
	extern ttrstrt();

	s = spltty();
	dev = tp - iu_tty;
	switch (cmd) 
	{
	case T_TIME:
		tp->t_state &= ~TIMEOUT;
		*(dual[dev].uart_cmnd) = STOP_BRK;
		if (dev == IUCONSOLE)		
			imaskreg |= XINT0;	
		else				
			imaskreg |= XINT1;	
		duart.is_imr = imaskreg;	
		goto start;

	case T_WFLUSH:
		tp->t_tbuf.c_size -= tp->t_tbuf.c_count;
		tp->t_tbuf.c_count = 0;

	case T_RESUME:
		tp->t_state &= ~TTSTOP;
		if (dual[dev].dmajob == 1)
			*(dual[dev].uart_cmnd) = ENB_TX;
		goto start;

	case T_OUTPUT:
	    start:
	    {
		register struct ccblock *tbuf;
		register struct ttint *ip;
		register unsigned char sr;

		if (tp->t_state & (BUSY | TTSTOP | TIMEOUT))
			break;
		tbuf = &tp->t_tbuf;
		ip = &iu_ttint[dev];

		/* check if tbuf is empty */
		if (tbuf->c_ptr == NULL || tbuf->c_count == 0) 
		{
			/* set xmt flag and issue PIR 9 */
			ip->xmtflag |= ONXMTQ;
			uartflag = 1;
			Wcsr->s_pir9 = ISSUE_PIR9;
		} 
		else 
		{
			tp->t_state |= BUSY;
			/* following may be removed ??? */
			if (tbuf->c_count < 2)
			{
				*(dual[dev].uart_cmnd) = ENB_TX;
				*(dual[dev].uart_data) = *tbuf->c_ptr++;
				tbuf->c_count--;
			}
				/*  end of remove stuff ????  */
			else
			{
				/* turn off transmitter interrupt for dev */
				imaskreg &= ~(dual[dev].INT);
				duart.is_imr = imaskreg;
				*(dual[dev].uart_cmnd) = ENB_TX;

				/* set dma job flag, and set channel */
				dual[dev].dmajob = 1;
				chan = dual[dev].dmac_chan;

				/* calculate physical address for dmac */
				addr = vtop(tbuf->c_ptr, 0);

				/* setup dma job when transmitter is ready */
				sr = *(dual[dev].uart_csr);
				if (sr & XMTRDY)
				{
					dma_access(chan, addr, tbuf->c_count, SNGLMOD, RDMA);
				}
				else 
				{
					return;
				}

				/* reset count to zero */
				tbuf->c_ptr += tbuf->c_count;
				tbuf->c_count = 0;
			}
			ip->xmtflag &= ~ONXMTQ;
		}
		break;
	    }

	case T_SUSPEND:
		if (dual[dev].dmajob == 1)
			*(dual[dev].uart_cmnd) = DIS_TX;
		tp->t_state |= TTSTOP;
		break;

	case T_BLOCK:
		tp->t_state &= ~TTXON;
		tp->t_state |= TBLOCK;

		if (tp->t_state & BUSY) 
			tp->t_state |= TTXOFF;
		else 
		{
			tp->t_state |= BUSY;
			if (dual[dev].dmajob == 1)
				dmac.WMKR = (dual[dev].dmac_chan | DIS_DMAC);
			*(dual[dev].uart_cmnd) = ENB_TX;
			*(dual[dev].uart_data) = CSTOP;
			if (dual[dev].dmajob == 1)
				dmac.WMKR = dual[dev].dmac_chan;
		}
		break;

	case T_RFLUSH:
		if (!(tp->t_state & TBLOCK))
			break;

	case T_UNBLOCK:
		tp->t_state &= ~(TTXOFF | TBLOCK);

		if (tp->t_state & BUSY)
			tp->t_state |= TTXON;
		else 
		{
			tp->t_state |= BUSY;
			if (dual[dev].dmajob == 1)
				dmac.WMKR = (dual[dev].dmac_chan | DIS_DMAC);
			*(dual[dev].uart_cmnd) = ENB_TX;
			*(dual[dev].uart_data) = CSTART;
			if (dual[dev].dmajob == 1)
				dmac.WMKR = dual[dev].dmac_chan;  /* enable DMAC */
		}
		break;

	case T_BREAK:
		if (dev == IUCONSOLE)		
			imaskreg &= ~XINT0;	
		else				
			imaskreg &= ~XINT1;	
		duart.is_imr = imaskreg;	

		*(dual[dev].uart_cmnd) = ENB_TX;
		while ((*dual[dev].uart_csr & XMTRDY) == 0)
			;
		*(dual[dev].uart_cmnd) = STRT_BRK;	
		tp->t_state |= TIMEOUT;
		timeout(ttrstrt, tp, HZ/4);
		break;

	case T_PARM:
		iuparam(dev) ;
		break ;

	}  /* end of switch cmd */

	splx(s);
}  /* iuproc */

/*
 * iumodem()
 *
 * Toggle data terminal ready output. Uses the local "bit" variable
 * to avoid an SGS "spurious read" bug (which would play havoc with
 * these device registers).
 */
iumodem(dev, flag)
register dev, flag;
{
	register unsigned char	bit;

	bit = DTR(dev);
	if (flag == OFF)
		duart.scc_ropbc = bit;
	else
		duart.scc_sopbc = bit;

	/* reverse logic used to check DCD on input port */
	return( !(duart.ip_opcr & DCD(dev)) );
}  /* iumodem */



iuscan()
{
	register struct ttint *ip;
	register dev;
	register char *workp;

	for (dev = 0, workp = iu_work; dev < IU_CNT; dev++, workp++) 
	{
		if (*workp == IU_TIMEOUT) 
		{
			ip = &iu_ttint[dev];
			if (ip->rcvq.c_cc == 0)
				*workp = IU_IDLE;
			else
				iurcvq(dev);
		}

		if (*workp == IU_ACTIVE)
			*workp = IU_TIMEOUT;
	}

	timeout(iuscan, 0, IU_SCAN);
}  /* iuscan */


iuflush(ip)
register struct ttint *ip;
{
	register struct cblock *cb;

	while ((cb = getcb(&ip->rcvq)) != NULL)
		putcf(CMATCH((struct cblock *)(cb)));
}  /* iuflush */


iuputchar(c)
register char c;
{
	register s;

	s = splhi();
	duart.a_cmnd = ENB_TX;

	while ((duart.a_sr_csr & XMTRDY) == 0)
		;
	if (c == 0)
		return;

	duart.a_data = c;
	if (c == '\n')
		iuputchar('\r');

	iuputchar(0);
	splx(s);
}  /* iuputchar */
/*
 * iutime()
 *
 * Call a given function after a specified number of milliseconds. The
 * given function is called at IPL 10 (via a pir9). The limit of 284
 * milliseconds is based on the clock rate (230525 Hz) and the width
 * of the timing hardware (16 bits). Note the lack of request queuing;
 * use of this function is now restricted to the integral floppy driver.
 * This could be made into a general-purpose high-resolution timer with
 * a bit of effort.
 */
iutime(ms, fn)
register unsigned int	ms;
int			(*fn)();
{
	register unsigned int	ticks;
	register int		s;
	register unsigned char	ignore;

	s = spltty();
	if (iutimefn)
		cmn_err(CE_PANIC, "iudelay race");
	if (ms > IUCTRMAX || (ticks = (IUCTRHZ * ms) / 1000) > IUCTRMAX)
		cmn_err(CE_PANIC, "iudelay range");
	iutimefn = fn;
	ignore = duart.scc_ropbc;	/* Stop counter */
	duart.ctur = (ticks & 0xff00) >> 8;
	duart.ctlr = ticks & 0xff;
	ignore = duart.scc_sopbc;	/* Start counter */
	splx(s);
}
