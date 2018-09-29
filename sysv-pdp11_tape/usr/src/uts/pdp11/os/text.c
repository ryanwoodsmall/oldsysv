/* @(#)text.c	1.2 */
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/map.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/text.h"
#include "sys/inode.h"
#include "sys/buf.h"
#include "sys/seg.h"
#include "sys/var.h"
#include "sys/sysinfo.h"

/*
 * Swap out process p.
 * The ff flag causes its core to be freed--
 * it may be off when called to create an image for a
 * child process in newproc.
 * Os is the old size of the data area of the process,
 * and is supplied during core expansion swaps.
 */
xswap(p, ff, os)
register struct proc *p;
{
	register a;

	if (os == 0)
		os = p->p_size;
	a = swapalloc(ctod(p->p_size), 1);
	p->p_flag |= SLOCK;
	xccdec(p->p_textp);
	swap(a, p->p_addr, os, B_WRITE);
	if (ff) {
		mfree(coremap, os, p->p_addr);
		if (runin) {
			runin = 0;
			setrun(&proc[0]);
		}
	}
	p->p_addr = a;
	p->p_flag &= ~(SLOAD|SLOCK);
	p->p_time = 0;
	if (runout) {
		runout = 0;
		wakeup((caddr_t)&runout);
	}
}

/*
 * relinquish use of the shared text segment
 * of a process.
 */
xfree()
{
	register struct text *xp;
	register struct inode *ip;

	if ((xp=u.u_procp->p_textp) == NULL)
		return;
	xlock(xp);
	xp->x_flag &= ~XLOCK;
	u.u_procp->p_textp = NULL;
	ip = xp->x_iptr;
	if (--xp->x_count==0 && (ip->i_mode&ISVTX)==0) {
		xp->x_iptr = NULL;
		if (xp->x_daddr)
			mfree(swapmap, ctod(xp->x_size), xp->x_daddr);
		mfree(coremap, xp->x_size, xp->x_caddr);
		ip->i_flag &= ~ITEXT;
		if (ip->i_flag&ILOCK)
			ip->i_count--;
		else
			iput(ip);
	} else
		xccdec(xp);
}

/*
 * Attach to a shared text segment.
 * If there is no shared text, just return.
 * If there is, hook up to it:
 * if it is not currently being used, it has to be read
 * in from the inode (ip); the written bit is set to force it
 * to be written out as appropriate.
 * If it is being used, but is not currently in core,
 * a swap has to be done to get it back.
 */
xalloc(ip)
register struct inode *ip;
{
	register struct text *xp;
	register unsigned ts;
	register struct text *xp1;

	if (u.u_exdata.ux_tsize == 0)
		return;
	xp1 = NULL;
loop:
	for (xp = &text[0]; xp < (struct text *)v.ve_text; xp++) {
		if (xp->x_iptr == NULL) {
			if (xp1 == NULL)
				xp1 = xp;
			continue;
		}
		if (xp->x_iptr == ip) {
			xlock(xp);
			xp->x_count++;
			u.u_procp->p_textp = xp;
			if (xp->x_ccount == 0)
				xexpand(xp);
			else
				xp->x_ccount++;
			xunlock(xp);
			return;
		}
	}
	if ((xp=xp1) == NULL) {
		printf("out of text\n");
		syserr.textovf++;
		if (xumount(NODEV))
			goto loop;
		psignal(u.u_procp, SIGKILL);
		return;
	}
	xp->x_flag = XLOAD|XLOCK;
	xp->x_count = 1;
	xp->x_ccount = 0;
	xp->x_lcount = 0;
	xp->x_iptr = ip;
	ip->i_flag |= ITEXT;
	ip->i_count++;
	ts = btoc(u.u_exdata.ux_tsize);
	xp->x_size = ts;
	xp->x_daddr = 0;	/* defer swap alloc til later */
	u.u_procp->p_textp = xp;
	xexpand(xp);
	estabur(ts, (unsigned)0, (unsigned)0, cputype == 40 ? 0 : 1, RW);
	u.u_count = u.u_exdata.ux_tsize;
	u.u_offset = sizeof(u.u_exdata);
	u.u_base = 0;
	u.u_segflg = 2;
	u.u_procp->p_flag |= SLOCK;
	readi(ip);
	u.u_procp->p_flag &= ~SLOCK;
	u.u_segflg = 0;
	xp->x_flag = XWRIT;
}

/*
 * Assure core for text segment
 * Text must be locked to keep someone else from
 * freeing it in the meantime.
 * x_ccount must be 0.
 */
