/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/sccsid.h	10.1"
#ifdef lint
#define VERSION(x);
#define HVERSION(x);
#else
#define VERSION(x) static char sccsid[]="x";
#define HVERSION(n,x) static char n[]="x";
#endif
