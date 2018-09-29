/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libnsl:nsl/t_close.c	1.4.1.2"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/errno.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "sys/tihdr.h"
#include "sys/timod.h"
#include "sys/tiuser.h"
#include "sys/signal.h"
#include "_import.h"


extern int t_errno;
extern int errno;
extern struct _ti_user *_t_checkfd();
extern void free();
extern void (*sigset())();
extern int close();
extern _null_tiptr();


t_close(fd)
int fd;
{
	register struct _ti_user *tiptr;
	void (*sigsave)();

	if ((tiptr = _t_checkfd(fd)) == NULL)
		return(-1);

	sigsave = sigset(SIGPOLL, SIG_HOLD);
	if (tiptr->ti_rcvbuf != NULL)
		(void)free(tiptr->ti_rcvbuf);
	if (tiptr->ti_lookdbuf != NULL)
		(void)free(tiptr->ti_lookdbuf);
	(void)free(tiptr->ti_ctlbuf);
	(void)free(tiptr->ti_lookcbuf);

	_null_tiptr(tiptr);

	close(fd);
	sigset(SIGPOLL, sigsave);

	return(0);
}
