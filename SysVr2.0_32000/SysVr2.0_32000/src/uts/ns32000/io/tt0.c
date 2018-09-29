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
/* @(#)tt0.c	6.3 */
/*
 * Line discipline 0
 * Includes Terminal Handling
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/conf.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/tty.h"
#include "sys/termio.h"
#include "sys/crtctl.h"
#include "sys/sysinfo.h"

extern char partab[];

char colsave, rowsave;		/* temp save for high queue restore */
struct clist tempq;		/* temp for echo during high queue  */

/*
 * routine called on first teletype open.
 * establishes a process group for distribution
 * of quits and interrupts from the tty.
 */
ttopen(tp)
register struct tty *tp;
{
	register struct proc *pp;

	pp = u.u_procp;
	if ((pp->p_pid == pp->p_pgrp)
	 && (u.u_ttyp == NULL)
	 && (tp->t_pgrp == 0)) {
		u.u_ttyp = &tp->t_pgrp;
		tp->t_pgrp = pp->p_pgrp;
	}
	ttioctl(tp, LDOPEN, 0, 0);
	tp->t_state &= ~WOPEN;
	tp->t_state |= ISOPEN;
}

ttclose(tp)
register struct tty *tp;
{
	if ((tp->t_state&ISOPEN) == 0)
		return;
	tp->t_state &= ~ISOPEN;
	tp->t_pgrp = 0;
	ttioctl(tp, LDCLOSE, 0, 0);
}

/*
 * Called from device's read routine after it has
 * calculated the tty-structure given as argument.
 */
ttread(tp)
register struct tty *tp;
{
	register struct clist *tq;

	tq = &tp->t_canq;

	if (tq->c_cc == 0)
		canon(tp);
	while (u.u_count!=0 && u.u_error==0) {
		if (u.u_count >= CLSIZE) {
			register n;
			register struct cblock *cp;

			if ((cp = getcb(tq)) == NULL)
				break;
			n = min(u.u_count, cp->c_last - cp->c_first);
			if (copyout(&cp->c_data[cp->c_first], u.u_base, n))
				u.u_error = EFAULT;
			putcf(cp);
			u.u_base += n;
			u.u_count -= n;
		} else {
			register c;

			if ((c = getc(tq)) < 0)
				break;
			if (subyte(u.u_base++, c))
				u.u_error = EFAULT;
			u.u_count--;
		}
	}
	if (tp->t_state&TBLOCK) {
		if (tp->t_rawq.c_cc<TTXOLO) {
			(*tp->t_proc)(tp, T_UNBLOCK);
		}
	}
}

/*
 * Called from device's write routine after it has
 * calculated the tty-structure given as argument.
 */
