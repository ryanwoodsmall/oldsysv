/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/* @(#)flock.c	1.5 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/proc.h"
#include "sys/user.h"
#include "sys/flock.h"

/* region types */
#define	S_BEFORE	010
#define	S_START		020
#define	S_MIDDLE	030
#define	S_END		040
#define	S_AFTER		050
#define	E_BEFORE	001
#define	E_START		002
#define	E_MIDDLE	003
#define	E_END		004
#define	E_AFTER		005

#define	SLEEP(ptr, pri)		sleep(ptr, pri)
#define	WAKEUP(ptr)		if (ptr->stat.wakeflg) { \
					wakeup(ptr); \
					ptr->stat.wakeflg = 0 ; \
				}
#define l_end 		l_len
#define MAXEND  	017777777777

extern	struct	flckinfo flckinfo;	/* configuration and acct info		*/
struct	filock	*frlock;		/* pointer to record lock free list	*/
struct	flino	*frfid;			/* file id free list			*/
struct	flino	*fids;			/* file id head list			*/
struct	flino	sleeplcks;		/* head of chain of sleeping locks	*/

/* find file id */
struct flino *
findfid(fp)
struct file *fp;
{
	register struct flino *flip;
	register i;
	register dev_t d;
	register ino_t n;

	d = fp->f_inode->i_dev;
	n = fp->f_inode->i_number;
	flip = fids;
	while (flip != NULL) {
		if (flip->fl_dev == d && flip->fl_number == n) {
			flip->fl_refcnt++;
			break;
		}
		flip = flip->next;
	}
	return (flip);
}

struct flino *
allocfid(fp)
struct file *fp;
{
	struct flino *flip;

	flip = frfid;
	if (flip != NULL) {
		++flckinfo.filcnt;
		++flckinfo.filtot;
		/* remove from free list */
		frfid = flip->next;
		if (frfid != NULL)
			frfid->prev = NULL;

		/* insert into allocated file identifier list */
		if (fids != NULL)
			fids->prev = flip;
		flip->next = fids;
		fids = flip;

		/* set up file identifier info */
		++flip->fl_refcnt;
		flip->fl_dev = fp->f_inode->i_dev;
		flip->fl_number = fp->f_inode->i_number;
	}
	return (flip);
}

freefid(flip)
struct flino *flip;
{
	if (--flip->fl_refcnt <= 0 && flip->fl_flck == NULL) {
		--flckinfo.filcnt;
		if (flip->prev != NULL)
			flip->prev->next = flip->next;
		else
			fids = flip->next;
		if (flip->next != NULL)
			flip->next->prev = flip->prev;
		flip->fl_dev = 0;
		flip->fl_number = 0;
		flip->fl_refcnt = 0;
		if (frfid != NULL)
			frfid->prev = flip;
		flip->next = frfid;
		flip->prev = NULL;
		frfid = flip;
	}
}
	

/* build file lock free list
 */
flckinit()
{
	register i;

	for (i=0; i<flckinfo.fils; i++) {
		freefid(&flinotab[i]);
	}
	flckinfo.filcnt = 0;

	for (i=0; i<flckinfo.recs; i++) {
		if (frlock == NULL) {
			flox[i].next = flox[i].prev = NULL;
			frlock = &flox[i];
		} else {
			flox[i].next = frlock;
			flox[i].prev = NULL;
			frlock = (frlock->prev = &flox[i]);
		}
	}
	flckinfo.reccnt = 0;
}

/* insert lock after given lock using locking data */

struct filock *
insflck(flip, lckdat, fl)
struct	flino	*flip;
struct	filock	*fl;
struct	flock	*lckdat;
{
	register struct filock *new;
	struct	filock	*f;

	new = frlock;
	if (new != NULL) {
		++flckinfo.reccnt;
		++flckinfo.rectot;
		frlock = new->next;
		if (frlock != NULL)
			frlock->prev = NULL;
		new->set = *lckdat;
		new->set.l_pid = u.u_procp->p_pid;
		new->stat.wakeflg = 0;
		if (fl == NULL) {
			new->next = flip->fl_flck;
			if (flip->fl_flck != NULL)
				flip->fl_flck->prev = new;
			flip->fl_flck = new;
		} else {
			new->next = fl->next;
			if (fl->next != NULL)
				fl->next->prev = new;
			fl->next = new;
		}
		new->prev = fl;
	}
	return (new);
}

/* delete lock
 */
