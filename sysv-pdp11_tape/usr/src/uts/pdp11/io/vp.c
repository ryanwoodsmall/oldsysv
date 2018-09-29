/* @(#)vp.c	1.1 */
/*
 *  Versatec matrix printer/plotter dma interface driver
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/systm.h"
#include "sys/lprio.h"
#include "sys/vp.h"

#define	VPPRI	(PZERO+10)

struct device {
	short	plbcr, pbae, prbcr, pbaddr;
	short	plcsr, plbuf, prcsr, prbuf;
};

struct device *vp_addr[];
int	vp_cnt;

/* status bits */
#define	ERROR	0100000
#define	READY	0200
#define	IENABLE	0100
#define	FFCOM	020
#define	RESET	02
#define	SPP	01

/*states */
#define	PRINT	0100
#define	PLOT	0200
#define	PPLOT	0400

vpopen(dev)
register dev;
{
	register struct vp *vp;
	register short *vpaddr;

	if (dev>=vp_cnt || (vp = &vp_vp[dev])->vp_buf) {
		u.u_error = ENXIO;
		return;
	}
	vpaddr = &(vp_addr[dev]->plcsr);
	*vpaddr = IENABLE|RESET;
	vp->vp_state = PRINT;
	if (vpready(dev)<0) {
		*vpaddr = 0;
		u.u_error = EIO;
		return;
	}
	*vpaddr = IENABLE|FFCOM;
	vp->vp_buf = getablk(1);
	vp->vp_offset = vp->vp_count = 0;
}

vpclose(dev)
{
	register struct vp *vp;
	register short *vpaddr;

	vp = &vp_vp[dev];
	vpaddr = &(vp_addr[dev]->plcsr);
	vpready(dev);
	*vpaddr = 0;
	if (vp->vp_buf)
		brelse(vp->vp_buf);
	vp->vp_buf = 0;
}

vpwrite(dev)
register dev;
{
	register struct vp *vp;
	register struct device *vpaddr;
	paddr_t addr;

	vp = &vp_vp[dev];
	vpaddr = vp_addr[dev];
	if (vp->vp_count) {
		u.u_error = EIO;
		return;
	}
	while (u.u_count && u.u_error==0) {
		vp->vp_count = min(256,u.u_count);
		pimove(paddr(vp->vp_buf)+vp->vp_offset, vp->vp_count, B_WRITE);
		if (vpready(dev)<0) {
			u.u_error = EIO;
			vpaddr->plcsr = IENABLE|RESET;
			break;
		}
		addr = ubmaddr(vp->vp_buf, 0) + vp->vp_offset;
		vpaddr->pbaddr = loword(addr);
		vpaddr->pbae = hiword(addr)<<4;
		if (vp->vp_state&PLOT)
			vpaddr->plbcr = vp->vp_count; else
			vpaddr->prbcr = vp->vp_count;
		vp->vp_offset = 256 - vp->vp_offset;
		if (vp->vp_state&PPLOT)
			vp->vp_state = PLOT;
	}
	vp->vp_count = 0;
}

vpready(dev)
{
	register struct vp *vp;
	register short *vpaddr;

	vp = &vp_vp[dev];
	vpaddr = (vp->vp_state&PLOT)?&(vp_addr[dev]->plcsr):
		&(vp_addr[dev]->prcsr);
	spl4();
	while ((*vpaddr&(READY|ERROR))==0)
		sleep(vp, VPPRI);
	spl0();
	return(*vpaddr);
}

vpioctl(dev, cmd, arg, mode)
{
	register struct vp *vp;
	register short *vpaddr;
	register bit;

	vp = &vp_vp[dev];
	vpaddr = &(vp_addr[dev]->plcsr);
	switch(cmd) {
	case LPRGETV:
		u.u_rval1 = vp->vp_state;
		break;
	case LPRSETV:
		vpready(dev);
		vp->vp_state = arg;
		if (vp->vp_state&PPLOT)
			*vpaddr = IENABLE|SPP; else
			*vpaddr = IENABLE;
		for (bit=2; bit<IENABLE; bit<<=1)
			if (vp->vp_state&bit) {
				vpready(dev);
				*vpaddr |= bit;
				vp->vp_state &= ~bit;
			}
		break;
	default:
		u.u_error = EINVAL;
	}
}

vpintr(dev)
{
	wakeup(&vp_vp[dev]);
}