ttwrite(tp)
register struct tty *tp;
{
	int hqflag;
	int col, row;

qwait:
	spl5();
	while(tp->t_tmflag & QLOCKB) {
		tp->t_state |= OASLP;
		sleep((caddr_t)&tp->t_outq, TTOPRI);
	}
	spl0();
	if (!(tp->t_state&CARR_ON))
		return;
	hqflag = 0;
	while (u.u_count) {
		if (tp->t_outq.c_cc > tthiwat[tp->t_cflag&CBAUD]) {
			if (hqflag) {
				col = tp->t_col;
				row = tp->t_row;
				hqrelse(tp);
			}
			spl5();
			(*tp->t_proc)(tp, T_OUTPUT);
			while (tp->t_outq.c_cc > tthiwat[tp->t_cflag&CBAUD]) {
				tp->t_state |= OASLP;
				if (sleep((caddr_t)&tp->t_outq,
					hqflag ? PZERO : (TTOPRI|PCATCH))) {
					spl0();
					goto out;
				}
			}
			spl0();
			if (hqflag) {
				tp->t_tmflag |= QLOCKI;
				colsave = tp->t_col;
				rowsave = tp->t_row;
				ttyctl(LCA, tp, col, row);
				continue;
			}
			if (tp->t_tmflag & QLOCKB)
				goto qwait;
		}
		if (u.u_count >= (CLSIZE/2) && tp->t_term == 0) {
			register n;
			register struct cblock *cp;

			if ((cp = getcf()) == NULL)
				break;
			n = min(u.u_count, cp->c_last);
			if (copyin(u.u_base, cp->c_data, n)) {
				u.u_error = EFAULT;
				putcf(cp);
				break;
			}
			u.u_base += n;
			u.u_count -= n;
			cp->c_last = n;
			ttxput(tp, cp, n);
		} else {
			register c;

			c = fubyte(u.u_base++);
			if (c<0) {
				u.u_error = EFAULT;
				break;
			}
			u.u_count--;
			if (c == ESC && tp->t_term) {
				switch (c = cpass()) {
					int col;

				case -1:
					continue;
				case ESC:
					goto norm;
				case HIQ:
					if (hqflag++)
						continue;
					tp->t_tmflag |= QLOCKB|QLOCKI;
					tp->t_hqcnt++;
					colsave = tp->t_col;
					rowsave = tp->t_row;
					continue;
				case LCA:
				case SVID:
				case DVID:
				case CVID:
					col = cpass();
				default:
					ttyctl(c, tp, col, c==LCA ? cpass() : 0);
				}
			} else {
norm:
				ttxput(tp, c, 0);
			}
		}
	}
	if (hqflag) {
		hqrelse(tp);
		putc(QESC, &tp->t_outq);
		putc(HQEND, &tp->t_outq);
		spl5();
		if (tp->t_state & OASLP) {
			tp->t_state &= ~OASLP;
			wakeup((caddr_t)&tp->t_outq);
		}
		spl0();
	}
out:
	tp->t_tmflag &= ~(QLOCKB|QLOCKI);
	spl5();
	(*tp->t_proc)(tp, T_OUTPUT);
	spl0();
}

/*
 * Place a character on raw TTY input queue, putting in delimiters
 * and waking up top half as needed.
 * Also echo if required.
 */
#define	LCLESC	0400

