/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/gentty.c	10.5"
/*
 *	indirect driver for controlling tty.
 */
#include "sys/types.h"
#include "sys/sbd.h"
#include "sys/param.h"
#include "sys/immu.h"
#include "sys/sysmacros.h"
#include "sys/fs/s5dir.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/conf.h"
#include "sys/tty.h"
#include "sys/stream.h"
#if !defined(u3b5) && !defined(u3b2)
#include "sys/seg.h"
#endif

syopen(dev, flag)
{

	if (sycheck()) {
		if (cdevsw[major(u.u_ttyd)].d_str) {
			if (!u.u_ttyip) {
				u.u_error = ENXIO;
				return;
			}
			stropen(u.u_ttyip, flag);
		} else
			(*cdevsw[major(u.u_ttyd)].d_open)(minor(u.u_ttyd), flag);
	}
}

syread(dev)
{

	if (sycheck()) {
		if (cdevsw[major(u.u_ttyd)].d_str)
			strread(u.u_ttyip);
		else
			(*cdevsw[major(u.u_ttyd)].d_read)(minor(u.u_ttyd));
	}
}

sywrite(dev)
{

	if (sycheck()) {
		if (cdevsw[major(u.u_ttyd)].d_str)
			strwrite(u.u_ttyip);
		else
			(*cdevsw[major(u.u_ttyd)].d_write)(minor(u.u_ttyd));
	}
}

syioctl(dev, cmd, arg, mode)
{

	if (sycheck()) {
		if (cdevsw[major(u.u_ttyd)].d_str)
			strioctl(u.u_ttyip, cmd, arg, mode);
		else
			(*cdevsw[major(u.u_ttyd)].d_ioctl)(minor(u.u_ttyd), cmd, arg, mode);
	}
}

sycheck()
{
	if (u.u_ttyp == NULL) {
		u.u_error = ENXIO;
		return(0);
	}
	if (*u.u_ttyp != u.u_procp->p_pgrp) {
		u.u_error = EIO;
		return(0);
	}
	return(1);
}
