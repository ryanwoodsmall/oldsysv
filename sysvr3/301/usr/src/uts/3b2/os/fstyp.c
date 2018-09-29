/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/fstyp.c	10.10"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/fstyp.h"
#include "sys/inode.h"
#include "sys/nami.h"
#include "sys/fs/s5dir.h"
#include "sys/fs/s5macros.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/buf.h"
#include "sys/conf.h"

/*
 * NOTE: Developers making use of the File System Switch mechanism to
 * build file system types in SVR3 should be aware that the architecture
 * is not frozen and may change in future releases of System V.  File
 * system types developed for SVR3 may require changes in order to work
 * with future releases of the system.
 */

int fsstray();
int fsnull();
int fsstray();
int fs2stray();
struct inode *fsinstray();
int *fsistray();



fsinit()
{
	register int i;

	/* fstyp 0 is invalid, so start with 1 */
	for (i = 1; i < nfstyp; i++)
		(*fstypsw[i].fs_init)();
}

fsnull()
{
	return(1);
}

fsstray()
{
	printf("stray fs call\n");
	u.u_error = EINVAL;
	return(NULL);
}

int *
fsistray()
{
	printf("stray fs (int *) call\n");
	u.u_error = EINVAL;
	return(NULL);
}

struct inode *
fsinstray()
{
	printf("stray fs (inode *) call\n");
	u.u_error = EINVAL;
	return(NULL);
}

sysfs()
{

	register struct uap {
		int	opcode;
	} *uap;

	uap = (struct uap *) u.u_ap;
	switch (uap->opcode) {
	case GETFSIND:   
	{

	/*
	 *	Translate fs identifier to an index
	 *	into the fsinfo structure.
	 */
		register struct	a {
			int	opcode;
			char	*fsname;
		} *uap;
		register int	index;
		register struct	fsinfo	*ap;
		char	fsbuf[FSTYPSZ];


		uap = (struct a *) u.u_ap;
		switch (upath(uap->fsname,fsbuf,FSTYPSZ)) {
		case -2:	/* too long  */
		case  0:	/* too short */
			u.u_error = EINVAL;
			return;
		case -1:	/* bad user address */
			u.u_error = EFAULT;
			return;
		}

	/*
	 *	Search fsinfo structure for the fs identifier
	 *	and return the index.
	 */

		for (ap=fsinfo, index=0; ap < &fsinfo[nfstyp]; ap++, index++)
			if (!strcmp(ap->fs_name,fsbuf)) {
				u.u_rval1 = index;
				return;
			}

		u.u_error = EINVAL;
		break;
	}

	case GETFSTYP:
	{

	/*
	 *	Translate fstype index into an fs identifier
	 */
		register struct a {
			int	opcode;
			int	fs_index;
			char	*cbuf;
		} *uap;
		register char	*src;
		register int	index;
		register struct	fsinfo	*ap;
		char	*osrc;


		uap = (struct a *) u.u_ap;
		index = uap->fs_index;
		if (index <= 0 || index >= nfstyp) {
			u.u_error = EINVAL;
			return;
		}
		ap = &fsinfo[index];
		src = ap->fs_name ? ap->fs_name : "";

		for (osrc = src; *src++;)
			;
	
		if (copyout(osrc, uap->cbuf, src - osrc))
			u.u_error = EFAULT;

		break;
	}

	case GETNFSTYP:

	/*
	 *	Return number of fstypes configured in the
	 *	system.
	 */

		u.u_rval1 = nfstyp - 1;
		break;

	default:
		u.u_error = EINVAL;
	}
}
