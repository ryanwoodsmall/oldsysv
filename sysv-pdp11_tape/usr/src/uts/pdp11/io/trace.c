/*	trace.c 1.5 of 3/1/82
	@(#)trace.c	1.5
 */


#include "sys/param.h"
#ifndef CBUNIX
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/errno.h"
#endif
#include "sys/dir.h"
#ifdef u3b
#include "sys/istk.h"
#endif
#include "sys/user.h"
#include "sys/systm.h"
#ifdef CBUNIX
#include "sys/userx.h"
#endif
#include "sys/tty.h"
#include "sys/trace.h"
#include "sys/csi.h"
#include "sys/vpmt.h"
#define OPEN	01
#define TRSLEEP	04
#define TRQMAX	1024
#define NIL	0377
#define TRPRI	(PZERO + 3)

#ifdef CBUNIX
#define CBMINOR(DEV)=minor(DEV)
#else
#define CBMINOR(DEV)
#endif
extern int tr_cnt;
extern struct trace trace[];

tropen(dev)
{
	register struct trace *tp;

	CBMINOR(dev);

	if (dev >= tr_cnt) {
		u.u_error = ENXIO;
		return;
	}
	tp = &trace[dev];
	if (tp->tr_state&OPEN) {
		u.u_error = EACCES;
		return;
	}
	tp->tr_chno = NIL;
	tp->tr_state |= OPEN;
}
trioctl(dev, cmd, arg, mode)
{
	register struct trace *tp;

	CBMINOR(dev);

	tp = &trace[dev];
	switch(cmd) {
	case VPMTRCO:
		tp->tr_chbits |= (01<<(int)arg);
		return;
	case VPMGETC:
		arg = tp->tr_chbits;
		return;
	case VPMSETC:
		tp->tr_chbits |= arg;
		return;
	case VPMCLRC:
		tp->tr_chbits &= ~(short)arg;
		return;
	default:
		u.u_error = EINVAL;
		return;
	}
}
trclose(dev)
{
	register struct trace *tp;

	CBMINOR(dev);

	tp = &trace[dev];
	tp->tr_chbits = 0;
	tp->tr_ct = 0;
	tp->tr_chno = 0;
	tp->tr_rcnt = 0;
	while (getc(&tp->tr_outq)>=0);
	tp->tr_state = 0;
}
trread(dev)
{
	register struct trace *tp;

	CBMINOR(dev);
	tp = &trace[dev];
	spl5();
	tp->tr_state |= TRSLEEP;
	while (tp->tr_rcnt == 0)
		sleep((caddr_t)&tp->tr_rcnt, TRPRI);
	spl0();
	while (u.u_count && tp->tr_rcnt) {
		if (tp->tr_chno == NIL) {
			tp->tr_chno = getc(&tp->tr_outq);
			tp->tr_ct = getc(&tp->tr_outq);
		}
		if (u.u_count < (tp->tr_ct + 2))
			return;
		passc(tp->tr_chno);
		passc(tp->tr_ct);
		while (tp->tr_ct--)
			passc(getc(&tp->tr_outq));
		tp->tr_chno = NIL;
		tp->tr_rcnt--;
	}
}
trsave(dev, chno, buf, ct)
char *buf, dev, chno, ct;
{

	register struct trace *tp;
	register int n;
	register char *cpt;

	if (dev >= tr_cnt)
		return;
	tp = &trace[dev];
	ct &= 0377;
	if ((tp->tr_chbits&(1<<chno)) == 0)
		return;
	if ((tp->tr_outq.c_cc + ct + 2 + sizeof(lbolt)) >TRQMAX)
		return;
	putc(chno, &tp->tr_outq);
	putc(ct + sizeof(lbolt), &tp->tr_outq);
	cpt = (char *)&lbolt;
	for (n = 0; n < sizeof(lbolt); ++n)
		putc(*cpt++, &tp->tr_outq);
	for (n=0;n<ct;n++)
		putc(buf[n], &tp->tr_outq);
	tp->tr_rcnt++;
	if (tp->tr_state&TRSLEEP) {
		tp->tr_state &= ~TRSLEEP;
		wakeup((caddr_t)&tp->tr_rcnt);
	}
}