ttin(tp, code)
register struct tty *tp;
{
	register c;
	register flg;
	register char *cp;
	ushort nchar, nc;


	if (code == L_BREAK){
		signal(tp->t_pgrp, SIGINT);
		ttyflush(tp, (FREAD|FWRITE));
		return;
	}
	nchar = tp->t_rbuf.c_size - tp->t_rbuf.c_count;
	/* reinit rx control block */
	tp->t_rbuf.c_count = tp->t_rbuf.c_size;
	if (nchar==0)
		return;
	flg = tp->t_iflag;
	/* KMC does all but IXOFF */
	if (tp->t_state&EXTPROC)
		flg &= IXOFF;
	nc = nchar;
	cp = tp->t_rbuf.c_ptr;
	if (nc < cfreelist.c_size || (flg & (INLCR|IGNCR|ICRNL|IUCLC))
		|| tp->t_term) {
			/* must do per character processing */
		for ( ;nc--; cp++) {
			c = *cp;
			if (tp->t_term) {
				c &= 0177;
				if ((c = (*termsw[tp->t_term].t_input) (c,tp))
					== -1)
					continue;
				if (c & CPRES) {
					putc(ESC, &tp->t_rawq);
					putc(c, &tp->t_rawq);
					continue;
				}
			}
			if (c == '\n' && flg&INLCR)
				*cp = c = '\r';
			else if (c == '\r')
				if (flg&IGNCR)
					continue;
				else if (flg&ICRNL)
					*cp = c = '\n';
			if (flg&IUCLC && 'A' <= c && c <= 'Z')
				c += 'a' - 'A';
			if (putc(c, &tp->t_rawq))
				continue;
			sysinfo.rawch++;
		}
		cp = tp->t_rbuf.c_ptr;
	} else {
		/* may do block processing */
		putcb(CMATCH((struct cblock *)cp), &tp->t_rawq);
		sysinfo.rawch += nc;
		/* allocate new rx buffer */
		if ((tp->t_rbuf.c_ptr = getcf()->c_data)
			== ((struct cblock *)NULL)->c_data) {
			tp->t_rbuf.c_ptr = NULL;
			return;
		}
		tp->t_rbuf.c_count = cfreelist.c_size;
		tp->t_rbuf.c_size = cfreelist.c_size;
	}


	if (tp->t_rawq.c_cc > TTXOHI) {
		if (flg&IXOFF && !(tp->t_state&TBLOCK))
			(*tp->t_proc)(tp, T_BLOCK);
		if (tp->t_rawq.c_cc > TTYHOG) {
			ttyflush(tp, FREAD);
			return;
		}
	}
	flg = lobyte(tp->t_lflag);
	if (tp->t_outq.c_cc > (tthiwat[tp->t_cflag&CBAUD] + TTECHI))
		flg &= ~(ECHO|ECHOK|ECHONL|ECHOE);
	if (flg) while (nchar--) {
		c = *cp++;
		if (flg&ISIG) {
			if (c == tp->t_cc[VINTR]) {
				signal(tp->t_pgrp, SIGINT);
				if (!(flg&NOFLSH) && tp->t_hqcnt==0)
					ttyflush(tp, (FREAD|FWRITE));
				continue;
			}
			if (c == tp->t_cc[VQUIT]) {
				signal(tp->t_pgrp, SIGQUIT);
				if (!(flg&NOFLSH) && tp->t_hqcnt==0)
					ttyflush(tp, (FREAD|FWRITE));
				continue;
			}
			if (c == tp->t_cc[VSWTCH]) {
				if (!(flg&NOFLSH) && tp->t_hqcnt==0)
					ttyflush(tp, FREAD);
				(*tp->t_proc)(tp, T_SWTCH);
				continue;
			}
		}
		if (flg&ICANON) {
			if (tp->t_state&CLESC) {
				flg |= LCLESC;
				tp->t_state &= ~CLESC;
			}
			if (c == '\n') {
				if (flg&ECHONL)
					flg |= ECHO;
				tp->t_delct++;
			} else if (c == '\\') {
				tp->t_state |= CLESC;
				if (flg&XCASE) {
					c |= QESC;
					if (flg&LCLESC)
						tp->t_state &= ~CLESC;
				}
			} else if (c == tp->t_cc[VEOL] || c == tp->t_cc[VEOL2])
				tp->t_delct++;
			else if (!(flg&LCLESC)) {
				if (c == tp->t_cc[VERASE] && flg&ECHOE) {
					if (flg&ECHO)
						ttxputi(tp, '\b');
					flg |= ECHO;
					ttxputi(tp, ' ');
					c = '\b';
				} else if (c == tp->t_cc[VKILL] && flg&ECHOK) {
					if (flg&ECHO)
						ttxputi(tp, c);
					flg |= ECHO;
					c = '\n';
				} else if (c == tp->t_cc[VEOF]) {
					flg &= ~ECHO;
					tp->t_delct++;
				}
			}
		}
		if (flg&ECHO) {
			ttxputi(tp, c);
			(*tp->t_proc)(tp, T_OUTPUT);
		}
	}
	if (!(flg&ICANON)) {
		tp->t_state &= ~RTO;
		if (tp->t_rawq.c_cc >= tp->t_cc[VMIN])
			tp->t_delct = 1;
		else if (tp->t_cc[VTIME]) {
			if (!(tp->t_state&TACT))
				tttimeo(tp);
		}
	}
	if (tp->t_delct && (tp->t_state&IASLP)) {
		tp->t_state &= ~IASLP;
		wakeup((caddr_t)&tp->t_rawq);
	}
}

/*
 * Interrupt interface to ttxput.
 * Checks for High Queue write in progress, and saves characters to be echoed.
 */
ttxputi(tp, c)
register struct tty *tp;
{
	if (tp->t_tmflag & QLOCKI) {
		putc(c,&tempq);
		return;
	} else
		ttxput(tp, c, 0);
}

/*
 * Put character(s) on TTY output queue, adding delays,
 * expanding tabs, and handling the CR/NL bit.
 * It is called both from the base level for output, and from
 * interrupt level for echoing.
 */