delflck(flip, fl)
struct flino *flip;
struct filock *fl;
{
	if (fl->prev != NULL)
		fl->prev->next = fl->next;
	else
		flip->fl_flck = fl->next;
	if (fl->next != NULL)
		fl->next->prev = fl->prev;
	WAKEUP(fl);

	--flckinfo.reccnt;
	if (frlock == NULL) {
		fl->next = fl->prev = NULL;
		frlock = fl;
	} else {
		fl->next = frlock;
		fl->prev = NULL;
		frlock = (frlock->prev = fl);
	}
}

/* regflck sets the type of span of this (un)lock relative to the specified
 * already existing locked section.
 * There are five regions:
 *
 *  S_BEFORE        S_START         S_MIDDLE         S_END          S_AFTER
 *     010            020             030             040             050
 *  E_BEFORE        E_START         E_MIDDLE         E_END          E_AFTER
 *      01             02              03              04              05
 * 			|-------------------------------|
 *
 * relative to the already locked section.  The type is two octal digits,
 * the 8's digit is the start type and the 1's digit is the end type.
 */
int
regflck(ld, flp)
struct flock *ld;
struct filock *flp;
{
	register int regntype;

	if (ld->l_start > flp->set.l_start) {
		if (ld->l_start > flp->set.l_end)
			return(S_AFTER|E_AFTER);
		else if (ld->l_start == flp->set.l_end)
			return(S_END|E_AFTER);
		else
			regntype = S_MIDDLE;
	} else if (ld->l_start == flp->set.l_start)
		regntype = S_START;
	else
		regntype = S_BEFORE;

	if (ld->l_end > flp->set.l_start) {
		if (ld->l_end > flp->set.l_end)
			regntype |= E_AFTER;
		else if (ld->l_end == flp->set.l_end)
			regntype |= E_END;
		else
			regntype |= E_MIDDLE;
	} else if (ld->l_end == flp->set.l_start)
		regntype |= E_START;
	else
		regntype |= E_BEFORE;

	return (regntype);
}

/* Adjust file lock from region specified by 'ld' starting at lock 'insrtp'
 */
flckadj(flip, insrtp, ld)
struct flino	*flip;
struct filock	*insrtp;
struct flock	*ld;
{
	struct	flock	td;			/* lock data for severed lock */
	struct	filock	*flp, *nflp, *tdi, *tdp;
	int	insrtflg, rv = 0;
	int	regtyp;

	insrtflg = (ld->l_type != F_UNLCK) ? 1 : 0;

	nflp = (insrtp == NULL) ? flip->fl_flck : insrtp;
	while (flp = nflp) {
		nflp = flp->next;
		if (flp->set.l_pid == u.u_procp->p_pid) {

			regtyp = regflck(ld, flp);

			/* release already locked region if necessary */

			switch (regtyp) {
			case S_BEFORE|E_BEFORE:
				nflp = NULL;
				break;
			case S_BEFORE|E_START:
				if (ld->l_type == flp->set.l_type) {
					ld->l_end = flp->set.l_end;
					if (insrtp == flp)
						insrtp = flp->prev;
					delflck(flip, flp);
				}
				nflp = NULL;
				break;
			case S_START|E_END:
				/* don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return(rv);
			case S_START|E_AFTER:
				insrtp = flp->prev;
				delflck(flip, flp);
				break;
			case S_BEFORE|E_END:
				if (ld->l_type == flp->set.l_type)
					nflp = NULL;
			case S_BEFORE|E_AFTER:
				if (insrtp == flp)
					insrtp = flp->prev;
				delflck(flip, flp);
				break;
			case S_BEFORE|E_MIDDLE:
				if (ld->l_type == flp->set.l_type)
					ld->l_end = flp->set.l_end;
				else {
					/* setup piece after end of (un)lock */
					td = flp->set;
					td.l_start = ld->l_end;
					tdp = tdi = flp;
					do {
						if (tdp->set.l_start < ld->l_start)
							tdi = tdp;
						else
							break;
					} while (tdp = tdp->next);
					if (insflck(flip, &td, tdi) == NULL)
						return(ENOSPC);
				}
				if (insrtp == flp)
					insrtp = flp->prev;
				delflck(flip, flp);
				nflp = NULL;
				break;
			case S_START|E_MIDDLE:
			case S_MIDDLE|E_MIDDLE:
				/* don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return(rv);
				/* setup piece after end of (un)lock */
				td = flp->set;
				td.l_start = ld->l_end;
				tdp = tdi = flp;
				do {
					if (tdp->set.l_start < ld->l_start)
						tdi = tdp;
					else
						break;
				} while (tdp = tdp->next);
				if (insflck(flip, &td, tdi) == NULL)
					return(ENOSPC);
				if (regtyp == (S_MIDDLE|E_MIDDLE)) {
					/* setup piece before (un)lock */
					flp->set.l_end = ld->l_start;
					WAKEUP(flp);
					insrtp = flp;
				} else {
					insrtp = flp->prev;
					delflck(flip, flp);
				}
				nflp = NULL;
				break;
			case S_MIDDLE|E_END:
				/* don't bother if this is in the middle of
				 * an already similarly set section.
				 */
				if (ld->l_type == flp->set.l_type)
					return(rv);
				flp->set.l_end = ld->l_start;
				WAKEUP(flp);
				insrtp = flp;
				break;
			case S_MIDDLE|E_AFTER:
			case S_END|E_AFTER:
				if (ld->l_type == flp->set.l_type) {
					ld->l_start = flp->set.l_start;
					insrtp = flp->prev;
					delflck(flip, flp);
				} else {
					flp->set.l_end = ld->l_start;
					WAKEUP(flp);
					insrtp = flp;
				}
				break;
			case S_AFTER|E_AFTER:
				insrtp = flp;
				break;
			}
		} else {
			if (flp->set.l_start > ld->l_end)
				nflp = NULL;
		}
	}

	if (insrtflg) {
		if (flp = insrtp) {
			do {
				if (flp->set.l_start < ld->l_start)
					insrtp = flp;
				else
					break;
			} while (flp = flp->next);
		}
		if (insflck(flip, ld, insrtp) == NULL)
			rv = ENOSPC;
	}

	return (rv);
}

