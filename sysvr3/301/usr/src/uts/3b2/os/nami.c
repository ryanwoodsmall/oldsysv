/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/nami.c	10.10"
#include "sys/param.h"
#include "sys/types.h"
#include "sys/systm.h"
#include "sys/sysinfo.h"
#include "sys/sysmacros.h"
#include "sys/nami.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/mount.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/var.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/debug.h"
#include "sys/cmn_err.h"
#include "sys/conf.h"

#if USIZE == 2
#define PATHSIZE	512
#else
#define PATHSIZE	1024
#endif

/*
 * Convert a pathname into a pointer to
 * a locked inode.
 *
 * func = function called to get the pathname
 *	&upath if name is in user space
 *	&spath if name is in system space
 * flagp
 *	0 for ordinary searches
 *	else ->flag structure with more parameters
 */

long nmmaxlen;	/* longest pathname encountered */
long nmcount;	/* number of time that namei has been called */
long nmchar;	/* sum of characters of all pathanmes correctly read in */
		/* by namei */
long nmfail;	/* total number of times that namei encountered excessively */
		/* long pathnames (exceeded PATHSIZE) */
struct inode *
namei(func, flagp)
int (*func)();
register struct argnamei *flagp;
{
	struct nx p;
	struct argnamei *tmpflagp;
	register struct inode *dp = NULL;
	struct inode *rem_dp;
	register caddr_t bufp; /* pointer to beginning of buffer */
	register caddr_t comp; /* pointer to the beginning of component */
	register caddr_t cp; /* stepping pointer */
	register i;
	struct mount *mp;
	char stkbuf[PATHSIZE];
	extern short dufstyp;

	sysinfo.namei++;

	cp = stkbuf;

	nmcount++;
	bufp = cp;

	/* Read in entire pathname.  The functions will return */
	/* the length of the pathname if it was read correctly. */
	/* Otherwise, the following values will be returned:
	/* 	-1	if the user supplied address was not valid */
	/*	-2	if the pathname length was >= PATHSIZE-1
	/*	0	if a null pathname was supplied. This is */
	/*		  normally an error since null pathnames */
	/*		  are not supported (however, they turn up */
	/*		  in Distributed Unix pathname searches: see */
	/*		  comment below) */
	switch (i = (*func)(u.u_dirp, bufp, PATHSIZE)) {
	case -2:	/* pathname too long */
		nmfail++;
		u.u_error = ENOENT;
		goto out;
	case 0:		/* null pathname */
		if (!server()) {
			u.u_error = ENOENT;
			goto out;
		}
		break;
	case -1:	/* bad user address */
		u.u_error = EFAULT;
		goto out;
	}

	if (i > nmmaxlen)
		nmmaxlen = i;
	nmchar += i;

restart:

	if (*bufp == '\0' && !server()) {
		u.u_error = ENOENT;
		goto out;
	}

	cp = u.u_nextcp = bufp;

	if (*cp == '/') {
		if (u.u_rdir && (u.u_rdir->i_fstyp == dufstyp)) {
			rem_dp = dp;	/* so can use address of (&) on it */
			switch (rnamei1(u.u_rdir, &rem_dp)) {
			case 1:  /* done */
				dp = rem_dp;
				goto out;
			case 0:  /* more */
				dp = rem_dp;
				cp = u.u_nextcp;
				goto dirloop;
			case -1: /* real error */
			default:
				dp = NULL;
				goto out;
			}
		} else {
			while (*cp == '/')
				cp++;
			if (*cp == '\0' && flagp) {
				u.u_error = ENOENT;
				goto out;
			}
			if ((dp = u.u_rdir) == NULL)
				dp = rootdir;
		}
	} else {
		if (u.u_cdir && (u.u_cdir->i_fstyp == dufstyp)) {
			rem_dp = dp;
			switch (rnamei1(u.u_cdir, &rem_dp)) {
			case 1:
				dp = rem_dp;
				goto out;
			case 0:
				dp = rem_dp;
				cp = u.u_nextcp;
				goto dirloop;
			case -1: 
			default:
				dp = NULL;
				goto out;
			}
		} else
			dp = u.u_cdir;
	}

	/* lock the node and bump the count. */
	plock(dp);
	dp->i_count++;

dirloop:
	ASSERT(dp->i_flag & ILOCK);
	if (dp->i_ftype != IFDIR || dp->i_nlink == 0) {
		iput(dp);
		u.u_error = ENOTDIR;
		goto out;
	}
	if (FS_ACCESS(dp, ICDEXEC)) {
		iput(dp);
		goto out;
	}

	/*
	 * Handle the pathname "/" as well as null pathnames.
	 * The latter can occur in Distributed Unix pathname
	 * searches which go remote and end up back on the local
	 * machine having processed all the components of the name;
	 * they can also occur in server processes which have been
	 * handed the (null) tail end of a pathname which started
	 * on a client machine.  A null component should be interpreted
	 * as a synonym for the directory which is "current" at this
	 * point in the processing.  Note that the non-remote case of a
	 * null pathname is handled at the beginning of namei, and
	 * rejected.
	 */

	if (*cp == '\0' && flagp == 0)	/* Works for "/" as well as for DU */
		goto out;
	comp = cp;
	u.u_comp = cp;

	/* skip over component to next '/' or '\0' */
	while (*cp != '/' && *cp != '\0') 
		cp++;

	/* If we are still in the middle of a pathname, terminate */
	/* the component name with '\0' and specify that a lookup */
	/* request be sent to the fs dependent namei. */
	/* If the end of the pathname has been encountered, pass the */
	/* requested action (flagp) to the fs dependent namei and set */
	/* a flag indicating that the last component has been */
	/* encountered */
	if (*cp == '/') {
		*cp++ = '\0';
		while (*cp == '/')
			cp++;
	} 
	if (*cp == '\0')  {
		tmpflagp = flagp;
	} else {
		ASSERT(*cp != '/');
		tmpflagp = (struct argnamei *)0;
	}
	u.u_nextcp = cp;
	/* If the component is . and if the operation is a lookup */
	/* we already have the info that we need, namely a pointer */
	/* to a locked inode for the current directory in the */
	/* pathname. There is no need to call the file system */
	/* dependent code. */
	if (*comp == '.' && *(comp+1) == '\0' && flagp == 0)
		goto dirloop;
nmount:
	/* set values that are to be passed to the fs specific namei */
	p.comp = comp;
	p.bufp = bufp;
	p.dp = dp;
	p.flags = 0;

	mp = dp->i_mntdev;

	/*
	 * Check if server is doing a '..' at a REMOTEly mounted point
	 */
	if (server() && (dp->i_flag & (IDOTDOT|ILBIN)) &&
			(comp[0] == '.' && comp[1] == '.' && comp[2] == '\0')){
		if (u.u_rdir == dp) {
			if (u.u_nextcp == '\0')
				goto out;
			else	
				goto dirloop;
		}
		else 
		if ( rnamei2(comp, dp, stkbuf) == 0){
			dp = NULL;
			goto out;
		}
	}
	ASSERT(dp->i_fstyp != 0 && dp->i_fstyp < nfstyp);
	switch (FS_NAMEI(dp, &p, tmpflagp)) {

	case NI_FAIL:	/* fs specific namei failed */
			/* All iputs should have been done */
			/* in the fs dependent namei */
		goto out;

	case NI_NULL:	/* Operation is complete and calling */
			/* function does not expect an ip returned. */
			/* All iputs should have been done */
			/* fs dependent namei */
		dp = NULL;
		goto out;

	case NI_DONE:	/* The fs dependent action specified by */
			/* flagp->cmd has successfully completed. */
			/* NI_PASS can not be returned since */
			/* the dependent routine may have unlocked */
			/* the inode pointed to by dp */
				
		dp = p.dp;
		goto out;

	case NI_PASS:	/* fs specific namei passed */
		/*
		 * Special handling for ".."
		 */
		if (*comp == '.' && *(comp+1) == '.' && *(comp+2) == '\0') {
			if (dp == u.u_rdir)
				p.ino = dp->i_number;
			else if ((p.flags&NX_ISROOT) && (dp->i_flag&IISROOT)
			  && mp != &mount[0]) {
				iput(dp);
				/* LBIN mount inodp is remote inode */
				if (mp->m_inodp->i_fstyp == dufstyp) {
					rem_dp = dp;
					switch (rnamei3(&rem_dp)) {
					case 1:
						dp = rem_dp;
						goto out;
					case 0:
						dp = rem_dp;
						cp = u.u_nextcp;
						goto dirloop;
					case -1: 
					default:
						dp = NULL;
						goto out;
					}
				} else {
					comp = u.u_comp; /* May have changed */
					dp = mp->m_inodp;
					plock(dp);
					dp->i_count++;
					goto nmount;
				}
			}
		}
		ASSERT(*cp != '/');
		if (flagp && *u.u_nextcp == '\0') {
			dp = p.dp;
			goto out;
		}
		iput(dp);
		if ((dp = iget(mp, p.ino)) == NULL)
			goto out;
#ifdef IFLNK
		/* If the file is a symbolic link, the */
		/* file system dependent code must be called */
		/* to read the symlink. Goto nmount to get it done */
		if (dp->i_ftype == IFLNK)
			goto nmount;
#endif
		/* if this is the last component - leave */
		/* after putting the name into u.u_dent.d_name */
		/* so that ps can work.. */
		if (*u.u_nextcp == '\0') {
			cp = u.u_dent.d_name;
			i = 0;
			while (*comp && i++ < PSCOMSIZ)
				*cp++ = *comp++;
			if (i <= PSCOMSIZ - 1)
				*cp ='\0';
			goto out;
		}
		if (u.u_rflags & (U_RSYS|U_DOTDOT)) {
			/* should ? have gone REM in iget */
			switch (rnamei4(dp)) {
			case 1:
				goto out;
			case 0:
				cp = u.u_nextcp;
				goto dirloop;
			case -1:
				dp = NULL;
				goto out;
			default:
				panic ("namei: switch (rnamei4)"); 
			}
		}
		cp = u.u_nextcp;
		goto dirloop;

	case NI_RESTART:
		iput(p.dp);
		goto restart;

	default:
		cmn_err(CE_CONT, "namei: Invalid fs dependent namei return");
		u.u_error = ENOENT;
	}
out:
	if (u.u_error)
		return(NULL);
	return(dp);
}