ttxput(tp, ucp, ncode)
register struct tty *tp;
union {
	ushort	ch;
	struct cblock *ptr;
} ucp;
{
	register c;
	register flg;
	register unsigned char *cp;
	register char *colp;
	int ctype;
	int cs;
	struct cblock *scf;

	/* KMC does all but XCASE, virt term needs CR info for t_col */
	if (tp->t_state&EXTPROC) {
		if (tp->t_term || tp->t_lflag&XCASE)
			flg = tp->t_oflag&(OPOST|OLCUC|ONLRET|ONLCR);
		else
			flg = 0;
	} else
		flg = tp->t_oflag;
	if (ncode == 0) {
		ncode++;
		if (!(flg&OPOST)) {
			sysinfo.outch++;
			putc(ucp.ch, &tp->t_outq);
			return;
		}
		cp = (unsigned char *)&ucp.ch;
		scf = NULL;
	} else {
		if (!(flg&OPOST)) {
			sysinfo.outch += ncode;
			putcb(ucp.ptr, &tp->t_outq);
			return;
		}
		cp = (unsigned char *)&ucp.ptr->c_data[ucp.ptr->c_first];
		scf = ucp.ptr;
	}
	while (ncode--) {
		c = *cp++;
		if (c >= 0200) {
	/* spl5-0 */
			if (c == QESC && !(tp->t_state&EXTPROC))
				putc(QESC, &tp->t_outq);
			sysinfo.outch++;
			putc(c, &tp->t_outq);
			continue;
		}
		/*
		 * Generate escapes for upper-case-only terminals.
		 */
		if (tp->t_lflag&XCASE) {
			colp = "({)}!|^~'`\\\\";
			while(*colp++)
				if (c == *colp++) {
					ttxput(tp, '\\'|0200, 0);
					c = colp[-2];
					break;
				}
			if ('A' <= c && c <= 'Z')
				ttxput(tp, '\\'|0200, 0);
		}
		if (flg&OLCUC && 'a' <= c && c <= 'z')
			c += 'A' - 'a';
		cs = c;
		/*
		 * Calculate delays.
		 * The numbers here represent clock ticks
		 * and are not necessarily optimal for all terminals.
		 * The delays are indicated by characters above 0200.
		 */
		ctype = partab[c];
		colp = &tp->t_col;
		c = 0;
		switch (ctype&077) {

		case 0:	/* ordinary */
			(*colp)++;

		case 1:	/* non-printing */
			break;

		case 2:	/* backspace */
			if (flg&BSDLY)
				c = 2;
			if (*colp)
				(*colp)--;
			break;

		case 3:	/* line feed */
top:
			if (tp->t_term) {
				if (tp->t_vrow && tp->t_row >= tp->t_lrow) {
					ttyctl(UVSCN, tp);
					continue;
				}
				if (tp->t_tmflag & SNL) {
					ttyctl(NL, tp);
					continue;
				}
			}
			if (flg&ONLRET)
				goto cr;
			if (tp->t_row < tp->t_lrow)
				tp->t_row++;
			if (flg&ONLCR) {
				if ((!(tp->t_state&EXTPROC)) &&
					!(flg&ONOCR && *colp==0)) {
					sysinfo.outch++;
					putc('\r', &tp->t_outq);
				}
				goto cr;
			}
		nl:
			if (flg&NLDLY)
				c = 5;
			break;

		case 4:	/* tab */
			c = 8 - ((*colp)&07);
			*colp += c;
			if (!(tp->t_state&EXTPROC)) {
				ctype = flg&TABDLY;
				if (ctype == TAB0) {
					c = 0;
				} else if (ctype == TAB1) {
					if (c < 5)
						c = 0;
				} else if (ctype == TAB2) {
					c = 2;
				} else if (ctype == TAB3) {
					sysinfo.outch += c;
					do
						putc(' ', &tp->t_outq);
					while (--c);
					continue;
				}
			}
			break;

		case 5:	/* vertical tab */
			if (flg&VTDLY)
				c = 0177;
			break;

		case 6:	/* carriage return */
			if (flg&OCRNL) {
				cs = '\n';
				goto nl;
			}
			if (flg&ONOCR && *colp == 0)
				continue;
		cr:
			if (!(tp->t_state&EXTPROC)) {
				ctype = flg&CRDLY;
				if (ctype == CR1) {
					if (*colp)
						c = max((*colp>>4) + 3, 6);
				} else if (ctype == CR2) {
					c = 5;
				} else if (ctype == CR3) {
					c = 9;
				}
			}
			*colp = 0;
			break;

		case 7:	/* form feed */
			if (flg&FFDLY)
				c = 0177;
			break;
		}
		sysinfo.outch++;
		if (tp->t_term && *colp >= 80 && tp->t_row >= tp->t_lrow
			&& tp->t_tmflag & LCF) {
			ttyctl(VHOME, tp);
			ttyctl(DL, tp);
			ttyctl(LCA, tp, 79, tp->t_lrow-1);
			(*colp)++;
		}
                if (tp->t_term==0)
			putc(cs, &tp->t_outq);
		else
			qputc(cs, &tp->t_outq);
		if (!(tp->t_state&EXTPROC)) {
			if (c) {
				if ((c < 32) && flg&OFILL) {
					if (flg&OFDEL)
						cs = 0177;
					else
						cs = 0;
					putc(cs, &tp->t_outq);
					if (c > 3)
						putc(cs, &tp->t_outq);
				} else {
					putc(QESC, &tp->t_outq);
					putc(c|0200, &tp->t_outq);
				}
			}
		}
		if (*colp >= 80 && tp->t_term && tp->t_tmflag&ANL)
			if (tp->t_tmflag&LCF)
				ttyctl(LCA, tp, 0, tp->t_row+1);
			else {
				if ((flg&ONLCR) == 0)
					ttxputi(tp,'\r');
				cs = '\n';
				goto top;
			}
	}
	if (scf != NULL)
		putcf(scf);
}

