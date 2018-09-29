/* @(#)sys1.c	1.5 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/map.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/proc.h"
#include "sys/buf.h"
#include "sys/reg.h"
#include "sys/file.h"
#include "sys/inode.h"
#include "sys/seg.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"
#include "sys/var.h"
#include "sys/lock.h"

/*
 * exec system call, with and without environments.
 */

struct execa {
	char	*fname;
	char	**argp;
	char	**envp;
};

exec()
{
	((struct execa *)u.u_ap)->envp = NULL;
	exece();
}

#define	NCABLK	(NCARGS+BSIZE-1)/BSIZE
exece()
{
	register unsigned nc;
	register char *cp;
	register struct buf *bp;
	register struct execa *uap;
	int na, ne, ucp, ap, c;
	unsigned bno;
	struct inode *ip;
	extern struct inode *gethead();

	sysinfo.sysexec++;
	if ((ip = gethead()) == NULL)
		return;
	bp = 0;
	na = nc = ne = 0;
	uap = (struct execa *)u.u_ap;
	/* collect arglist */
	if ((bno = swapalloc(NCABLK, 0)) == 0) {
		printf("No swap space for exec args\n");
		iput(ip);
		u.u_error = ENOMEM;
		return;
	}
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
			if (nc >= NCARGS-1)
				u.u_error = E2BIG;
			if ((c = fubyte((caddr_t)ap++)) < 0)
				u.u_error = EFAULT;
			if (u.u_error)
				goto bad;
			if ((nc&BMASK) == 0) {
				if (bp)
					bdwrite(bp);
				bp = getblk(swapdev, swplo+bno+(nc>>BSHIFT));
				cp = NULL;
			}
			nc++;
			biputc(bp, cp++, c);
		} while (c>0);
	}
	if (bp)
		bdwrite(bp);
	bp = 0;
	nc = (nc + NBPW-1) & ~(NBPW-1);
	getxfile(ip, nc + sizeof(char *)*na);
	if (u.u_error) {
		psignal(u.u_procp, SIGKILL);
		goto bad;
	}
	if (u.u_exdata.ux_mag == 0405)
		goto bad;

	/* copy back arglist */

	ucp = USRSTACK - nc - NBPW;
	ap = ucp - na*NBPW - 3*NBPW;
	u.u_ar0[SP] = ap;
	suword((caddr_t)ap, na-ne);
	nc = 0;
	for (;;) {
		ap += NBPW;
		if (na==ne) {
			ap += NBPW;
		}
		if (--na < 0)
			break;
		suword((caddr_t)ap, ucp);
		do {
			if ((nc&BMASK) == 0) {
				if (bp)
					brelse(bp);
				bp = bread(swapdev, swplo+bno+(nc>>BSHIFT));
				bp->b_flags |= B_AGE;
				bp->b_flags &= ~B_DELWRI;
				cp = NULL;
			}
			subyte((caddr_t)ucp++, (c = bigetc(bp, cp++)));
			nc++;
		} while (c&0377);
	}
	setregs();
	if (bp)
		brelse(bp);
	iput(ip);
	mfree(swapmap, NCABLK, bno);
	return;
bad:
	if (bp)
		brelse(bp);
	iput(ip);
	for (nc = 0; nc < NCABLK; nc++) {
		bp = getblk(swapdev, swplo+bno+nc);
		bp->b_flags |= B_AGE;
		bp->b_flags &= ~B_DELWRI;
		brelse(bp);
	}
	mfree(swapmap, NCABLK, bno);
}

