/* @(#)du.c	1.1 */
/*
 * DU-11 Synchronous interface driver
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/du.h"

/* device registers */
struct device {
	short	rxcsr, rxdbuf;
	short	txcsr, txdbuf;
};

struct device *du_addr[];
int	du_cnt;

#define	DONE	0200
#define	IE	0100
#define	SIE	040
#define CTS	020000
#define	CARRIER	010000
#define	RCVACT	04000
#define	DSR	01000
#define STRIP	0400
#define SCH	020
#define RTS	04
#define	DTR	02
#define MR	0400
#define SEND	020
#define	HALF	010

#define	READ	0
#define	WRITE	1
#define PWRIT	2

#define	DUPRI	(PZERO+5)

duopen(dev)
{
	register struct du *dp;
	register struct device *lp;

	if (dev >= du_cnt ||
	   ((dp = &du_du[dev])->du_proc!=NULL && dp->du_proc!=u.u_procp)) {
		u.u_error = ENXIO;
		return;
	}
	dp->du_proc = u.u_procp;
	if (dp->du_buf==NULL) {
		lp = du_addr[dev];
		lp->txcsr = MR;
		lp->rxdbuf = 035026;
		dp->du_buf = (char *)getablk(1);
		dp->du_bufb = (char *)loword(paddr(((struct buf *)dp->du_buf)));
		dp->du_bufe = dp->du_bufb+512;
		dp->du_state = WRITE;
		duturn(dev);
	}
}

duclose(dev)
{
	register struct du *dp;
	register struct device *lp;

	dp = &du_du[dev];
	lp = du_addr[dev];
	lp->rxcsr = 0;
	lp->txcsr = 0;
	dp->du_proc = 0;
	if (dp->du_buf != NULL) {
		brelse(dp->du_buf);
		dp->du_buf = NULL;
	}
}

duread(dev)
{
	register char *bp;
	register struct du *dp;
	register n;
	int	flag;

	dp = &du_du[dev];
	bp = dp->du_bufb;
	flag = 0;
	for(;;) {
		if(duwait(dev))
			return;
		spl5();
		if (n = dp->du_nleft) {
			dp->du_nleft = 0;
			spl0();
			flag++;
			while (n--) {
				if (passc(*bp++)<0)
					break;
				if (bp == dp->du_bufe)
					bp = dp->du_bufb;
			}
		} else {
			if ( flag) {
				spl0();
				break;
			}
			sleep((caddr_t)dp, DUPRI);
			spl0();
		}
	}
	dp->du_bufp = dp->du_bufb;
}

duwrite(dev)
{
	register struct du *dp;
	register char *bp,*ep;
	int	n;
	struct device *lp;

	dp = &du_du[dev];
	lp = du_addr[dev];
	if (u.u_count==0 || duwait(dev))
		return;
	dp->du_bufp = ep = bp = dp->du_bufb;
	dp->du_state = PWRIT;
	lp->rxcsr &= ~SCH;
	lp->rxcsr = SIE|RTS|DTR;
	n = 0;
	while (u.u_count!=0) {
		*bp++ = fubyte(u.u_base++);
		u.u_count--;
		n++;
		if (bp==dp->du_bufe)
			bp = dp->du_bufb;
		if (bp==ep) {
			duset(dev, n);
			if (u.u_count!=0) {
				spl5();
				sleep((caddr_t)dp, DUPRI);
				spl0();
			} else
				return;
			ep = dp->du_bufp;
			n = 0;
		}
	}
	duset(dev, n);
}

duwait(dev)
{
	register struct du *dp;
	register struct device *lp;

	dp = &du_du[dev];
	lp = du_addr[dev];
	for(;;) {
		if ((lp->rxcsr&DSR)==0 || dp->du_buf==0) {
			u.u_error = EIO;
			return(1);
		}
		spl5();
		if (dp->du_state==READ &&
			((lp->rxcsr&RCVACT)==0 || dp->du_nleft)) {
			spl0();
			return(0);
		}
		sleep((caddr_t)dp, DUPRI);
		spl0();
	}
}

dustart(dev)
{
	register struct du *dp;
	register struct device *lp;

	dp = &du_du[dev];
	lp = du_addr[dev];
	if (dp->du_nleft > 0) {
		if (--dp->du_nleft==200)
			wakeup(dp);
		lp->txdbuf = *dp->du_bufp++;
		if (dp->du_bufp==dp->du_bufe)
			dp->du_bufp = dp->du_bufb;
	} else {
		duturn(dev);
	}
}

duset(dev, an)
{
	register struct du *dp;
	register struct device *lp;

	dp = &du_du[dev];
	lp = du_addr[dev];
	spl5();
	while((lp->rxcsr&CTS)==0)
		sleep((caddr_t)dp, DUPRI);
	dp->du_nleft += an;
	if (dp->du_state != WRITE) {
		dp->du_state = WRITE;
		lp->txcsr = IE|SIE|SEND|HALF;
		dustart(dev);
	}
	spl0();
}

durint(dev)
{
	register struct du *dp;
	register c, s;
	int	dustat;
	struct device *lp;

	dp = &du_du[dev];
	lp = du_addr[dev];
	dustat = lp->rxcsr;
	if(dustat<0) {
		if((dustat&CARRIER)==0 && dp->du_state==READ)
			duturn(dev); else
			wakeup(dp);
	} else
	if(dustat&DONE) {
		lp->rxcsr = IE|SIE|SCH|DTR;
		c = s = lp->rxdbuf;
		c &= 0177;
		if(s<0)
			c |= 0200;
		if (++dp->du_nleft==384)
			wakeup(dp);
		if (dp->du_bufp == dp->du_bufe)
			dp->du_bufp = dp->du_bufb;
		*dp->du_bufp++ = c;
		if (c == 0377) duturn(dev);
	}
}

duxint(dev)
{
	register struct device *lp;
	register int dustat;

	lp = du_addr[dev];
	dustat = lp->txcsr;
	if(dustat<0)
		duturn(dev); else
	if(dustat&DONE)
		dustart(dev);
}

duturn(dev)
{
	register struct du *dp;
	register struct device *lp;

	dp = &du_du[dev];
	lp = du_addr[dev];
	if (dp->du_state!=READ) {
		dp->du_state = READ;
		dp->du_bufp = dp->du_bufb;
		dp->du_nleft = 0;
	}
	lp->txcsr = HALF;
	lp->rxcsr &= ~SCH;
	lp->rxcsr = STRIP|IE|SIE|SCH|DTR;
	wakeup(dp);
}
