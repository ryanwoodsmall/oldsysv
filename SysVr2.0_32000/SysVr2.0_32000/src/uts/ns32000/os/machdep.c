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
#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/page.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/reg.h"
#include "sys/acct.h"
#include "sys/file.h"
#include "sys/clock.h"

extern int	maxumem;

clkstart()
{
	register struct n8253 *dp;

	dp = (struct n8253 *)0xfff600;
	dp->mode = C2MODE;
	dp->clock = C2VAL & 0xff;
	dp->clock = C2VAL >> 8;
	icuenable(2 | 0x8000);
	/* enable clock ints and soft ints */
}

clkset(oldtime)
time_t oldtime;
{
	time = oldtime;
}

/*
 * Send an interrupt to process
 */
sendsig(hdlr, signo)
{
	register *usp, *regs;

	regs = u.u_ar0;
	usp = (int *)regs[SP];
	grow((unsigned)(usp-3));
	suword((caddr_t)--usp, signo);
	suword((caddr_t)--usp, regs[PSR_MOD]);
	suword((caddr_t)--usp, regs[PC]);
	regs[PC] = hdlr;
	regs[SP] = (int)usp;
}

/*
 * Clear registers on exec
 */
setregs()
{
	register int *rp;
	register i;

	u.u_ar0[FP] = 0;
	u.u_ar0[PC] = u.u_exdata.ux_entloc;
	/* initialize Floating Status Register */
	u.u_fps[0] = 0;
	u.u_fpsaved = 1;
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

chksize(text, data, stack)
{
	register n;

	n = text + data + stack;
	if (n > maxumem) {
		u.u_error = ENOMEM;
		return(-1);
	}
	return(0);
}

/*
 * dump out the core of a process
 */
coredump(ip, n)
register struct inode *ip;
{
	register preg_t	*t_prp, *d_prp, *s_prp;
	register reg_t	*rp;
	register struct proc *pp;

	/*	Put the region sizes into the u-block for the
	 *	dump.
	 */
	
	pp = u.u_procp;
	if(t_prp = findpreg(pp, PT_TEXT))
		u.u_tsize = t_prp->p_reg->r_pgsz;
	else
		u.u_tsize = 0;
	if(d_prp = findpreg(pp, PT_DATA))
		u.u_dsize = d_prp->p_reg->r_pgsz;
	else
		u.u_dsize = 0;
	if(s_prp = findpreg(pp, PT_STACK))
		u.u_ssize = s_prp->p_reg->r_pgsz;
	else
		u.u_ssize = 0;

	/*	Check the sizes against the current ulimit and
	 *	don't write a file bigger than ulimit.  Try to
	 *	write the stack if the data + stack is too big.
	 *	If that fails, at least they've got the registers.
	 */

	if (USIZE + u.u_dsize + u.u_ssize > dtop(u.u_limit)) {
		u.u_dsize = 0;
		if (USIZE + u.u_ssize > dtop(u.u_limit))
			u.u_ssize = 0;
	}

	/*
  	 *	Write the u-block to the dump file.
	 */

	itrunc(ip);
	u.u_offset = 0;
	u.u_base = (caddr_t)&u;
	u.u_count = ptob(USIZE);
	u.u_segflg = 1;
	u.u_fmode = FWRITE;
	writei(ip);

	/*
	 *	Write the data to the dump file.
	 */
	
	u.u_segflg = 0;

	if (u.u_dsize) {
		rp = d_prp->p_reg;
		u.u_base = (caddr_t)(d_prp->p_regva + ptob(rp->r_pgoff));
		u.u_count = ptob(u.u_dsize);
		writei(ip);
	}

	/*
	 *	Write out the stack: 
	 */

	if (u.u_ssize) {
		rp = s_prp->p_reg;
		u.u_base = (caddr_t)(USRSTACK - ptob(rp->r_pgsz));
		u.u_count = ptob(u.u_ssize);
		writei(ip);
	}
}
