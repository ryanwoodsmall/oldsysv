/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libnsl:nsl/_import.h	1.3"

#define free		(*__free)
#define calloc 		(*__calloc)
#define perror		(*__perror)
#define strlen		(*__strlen)
#define write		(*__write)
#define ioctl		(*__ioctl)
#define putmsg		(*__putmsg)
#define getmsg		(*__getmsg)
#define errno		(*__errno)
#define memcpy		(*__memcpy)
#define fcntl		(*__fcntl)
#define sigset		(*__sigset)
#define open		(*__open)
#define close		(*__close)
#define ulimit		(*__ulimit)
