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
 *	UTS driver for SIO board
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/conf.h"
#include "sys/termio.h"
#include "sys/sysinfo.h"
#include "sys/sio.h"

extern struct tty sio_tty[];
extern int sio_cnt;
extern struct device *sio_addr[];
#define SIOADDR sio_addr[0]
#define SIOSCAN	2
short	sio_scan;

#define SIO_CHAR	01
#define SIO_ACT		02
/* sio_work should be done by config */
char sio_work[SIO_MAX];

static char	sio_speeds[16] = {
	0, 00, 01, 02, 03, 04, 0, 05,
	06, 07, 010, 012, 014, 016, 017, 017
};

#define	BITS6	04
#define	BITS7	010
#define	PENABLE	020
#define	EPAR	040
#define TWOSB	0300
#define DEFAULT	030102

#define	ON	1
#define	OFF	0

char sio_c[2] = {
	CSTOP, CSTART};


sioopen(dev, flag)
{
	register struct tty *tp;
	extern sioproc();

	if ((dev&077) >= sio_cnt) {
		u.u_error = ENXIO;
		return;
	}
	tp = &sio_tty[dev&077];
	if ((tp->t_state&(ISOPEN|WOPEN)) == 0) {
		ttinit(tp);
		tp->t_proc = sioproc;
		sioparam(dev);
	}
	spl5();
	if (tp->t_cflag&CLOCAL || siomodem(dev, ON))
		tp->t_state |= CARR_ON;
	else
		tp->t_state &= ~CARR_ON;
	if (!(flag&FNDELAY))
		while ((tp->t_state&CARR_ON)==0) {
			tp->t_state |= WOPEN;
			sleep((caddr_t)&tp->t_canq, TTIPRI);
		}
	(*linesw[tp->t_line].l_open)(tp);
	spl0();
}

sioclose(dev)
{
	register struct tty *tp;

	tp = &sio_tty[dev&077];
	(*linesw[tp->t_line].l_close)(tp);
	if (tp->t_cflag&HUPCL && !(tp->t_state&ISOPEN))
		siomodem(dev, OFF);
}

sioread(dev)
{
	register struct tty *tp;

	tp = &sio_tty[dev&077];
	(*linesw[tp->t_line].l_read)(tp);
}

siowrite(dev)
{
	register struct tty *tp;

	tp = &sio_tty[dev&077];
	(*linesw[tp->t_line].l_write)(tp);
}

sioioctl(dev, cmd, arg, mode)
register dev;
{
	if (ttiocom(&sio_tty[dev&077], cmd, arg, mode))
		sioparam(dev);
}

sioparam(dev)
{
	register struct tty *tp;
	register flags, lpr;

	tp = &sio_tty[dev&077];
	if (sio_scan==0) {
		sio_scan++;
		sioinit();
		sioscan();
	}
	flags = tp->t_cflag;
	if ((flags&CBAUD) == 0) {
		/* hang up line */
		siomodem(dev, OFF);
		return;
	}
	lpr = (sio_speeds[flags&CBAUD]<<8) | DEFAULT;
	if (flags&CS6)
		lpr |= BITS6;
	if (flags&CS7)
		lpr |= BITS7;
	if (flags&PARENB) {
		lpr |= PENABLE;
		if ((flags&PARODD) == 0)
			lpr |= EPAR;
	}
	if (flags&CSTOPB)
		lpr |= TWOSB;
	/*
	 * Now pass the relevant parameters to the Z80
	 */
	z80load(dev&07, LINE_ON, 0);
	z80load(dev&07, CONFIG, lpr);
}

siointr()
{
	register struct device *dp;
	register struct tty *tp;
	register dev;
	char type, lines;

	dp = SIOADDR;
	type = dp->m_status;
	if (type & I_MODEM) {
		sysinfo.mdmint++;
		siomintr();
	}
	if (type & I_FIFO) {
		sysinfo.rcvint++;
		siorint();
	}
	if (type & I_DONE) {
		sysinfo.xmtint++;
		lines = dp->m_done;
		dp->m_done = 0;
		for (dev = 0; dev < sio_cnt; dev++) {
			if (lines & (1<<dev)) {
				tp = &sio_tty[dev];
				if (tp->t_state & TTXON) {
					z80dma(dev, &sio_c[ON], 1);
					z80load(dev, DMA_GO, 0);
					tp->t_state &= ~TTXON;
					continue;
				}
				if (tp->t_state & TTXOFF) {
					z80dma(dev, &sio_c[OFF], 1);
					z80load(dev, DMA_GO, 0);
					tp->t_state &= ~TTXOFF;
					continue;
				}
				if (tp->t_state & BUSY) {
					tp->t_state &= ~BUSY;
					tp->t_tbuf.c_count = 0;
					sioproc(tp, T_OUTPUT);
				}
			}
		}
	}
}

