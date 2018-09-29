/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:debug/prtabs.c	10.4"
#include	"sys/types.h"
#include	"sys/param.h"
#include	"sys/immu.h"
#include	"sys/region.h"
#include	"sys/proc.h"
#include	"sys/signal.h"
#include	"sys/fs/s5dir.h"
#include	"sys/psw.h"
#include	"sys/pcb.h"
#include	"sys/user.h"
#include	"sys/var.h"
#include	"sys/cmn_err.h"
#include	"sys/inline.h"

char	prtabs_buf[200];

char	*prproc_hdg =
	"  ADDR    PID PPID STATUS FLAGS           WCHAN   CMD\n\n";

char	*prproc_stat[] = {
		"UNUSED ",
		"SLEEP  ",
		"RUN    ",
		"ZOMBIE ",
		"STOP   ",
		"IDLE   ",
		"ONPROC ",
		"SXBRK  ",
	};

char	*prproc_flgs = "STWNLCGPstixo";

char	*prreg_hdg =
	"  ADDR   SIZE NVLD REFCNT TYPE FLAGS WCNT LIST-PTR LSIZ  INODE   FILE-SIZ\n\n";

char	*prreg_type[] = {
		"FREE ",
		"PRVT ",
		"STXT ",
		"SMEM ",
	};

printprocs()
{
	register proc_t	*pp;
	register int	lc;
	register user_t	*up;
	int		winaddr_save;
	int		delay;
	char		buf[20];
	int		oldpri;
	extern int	dmd_delay;

	asm("	PUSHW  %r0");
	asm("	PUSHW  %r1");
	asm("	PUSHW  %r2");

	oldpri = splhi();

	cmn_err(CE_CONT, "^%s", prproc_hdg);

	up = (user_t *)win_ublk;
	winaddr_save = kvtokstbl(up)->wd2.address;

	for (pp = &proc[0] ; pp < (proc_t *)v.ve_proc ; pp++) {
		if (pp->p_stat == 0)
			continue;
		sprintf(prtabs_buf, "%.8x ", pp);
		sprintf(buf, "%4d ", pp->p_pid);
		strcat(prtabs_buf, buf);
		sprintf(buf, "%4d ", pp->p_ppid);
		strcat(prtabs_buf, buf);
		strcat(prtabs_buf, prproc_stat[pp->p_stat]);

		for (lc = 0  ;  lc < 13  ;  lc++) {
			if (pp->p_flag & (1 << lc))
				buf[lc] = prproc_flgs[lc];
			else
				buf[lc] = ' ';
		}
		buf[lc++] = ' ';
		buf[lc] = '\0';
		strcat(prtabs_buf, buf);

		sprintf(buf, "%.8x  ", pp->p_wchan);
		strcat(prtabs_buf, buf);

		if (pp->p_stat == SZOMB  ||  pp->p_stat == SIDL) {
			cmn_err(CE_CONT, "^%s\n", prtabs_buf);
			continue;
		}

		kvtokstbl(up)->wd2.address = kvtophys(ubptbl(pp));
		flushmmu(up, USIZE);
		cmn_err(CE_CONT, "^%s%s\n", prtabs_buf, up->u_psargs);

		if (dmd_delay) {
			delay = dmd_delay;
			while (delay--) ;
		}
	}

	kvtokstbl(up)->wd2.address = winaddr_save;
	splx(oldpri);

	asm("	POPW  %r2");
	asm("	POPW  %r1");
	asm("	POPW  %r0");
}

printregions()
{
	register reg_t	*rp;
	register int	delay;
	register int	oldpri;
	char		buf[10];
	extern int	dmd_delay;

	asm("	PUSHW  %r0");
	asm("	PUSHW  %r1");
	asm("	PUSHW  %r2");

	oldpri = splhi();

	cmn_err(CE_CONT, "^%s", prreg_hdg);

	for (rp = ractive.r_forw  ;  rp != &ractive  ;  rp = rp->r_forw) {
		sprintf(prtabs_buf, "%.8x ", rp);
		sprintf(buf, "%4d ", rp->r_pgsz);
		strcat(prtabs_buf, buf);
		sprintf(buf, "%4d ", rp->r_nvalid);
		strcat(prtabs_buf, buf);
		sprintf(buf, "  %4d ", rp->r_refcnt);
		strcat(prtabs_buf, buf);
		strcat(prtabs_buf, prreg_type[rp->r_type]);
		sprintf(buf, " %.4x ", rp->r_flags);
		strcat(prtabs_buf, buf);
		sprintf(buf, "%4d ", rp->r_waitcnt);
		strcat(prtabs_buf, buf);
		sprintf(buf, "%.8x ", rp->r_list);
		strcat(prtabs_buf, buf);
		sprintf(buf, "%4d ", rp->r_listsz);
		strcat(prtabs_buf, buf);
		sprintf(buf, "%.8x ", rp->r_iptr);
		strcat(prtabs_buf, buf);
		sprintf(buf, "%.8x", rp->r_filesz);
		strcat(prtabs_buf, buf);

		cmn_err(CE_CONT, "^%s\n", prtabs_buf);

		if (dmd_delay) {
			delay = dmd_delay;
			while (delay--) ;
		}
	}

	splx(oldpri);

	asm("	POPW  %r2");
	asm("	POPW  %r1");
	asm("	POPW  %r0");
}
