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
/* @(#)exec.c	1.3 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/sysmacros.h>
#include <sys/systm.h>
#include <sys/map.h>
#include <sys/dir.h>
#include <sys/signal.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/file.h>
#include <sys/page.h>
#include <sys/region.h>
#include <sys/buf.h>
#include <sys/inode.h>
#include <sys/acct.h>
#include <sys/sysinfo.h>
#include <sys/seg.h>
#include <sys/reg.h>
#include <sys/var.h>
#include <sys/proc.h>
#include <sys/mmu.h>
#include <sys/debug.h>


#define NPARGS 	btop(NCARGS)

reg_t	execreg;

/*
 * exec system call, with and without environments.
 */
struct execa {
	char	*fname;
	char	**argp;
	char	**envp;
};

exece()
{
	register unsigned nc;
	register char *cp;
	register struct execa *uap;
	register char *psap;
	int na, ne, ucp, ap, c;
	char *args;
	int saveargs;
	unsigned bno;
	struct inode *ip;
	extern struct inode *gethead();

	sysinfo.sysexec++;
	if ((ip = gethead()) == NULL)
		return;
	na = nc = ne = 0;
	uap = (struct execa *)u.u_ap;
	/* collect arglist */
	if ((saveargs = sptalloc(NPARGS, PG_V|PG_KW, 0)) == NULL) {
		iput(ip);
		u.u_error = ENOMEM;
		return;
	}
	args = (char *) saveargs;
	if (uap->argp) for (;;) {
		ap = NULL;
		if (uap->argp) {
			ap = fuword((caddr_t)uap->argp);
			uap->argp++;
		}
		if (ap==NULL && uap->envp) {
			uap->argp = NULL;
			if ((ap = fuword((caddr_t)uap->envp)) == NULL)
				break;
			uap->envp++;
			ne++;
		}
		if (ap==NULL)
			break;
		na++;
		if (ap == -1)
			u.u_error = EFAULT;
		do {
			if (nc >= NCARGS)
				u.u_error = E2BIG;
			if ((c = fubyte((caddr_t)ap++)) < 0)
				u.u_error = EFAULT;
			if (u.u_error)
				goto bad;
			nc++;
			*args++ = c;
		} while (c>0);
	}
	nc = (nc + NBPW-1) & ~(NBPW-1);
	getxfile(ip, nc + sizeof(char *)*na);
	if (u.u_error) {
		psignal(u.u_procp, SIGKILL);
		goto bad;
	}

	/* copy back arglist */

	args = (char *) saveargs;
	psap = u.u_psargs;
	ucp = USRSTACK - nc - NBPW;
	ap = ucp - na*NBPW - 3*NBPW;
	u.u_ar0[SP] = ap;
	suword((caddr_t)ap, na-ne);
	nc = 0;
	for (;;) {
		ap += NBPW;
		if (na==ne) {
			suword((caddr_t)ap, 0);
			while (psap < &u.u_psargs[PSARGSZ])
				*psap++ = 0;
			ap += NBPW;
		}
		if (--na < 0)
			break;
		suword((caddr_t)ap, ucp);
		do {
			subyte((caddr_t)ucp++, (c = *args++));
			if (psap < &u.u_psargs[PSARGSZ])
				*psap++ = (c&0377) ? c : ' ';
			nc++;
		} while (c&0377);
	}
	while (psap < &u.u_psargs[PSARGSZ])
		*psap++ = 0;
	setregs();
	iput(ip);
	sptfree(saveargs, NPARGS, 1);
	return;
bad:
	iput(ip);
	sptfree(saveargs, NPARGS, 1);
}


