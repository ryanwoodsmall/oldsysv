/* @(#)mem.c	1.1 */
/*
 *	Memory special file
 *	minor device 0 is physical memory
 *	minor device 1 is kernel memory
 *	minor device 2 is EOF/NULL
 *	minor devices >= 8 are MAUS
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/maus.h"

long	mmoff();

/*
 * Note: read/write memory in blocks of BSIZE; MAUS in blocks of NBPC.
 */

mmread(dev)
register dev;
{
	register unsigned n;
	long offset;

	while(u.u_error == 0 && u.u_count != 0) {
		if ( (offset=mmoff(dev)) < 0 )
			break;
		n = min(u.u_count, (dev>2) ? NBPC : BSIZE);
		if (dev == 0 || dev > 2) {
			if (copyio(offset, u.u_base, n, U_RUD))
				u.u_error = ENXIO;
		} else if (dev == 1) {
			if (copyout((short)offset, u.u_base, n))
				u.u_error = ENXIO;
		} else
			break;
		u.u_offset += n;
		u.u_base += n;
		u.u_count -= n;
	}
}

mmwrite(dev)
register dev;
{
	register unsigned n;
	long offset;

	while(u.u_error == 0 && u.u_count != 0) {
		if ( (offset=mmoff(dev)) < 0 )
			break;
		n = min(u.u_count, (dev>2) ? NBPC : BSIZE);
		if (dev == 0 || dev > 2) {
			if (copyio(offset, u.u_base, n, U_WUD))
				u.u_error = ENXIO;
		} else if (dev == 1) {
			if (copyin(u.u_base, (short)offset, n))
				u.u_error = ENXIO;
		}
		u.u_offset += n;
		u.u_base += n;
		u.u_count -= n;
	}
}

/*
 *	Calculate starting block number (64 bytes/block).
 *	Check for MAUS minor device.
 */

long
mmoff(dev)
register dev;
{
	register cn;	/* click number */

	if (dev <= 2)
		return(u.u_offset);

	/* The rest for MAUS */

	cn = u.u_offset>>6;
	if ((dev -= 8) < 0 || dev >= nmausent || cn < 0
	    || cn >= mausmap[dev].bsize) {
		u.u_error = ENXIO;
		return(-1L);
	}
	else
		return( ((long)(mauscore + mausmap[dev].boffset)<<6)
			+ u.u_offset);

}
