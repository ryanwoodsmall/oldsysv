static char	sccsid[] = "@(#)Ainit.c	1.10";

# include "stand.h"

extern gd_boot;

static char	rp[] = "/dev/rp00";
static char	dsk[] = "/dev/dsk/0s0";
static char	mt[] = "/dev/mt/0ln";
static char	omt[] = "/dev/mt0";
#ifdef vax
static char	ts[] = "/dev/ts0";
#endif

_init ()
{
	/* Old disk drive names */
	if (gd_boot == 0)
		rp[8] = '\0';
	else
		rp[7] = gd_boot + '0';
	if (MKNOD (rp, 0, gd_boot, (daddr_t) 0) < 0)
		perror (rp);

	if (gd_boot == 0)
		rp[7] = '1';
	else
		rp[8] = '1';
	if (MKNOD (rp, 0, gd_boot, (daddr_t) 18392) < 0)
		perror (rp);

	/* New disk drive names */
	dsk[9] = gd_boot + '0';
	if (MKNOD (dsk, 0, gd_boot, (daddr_t) 0) < 0)
		perror (dsk);
	if (mount (dsk, "") < 0)
		perror (dsk);

	dsk[11] = '1';
	if (MKNOD (dsk, 0, gd_boot, (daddr_t) 18392) < 0)
		perror (dsk);
	if (mount (dsk, "/usr") < 0)
		perror (dsk);

	/* Old tape drive names */
	if (MKNOD (omt, 1, 0, (daddr_t) 0) < 0)
		perror (omt);

	omt[7] = '4';
	if (MKNOD (omt, 1, 4, (daddr_t) 0) < 0)
		perror (omt);

	/* New tape drive names */
	if (MKNOD (mt, 1, 4, (daddr_t) 0) < 0)
		perror (mt);

	mt[10] = '\0';
	if (MKNOD (mt, 1, 0, (daddr_t) 0) < 0)
		perror (mt);

#ifdef vax
	if ( MKNOD( ts, 2, 0, (daddr_t) 0) < 0)
		perror( ts);

	ts[7] = '4';
	if ( MKNOD( ts, 2, 4, (daddr_t) 0) < 0)
		perror( ts);
#endif
}
