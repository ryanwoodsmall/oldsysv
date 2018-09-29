/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/callo.h	10.2"
/*
 *	The callout structure is for a routine arranging
 *	to be called by the clock interrupt
 *	(clock.c) with a specified argument,
 *	in a specified amount of time.
 *	Used, for example, to time tab delays on typewriters.
 */

struct	callo
{
	int	c_time;		/* incremental time */
	int	c_id;		/* timeout id */
	caddr_t	c_arg;		/* argument to routine */
	int	(*c_func)();	/* routine */
};
extern	struct	callo	callout[];
