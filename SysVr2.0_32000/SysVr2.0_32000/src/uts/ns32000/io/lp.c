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
/* @(#)lp.c	6.1 */
/*
 *  Line printer driver
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/tty.h"
#include "sys/lprio.h"

extern struct device *lp_addr[];

#define	LPPRI	(PZERO+8)
#define	LPLOWAT	40
#define	LPHIWAT	100

struct device {
	unsigned char	data;
	unsigned char	fil1;
	unsigned char	status;
	unsigned char	fil2;
	unsigned char	ctrl;
	unsigned char	fil3;
	unsigned char	init;
};

#define LPIDLE	0x10	/* lp not busy */
#define NO_PE	0x20	/* no printer error */
#define NOT_SEL	0x40	/* lp not selected */
#define FAULT	0x80	/* out of paper, etc. */
#define SMASK	0xf0	/* status bits */
#define LPREADY	0x30	/* no fault, selected, no pe, not busy */

#define PRIME	0xa
#define POPRIME	0xb
#define LPINIT	0xa2
#define IENA_A	0xd	/* enable ints, port a */
#define IENA_B	0x5	/* enable ints, port b */
#define PIOMASK	0x2000	/* icu mask bits */

struct lp {
	struct	clist l_outq;
	int	flag;
	int	ccc, mcc, mlc;
	int	line, col;
	char	ind;
} lp_dt;

#define	OPEN	010
#define	CAP	020
#define	NOCR	040
#define	ONLCR	0100
#define	ASLP	0200
#define	LPBUSY	0400
#define	LPFLUSH	01000

#define	FORM	014

int	lpinitf = 0;

lpopen(dev, mode)
{
	register struct lp *lp;
	register struct device *rp;

	lp = &lp_dt;
	rp = lp_addr[0];

	if (!lpinitf) {
		rp->init = LPINIT;
		rp->init = IENA_A;
		if ((dev&1) == 0) { /* Printronix */
			rp->init = POPRIME;
			rp->init = PRIME;
		} else {		/* Centronix */
			rp->init = PRIME;
			rp->init = POPRIME;
		}
		lpinitf++;
	}

	/*
	 * If first open let the printer catch up with the driver.
	 */
	if (!(lp->flag)) {
		delay(50);
	}	

	if (lp->flag || (rp->status&(NO_PE|NOT_SEL|FAULT)) != NO_PE) {
		u.u_error = EIO;
		return;
	}
	lp->flag = (dev&0177)|OPEN;
	if (lp->col == 0) {	/* first time only */
		lp->ind = 4;
		lp->col = 132;
		lp->line = 66;
	}
	icuenable(PIOMASK);

	lpoutput(FORM);
}

lpclose(dev)
{
	register struct lp *lp;
	register s;

	lp = &lp_dt;

	lpoutput(FORM);

	/*
	 * drain the device
	 */
	s = spl4();
	if (lp->l_outq.c_cc) {
		lp->flag |= LPFLUSH;
		lpstart();
		while (lp->flag&LPFLUSH)
			sleep((caddr_t)lp, LPPRI);
	}
	splx(s);

	lp->flag = 0;
}

lpwrite(dev)
{
	register c;
	register struct lp *lp;

	lp = &lp_dt;
	while (u.u_count) {
		spl4();
		while(lp->l_outq.c_cc > LPHIWAT) {
			lpstart();
			lp->flag |= ASLP;
			sleep(lp, LPPRI);
		}
		spl0();
		c = fubyte(u.u_base++);
		if (c < 0) {
			u.u_error = EFAULT;
			break;
		}
		u.u_count--;
		lpoutput(c);
	}
	spl4();
	lpstart();
	spl0();
}

lpoutput(c)
register c;
{
	register struct lp *lp;

	lp = &lp_dt;
	if(lp->flag&CAP) {
		if(c>='a' && c<='z')
			c += 'A'-'a'; else
		switch(c) {
		case '{':
			c = '(';
			goto esc;
		case '}':
			c = ')';
			goto esc;
		case '`':
			c = '\'';
			goto esc;
		case '|':
			c = '!';
			goto esc;
		case '~':
			c = '^';
		esc:
			lpoutput(c);
			lp->ccc--;
			c = '-';
		}
	}
	switch(c) {
	case '\t':
		lp->ccc = ((lp->ccc+8-lp->ind) & ~7) + lp->ind;
		return;
	case '\n':
		lp->mlc++;
		if (lp->mlc >= lp->line )
			c = FORM;
		if (lp->flag&ONLCR)
			putc('\r', &lp->l_outq);
	case FORM:
		lp->mcc = 0;
		if (lp->mlc) {
			putc(c, &lp->l_outq);
			if(c == FORM)
				lp->mlc = 0;
		}
	case '\r':
		lp->ccc = lp->ind;
		spl4();
		lpstart();
		spl0();
		return;
	case 010:
		if(lp->ccc > lp->ind)
			lp->ccc--;
		return;
	case ' ':
		lp->ccc++;
		return;
	default:
		if(lp->ccc < lp->mcc) {
			if (lp->flag&NOCR) {
				lp->ccc++;
				return;
			}
			putc('\r', &lp->l_outq);
			lp->mcc = 0;
		}
		if(lp->ccc < lp->col) {
			while(lp->ccc > lp->mcc) {
				putc(' ', &lp->l_outq);
				lp->mcc++;
			}
			putc(c, &lp->l_outq);
			lp->mcc++;
		}
		lp->ccc++;
	}
}

lpxint()
{

	lp_dt.flag &= ~LPBUSY;
	lpstart();
}

lpstart()
{
	register struct lp *lp;
	register struct device *rp;
	register c;

	lp = &lp_dt;
	rp = lp_addr[0];

	if (lp->flag&LPBUSY)
		return;

	if (rp->status&LPIDLE && (c = getc(&lp->l_outq)) >= 0) {
		rp->data = c;
		c = rp->ctrl;
		lp->flag |= LPBUSY;
	}

	if (lp->l_outq.c_cc <= LPLOWAT && lp->flag&ASLP) {
		lp->flag &= ~ASLP;
		wakeup(lp);
	}
	if (lp->flag&LPFLUSH && lp->l_outq.c_cc == 0) {
		lp->flag &= ~LPFLUSH;
		wakeup(lp);
	}
}

lpioctl(dev, cmd, arg, mode)
{
	register struct lp *lp;
	struct lprio lpr;

	lp = &lp_dt;
	switch(cmd) {
	case LPRGET:
		lpr.ind = lp->ind;
		lpr.col = lp->col;
		lpr.line = lp->line;
		if (copyout(&lpr, arg, sizeof lpr))
			u.u_error = EFAULT;
		break;
	case LPRSET:
		if (copyin(arg, &lpr, sizeof lpr)) {
			u.u_error = EFAULT;
			break;
		}
		lp->ind = lpr.ind;
		lp->col = lpr.col;
		lp->line = lpr.line;
		break;
	default:
		u.u_error = EINVAL;
	}
}

/* lp error interrupt - should never happed */
lprint()
{
	printf("lp: got eintr\n");
}
