/* @(#)dzb.c	1.2 */
/*
 *	UTS driver for DZ-11 with KMC assist
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
#include "sys/uba.h"
#include "sys/sysinfo.h"
#include "sys/dz.kmc.h"

struct device *dzb_addr[];
int	dzb_cnt;
struct tty dzb_tty[];
short	dzb_scan;
short	dzb_stat;
short	dzb_kmc[8];

static char	dzb_speeds[16] = {
	0, 00, 01, 02, 03, 04, 0, 05,
	06, 07, 010, 012, 014, 016, 017, 017
};

static char	dzb_tmr[16] = {
	0, 0, 0, 0, 0, 0, 0, 45,
	23, 12, 8, 7, 5, 4, 2, 1
};

#define	BITS6	010
#define	BITS7	020
#define	TWOSB	040
#define	PENABLE	0100
#define	OPAR	0200
#define	RCVENB	010000

#define	IE	040

struct device {
	short	dzbcsr, dzbrbuf;
	char	dzbtcr, dzbdtr;
	char	dzbtbuf, dzbbrk;
};
#define	dzblpr	dzbrbuf
#define	dzbmsr	dzbbrk

#define	ON	1
#define	OFF	0


dzbopen(dev, flag)
{
	register struct tty *tp;
	register kf;
	extern dzbproc(), dzbkint();

	if ((dev&077) >= dzb_cnt) {
		u.u_error = ENXIO;
		return;
	}
	kf = 1<<(((dev>>6)&03)+8);
	if ((dzb_stat&kf) == 0) {
		if (kmcset(dev,03,dzbkint)) {
			u.u_error = ENXIO;
			return;
		}
		dzb_stat |= kf;
	}
	tp = &dzb_tty[dev&077];
	if ((tp->t_state&(ISOPEN|WOPEN)) == 0) {
		ttinit(tp);
		tp->t_proc = dzbproc;
		tp->t_state |= EXTPROC;
		dzbparam(dev);
	}
	spl5();
	if (tp->t_cflag&CLOCAL || dzbmodem(dev, ON))
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

dzbclose(dev)
{
	register struct tty *tp;

	tp = &dzb_tty[dev&077];
	(*linesw[tp->t_line].l_close)(tp);
	if (tp->t_cflag&HUPCL && !(tp->t_state&ISOPEN))
		dzbmodem(dev, OFF);
}

dzbread(dev)
{
	register struct tty *tp;

	tp = &dzb_tty[dev&077];
	(*linesw[tp->t_line].l_read)(tp);
}

dzbwrite(dev)
{
	register struct tty *tp;

	tp = &dzb_tty[dev&077];
	(*linesw[tp->t_line].l_write)(tp);
}

dzbioctl(dev, cmd, arg, mode)
register dev;
{
	register sel4,sel6;

	switch(cmd) {
	case TCDSET:
		sel4 = SETTMR;
		sel6 = arg<<8;
		kmcload(dzb_kmc[(dev>>3)&07]|(dev&07), CMDIN, sel4, sel6);
		break;
	default:
		if (ttiocom(&dzb_tty[dev&077], cmd, arg, mode))
			dzbparam(dev);
	}
}

dzbparam(dev)
{
	register struct tty *tp;
	register struct device *dzbaddr;
	register flags, lpr;

	tp = &dzb_tty[dev&077];
	dzbaddr= dzb_addr[(dev>>3)&07];
	dzbaddr->dzbcsr = IE;
	if (dzb_scan==0) {
		dzbscan();
		dzb_scan++;
	}
	flags = tp->t_cflag;
	if ((flags&CBAUD) == 0) {
		/* hang up line */
		dzbmodem(dev, OFF);
		return;
	}
	lpr = (dzb_speeds[flags&CBAUD]<<8)|(dev&07);
	if (flags&CREAD)
		lpr |= RCVENB;
	if (flags&CS6)
		lpr |= BITS6;
	if (flags&CS7)
		lpr |= BITS7;
	if (flags&PARENB) {
		lpr |= PENABLE;
		if (flags&PARODD)
			lpr |= OPAR;
	}
	if (flags&CSTOPB)
		lpr |= TWOSB;
	dzbaddr->dzblpr = lpr;
/*
 * Now pass the relevant parameters to the KMC
 */
	lpr = (dev>>3)&07;
	if ((dzb_stat&(1<<lpr)) == 0) {
		dzb_kmc[lpr] = dev&0370;
		kmcload(dzb_kmc[lpr]|(dev&07),BASEIN,ubmdev(dzb_addr[lpr]),0);
		dzb_stat |= 1<<lpr;
	}
	kmcload(dzb_kmc[lpr]|(dev&07), SFLAG, tp->t_iflag, tp->t_oflag);
	kmcload(dzb_kmc[lpr]|(dev&07), CMDIN, SETTMR, dzb_tmr[flags&CBAUD]<<8);
}

/*
 * Routine to handle interrupts from the KMC
 * This routine gets called from the KMC driver whenever
 * the KMC generates an unsolicited interrupt (VEC4 == 1)
 *
 * These interrupts are used by the KMC to notify dzb.c
 * of events such as output buffer completions
 *	sel2 = report type
 *	sel4 = residual character count
 *	sel6 = copletion code
 */
