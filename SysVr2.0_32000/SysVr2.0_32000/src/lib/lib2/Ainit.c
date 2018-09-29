/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*
 *	Ainit.c
 */

#include <stand.h>

static char *rootdev = "/dev/dsk/1s0";	/* Modify these as needed */
static char *swapdev = "/dev/dsk/1s1";

_init ()
{
	/* Changed to allow dynamic allocation of device table entries
	   as needed.  (Saves overflowing device tables from a max of
	   8 drives and 8 slices possible.)  If the root or swap devices
	   are changed above, be sure that the appropriate MKNOD unit #
	   below (3rd arg) is also changed.  (Unit = 8*drive + slice) */

	if (MKNOD (rootdev, 0, 8, (daddr_t) 0) < 0)
		perror(rootdev);
	if (mount (rootdev, "") < 0)
		perror(rootdev);
	if (MKNOD (swapdev, 0, 9, (daddr_t) 0) < 0)
		perror(swapdev);
	if (mount (swapdev, "/dev/swap") < 0)
		perror(swapdev);
}
