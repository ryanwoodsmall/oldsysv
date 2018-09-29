/* @(#)da.c	1.1 */
/*
 *	DA11 UNIBUS link
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/proc.h"
#include "sys/da.h"
#include "sys/systm.h"

#define GO	01
#define OM	02
#define OD	04
#define OIR	010
#define IE	0100
#define CYCLE	0400
#define IM	01000
#define ID	02000

#define DAPRI	1+PZERO

struct device *da_addr[];
int	da_cnt;

struct device {
	short	wc, ba, cs, db;
};

daopen(dev)
{
	register struct da *dp;
	register struct device *lp;

	if (dev>=da_cnt ||
	    ((dp = &da_da[dev])->da_proc!=0 && dp->da_proc!=u.u_procp)) {
		u.u_error=ENXIO;
		return;
	}
	dp->da_proc = u.u_procp;
	if(dp->da_bp)	return;
	dp->da_cnt = dp->da_flags = 0;
	lp = da_addr[dev];
	lp->cs = IE;	/* expect interrupt */
	dp->da_bp = getablk(1);
	if (dp->da_db==0)	dp->da_db++;
}

daclose(dev)
{
	register struct da *dp;
	register struct device *lp;

	dp = &da_da[dev];
	lp = da_addr[dev];
	if(dp->da_bp!=0) brelse(dp->da_bp);
	dp->da_bp = (struct buf *)0;
	dp->da_flags = 0;
	dp->da_proc = (struct proc *)0;
	lp->db = 0;
	lp->cs = IE;
	lp->cs = OIR|OM;
}

daread(dev)
{
	register struct da *dp;
	register struct device *lp;
	register i;

	dp = &da_da[dev];
	if(dp->da_flags) {
		u.u_error = EIO;
		return;
	}
	dp->da_flags++;
	lp = da_addr[dev];
	while(u.u_count && u.u_error==0) {
		if(dp->da_cnt<=0) {
			spl5();
			if (dp->da_db==0)	break;
			lp->cs = IE;
			lp->cs = IE|OIR|OD|OM;
			sleep(lp,DAPRI);
			if ((dp->da_cs&ID) || (dp->da_cnt = dp->da_db)==0)
				break;
			dp->da_off = 0;
			lp->wc = -257;
			lp->ba = loword(dp->da_bp->b_paddr);
			if(dp->da_cs&IM)
				lp->cs = IE|OIR|OD|GO; else
				lp->cs = IE|OD|GO;
			sleep(lp,DAPRI);
			spl0();
		}
		i = min(dp->da_cnt,u.u_count);
		pimove(paddr(dp->da_bp)+dp->da_off,i,B_READ);
		dp->da_cnt -= i;
		dp->da_off += i;
	}
	spl0();
	dp->da_flags = 0;
}

dawrite(dev)
{
	register struct da *dp;
	register struct device *lp;
	register i;

	dp = &da_da[dev];
	if(dp->da_flags) {
		u.u_error = EIO;
		return;
	}
	dp->da_flags++;
	dp->da_cnt = 0;
	lp = da_addr[dev];
	while(u.u_count && u.u_error==0) {
		spl5();
		if (dp->da_db==0)	break;
		i = min(512,u.u_count);
		lp->db = i;
		lp->cs = IE;
		lp->cs = IE|OIR|OM;
		sleep(lp,DAPRI);
		if ((dp->da_cs&ID)==0)	break;
		spl0();
		pimove(paddr(dp->da_bp),i++,B_WRITE);
		spl5();
		lp->wc = -(i>>1);
		lp->ba = loword(dp->da_bp->b_paddr);
		if(dp->da_cs&IM)
			lp->cs = IE|OIR|GO; else
			lp->cs = IE|CYCLE|GO;
		sleep(lp,DAPRI);
		lp->cs = IE|OIR;
	}
	spl0();
	dp->da_flags = 0;
}

daintr(dev)
{
	register struct da *dp;
	register struct device *lp;

	dp = &da_da[dev];
	lp = da_addr[dev];
	dp->da_cs = lp->cs;
	if ((lp->cs&(IM|ID))==IM)
		dp->da_db = lp->db;
	lp->cs &= ~OIR;
	wakeup(lp);
}
