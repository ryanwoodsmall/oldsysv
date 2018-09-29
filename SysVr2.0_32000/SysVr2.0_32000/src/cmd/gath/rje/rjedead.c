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
*	rjedead - creates file "dead" in directory (place)
*		containing message (msg).
*/

static char Sccsid[] = "@(#)rjedead.c	1.1";

static char *dead = "dead";

rjedead(msg,place)
char *msg, *place;
{
	register int fd;

	chdir(place);
	unlink(dead);
	fd = creat(dead,0644);
	write(fd,msg,strlen(msg));
	exit(1);
}
