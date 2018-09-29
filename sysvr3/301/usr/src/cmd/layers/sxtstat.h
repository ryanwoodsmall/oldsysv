/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)attwin:cmd/layers/sxtstat.h	1.2"

#define	SXTIOCLINK	('b'<<8)
#define SXTIOCSTAT	(SXTIOCLINK|7)


/* the following structure is used for the SXTIOCSTAT ioctl call */
struct sxtblock
{
	char	input;		/* channels blocked on input  */
	char	output;		/* channels blocked on output */
};
