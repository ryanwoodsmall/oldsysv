/*	dmk.c 1.3 of 3/18/82
	@(#)dmk.c	1.3
 */

/*
 *	DM11-BA driver for multiple units
 *
 *	Each unit provides modem control and status information
 *	for eight lines.  Each DM11-BA is normally associated with
 *	a DMS11-DA synchronous line interface as part of a KMS11.
 */

#include "sys/param.h"
#ifndef CBUNIX
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/sysmacros.h"
#include "sys/errno.h"
#endif
#include "sys/dir.h"
#include "sys/user.h"
#ifdef CBUNIX
#include "sys/userx.h"
#endif
#include "sys/dmk.h"

extern int	dmk_cnt;
extern struct devdmk *dmk_addr[];
extern struct dmksave dmksave[];


struct devdmk {
	short	dmkcsr, dmklstat;
};

dmkopen(dev, flag)
{

	if (dev >= dmk_cnt) {
		u.u_error = ENXIO;
		return;
	}
}

dmkclose(dev)
{
}

dmkioctl(dev, cmd, arg, mode)
register dev;
{
	struct dmkcmd	dmkcmd, *ap;
	switch(cmd) {
	case DMKSETM:
		if(copyin(arg, &dmkcmd, sizeof(dmkcmd))) {
			u.u_error = EFAULT;
			return;
		}
		ap = &dmkcmd;
		if (ap->line > 7) {
			u.u_error = ENXIO;
			return;
		}
		dmkctl(dev, ap->line, ap->mode);
		break;
	default:
		u.u_error = ENXIO;
		return;
	}
}

/*
 * Dump control bits into the DM registers.
 */
dmkctl(dev, line, bits)
register dev;
{
	register struct devdmk *dmkaddr;

	bits &= DMKDTR|DMKRTS|DMKNS;
	dmksave[dev].status[line] = bits;
	dmkaddr = dmk_addr[dev];
	dmkaddr->dmkcsr = line;
	dmkaddr->dmklstat = bits;
}
/*
 * Power-fail recovery
 */
dmkclr()
{
	register dev,line;

	for(dev = 0; dev < dmk_cnt; dev++)
		for(line = 0; line < 8; line++)
			dmkctl(dev,line,dmksave[dev].status[line]);
}