/*
 * Get next packet from output queue.
 * Called from xmit interrupt complete.
 */

ttout(tp)
register struct tty *tp;
{
	register struct ccblock *tbuf;
	register c;
	register char *cptr;
	register retval;

	extern ttrstrt();

	if (tp->t_state&TTIOW && tp->t_outq.c_cc==0) {
		tp->t_state &= ~TTIOW;
		wakeup((caddr_t)&tp->t_oflag);
	}
delay:
	tbuf = &tp->t_tbuf;
	if (hibyte(tp->t_lflag)) {
		if (tbuf->c_ptr) {
			putcf(CMATCH((struct cblock *)tbuf->c_ptr));
			tbuf->c_ptr = NULL;
		}
		tp->t_state |= TIMEOUT;
		timeout(ttrstrt, tp, (hibyte(tp->t_lflag)&0177)+6);
		hibyte(tp->t_lflag) = 0;
		return(0);
	}
	retval = 0;
	if (((tp->t_state&EXTPROC) || (!(tp->t_oflag&OPOST))) &&
		tp->t_term==0) {
		if (tbuf->c_ptr)
			putcf(CMATCH((struct cblock *)tbuf->c_ptr));
		if ((tbuf->c_ptr = (char *)getcb(&tp->t_outq)) == NULL)
			return(0);
		tbuf->c_count = ((struct cblock *)tbuf->c_ptr)->c_last -
				((struct cblock *)tbuf->c_ptr)->c_first;
		tbuf->c_size = tbuf->c_count;
		tbuf->c_ptr = &((struct cblock *)tbuf->c_ptr)->c_data
				[((struct cblock *)tbuf->c_ptr)->c_first];
		retval = CPRES;
	} else {			/* watch for timing	*/
		if (tbuf->c_ptr == NULL) {
			if ((tbuf->c_ptr = getcf()->c_data)
				== ((struct cblock *)NULL)->c_data) {
				tbuf->c_ptr = NULL;
				return(0);	/* Add restart? */
			}
		}
		tbuf->c_count = 0;
		cptr = tbuf->c_ptr;
		while ((c=getc(&tp->t_outq)) >= 0) {
			if (c == QESC) {
				if ((c = getc(&tp->t_outq)) < 0)
					break;
				if (c == HQEND) {
					if (tp->t_term)
						tp->t_hqcnt--;
					continue;
				}
				if (c > 0200 && !(tp->t_state&EXTPROC)) {
					hibyte(tp->t_lflag) = c;
					if (!retval)
						goto delay;
					break;
				}
			}
			retval = CPRES;
			*cptr++ = c;
			tbuf->c_count++;
			if (tbuf->c_count >= cfreelist.c_size)
				break;
		}
		tbuf->c_size = tbuf->c_count;
	}

	if (tp->t_state&OASLP &&
		tp->t_outq.c_cc<=ttlowat[tp->t_cflag&CBAUD]) {
		tp->t_state &= ~OASLP;
		wakeup((caddr_t)&tp->t_outq);
	}
	return(retval);
}