/* blocked checks whether a new lock (lckdat) would be
 * blocked by a previously set lock owned by another process.
 * When blocked is called, 'flp' should point
 * to the record from which the search should begin.
 * Insrt is set to point to the lock before which the new lock
 * is to be placed.
 */
struct filock *
blocked(flp, lckdat, insrt)
struct filock *flp;
struct flock *lckdat;
struct filock **insrt;
{
	struct filock *f;

	*insrt = NULL;
	for (f = flp; f != NULL; f = f->next) {
		if (f->set.l_start < lckdat->l_start)
			*insrt = f;
		else
			break;
		if (f->set.l_pid == u.u_procp->p_pid) {
			if (lckdat->l_start <= f->set.l_end
			    && lckdat->l_end >= f->set.l_start) {
				*insrt = f;
				break;
			}
		} else	if (lckdat->l_start < f->set.l_end
			    && lckdat->l_end > f->set.l_start
			    && (f->set.l_type == F_WRLCK
				|| (f->set.l_type == F_RDLCK
				    && lckdat->l_type == F_WRLCK)))
				return(f);
	}

	for ( ; f != NULL; f = f->next) {
		if (lckdat->l_start < f->set.l_end
		    && lckdat->l_end > f->set.l_start
		    && f->set.l_pid != u.u_procp->p_pid
		    && (f->set.l_type == F_WRLCK
			|| (f->set.l_type == F_RDLCK && lckdat->l_type == F_WRLCK)))
			return(f);
		if (f->set.l_start > lckdat->l_end)
			break;
	}

	return(NULL);
}

/* locate overlapping file locks
 */
getflck(fp, lckdat)
struct file *fp;
struct flock *lckdat;
{
	register struct flino *flip;
	struct filock *found, *insrt = NULL;
	register int retval = 0;
	int inwhence;		/* whence of request */

	/* get file identifier and file lock list pointer if there is one */
	flip = findfid(fp);
	if (flip == NULL) {
		lckdat->l_type = F_UNLCK;
		return (0);
	}

	/* convert start to be relative to beginning of file */
	inwhence = lckdat->l_whence;
	if (retval=convoff(fp, lckdat, 0))
		return (retval);
	if (lckdat->l_len == 0)
		lckdat->l_end = MAXEND;
	else
		lckdat->l_end += lckdat->l_start;

	/* find overlapping lock */

	found = blocked(flip->fl_flck, lckdat, &insrt);
	if (found != NULL)
		*lckdat = found->set;
	else
		lckdat->l_type = F_UNLCK;
	freefid(flip);

