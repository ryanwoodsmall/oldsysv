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
/* @(#)imsg.c	1.3 */
#include "uucp.h"

char Msync[2] = "\020";

/*
 * read message routine used before a
 * protocol is agreed upon.
 *	msg	-> address of input buffer
 *	fn	-> input file descriptor 
 * returns:
 *	EOF	-> no more messages
 *	0	-> message returned
 */
imsg(msg, fn)
register char *msg;
register int fn;
{
	register char c;
	short fndsync;
	int ret;
	char *bmsg, c1;

	fndsync = 0;
	bmsg = msg;
	DEBUG(7, "imsg %s>", "");
	while ((ret = read(fn, &c1, sizeof(c))) == sizeof(c)){
		c = c1;
		DEBUG(7, (c>037) ? "%c" : "\\%3o ", c & 0377);
		if (c == Msync[0]) {
			msg = bmsg;
			fndsync = 1;
		}
		else if (c == '\0' || c == '\n') {
			if (fndsync) {
				*msg = '\0';
				return(0);
			}
		} else {
			if (fndsync)
				*msg++ = c;
		}

	}
	*msg = '\0';
	return(EOF);
}

/*
 * initial write message routine -
 * used before a protocol is agreed upon.
 *	type	-> message type
 *	msg	-> message body address
 *	fn	-> file descriptor
 * return: 
 *	always 0
 */
omsg(type, msg, fn)
register char *msg;
register char type;
int fn;
{
	register char *c;
	char buf[BUFSIZ];

	c = buf;
	*c++ = Msync[0];
	*c++ = type;
	while (*msg)
		*c++ = *msg++;
	*c++ = '\0';
	write(fn, buf, (unsigned) strlen(buf) + 1);
	return(0);
}