tttimeo(tp)
register struct tty *tp;
{
	tp->t_state &= ~TACT;
	if (tp->t_lflag&ICANON || tp->t_cc[VTIME] == 0)
		return;
	if (tp->t_rawq.c_cc == 0 && tp->t_cc[VMIN])
		return;
	if (tp->t_state&RTO) {
		tp->t_delct = 1;
		if (tp->t_state&IASLP) {
			tp->t_state &= ~IASLP;
			wakeup((caddr_t)&tp->t_rawq);
		}
	} else {
		tp->t_state |= RTO|TACT;
		timeout(tttimeo, tp, tp->t_cc[VTIME]*(HZ/10));
	}
}

/*
 * I/O control interface
 */
ttioctl(tp, cmd, arg, mode)
register struct tty *tp;
{
	ushort	chg;
	struct termcb termblk;

	switch(cmd) {
	case LDOPEN:
		if (tp->t_rbuf.c_ptr == NULL) {
			/* allocate RX buffer */
			while((tp->t_rbuf.c_ptr = getcf()->c_data)
				== ((struct cblock *)NULL)->c_data) {
				tp->t_rbuf.c_ptr = NULL;
				cfreelist.c_flag = 1;
				sleep(&cfreelist, TTOPRI);
			}
			tp->t_rbuf.c_count = cfreelist.c_size;
			tp->t_rbuf.c_size  = cfreelist.c_size;
			(*tp->t_proc)(tp, T_INPUT);
		}
		break;

	case LDCLOSE:
		spl5();
		(*tp->t_proc)(tp, T_RESUME);
		spl0();
		ttywait(tp);
		ttyflush(tp, (FREAD|FWRITE));
		if (tp->t_tbuf.c_ptr) {
			putcf(CMATCH((struct cblock *)tp->t_tbuf.c_ptr));
			tp->t_tbuf.c_ptr = NULL;
			tp->t_tbuf.c_count = 0;
			tp->t_tbuf.c_size = 0;
		}
		if (tp->t_rbuf.c_ptr) {
			putcf(CMATCH((struct cblock *)tp->t_rbuf.c_ptr));
			tp->t_rbuf.c_ptr = NULL;
			tp->t_rbuf.c_count = 0;
			tp->t_rbuf.c_size = 0;
		}
		tp->t_tmflag = 0;
		break;

	case LDCHG:
		chg = tp->t_lflag^arg;
		if (!(chg&ICANON))
			break;
		spl5();
		if (tp->t_canq.c_cc) {
			if (tp->t_rawq.c_cc) {
				tp->t_canq.c_cc += tp->t_rawq.c_cc;
				tp->t_canq.c_cl->c_next = tp->t_rawq.c_cf;
				tp->t_canq.c_cl = tp->t_rawq.c_cl;
			}
			tp->t_rawq = tp->t_canq;
			tp->t_canq = ttnulq;
		}
		tp->t_delct = tp->t_rawq.c_cc;
		spl0();
		break;

	case LDGETT:
		termblk.st_flgs = tp->t_tmflag;
		termblk.st_termt = tp->t_term;
		termblk.st_crow = tp->t_row;
		termblk.st_ccol = tp->t_col;
		termblk.st_vrow = tp->t_vrow;
		termblk.st_lrow = tp->t_lrow;
		if (copyout((caddr_t)&termblk, arg, sizeof(termblk)))
			u.u_error = EFAULT;
		break;

	case LDSETT:
		if (copyin(arg, (caddr_t)&termblk, sizeof(termblk))) {
			u.u_error = EFAULT;
			break;
		}
		if ((unsigned)termblk.st_termt >= termcnt) {
			u.u_error = ENXIO;
			break;
		}
		if (termblk.st_termt) {
			(*termsw[termblk.st_termt].t_ioctl)
			    (tp,
			    tp->t_term==termblk.st_termt ? LDCHG : LDOPEN,
			    termblk.st_vrow);
			if (u.u_error)
				break;
			tp->t_vrow = termblk.st_vrow;
			tp->t_term = termblk.st_termt;
			if (termblk.st_flgs&TM_SET)
				tp->t_tmflag = termblk.st_flgs & ~TM_SET;
		} else {
			tp->t_term = 0;
		}
		tp->t_state &= ~CLESC;
		break;
	default:
		break;
	}
}

