/*	dmk.c 6.2 of 9/16/83
	@(#)dmk.c	6.2
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
#include "sys/map.h"
#include "sys/csi.h"
#include "sys/csihdw.h"
#include "sys/csikmc.h"
#include "sys/uba.h"

#define		NOATTCH   0377

extern int	dmk_cnt;
extern struct devdmk *dmk_addr[];
extern struct dmksave dmksave[];
extern struct csik csik[];
extern int kmc_cnt;
#ifdef vax
extern int kmc_uba[], dmk_uba[];
#endif
extern unsigned short kmc_dmk[];


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
	unsigned short dmkuba_addr;
	int index, i;
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
	case DMKATTACH:
#ifdef vax
		dmkuba_addr = mubmdev(dmk_addr[dev],dmk_uba[dev]);
		if (kmc_uba[arg] != dmk_uba[dev]) {
			u.u_error = EACCES;
			return;
		}
#else
		dmkuba_addr = ubmdev(dmk_addr[dev]);
#endif
		for ( i=0; i < kmc_cnt; i++)
#ifdef vax
			if ( (kmc_dmk[i] == dmkuba_addr) && 
			     (kmc_uba[i] == dmk_uba[dev]) ) {
				u.u_error = EACCES;
				return;
			}
#else
			if ( kmc_dmk[i] == dmkuba_addr ) {
				u.u_error = EACCES;
				return;
			}
#endif
		if ( kmc_dmk[arg] == 0)
			kmc_dmk[arg] = dmkuba_addr;
		else {
			u.u_error = EACCES;
			return;
		}
		break;
	case DMKDETACH:
#ifdef vax
		dmkuba_addr = mubmdev(dmk_addr[dev],dmk_uba[dev]);
#else
		dmkuba_addr = ubmdev(dmk_addr[dev]);
#endif
		for ( i=0, index = NOATTCH; i < kmc_cnt; i++)
			if ( kmc_dmk[i] == dmkuba_addr )
				index = i;
		if ( index == NOATTCH ) {
			u.u_error = EUNATCH;
			return;
		}
		kmc_dmk[index] = 0;
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