struct inode *
gethead()
{
	register struct inode *ip;
	register unsigned ds, ts;

	struct	naout	{
		short	magic;
		short	vstamp;
		long	tsize,
			dsize,
			bsize,
			msize,
			mstart,
			entry,
			ts,
			ds,
			flags;
	};

	struct	filhd	{
		unsigned short	magic;
		unsigned short	nscns;
		long		timdat,
				symptr,
				nsyms;
		unsigned short	opthdr,
				flags;
	};

	struct	scnhdr	{
		char		s_name[8];
		long		s_paddr,
				s_vaddr,
				s_size,
				s_scnptr,
				s_relptr,
				s_lnnoptr;
		unsigned short	s_nreloc,
				s_nlnno;
		long		s_flags,
				s_symptr;
		unsigned short	s_modno,
				s_pad;
	};
	struct  ehd{
		struct filhd ef;
		struct naout af;
		struct scnhdr sf[4];
	}*ep;
	struct buf *bp;
	daddr_t	bn;

	if ((ip = namei(uchar, 0)) == NULL)
		return(NULL);
	if (access(ip, IEXEC) ||
	   (ip->i_mode & IFMT) != IFREG ||
	   (ip->i_mode & (IEXEC|(IEXEC>>3)|(IEXEC>>6))) == 0) {
		u.u_error = EACCES;
		goto bad;
	}
	/*
	 * read in first few bytes of file for segment sizes
	 * ux_mag = 407/410/413
	 *  407 is plain executable
	 *  410 is RO text
	 *  413 is page-aligned text and data
	 *  524 Common object
	 *  set ux_tstart to start of text portion
	 */
	/*
	 * Read file header assuming new post-5.0 format
	 */
	u.u_count = sizeof(*ep);
	u.u_offset = 0;
	bp = 0;
	bn = bmap(ip, B_READ);
	if (u.u_error)
		goto bad;
	if (bn < 0) {
		u.u_error = ENOEXEC;
		goto bad;
	}
	bp = bread(u.u_pbdev, (daddr_t)bn);
	ep = (struct ehd *)bp->b_un.b_addr;
	/*
	 * Setup pointers to virtual address space
	 */
	if (ep->ef.magic == 0524) {
		bcopy(&ep->af, &u.u_exdata, 4*NBPW);
		bcopy(&ep->af.entry, &u.u_exdata.ux_entloc, 3*NBPW);
		u.u_exdata.ux_tstart = sizeof(struct naout) +
			sizeof(struct filhd) + (ep->ef.nscns * sizeof(struct scnhdr));
	} else {
		u.u_exdata.ux_mag = 0;
	}
	brelse(bp);
	u.u_segflg = 0;
	/*
	 * Nonshared 407's: A vestige from the past
	 */
	if (u.u_exdata.ux_mag == 0407) {
		ds = btop((long)u.u_exdata.ux_tsize +
			(long)u.u_exdata.ux_dsize +
			(long)u.u_exdata.ux_bsize);
		ts = 0;
		u.u_exdata.ux_dsize += u.u_exdata.ux_tsize;
		u.u_exdata.ux_tsize = 0;
		u.u_exdata.ux_datorg = u.u_exdata.ux_txtorg;
	} else if (u.u_exdata.ux_mag == 0410 || u.u_exdata.ux_mag == 0413) {
		ts = btop(u.u_exdata.ux_tsize);
		ds = btop(u.u_exdata.ux_dsize+u.u_exdata.ux_bsize);
		if ((ip->i_flag&ITEXT)==0 && ip->i_count!=1) {
			register struct file *fp;

			for (fp = file; fp < (struct file *)v.ve_file; fp++)
				if (fp->f_count && fp->f_inode == ip &&
				    (fp->f_flag&FWRITE)) {
					u.u_error = ETXTBSY;
					goto bad;
				}
		}
		ip->i_flag |= ITEXT;
	} else  {
		u.u_error = ENOEXEC;
		goto bad;
	}
	chksize(ts, ds, SSIZE + btop(NCARGS - 1));
bad:
	if (u.u_error) {
		iput(ip);
		ip = NULL;
	}
	return(ip);
}
/*
 * Read in and set up memory for executed file.
 */
