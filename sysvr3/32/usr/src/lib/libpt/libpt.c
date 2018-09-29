/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libpt:libpt.c	1.1"
/*
 *	         Copyright (c) 1987 AT&T
 *		   All Rights Reserved
 *
 *     THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
 *   The copyright notice above does not evidence any actual
 *   or intended publication of such source code.
 *
 ================================================================
 =   WARNING!!!! This source is not supported in future source  =
 =   releases.                                                  =
 ================================================================
 */


#include "sys/types.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/fcntl.h"
#include "sys/stat.h"
#include "sys/ptms.h"

#define PTSNAME "/dev/pts"		/* slave name */
#define PTLEN   12			/* slave name length */
#define PTPATH  "/usr/lib/pt_chmod"    	/* setuid root program */
#define PTPGM   "pt_chmod"		/* setuid root program */

static char sname[PTLEN];

/*
 *  Check that fd argument is a file descriptor of an opened master.
 *  Do this by sending an ISPTM ioctl message down stream. Ioctl()
 *  will fail if: (1) fd is not a valid file descriptor. (2) the file 
 *  represented by fd does not understand ISPTM (not a master device). 
 *  If have a valid master, get its minor number via fstat(). 
 *  Concatenate it to PTSNAME and return it as the name of the slave
 *  device.
 */
char *
ptsname(fd)
{
	register dev_t dev;
	struct stat status;
	struct strioctl istr;

	istr.ic_cmd = ISPTM;
	istr.ic_len = 0;
	istr.ic_timout = 0;
	istr.ic_dp = NULL;

	if (ioctl(fd, I_STR, &istr) < 0)
		return(NULL);
	if (fstat(fd, &status) < 0 )
		return(NULL);

	dev = minor(status.st_rdev);

	sprintf(sname, "%s%03d", PTSNAME, dev);
	return(sname);
}


/*
 * Send an ioctl down to the master device requesting the
 * master/slave pair be unlocked.
 */
unlockpt(fd)
{
	struct strioctl istr;

	istr.ic_cmd = UNLKPT;
	istr.ic_len = 0;
	istr.ic_timout = 0;
	istr.ic_dp = NULL;

	if (ioctl(fd, I_STR, &istr) < 0)
		return(-1);
	return(0);
}


/*
 * Execute a setuid root program to change the mode, ownership and
 * group of the slave device. The parent forks a child process that
 * executes the setuid program. It then waits for the child to return.
 */
grantpt(fd)
int fd;
{
	int	st_loc, pid, w;
	char	fd2[4];

	sprintf(fd2, "%d", fd);
	if (!(pid = fork())) {
		execl(PTPATH, PTPGM, (char *)fd2, (char *) 0);
		/*
		 * the process should not return, unless exec() failed.
		 */
		return(-1);
	}

	/*
	 * wait() will return the process id for the child process
	 * or -1 (on failure).
	 */
	while ((w = wait (&st_loc)) != pid && w != -1);

	/*
	 * if w == -1, the child process did not fork properly.
	 * errno is set to ECHILD.
	 */
	if(w == -1)
		return(-1);

	/*
	 * If child terminated due to exit()...
	 *         if high order bits are zero
	 *                   was an exit(0). 
	 *         else it was an exit(-1);
	 * Else it was struck by a signal.
	 */
	if((st_loc & 0377) == 0)
		return(((st_loc & 0177400) == 0) ? 0 : -1);
	else
		return(-1);
}
