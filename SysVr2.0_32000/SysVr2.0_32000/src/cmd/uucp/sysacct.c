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
/* @(#)sysacct.c	1.2 */
#include <sys/types.h>

/*
 * output accounting info
 */
sysacct(bytes, time)
time_t time;
long bytes;
{

	/*
	 * shuts lint up
	 */
	if (time == bytes)
	return;
}
