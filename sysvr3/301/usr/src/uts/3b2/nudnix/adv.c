/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:nudnix/adv.c	10.17"
/*
 *	advertise / unadvertise
 */

#include "sys/types.h"
#include "sys/sema.h"
#include "sys/param.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/comm.h"
#include "sys/inode.h"
#include "sys/mount.h"
#include "sys/errno.h"
#include "sys/adv.h"
#include "sys/nserve.h"
#include "sys/debug.h"
#include "sys/rdebug.h"
#include "sys/inline.h"


extern	int	bootstate;
extern	int	nadvertise;
char	*addalist();
struct advertise	*getadv();
int	adv_lck =  0;		/* advertise table lock */

/*
 *	Advertise a file system service to the name server.
 *
 */

advfs()
{
	struct	a	{
		char	*fs;		/* root of file system */
		char	*svcnm;		/* global name given to name server */
		int	rwflag;		/* readonly/read write flag	*/
		char	**clist;	/* client list			*/
	} *uap = (struct a *) u.u_ap;
	struct	inode	*ip;
	struct	advertise	*ap, *a, *findadv();
	rcvd_t	gift;			/* rcvd for advertised resource */

	char	*to, *from, adv_name [NMSZ];	/* temps */
	struct	inode	*namei();
	extern	rcvd_t	cr_rcvd();
	extern  int 	nservers;

	if (bootstate != DU_UP)  {
		u.u_error = ENONET;
		return;
	}

	if(nservers == 0) {
		u.u_error = ENOMEM;
		return;
	}
	if (!suser())  
		return;

	/* get the name to be advertised */
	switch (upath(uap->svcnm,adv_name,NMSZ)) {
	case -2:	/* too long	*/
	case  0:	/* too short	*/
		u.u_error = EINVAL;
		return;
	case -1:	/* bad user address	*/
		u.u_error = EFAULT;
		return;
	}

	/*
	 * if this is a modify request, all we need to do is replace
	 * the client list and return.
	 */
	if (uap->rwflag & A_MODIFY) {
		if ((ap = findadv (adv_name)) == NULL) {
			u.u_error = ENODEV;
			return;
		}
		if(ap->a_flags & A_MINTER) {
			u.u_error = ENODEV;
			return;
		}
		/* remalist/addalist   can not go to sleep */
		if (uap->rwflag & A_CLIST) {
			if (ap->a_clist)
				remalist(ap->a_clist);
		
			ap->a_clist = (uap->clist)?addalist(uap->clist):NULL;
		}
		return;
	}

	if ((ip = namei(upath, 0)) == NULL)  {
		if (!u.u_error)
			u.u_error = ENOENT;
		return;
	}

	if (ip->i_ftype != IFDIR)  {
		u.u_error = ENOTDIR;
		iput(ip);
		return;
	}

	if (ip->i_flag & IADV)  {
		u.u_error = EADV;
		iput(ip);
		return;
	}


	/* search the advertise table for a free slot */
	for (ap = advertise, a = 0; ap < &advertise[nadvertise]; ap++)
		if (ap->a_flags == A_FREE) {
			if (!a)
				a = ap;
		} else  {
			if (!strcmp(ap->a_name, adv_name)) {
				u.u_error = (ap->a_flags & A_MINTER)?
					ENXIO:EBUSY;
				iput(ip);
				return;
			} 
			if (ap->a_queue->rd_inode == ip) {
				u.u_error = EEXIST;
				iput(ip);
				return;
			}
		}
		
	if (!(ap = a)) {
		iput(ip);
		u.u_error = ENOSPC;
		return;
	}

	ap->a_flags = A_INUSE;

	/*
	 * if file system was mounted read only,
	 * the advertisement must be read only.
	 */
	if (((ip->i_mntdev)->m_flags & MRDONLY) && !(uap->rwflag & A_RDONLY)) {
		iput(ip);
		u.u_error = EROFS;
		ap->a_flags = A_FREE;
		return;
	}
	if (uap->rwflag & A_RDONLY)
		ap->a_flags |= A_RDONLY;
	/*
	 * now add authorization list, if necessary.
	 */
	if ((uap->rwflag & A_CLIST) && uap->clist != NULL) {
		if ((ap->a_clist=addalist(uap->clist)) == NULL) {
			iput(ip);
			ap->a_flags = A_FREE;
			return;
		}
	}
	else
		ap->a_clist = NULL;

	/*
	 * allocate queue for advertised object.
	 */
	if ((gift = cr_rcvd (MOUNT_QSIZE, GENERAL)) == NULL) { 
			u.u_error = ENOMEM;
			iput(ip);
			if (ap->a_clist)
				remalist(ap->a_clist);
			ap->a_flags = A_FREE;
			return;
	}
	ip->i_flag |= IADV;
	ip->i_rcvd = gift;	/* add gift to inode */
	prele(ip);
	ap->a_count = 0;
	ap->a_queue = gift;
	for (from = adv_name, to = ap->a_name;
		*from != NULL  &&  from < &adv_name[NMSZ];)
			*to++ = *from++;
	*to = NULL;
	gift->rd_max_serv = MOUNT_QSIZE;
	gift->rd_inode = ip;	/* associate gift with inode */
	DUPRINT2(DB_MNT_ADV,"exit adv: u_error is %d\n",u.u_error);
}

/*
 *	Unadvertise a remotely accessible filesystem.
 */

unadvfs()
{
	struct	a	{
		char	*svcnm;		/* global name given to name server */
	} *uap = (struct a *) u.u_ap;
	struct	inode	*ip;
	struct	advertise	*ap, *findadv();
	char	adv_name [NMSZ];	/* temp */

	if (bootstate != DU_UP)  {
		u.u_error = ENONET;
		return;
	}

	if (!suser()) 
		return;

	/* get the name to be unadvertised */
	switch (upath(uap->svcnm,adv_name,NMSZ)) {
	case -2:	/* too long	*/
	case  0:	/* too short	*/
		u.u_error = EINVAL;
		return;
	case -1:	/* bad user address	*/
		u.u_error = EFAULT;
		return;
	}

	if ((ap = findadv (adv_name)) == NULL) {
		u.u_error = ENODEV;
		return;

	}
	if(ap->a_flags & A_MINTER) {
		u.u_error = ENODEV;
		return;
	}
	ap->a_flags |= A_MINTER;

	ip = ap->a_queue->rd_inode;
	plock (ip);
	ip->i_flag &= ~IADV;
	del_rcvd (ap->a_queue, NULL);
	/*
	 * get rid of auth list regardless of whether entry goes
	 * away now or later.
	 */
	if (ap->a_clist) {
		remalist(ap->a_clist);
		ap->a_clist = NULL;
	}
	if (ap->a_count <= 0) {
		ap->a_flags = A_FREE;
		iput (ip);	/* advertise did the nami/iget */
	}  else
		prele(ip);  /* else this is done in rumount */
}

/*  Advertise table lookup using inode pointer as the key.
 */

struct	advertise *
getadv (ip)
struct	inode	*ip;
{
	register	struct	advertise	*ap;
	
	for (ap = advertise; ap < &advertise[nadvertise]; ap++)
		if (ap->a_flags & A_INUSE)
		if (ap->a_queue->rd_inode == ip)
			return (ap);
	return (NULL);
}

/* Advertise table lookup using a_name as the key.
 */

struct advertise *
findadv(name)
char	*name;
{
	register struct advertise	*ap;

	for (ap=advertise; ap < &advertise[nadvertise]; ap++)
		if ((ap->a_flags & A_INUSE) &&
		    (strcmp(name,ap->a_name) == NULL))
			return(ap);
	return(NULL);
}

