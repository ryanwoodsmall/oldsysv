static char	sccsid[] = "@(#)Ainit.c	1.4";

# include "stand.h"

extern gd_boot;

static char	rp[] = "/dev/rp00";
static char	mt[] = "/dev/mt0";
#ifdef vax
static char	ts[] = "/dev/ts0";
#endif

_init ()
{
	if (gd_boot == 0)
		rp[8] = '\0';
	else
		rp[7] = gd_boot + '0';
	if (MKNOD (rp, 0, gd_boot, (daddr_t) 0) < 0)
		perror (rp);
	if (mount (rp, "") < 0)
		perror (rp);

	if (gd_boot == 0)
		rp[7] = '1';
	else
		rp[8] = '1';
	if (MKNOD (rp, 0, gd_boot, (daddr_t) 18392) < 0)
		perror (rp);
	if (mount (rp, "/usr") < 0)
		perror (rp);

	if (MKNOD (mt, 1, 0, (daddr_t) 0) < 0)
		perror (mt);

	mt[7] = '4';
	if (MKNOD (mt, 1, 4, (daddr_t) 0) < 0)
		perror (mt);

#ifdef vax
	if ( MKNOD( ts, 2, 0, (daddr_t) 0) < 0)
		perror( ts);

	ts[7] = '4';
	if ( MKNOD( ts, 2, 4, (daddr_t) 0) < 0)
		perror( ts);
#endif
}