xexpand(xp)
register struct text *xp;
{
	if ((xp->x_caddr = malloc(coremap, xp->x_size)) != NULL) {
		if ((xp->x_flag&XLOAD)==0)
			swap(xp->x_daddr, xp->x_caddr, xp->x_size, B_READ);
		xp->x_ccount++;
		xunlock(xp);
		return;
	}
	if (save(u.u_ssav)) {
		sureg();
		return;
	}
	xswap(u.u_procp, 1, 0);
	xunlock(xp);
	u.u_procp->p_flag |= SSWAP;
	qswtch();
	/* no return */
}

/*
 * Lock and unlock a text segment from swapping
 */
xlock(xp)
register struct text *xp;
{

	while(xp->x_flag&XLOCK) {
		xp->x_flag |= XWANT;
		sleep((caddr_t)xp, PSWP);
	}
	xp->x_flag |= XLOCK;
}

xunlock(xp)
register struct text *xp;
{

	if (xp->x_flag&XWANT)
		wakeup((caddr_t)xp);
	xp->x_flag &= ~(XLOCK|XWANT);
}

/*
 * Decrement the in-core usage count of a shared text segment.
 * When it drops to zero, free the core space.
 */
xccdec(xp)
register struct text *xp;
{

	if (xp==NULL || xp->x_ccount==0)
		return;
	xlock(xp);
	if (--xp->x_ccount==0) {
		if (xp->x_flag&XWRIT) {
			if (xp->x_daddr == 0)
				xp->x_daddr = swapalloc(ctod(xp->x_size), 1);
			xp->x_flag &= ~XWRIT;
			swap(xp->x_daddr, xp->x_caddr, xp->x_size, B_WRITE);
		}
		mfree(coremap, xp->x_size, xp->x_caddr);
	}
	xunlock(xp);
}

/*
 * free the swap image of all unused saved-text text segments
 * which are from device dev (used by umount system call).
 */
xumount(dev)
register dev_t dev;
{
	register struct inode *ip;
	register struct text *xp;
	register count = 0;

	for (xp = &text[0]; xp < (struct text *)v.ve_text; xp++) {
		if ((ip = xp->x_iptr) == NULL)
			continue;
		if (dev != NODEV && dev != ip->i_dev)
			continue;
		if (xuntext(xp))
			count++;
	}
	return(count);
}

/*
 * remove a shared text segment from the text table, if possible.
 */
xrele(ip)
register struct inode *ip;
{
	register struct text *xp;

	if ((ip->i_flag&ITEXT) == 0)
		return;
	for (xp = &text[0]; xp < (struct text *)v.ve_text; xp++)
		if (ip==xp->x_iptr)
			xuntext(xp);
}

/*
 * remove text image from the text table.
 * the use count must be zero.
 */
xuntext(xp)
register struct text *xp;
{
	register struct inode *ip;

	xlock(xp);
	if (xp->x_count) {
		xunlock(xp);
		return(0);
	}
	ip = xp->x_iptr;
	xp->x_flag &= ~XLOCK;
	xp->x_iptr = NULL;
	if (xp->x_daddr)
		mfree(swapmap, ctod(xp->x_size), xp->x_daddr);
	ip->i_flag &= ~ITEXT;
	if (ip->i_flag&ILOCK)
		ip->i_count--;
	else
		iput(ip);
	return(1);
}

/*
 * allocate swap blocks, freeing and sleeping as necessary
 */
swapalloc(size, sflg)
{
	register addr;

	for (;;) {
		if (addr = malloc(swapmap, size))
			return(addr);
		if (swapclu()) {
			printf("\nWARNING: swap space running out\n");
			printf("  needed %d blocks\n", size);
			continue;
		}
		printf("\nDANGER: out of swap space\n");
		printf("  needed %d blocks\n", size);
		if (sflg) {
			mapwant(swapmap)++;
			sleep((caddr_t)swapmap, PSWP);
		} else
			return(0);
	}
}

/*
 * clean up swap used by text
 */
swapclu()
{
	register struct text *xp;
	register ans = 0;

	for (xp = text; xp < (struct text *)v.ve_text; xp++) {
		if (xp->x_iptr == NULL)
			continue;
		if (xp->x_flag&XLOCK)
			continue;
		if (xp->x_daddr == 0)
			continue;
		if (xp->x_count) {
			if (xp->x_ccount) {
				mfree(swapmap, ctod(xp->x_size), xp->x_daddr);
				xp->x_flag |= XWRIT;
				xp->x_daddr = 0;
				ans++;
			}
		} else {
			xuntext(xp);
			ans++;
		}
	}
	return(ans);
}
