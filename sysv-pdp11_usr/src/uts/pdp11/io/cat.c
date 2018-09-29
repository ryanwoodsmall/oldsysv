/* @(#)cat.c	1.1 */
/*
 * DR11C driver used for C/A/T
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/tty.h"

struct device *cat_addr[];
int	cat_cnt;

#define	CTPRI	PZERO+9
#define	CTMAX	4
#define	CTHIWAT	80
#define	CTLOWAT	20

struct cat {
	struct	clist	cat_outq;
	int	cat_lock;
} cat[CTMAX];

struct device {
	short	drcsr, drobuf, dribuf;
};

catopen(dev)
register dev;
{
	if (dev >= cat_cnt || dev >= CTMAX || cat[dev].cat_lock) {
		u.u_error = ENXIO;
		return;
	}
	cat[dev].cat_lock++;
	cat_addr[dev]->drcsr |= IENABLE;
}

catclose(dev)
{
	cat[dev].cat_lock = 0;
	catrint(dev);
}

catwrite(dev)
{
	register c;
	register struct cat *cp;

	cp = &cat[dev];
	while ((c=cpass()) >= 0) {
		spl5();
		while (cp->cat_outq.c_cc > CTHIWAT)
			sleep(cp, CTPRI);
		while (putc(c, &cp->cat_outq) < 0)
			delay(100);
		catrint(dev);
		spl0();
	}
}

catrint(dev)
{
	register c;

	register struct cat *cp;
	register struct device *dp;

	dp = cat_addr[dev];
	if (dp->drcsr&DONE) {
		cp = &cat[dev];
		if ((c = getc(&cp->cat_outq)) >= 0) {
			dp->drobuf = c;
			if (cp->cat_outq.c_cc==0 || cp->cat_outq.c_cc==CTLOWAT)
				wakeup(cp);
		} else if (cp->cat_lock==0)
			dp->drcsr = 0;
	}
}
catxint() {}