/************** ADDITIONS FOR TERMINAL HANDLERS **********************/

/*
 * release the high priority queue for interrupts.
 * copy over any received characters while queue was locked.
 */
hqrelse(tp)
register struct tty *tp;
{
	register c;

	ttyctl(LCA, tp, colsave, rowsave);
	spl5();
	while((c = getc(&tempq)) >= 0)
		ttxput(tp, c, 0);
	tp->t_tmflag &= ~QLOCKI;
	spl0();
}


/*
 * put a character on the output queue,
 * checking first to see if it is a ESC.
 */
qputc(c, qp)
register c;
register qp;
{
	if (c == ESC)
		putc(c, qp);
	putc(c, qp);
}

/* simulate Up Variable SCreeN as common routine */
ttuvscn(tp)
register struct tty *tp;
{
	ttyctl(VHOME, tp);
	ttyctl(DL, tp);
	ttyctl(LCA, tp, 0, tp->t_lrow);
}

/* simulate Down Variable SCreeN as common routine */
ttdvscn(tp)
register struct tty *tp;
{
	ttyctl(VHOME, tp);
	ttyctl(IL, tp);
}

char colpres, rowpres;

ttyctl(ac, tp, acol, arow)
register struct tty *tp;
{
	register char *colp;
	register c;
	int sps;

	c = ac;
	colp = &tp->t_col;
	sps = spl5();

	colpres = *colp;
	rowpres = tp->t_row;
	switch(c) {
	case CUP:
	case DSCRL:
		if (tp->t_row == 0)
			goto out;
		tp->t_row--;
		break;
	case CDN:
	case USCRL:
		if (tp->t_row >= tp->t_lrow)
			goto out;
		tp->t_row++;
		break;
	case UVSCN:
		*colp = 0;
		tp->t_row = tp->t_lrow;
		break;
	case DVSCN:
		*colp = 0;
		tp->t_row = tp->t_vrow;
		break;
	case CRI:
	case STB:
	case SPB:
		if (*colp >= 79)
			goto out;
		(*colp)++;
		break;
	case CLE:
		if (*colp == 0)
			goto out;
		(*colp)--;
		break;
	case HOME:
	case CS:
	case CM:
		tp->t_row = 0;
	case DL:
	case IL:
		*colp = 0;
		break;
	case VHOME:
		*colp = 0;
		tp->t_row = tp->t_vrow;
		c = LCA;
		break;
	case LCA:
		*colp = acol;
		tp->t_row = arow;
		break;
	case ASEG:
		tp->t_row = (tp->t_row+24)%(tp->t_lrow+1);
		break;
	case NL:
		if (tp->t_row < tp->t_lrow)
			tp->t_row++;
	case CRTN:
		*colp = 0;
		break;
	case SVID:
		tp->t_dstat |= acol;
		c = DVID;
		break;
	case CVID:
		tp->t_dstat &= ~acol;
		c = DVID;
		break;
	case DVID:
		tp->t_dstat = acol;
		break;
	}
	(*termsw[tp->t_term].t_output)(c, tp);
    out:
	splx(sps);
}