dzbkint(dev, sel2, sel4, sel6)
{
	register struct tty *tp;
	register ccode;

	tp = &dzb_tty[dev&077];
	switch(sel2&7) {
	case RBUFOUT:
		sysinfo.rcvint++;
		if (tp->t_rbuf.c_ptr == NULL)
			break;
		ccode = sel6;
		if (ccode&ICFLU)
			tp->t_rbuf.c_count = tp->t_rbuf.c_size;
		else
			tp->t_rbuf.c_count = sel4;
		if (tp->t_state&ISOPEN) {
			(*linesw[tp->t_line].l_input)(tp);
			dzbproc(tp, T_INPUT);
		}
/* else free?? */

		if (ccode&ICBRK) 
			goto brk;
		break;
	case XBUFOUT:
		sysinfo.xmtint++;
		tp->t_tbuf.c_count = sel4;
		tp->t_state &= ~BUSY;
		dzbproc(tp, T_OUTPUT);
		break;
	case RBRK:
		sysinfo.rcvint++;
	brk:
		signal(tp->t_pgrp, SIGINT);
		ttyflush(tp, (FREAD|FWRITE));
		break;
	case XPRSOUT:
		break;
	default:
		printf("dzb: %o %o %o %o\n", dev, sel2, sel4, sel6);
	}
}

dzbproc(tp, cmd)
register struct tty *tp;
{
	register dev;
	register sel4, sel6;

	dev = tp - dzb_tty;
	sel4 = 0;
	switch(cmd) {

	case T_TIME:
		tp->t_state &= ~TIMEOUT;
		goto start;

	case T_WFLUSH:
		sel4 |= OFLUSH;

	case T_RESUME:
		sel4 |= ORSME;
		goto start;

	case T_INPUT:
		/* Pass new input buffer to KMC */
		if (tp->t_rbuf.c_ptr != NULL) {
			paddr_t addr;

			addr = ubmdata(tp->t_rbuf.c_ptr);
			if (kmcload(dzb_kmc[(dev>>3)&07]|(dev&07),
				RBUFIN, loword(addr),
				tp->t_rbuf.c_size<<2 | hiword(addr))<0)
		printf("KMC ERROR ON LOAD\n") /* error on kmcload */ ;
		}
		break;

	case T_OUTPUT:
	start:
		if (tp->t_state&(TIMEOUT|TTSTOP|BUSY))
			break;
		if (!(CPRES&(*linesw[tp->t_line].l_output)(tp)))
			break;
		else {
			paddr_t addr;

			addr = ubmdata(tp->t_tbuf.c_ptr);
			if (kmcload(dzb_kmc[(dev>>3)&07]|(dev&07),
			    XBUFIN, loword(addr),
			    (tp->t_tbuf.c_count)<<2|hiword(addr)) < 0)
				goto start;
			tp->t_state |= BUSY;
		}
		break;

	case T_SUSPEND:
		sel4 |= OSPND;
		break;

	case T_BLOCK:
		tp->t_state |= TBLOCK;
		sel4 |= SCHAR;
		sel6 = CSTOP;
		break;

	case T_RFLUSH:
		sel4 |= IFLUSH;
		if (!(tp->t_state&TBLOCK))
			break;

	case T_UNBLOCK:
		tp->t_state &= ~TBLOCK;
		sel4 |= SCHAR;
		sel6 = CSTART;
		break;

	case T_BREAK:
		sel4 |= SBRK;
		break;
	}
	if (sel4)
		kmcload(dzb_kmc[(dev>>3)&07]|(dev&07), CMDIN, sel4, sel6);
}

dzbmodem(dev, flag)
{
	register struct device *dzbaddr;
	register bit;

	dzbaddr = dzb_addr[(dev>>3)&07];
	bit = 1<<(dev&07);
	if (flag==OFF)
		dzbaddr->dzbdtr &= ~bit;
	else
		dzbaddr->dzbdtr |= bit;
	return(dzbaddr->dzbmsr & bit);
}

dzbscan()
{
	register i;
	register struct device *dzbaddr;
	register struct tty *tp;
	char	bit;

	for (i=0; i<dzb_cnt; i++) {
		tp = &dzb_tty[i];
		if (!(tp->t_state&(ISOPEN|WOPEN)))
			continue;
		dzbaddr = dzb_addr[i>>3];
		bit = 1<<(i&07);
		if (tp->t_cflag&CLOCAL || dzbaddr->dzbmsr&bit) {
			if ((tp->t_state&CARR_ON)==0) {
				wakeup(&tp->t_canq);
				tp->t_state |= CARR_ON;
			}
		} else {
			if (tp->t_state&CARR_ON) {
				if (tp->t_state&ISOPEN) {
					signal(tp->t_pgrp, SIGHUP);
					tp->t_pgrp = 0;
					dzbaddr->dzbdtr &= ~bit;
					ttyflush(tp, (FREAD|FWRITE));
				}
				tp->t_state &= ~CARR_ON;
			}
		}
	}
	timeout(dzbscan, 0, HZ);
}

dzbclr()
{
	register dev;
	register struct tty *tp;

	if (dzb_stat == 0)
		return;
	for (dev = 0; dev < 4; dev++)
		if (dzb_stat&(1<<(dev+8)))
			kmcdclr(dev);
	dzb_stat = 0;
	for (dev = 0; dev < dzb_cnt; dev++) {
		tp = &dzb_tty[dev];
		if ((tp->t_state&(ISOPEN|WOPEN)) == 0)
			continue;
		dzbparam(dzb_kmc[(dev>>3)&07]|(dev&07));
		dzbmodem(dev, ON);
		tp->t_state &= ~BUSY;
		dzbproc(tp, T_INPUT);
		dzbproc(tp, T_OUTPUT);
	}
}