struct inode *
gethead()
{
	register struct inode *ip;
	register unsigned ds, ts;
	register sep;
	int savbitm;

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
	 * ux_mag = 407/410/411/405
	 *  407 is plain executable
	 *  410 is RO text
	 *  411 is separated ID
	 *  405 is overlaid text
	 */
	u.u_base = (caddr_t)&u.u_exdata;
	u.u_count = sizeof(u.u_exdata);
	u.u_offset = 0;
	u.u_segflg = 1;
	readi(ip);
	u.u_segflg = 0;
	if (u.u_count!=0)
		u.u_exdata.ux_mag = 0;
	sep = 0;
	if (u.u_exdata.ux_mag == 0407) {
		ds = btoc((long)u.u_exdata.ux_tsize +
			(long)u.u_exdata.ux_dsize +
			(long)u.u_exdata.ux_bsize);
		ts = 0;
		u.u_exdata.ux_dsize += u.u_exdata.ux_tsize;
		u.u_exdata.ux_tsize = 0;
	} else {
		ts = btoc(u.u_exdata.ux_tsize);
		ds = btoc(u.u_exdata.ux_dsize+u.u_exdata.ux_bsize);
		if ((ip->i_flag&ITEXT)==0 && ip->i_count!=1) {
			register struct file *fp;

			for (fp = file; fp < (struct file *)v.ve_file; fp++)
				if (fp->f_count && fp->f_inode == ip &&
				    (fp->f_flag&FWRITE)) {
					u.u_error = ETXTBSY;
					goto bad;
				}
		}
		if (u.u_exdata.ux_mag == 0411)
			sep++;
		else if (u.u_exdata.ux_mag == 0405) {
			if (u.u_sep==0 && ctos(ts) != ctos(u.u_tsize))
				u.u_error = ENOMEM;
			goto bad;
		}
		else if (u.u_exdata.ux_mag != 0410) {
			u.u_error = ENOEXEC;
			goto bad;
		}
	}
	savbitm = u.u_mbitm;
	u.u_mbitm = 0;
	if (checkur(ts,ds,SSIZE+btoc(NCARGS-1),sep))
		u.u_mbitm = savbitm;
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
	register unsigned ds;
	register i;

	if (u.u_lock & (TXTLOCK|DATLOCK))
		punlock();
	xfree();
	u.u_tsize = btoc(u.u_exdata.ux_tsize);
	if (u.u_exdata.ux_mag == 0405) {
		xalloc(ip);
		u.u_ar0[PC] = u.u_exdata.ux_entloc & ~01;
	} else {
		u.u_prof.pr_scale = 0;
		u.u_ssize = SSIZE + btoc(nargc);
		u.u_dsize = btoc(u.u_exdata.ux_dsize + u.u_exdata.ux_bsize);
		i = USIZE+u.u_dsize+u.u_ssize;
		expand(i);
		ds = USIZE+((u.u_exdata.ux_dsize>>6)&01777);
		while (--i >= ds)
			clearseg(u.u_procp->p_addr+i);
		xalloc(ip);
		/* read in data segment */
		estabur((unsigned)0, u.u_dsize, (unsigned)0, 0, RO);
		u.u_base = 0;
		u.u_offset = sizeof(u.u_exdata)+u.u_exdata.ux_tsize;
		u.u_count = u.u_exdata.ux_dsize;
		readi(ip);
		if (u.u_count!=0)
			u.u_error = EFAULT;
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
		u.u_sep = (u.u_exdata.ux_mag == 0411)?1:0;
	}
	estabur(u.u_tsize, u.u_dsize, u.u_ssize,u.u_sep,RO);
}

/*
 * Clear registers on exec
 */
setregs()
{
	register int *rp;
	register char *cp;
	register i;

	for (rp = &u.u_signal[0]; rp < &u.u_signal[NSIG]; rp++)
		if ((*rp & 1) == 0)
			*rp = 0;
	for (cp = &regloc[0]; cp < &regloc[6];)
		u.u_ar0[*cp++] = 0;
	u.u_ar0[PC] = u.u_exdata.ux_entloc & ~01;
	for (rp = (int *)&u.u_fps; rp < (int *)&u.u_fps.u_fpregs[6];)
		*rp++ = 0;
	for (i=0; i<NOFILE; i++) {
		if ((u.u_pofile[i]&EXCLOSE) && u.u_ofile[i] != NULL) {
			closef(u.u_ofile[i]);
			u.u_ofile[i] = NULL;
		}
	}
	/*
	 * Remember file name for accounting.
	 */
	u.u_acflag &= ~AFORK;
	bcopy((caddr_t)u.u_dent.d_name, (caddr_t)u.u_comm, DIRSIZ);
}

/*
 * exit system call:
 * pass back caller's arg
 */
rexit()
{
	register struct a {
		int	rval;
	} *uap;

	uap = (struct a *)u.u_ap;
	exit((uap->rval & 0377) << 8);
}

/*
 * Release resources.
 * Enter zombie state.
 * Wake up parent and init processes,
 * and dispose of children.
 */
exit(rv)
{
	register int i;
	register struct proc *p, *q;

	p = u.u_procp;
	p->p_flag &= ~(STRC);
	p->p_clktim = 0;
	for (i=0; i<NSIG; i++)
		u.u_signal[i] = 1;
	expand(USIZE);
	if ((p->p_pid == p->p_pgrp)
	 && (u.u_ttyp != NULL)
	 && (*u.u_ttyp == p->p_pgrp)) {
		*u.u_ttyp = 0;
		signal(p->p_pgrp, SIGHUP);
	}
	p->p_pgrp = 0;
	for (i=0; i<NOFILE; i++) {
		if (u.u_ofile[i] != NULL)
			closef(u.u_ofile[i]);
	}
	semexit();
	if (u.u_lock & (TXTLOCK|DATLOCK))
		punlock();
	plock(u.u_cdir);
	iput(u.u_cdir);
	if (u.u_rdir) {
		plock(u.u_rdir);
		iput(u.u_rdir);
	}
	xfree();
	acct(rv);
	mfree(coremap, p->p_size, p->p_addr);
	if (runin) {
		runin = 0;
		setrun(&proc[0]);
	}
	p->p_stat = SZOMB;
	((struct xproc *)p)->xp_xstat = rv;
	((struct xproc *)p)->xp_utime = u.u_cutime + u.u_utime;
	((struct xproc *)p)->xp_stime = u.u_cstime + u.u_stime;
	for (q = &proc[1]; q < (struct proc *)v.ve_proc; q++) {
		if (p->p_pid == q->p_ppid) {
			q->p_ppid = 1;
			if (q->p_stat == SZOMB)
				psignal(&proc[1], SIGCLD);
			if (q->p_stat == SSTOP)
				setrun(q);
		} else
		if (p->p_ppid == q->p_pid)
			psignal(q, SIGCLD);
		if (p->p_pid == q->p_pgrp)
			q->p_pgrp = 0;
	}
	resume(proc[0].p_addr, u.u_qsav);
	/* no deposit, no return */
}

