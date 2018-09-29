/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/acct.c	10.8"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/systm.h"
#include "sys/acct.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/file.h"
#include "sys/debug.h"
#include "sys/conf.h"

struct acct	acctbuf;
struct inode	*acctp;
int		aclock;

/*
 * Perform process accounting functions.
 */

sysacct()
{
	register struct inode *ip;
	register struct a {
		char	*fname;
	} *uap;

	uap = (struct a *)u.u_ap;
	if (aclock || !suser())
		return;
	aclock++;
	if (uap->fname == NULL) {
		if (acctp) {
			plock(acctp);
			iput(acctp);
			acctp = NULL;
		}
	} else {
		if (acctp) {
			u.u_error = EBUSY;
			goto out;
		}
		if ((ip = namei(upath, 0)) == NULL)
			goto out;
		if (ip->i_ftype != IFREG)
			u.u_error = EACCES;
		else	
			FS_ACCESS(ip, IWRITE);
		if (u.u_error) {
			iput(ip);
			goto out;
		}
		acctp = ip;
		prele(ip);
	}
out:
	aclock--;
}

/*
 * On exit, write a record on the accounting file.
 */
acct(st)
{
	register struct inode *ip;
	off_t siz;

	if ((ip=acctp) == NULL)
		return;
	plock(ip);
	bcopy(u.u_comm, acctbuf.ac_comm, sizeof(acctbuf.ac_comm));
	acctbuf.ac_btime = u.u_start;
	acctbuf.ac_utime = compress(u.u_utime);
	acctbuf.ac_stime = compress(u.u_stime);
	acctbuf.ac_etime = compress(lbolt - u.u_ticks);
	acctbuf.ac_mem = compress(u.u_mem);
	acctbuf.ac_io = compress(u.u_ioch);
	acctbuf.ac_rw = compress(u.u_ior+u.u_iow);
	acctbuf.ac_uid = u.u_ruid;
	acctbuf.ac_gid = u.u_rgid;
	acctbuf.ac_tty = u.u_ttyp ? u.u_ttyd : NODEV;
	acctbuf.ac_stat = st;
	acctbuf.ac_flag = u.u_acflag;
	siz = ip->i_size;
	u.u_offset = siz;
	u.u_base = (caddr_t)&acctbuf;
	u.u_count = sizeof(acctbuf);
	u.u_segflg = 1;
	u.u_error = 0;
	u.u_limit = (daddr_t)5000;
	u.u_fmode = FWRITE;

	FS_WRITEI(ip);
	if (u.u_error)
		ip->i_size = siz;
	prele(ip);
}


/*
 * Produce a pseudo-floating point representation
 * with 3 bits base-8 exponent, 13 bits fraction.
 */
compress(t)
register time_t t;
{
	register exp = 0, round = 0;

	while (t >= 8192) {
		exp++;
		round = t&04;
		t >>= 3;
	}
	if (round) {
		t++;
		if (t >= 8192) {
			t >>= 3;
			exp++;
		}
	}
	return((exp<<13) + t);
}