	/* restore length */
	if (lckdat->l_end == MAXEND)
		lckdat->l_len = 0;
	else
		lckdat->l_len -= lckdat->l_start;

	retval = convoff(fp, lckdat, inwhence);
	return (retval);
}

/* clear and set file locks
 */
setflck(fp, lckdat, slpflg)
struct file *fp;
struct flock *lckdat;
int slpflg;
{
	register struct flino *flip;
	register struct filock *found, *sf;
	struct filock *insrt = NULL;
	register int retval = 0;
	int contflg = 0;

	/* check access permissions */
	if ((lckdat->l_type == F_RDLCK && (fp->f_flag&FREAD) == 0)
	    || (lckdat->l_type == F_WRLCK && (fp->f_flag&FWRITE) == 0))
		return (EBADF);
	
	/* convert start to be relative to beginning of file */
	if (retval=convoff(fp, lckdat, 0))
		return (retval);
	if (lckdat->l_len == 0)
		lckdat->l_end = MAXEND;
	else
		lckdat->l_end += lckdat->l_start;

	/* get or create a file/inode record lock header */
	flip = findfid(fp);
	if (flip == NULL) {
		if (lckdat->l_type == F_UNLCK)
			return (0);
		if ((flip=allocfid(fp)) == NULL)
			return (EMFILE);
	}

	do {
		contflg = 0;
		switch (lckdat->l_type) {
		case F_RDLCK:
		case F_WRLCK:
			if ((found=blocked(flip->fl_flck, lckdat, &insrt)) == NULL)
				retval = flckadj(flip, insrt, lckdat);
			else if (slpflg) {
				/* do deadlock detection here */
				if (deadflck(found))
					retval = EDEADLK;
				else
					if ((sf=insflck(&sleeplcks, lckdat, NULL)) == NULL)
						retval = ENOSPC;
					else {
						found->stat.wakeflg++;
						sf->stat.blkpid = found->set.l_pid;
						if (SLEEP(found, PCATCH|(PZERO+1)))
							retval = EINTR;
						else
							contflg = 1;
						sf->stat.blkpid = 0;
						delflck(&sleeplcks, sf);
					}
			} else
				retval = EACCES;
			break;
		case F_UNLCK:
			/* removing a file record lock */
			retval = flckadj(flip, flip->fl_flck, lckdat);
			break;
		default:
			retval = EINVAL;	/* invalid lock type */
			break;
		}
	} while (contflg);
	freefid(flip);
	return(retval);
}

/* convoff - converts the given data (start, whence) to the
 * given whence.
 */
int
convoff(fp, lckdat, whence)
struct file *fp;
struct flock *lckdat;
int whence;
{
	if (lckdat->l_whence == 1)
		lckdat->l_start += fp->f_offset;
	else if (lckdat->l_whence == 2)
		lckdat->l_start += fp->f_inode->i_size;
	else if (lckdat->l_whence != 0)
		return (EINVAL);
	if (lckdat->l_start < 0)
		return (EINVAL);
	if (whence == 1)
		lckdat->l_start -= fp->f_offset;
	else if (whence == 2)
		lckdat->l_start -= fp->f_inode->i_size;
	else if (whence != 0)
		return (EINVAL);
	lckdat->l_whence = whence;
	return (0);
}

/* deadflck does the deadlock detection for the given record */
int
deadflck(flp)
struct filock *flp;
{
	register struct filock *blck, *sf;
	int blckpid;

	blck = flp;	/* current blocking lock pointer */
	blckpid = blck->set.l_pid;
	do {
		if (blckpid == u.u_procp->p_pid)
			return(1);
		/* if the blocking process is sleeping on a locked region,
		 * change the blocked lock to this one.
		 */
		for (sf = sleeplcks.fl_flck; sf != NULL; sf = sf->next) {
			if (blckpid == sf->set.l_pid) {
				blckpid = sf->stat.blkpid;
				break;
			}
		}
		blck = sf;
	} while (blck != NULL);
	return(0);
}

/* Clean up record locks left around by process (called in closef) */
cleanlocks(fp)
struct file *fp;
{
	register struct filock *flp, *nflp;
	register struct flino *flip;

	flip = findfid(fp);
	if (flip == NULL)
		return;

	for (flp=flip->fl_flck; flp!=NULL; flp=nflp) {
		nflp = flp->next;
		if (flp->set.l_pid == u.u_procp->p_pid)
			delflck(flip, flp);
	}
	freefid(flip);

	return;
}
