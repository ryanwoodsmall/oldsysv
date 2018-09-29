/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:fs/du/durdwri.c	10.4.5.6"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/param.h"
#include "sys/fstyp.h"
#include "sys/fs/s5param.h"
#include "sys/fs/s5inode.h"
#include "sys/inode.h"
#include "sys/mount.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/conf.h"
#include "sys/file.h"
#include "sys/systm.h"
#include "sys/debug.h"
#include "sys/rdebug.h"
#include "sys/message.h"
#include "sys/sysinfo.h"
#include "sys/sema.h"
#include "sys/comm.h"
#include "sys/fs/s5macros.h"
#include "sys/rbuf.h"

dureadi(ip)
register struct inode *ip;
{
	register sndd_t sd;
	register daddr_t blkno; 
	register int blkcnt;
	register struct rbuf  *rbp;
	int rval, rem, count, blkct, savescall;
	int unlocked = 0;

	DUPRINT3 (DB_FSS,"dureadi: ip = %x   sndd = %x \n",ip,ip->i_fsptr);
	savescall = u.u_syscall;
	if (u.u_syscall != DUREAD) 
		u.u_syscall = DUREADI;
	sd = (sndd_t)(ip->i_fsptr);
	u.u_rcstat &= ~U_RCACHE;
	switch (ip->i_ftype ) {
		case IFIFO:
			ASSERT(ip->i_flag & ILOCK);
			prele(ip);
			unlocked = 1;
			break;
		case IFDIR:
			break;
		case IFREG:
			if (sd->sd_stat & SDCACHE && !(sd->sd_stat & SDMNDLCK)
			   && !(u.u_offset >= ip->i_size || u.u_count <= 0)) 
				u.u_rcstat |= U_RCACHE;
			if (u.u_rcstat & U_RCACHE) {
				/*
				 * For remote files with caching turned on,
				 * isize is kept up to date.
				 */
				sd->sd_count = u.u_count;
				sd->sd_offset = u.u_offset;
				if ((rem =(u.u_offset+u.u_count-ip->i_size))> 0)
					sd->sd_count -= rem;
				blkct = ct_to_blkct(sd->sd_count,sd->sd_offset);
				rcinfo.clread += blkct;
			 	if ((rbp = rget_cache(sd,
				   off_to_blkoff(sd->sd_offset),blkct))!=NULL) {
					DUPRINT4(DB_CACHE, "dureadi: cache hit on sd %x offset %x clread %x\n", sd, sd->sd_offset, rcinfo.clread);
					count = sd->sd_count;
					/*cache_out increments u.u_base */
					rval=rcache_out(rbp,sd);
					u.u_offset += count - rval;
					u.u_count -= (count - rval);
					dinfo.osyscall++;
					dinfo.osysread++;
					dinfo.oreadch += (count - rval);
					u.u_syscall = savescall;
					u.u_rcstat &= ~U_RCACHE;
					return;
				}
				rcinfo.cbread += blkct;
			}
			ASSERT(ip->i_flag & ILOCK);
			if (sd->sd_stat & SDMNDLCK) {
				prele(ip);
				unlocked = 1;
			}
	}
	remfileop (ip,NULL,NULL);
	u.u_syscall = savescall;
	u.u_rcstat &= ~U_RCACHE;
	if ( unlocked == 1)
		plock(ip);
}
duwritei(ip)
register struct inode *ip;
{
	register sndd_t sd;
	register int savescall;
	int unlocked = 0;
	savescall = u.u_syscall;


	sd = (sndd_t)(ip->i_fsptr);
	u.u_rcstat &= ~U_RCACHE;
	switch (ip->i_ftype) { 
		case IFIFO:
			ASSERT(ip->i_flag & ILOCK);
			prele(ip);
			unlocked = 1;
			break;
		case IFREG:
			ASSERT(ip->i_flag & ILOCK);
			if (sd->sd_stat & SDMNDLCK) {
				prele(ip);
				unlocked = 1;
			}
	   		else if (sd->sd_stat & SDCACHE)
				u.u_rcstat |= U_RCACHE;
	}

	DUPRINT3 (DB_FSS,"duwritei: ip = %x   sndd = %x \n",ip,ip->i_fsptr);
	if (u.u_syscall != DUWRITE) 
		u.u_syscall = DUWRITEI;
	remfileop (ip,NULL,NULL);
	u.u_syscall = savescall;
	u.u_rcstat &= ~U_RCACHE;
	if (unlocked == 1)
		plock(ip);
}
