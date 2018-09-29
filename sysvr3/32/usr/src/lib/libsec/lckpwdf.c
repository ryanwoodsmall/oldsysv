/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libsec:lckpwdf.c	1.1"

#include <stdio.h>
#include <signal.h>
#include <fcntl.h>

#define LOCKFILE	"/etc/.pwd.lock"
#define S_WAITTIME	15

struct flock flock =	{
			0,	/* l_type */
			0,	/* l_whence */
			0,	/* l_start */
			0,	/* l_len */
			0,	/* l_sysid */
			0	/* l_pid */
			} ;

/*	lckpwdf() returns a 0 for a successful lock within W_WAITTIME
	and -1 otherwise
*/

static int fildes = -1 ;
extern int fcntl() ;
extern void (*sigset ())() ;
extern unsigned alarm() ;

int
lckpwdf()
{
	extern void almhdlr() ;
	int retval ;
	if ( (fildes = creat (LOCKFILE, 0600)) == -1 )
		return (-1) ;
	else
		{
		flock.l_type = F_WRLCK ;
		(void) sigset (SIGALRM, almhdlr) ;
		(void) alarm (S_WAITTIME) ;
		retval = fcntl (fildes, F_SETLKW, &flock) ; 
		(void) alarm (0) ;
		(void) sigset (SIGALRM, SIG_DFL) ;
		return ( retval ) ;
		}
}

/* 	ulckpwdf() returns 0 for a successful unlock and -1 otherwise
*/
int
ulckpwdf()
{
	if ( fildes == -1 ) 
		return (-1) ;
	else	{
		flock.l_type = F_UNLCK ;
		(void) fcntl (fildes, F_SETLK, &flock) ;
		(void) close (fildes) ;
		fildes = -1 ;
		return (0) ;
		}
}	
	
void
almhdlr()
{
}
