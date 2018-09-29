/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libnsl:nsl/t_connect.c	1.4"
#include "sys/param.h"
#include "sys/stropts.h"
#include "sys/tiuser.h"
#include "fcntl.h"
#include "sys/signal.h"
#include "_import.h"

extern snd_conn_req(), rcv_conn_con();
extern t_errno;
extern int fcntl();
extern int (*sigset())();


t_connect(fd, sndcall, rcvcall)
int fd;
struct t_call *sndcall;
struct t_call *rcvcall;
{
	int fctlflg;
	int (*sigsave)();

	if (_t_checkfd(fd) == NULL)
		return(-1);

	if (_snd_conn_req(fd, sndcall) < 0)
		return(-1);

	if ((fctlflg = fcntl(fd, F_GETFL, 0)) < 0) {
		t_errno = TSYSERR;
		return(-1);
	}

	if (fctlflg&O_NDELAY) {
		t_errno = TNODATA;
		return(-1);
	}

	if (_rcv_conn_con(fd, rcvcall) < 0) {
		return(-1);
	}

	return(0);
}