siorint()
{
	register struct tty *tp;
	register dev;
	register struct device *dp;
	short c;

	dp = SIOADDR;
	while (((c = dp->m_fifo) & FI_VALID) == 0) {  /* char present */
		dev = (c>>11)&07;
		tp = &sio_tty[dev];
		if (!(tp->t_state&(ISOPEN|WOPEN)))
			continue;
		if (tp->t_iflag&IXON) {
			register char ctmp;
			ctmp = c & 0177;
			if (tp->t_state&TTSTOP) {
				if (ctmp == CSTART || tp->t_iflag&IXANY)
					(*tp->t_proc)(tp, T_RESUME);
			} 
			else {
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
			if (c&FI_PAR && !(flg&INPCK))
				c &= ~FI_PAR;
			if (c&(FI_FRAME|FI_PAR|FI_OVER)) {
				if ((c&0377) == 0) {
					if (flg&IGNBRK)
						continue;
					if (flg&BRKINT) {
						(*linesw[tp->t_line].l_input)
								(tp, L_BREAK);
						continue;
					}
				} 
				else {
					if (flg&IGNPAR)
						continue;
				}
				if (flg&PARMRK) {
					lbuf[2] = 0377;
					lbuf[1] = 0;
					lcnt = 3;
					sysinfo.rawch += 2;
				} 
				else
					c = 0;
			} 
			else {
				if (flg&ISTRIP)
					c &= 0177;
				else {
					c &= 0377;
					if (c == 0377 && flg&PARMRK) {
						lbuf[1] = 0377;
						lcnt = 2;
					}
				}
			}
			/* stash character in r_buf */
			lbuf[0] = c;
			if (tp->t_rbuf.c_ptr == NULL)
				return;
			while (lcnt) {
				*tp->t_rbuf.c_ptr++ = lbuf[--lcnt];
				if (--tp->t_rbuf.c_count == 0) {
					sio_work[dev] = 0;
					tp->t_rbuf.c_ptr -=
					    tp->t_rbuf.c_size - tp->t_rbuf.c_count;
					(*linesw[tp->t_line].l_input)(tp, L_BUF);
				} 
				else	
				    sio_work[dev] = SIO_ACT;
				/* note SIO_CHAR bit cleared */
			}
		}
	}
}

sioproc(tp, cmd)
register struct tty *tp;
{
	register dev;
	extern ttrstrt();

	dev = tp - sio_tty;
	switch(cmd) {

	case T_TIME:
		tp->t_state &= ~TIMEOUT;
		if (SIOADDR->m_modem[dev] & M_BRK)
			z80load(dev, MDM_CLR, M_BRK);
		goto start;

	case T_WFLUSH:
		tp->t_tbuf.c_size -= tp->t_tbuf.c_count;
		tp->t_tbuf.c_count = 0;

	case T_RESUME:
		tp->t_state &= ~TTSTOP;

	case T_OUTPUT:
start:
		{
			register struct ccblock *tbuf;
			register nch;

			if (tp->t_state&(TIMEOUT|TTSTOP|BUSY))
				break;
			tbuf = &tp->t_tbuf;
			if (tbuf->c_ptr == NULL || tbuf->c_count == 0) {
				if (!(CPRES&(*linesw[tp->t_line].l_output)(tp)))
					break;
			}
			nch = tp->t_tbuf.c_count;
			if (nch <= 0)
				break;
			z80dma(dev, tbuf->c_ptr, nch);
			z80load(dev, DMA_GO, 0);
			tp->t_state |= BUSY;
			break;
		}

	case T_SUSPEND:
		tp->t_state |= TTSTOP;
		break;

	case T_BLOCK:
		tp->t_state &= ~TTXON;
		tp->t_state |= TBLOCK;
		if (tp->t_state & BUSY) {
			tp->t_state |= TTXOFF;
		} else {
			z80dma(dev, &sio_c[OFF], 1);
			z80load(dev, DMA_GO, 0);
			tp->t_state |= BUSY;
		}
		break;

	case T_RFLUSH:
		if (!(tp->t_state&TBLOCK))
			break;

	case T_UNBLOCK:
		tp->t_state &= ~(TTXOFF | TBLOCK);
		if (tp->t_state & BUSY) {
			tp->t_state |= TTXON;
		} else {
			z80dma(dev, &sio_c[ON], 1);
			z80load(dev, DMA_GO, 0);
			tp->t_state |= BUSY;
		}
		break;

	case T_BREAK:
		z80load(dev, MDM_SET, M_BRK);
		tp->t_state |= TIMEOUT;
		timeout(ttrstrt, tp, HZ/4);
		break;

	case T_PARM:
		sioparam(dev);
		break;
	}
}

siomodem(dev, flag)
{
	dev &= 07;

	if (flag==OFF) {
		z80load(dev, MDM_CLR, (M_DTR|M_RTS));
		z80load(dev, LINE_OFF, 0);
		return (0);
	} else {
		z80load(dev, MDM_SET, (M_DTR|M_RTS));
		delay(HZ);
		return(SIOADDR->m_modem[dev] & M_DCD);
	}
}

siomintr()
{
	register int dev;
	register struct tty *tp;
	char mdmlines;

	mdmlines = SIOADDR->m_mdm_lines;

	for (dev=0; dev < sio_cnt; dev++) {
		if ((mdmlines & (1<<dev)) == 0)
			continue;

		tp = &sio_tty[dev];

		if (!(tp->t_state&(ISOPEN|WOPEN)))
			continue;

		if (tp->t_cflag&CLOCAL || SIOADDR->m_modem[dev] & M_DCD) {
			if ((tp->t_state & CARR_ON) == 0) {
				wakeup(&tp->t_canq);
				tp->t_state |= CARR_ON;
			}

		} else {
			if (tp->t_state&CARR_ON) {
				if (tp->t_state & ISOPEN) {
					signal(tp->t_pgrp, SIGHUP);
					z80load(dev, MDM_CLR, (M_DTR|M_RTS));
					ttyflush(tp, FREAD|FWRITE);
				}
				tp->t_state &= ~CARR_ON;
			}
		}
	}
	if (mdmlines)
		SIOADDR->m_mdm_lines = 0;
}

sioscan()
{
	register line, dev;
	register struct tty *tp;
	register char *workp;
	int s;
	static mdmcnt = 0;

	s = spl5();
	siorint();
	for (line = 0, workp = sio_work; line < sio_cnt; line++, workp++) {
		if (*workp & SIO_CHAR) {
			tp = &sio_tty[line];
			*workp = 0;
			if (tp->t_rbuf.c_ptr) {
				tp->t_rbuf.c_ptr -= tp->t_rbuf.c_size -
				    tp->t_rbuf.c_count;
				(*linesw[tp->t_line].l_input)(tp, L_BUF);
			}
		}
		if (*workp & SIO_ACT)
			*workp |= SIO_CHAR;
	}

	if (--mdmcnt <= 0) {
		siomintr();
		mdmcnt = HZ/SIOSCAN*4;
	}
	timeout(sioscan, 0, SIOSCAN);
	splx(s);
}

sioinit()
{
	register dev;

	SIOADDR->m_mask = I_DONE | I_FIFO | I_MODEM;
	icuenable(0x4000);
	z80load(0, INT_MANY, 0);

	for (dev=0; dev < sio_cnt; dev++)
		siomodem(dev, OFF);
}

z80load(dev, cmd, args)
{
	register struct sio_cmd *cp;
	register i;
	int j;

	cp = SIOADDR->m_cmd;
	for (i=0; i<100; i++) {
		if (cp->cmd == 0)
			goto ok;
		for (j=0; j<5; j++)
			;
	}
	panic("sio wait");
ok:
	cp->cmd = dev<<4 | cmd;
	cp->parm1 = args;
	cp->parm2 = args >> 8;
	SIOADDR->m_go = 1;
}

z80dma(dev, adr, cnt)
{
	register char *cp;

	cp = (char *)&SIOADDR->m_dma[dev];
	*cp++ = adr;
	adr >>= 8;
	*cp++ = adr;
	*cp = adr>>8;
	cp += 2;
	*cp++ = cnt;
	*cp = cnt>>8;
}

#ifdef SIODEBUG
int sio_mp;
char sio_mon[512];
siomon(n, a1, a2, a3, a4)
char a1, a2, a3, a4;
{
	int i;
	int *ap = (int *) &a1;

	for (i=0; i<n; i++) {
		sio_mon[sio_mp++] = (char)*ap++;
		if (sio_mp >= 512)
			sio_mp = 0;
	}
}
#endif