/*
 * Wait system call.
 * Search for a terminated (zombie) child,
 * finally lay it to rest, and collect its status.
 * Look also for stopped (traced) children,
 * and pass back status from them.
 */
wait()
{
	register f;
	register struct proc *p;

loop:
	f = 0;
	for (p = &proc[1]; p < (struct proc *)v.ve_proc; p++)
	if (p->p_ppid == u.u_procp->p_pid) {
		f++;
		if (p->p_stat == SZOMB) {
			freeproc(p, 1);
			return;
		}
		if (p->p_stat == SSTOP) {
			if ((p->p_flag&SWTED) == 0) {
				p->p_flag |= SWTED;
				u.u_rval1 = p->p_pid;
				u.u_rval2 = (fsig(p)<<8) | 0177;
				return;
			}
			continue;
		}
	}
	if (f) {
		sleep((caddr_t)u.u_procp, PWAIT);
		goto loop;
	}
	u.u_error = ECHILD;
}

/*
 * Remove zombie children from the process table.
 */
freeproc(p, flag)
register struct proc *p;
{
	if (flag) {
		register n;

		n = u.u_procp->p_cpu + p->p_cpu;
		if (n > 80)
			n = 80;
		u.u_procp->p_cpu = n;
		u.u_rval1 = p->p_pid;
		u.u_rval2 = ((struct xproc *)p)->xp_xstat;
	}
	u.u_cutime += ((struct xproc *)p)->xp_utime;
	u.u_cstime += ((struct xproc *)p)->xp_stime;
	p->p_stat = NULL;
	p->p_pid = 0;
	p->p_ppid = 0;
	p->p_sig = 0L;
	p->p_flag = 0;
	p->p_wchan = 0;
}

/*
 * fork system call.
 */
fork()
{
	register n;
	register struct proc *p1, *p2;
	register a;

	sysinfo.sysfork++;
	a = 0;
	p1 = &proc[0];
	p2 = NULL;
	n = v.v_proc;
	do {
		if (p1->p_stat==NULL && p2==NULL)
			p2 = p1;
		else {
			if (p1->p_uid==u.u_ruid && p1->p_stat!=NULL)
				a++;
		}
		p1++;
	} while (--n);
	/*
	 * Disallow if
	 *  No processes at all;
	 *  not su and too many procs owned; or
	 *  not su and would take last slot; or
	 *  not su and no space on swap.
	 */
	if (p2 == NULL) {
		syserr.procovf++;
		u.u_error = EAGAIN;
		goto out;
	}
	if (u.u_uid && u.u_ruid) {
		if (p2 == &proc[v.v_proc-1] || a > v.v_maxup) {
			u.u_error = EAGAIN;
			goto out;
		}
		if ((a = malloc(swapmap, ctod(MAXMEM))) == 0) {
			u.u_error = ENOMEM;
			goto out;
		}
		mfree(swapmap, ctod(MAXMEM), a);
	}
	if (newproc()) {
		u.u_rval1 = p2->p_ppid;
		u.u_start = time;
		u.u_ticks = lbolt;
		u.u_mem = p2->p_size;
		u.u_ior = u.u_iow = u.u_ioch = 0;
		u.u_cstime = 0;
		u.u_stime = 0;
		u.u_cutime = 0;
		u.u_utime = 0;
		u.u_acflag = AFORK;
		u.u_lock = 0;
		return;
	}
	u.u_rval1 = p2->p_pid;

out:
	u.u_ar0[PC] += NBPW;
}

/*
 * break system call.
 *  -- bad planning: "break" is a dirty word in C.
 */
sbreak()
{
	struct a {
		unsigned nsiz;
	};
	register a, n, d;
	int i;

	/*
	 * set n to new data size
	 * set d to new-old
	 * set n to new total size
	 */

	n = btoc(((struct a *)u.u_ap)->nsiz);
	if (!u.u_sep)
		n -= ctos(u.u_tsize) * stoc(1);
	if (n < 0)
		n = 0;
	d = n - u.u_dsize;
	n += USIZE+u.u_ssize;
	if (estabur(u.u_tsize, u.u_dsize+d, u.u_ssize, u.u_sep, RO))
		return;
	u.u_dsize += d;
	if (d > 0)
		goto bigger;
	a = u.u_procp->p_addr + n - u.u_ssize;
	i = n;
	n = u.u_ssize;
	while (n--) {
		copyseg(a-d, a);
		a++;
	}
	expand(i);
	return;

bigger:
	expand(n);
	a = u.u_procp->p_addr + n;
	n = u.u_ssize;
	while (n--) {
		a--;
		copyseg(a-d, a);
	}
	while (d--)
		clearseg(--a);
}
