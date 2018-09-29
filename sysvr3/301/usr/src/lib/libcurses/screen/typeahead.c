/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/typeahead.c	1.3"
#include "curses.ext"

/*
 * Set the file descriptor for typeahead checks to fd.  fd can be -1
 * to disable the checking.
 */
int
typeahead(fd)
int fd;
{
#ifdef SYSV
	/*
	 * Doing fcntls before and after each typeahead check 
	 * read is a serious problem on the 3b2. Profiling
	 * results indicated that a simple program (edit.c from
	 * "The New Curses and Terminfo Package") was spending
	 * 9.2% of the time in fcntl().
	 */

#	include <fcntl.h>
	extern char *ttyname();
	register int savefd = SP->check_fd;
	register char *tty;

	/* Close the previous duped file descriptor. */
	if (savefd >= 0)
		close(savefd);

	/* Duplicate the file descriptor so we have one to play with. */
	/* We cannot use dup(2), unfortunately, so we do typeahead */
	/* checking on terminals only. Besides, if typeahead is done */
	/* when input is coming from a file, nothing would EVER get */
	/* drawn on the screen! */
	if (tty = ttyname(fd))
		SP->check_fd = open (tty, O_RDONLY | O_NDELAY);
	else
		SP->check_fd = -1;
#else
	int savefd = SP->check_fd;
	/* Only do typeahead checking if the input is a tty. */
	if (isatty(fd))
		SP->check_fd = fd;
	else
		SP->check_fd = -1;
#endif /* SYSV */
	return savefd;
}