getxfile(ip, nargc)
register struct inode *ip;
{
	register	size, npgs, t, base;
	register reg_t	*rp;
	register preg_t	*prp;
	struct proc	*p;
	dbd_t		*dbd;
	int		rgva;
	int		offset;
	int		(**fptr)();
	pte_t		*pt;
	int		*ptr;

	shmexec();
	punlock();
	u.u_prof.pr_scale = 0;
	p = u.u_procp;

	/*	Free the processes current regions.  Note that
	 *	detachreg compacts the region list in the proc
	 *	table.
	 */

	prp = p->p_region;
	while(rp = prp->p_reg){
		reglock(rp);
		detachreg(&u, prp);
	}

	clratb(USRATB);

	/*
	 *	the process can no longer deal for itself
	 */
	for (ptr = &u.u_signal[0]; ptr < &u.u_signal[NSIG]; ptr++)
		if (*ptr != 1)
			*ptr = 0;

	/*
	 *	if 410, load it like a 407
	 */
	if (u.u_exdata.ux_mag == 0410) {
		if (get410(ip) < 0) {
			goto out;
		}
		else
			goto stack;
	}

	/*	Load text region
	 */

	if (xalloc(ip) == -1){
		goto out;
	}

	/*	Allocate the data region.
	 */

	base = u.u_exdata.ux_datorg;
	u.u_datorg = u.u_exdata.ux_datorg;
	size = u.u_exdata.ux_dsize;
	rgva = base & ~SOFFMASK;

	if((rp = allocreg(ip, RT_PRIVATE)) == NULL){
		goto out;
	}

	/*	Attach the data region to this process.
	 */
	
	if((prp = attachreg(rp, &u,  rgva, PT_DATA, PG_V | PG_UW)) == NULL){
		freereg(rp);
		goto out;
	}

	/*
	 * Load data region
	 */

	if (size) {
		offset = u.u_exdata.ux_tstart + u.u_exdata.ux_tsize;
		if(u.u_exdata.ux_mag == 0413){
			if(mapreg(prp, base, ip, offset, size) < 0){
				detachreg(&u, prp);
				goto out;
			}
		} else {
			if(loadreg(prp, base,  ip, offset, size) < 0){
				detachreg(&u, prp);
				goto out;
			}
		}
	}

	/*
	 * Allocate bss as demand zero
	 */
	npgs = btop(soff(base) + size + u.u_exdata.ux_bsize) - btop(soff(base) + size);
	if (npgs) {
		if(growreg(&u, prp, npgs, 0, DBD_DZERO) < 0){
			detachreg(&u, prp);
			goto out;
		}
	}
	regrele(rp);

	/*	Grow the stack.
	 */
	
stack:
	if ((rp=allocreg(NULL, RT_PRIVATE)) == NULL)
		goto out;
	if ((prp=attachreg(rp, &u, 0x1000000, PT_STACK, SEG_RW)) == NULL){
		freereg(rp);
		goto out;
	}
	npgs = SSIZE + btop(nargc);
	if(growreg(&u, prp, npgs, 0, DBD_DZERO) < 0){
		regrele(prp->p_reg);
		goto out;
	}
	regrele(prp->p_reg);

	/*
	 * set SUID/SGID protections, if no tracing
	 */
	if ((u.u_procp->p_flag&STRC)==0) {
		if (ip->i_mode&ISUID)
			u.u_uid = ip->i_uid;
		if (ip->i_mode&ISGID)
			u.u_gid = ip->i_gid;
		u.u_procp->p_suid = u.u_uid;
	} else
		psignal(u.u_procp, SIGTRAP);

	return;

out:
	ip->i_flag &= ~ITEXT;
	u.u_error = ENOEXEC;
}

/*
 *	load 410 text and data into a single region
 */
get410(ip)
struct inode *ip;
{
	int size, org, base, change, count, npgs;
	preg_t *prp;
	reg_t *rp;

	/*
	 *	load the text
	 */
	size =  u.u_exdata.ux_tsize;
	if ((rp = allocreg(ip, RT_PRIVATE)) == NULL) {
		return(-1);
	}
	
	org = u.u_exdata.ux_txtorg;
	if ((prp = attachreg(rp, &u, 0, PT_DATA, SEG_RW)) == NULL) {
		freereg(rp);
		return(-1);
	}

	if (loadreg(prp, org, ip, u.u_exdata.ux_tstart, size) < 0) {
		detachreg(&u, prp);
		return(-1);
	}

	/*
	 *	load data
	 */
	base = u.u_exdata.ux_datorg;
	u.u_datorg = u.u_exdata.ux_datorg;
	size = u.u_exdata.ux_dsize;

	if (size) {
		int tmp;

		change = btop(base + size) - rp->r_pgsz;
		if (growreg(&u, prp, change, change, DBD_DZERO) <0) {
			detachreg(&u, prp);
			return(-1);
		}

		u.u_segflg = 0;
		u.u_base = (caddr_t)base;
		u.u_count = size;
		u.u_offset = u.u_exdata.ux_tstart + u.u_exdata.ux_tsize;

		regrele(rp);
		readi(ip);
		reglock(rp);

		if (u.u_count) {
			regrele(rp);
			return(-1);
		}

	}

	/*
	 *	allocate bss as demand zero
	 */
	npgs = btop(size + u.u_exdata.ux_bsize) - btop(size);
	if (npgs)
		if (growreg(&u, prp, npgs, 0, DBD_DZERO) < 0) {
			detachreg(&u, prp);
			return(-1);
		}
	
	regrele(rp);
	return(0);
}
