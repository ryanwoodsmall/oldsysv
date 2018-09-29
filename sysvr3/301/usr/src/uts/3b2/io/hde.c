/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/hde.c	10.5"
/* This file contains the Hard Disk Error Log Driver
 * that is part of the Bad Block Handling Feature.
 *
 * This driver provides the hdeeqd() and hdelog() subroutines
 * which hard disk drivers call.
 *
 * It also provides open(), close(), and ioctl() system calls.
 * These calls implement the special operations that are required
 * by the utility commands that are part of bad block handling.
 * As such, they are considered an internal interface of the
 * feature and are intended to be used only by those commands.
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/sysmacros.h"
#include "sys/systm.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/user.h"
#include "sys/file.h"
#include "sys/conf.h"
#include "sys/errno.h"
#include "sys/inode.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/vtoc.h"
#include "sys/hdelog.h"
#include "sys/hdeioctl.h"
#include "sys/cmn_err.h"
#include "sys/open.h"

/* Configuration dependent variables declared in "space.h" */
extern int hdeedct;	/* count of slots available in hdeeqdt table */
extern struct hdeedd hdeeqdt[];

int hdeedcc;		/* hdeeqd() call count */
int hdeeduc;		/* count of slots used in hdeeqdt table */

/* temporary error report queue */
#define hdeqincr(ndx)	((ndx+1) % HDEQSIZE)
struct hdedata hdeerq[HDEQSIZE];
int hdeqondx = 0;		/* index of oldest report */
int hdeqnndx = 0;		/* index of newest report + 1 */
/* queue empty when hdeqnndx == hdeqondx */
/* queue full when hdeqincr(hdeqnndx) == hdeqondx */
/* this wastes one slot, but so what */

/* hdefix lockout control stuff */
int hdefixl;		/* hdefix lock */
int hdefixw;		/* hdefix is waiting */
int hdefixp;		/* hdefix process ID */

/* Log in progress (LIP) control stuff */
int hdeerwt;		/* wait for error reports */
int hdelipid;		/* LIP process ID */
int hdelipct;		/* count of reports in progress */

/* flags for hdeflgs (in struct hdeedd) */
#define HDEWANTD	0x0001

#define HDEOPRI	(PZERO+1)	/* sleep priority */

int hdeotyp[OTYPCNT];
int hdeosum;
int hdeclinp;
#define MAJORMASK	0x8000

dev_t hdeddev(ddev)
dev_t ddev;
{
	/* The purpose of this function is to get rid of the */
	/* extra bit used in the SV file system macros. */
	/* Since the 3B2 assumes a maximum of 128 major */
	/* devices, only the least significant 7 bits */
	/* of the major have to be maintained */
	ddev &= ~MAJORMASK;
	return (ddev);
}

hdeeqd(ddev, pdsno, edtyp)
 dev_t ddev;
{
	if (major(ddev) >= cdevcnt) {
		cmn_err (CE_WARN, "hdeeqd: major(ddev) = %d (>=cdevcnt)\n",
			major(ddev));
		return(0);
	}
	ddev = hdeddev(ddev);
	hdeedcc++;
	if (hdeeduc >= hdeedct) {
		cmn_err (CE_WARN, "too few HDE equipped disk slots\n");
		cmn_err (CE_CONT, "Bad block handling skipped for maj/min = %d/%d\n",
			emajor(ddev), eminor(ddev));
		return(0);
	}
	hdeeqdt[hdeeduc].hdepdsno = pdsno;
	hdeeqdt[hdeeduc].hdetype = edtyp;
	hdeeqdt[hdeeduc++].hdeedev = ddev;
	return(0);
}

hdelog(eptr)
 struct hdedata *eptr;
{
	eptr->diskdev = hdeddev(eptr->diskdev);
	if (hdeqincr(hdeqnndx) == hdeqondx) {
		cmn_err (CE_WARN, "HDE queue full, following report not logged\n");
		hdeermsg(eptr);
		return(0);
	}
	hdeermsg(eptr);
	hdeerq[hdeqnndx] = *eptr;
	hdeqnndx = hdeqincr(hdeqnndx);
	if (hdeerwt && !hdefixl) {
		hdeerwt = 0;
		wakeup ((caddr_t)&hdeerwt);
	}
	return(0);
}

hdeermsg(esp)
struct hdedata *esp;
{
	char serno[13];
	register int i;

	for (i = 0; i < 12; i++) serno[i] = esp->dskserno[i];
	serno[12] = 0;
	cmn_err (CE_WARN, "%s %s hard disk error: maj/min = %d/%d",
		((esp->severity == HDEMARG) ? "marginal" : "unreadable"),
		((esp->readtype == HDECRC) ? "CRC" : "ECC"),
		emajor(esp->diskdev), eminor(esp->diskdev));
	if (serno[0])
		cmn_err(CE_CONT, ", serial # = %s", serno);
	cmn_err (CE_CONT, "\n\t block # = %d", esp->blkaddr);
	if (esp->severity == HDEMARG) cmn_err (CE_CONT, ", bad try count = %d",
		esp->badrtcnt);
	cmn_err (CE_CONT, "\n");
	if (esp->severity == HDEMARG && esp->readtype == HDEECC)
		cmn_err (CE_CONT, "bit width of corrected error = %d\n",
			esp->bitwidth);
	else esp->bitwidth = 0;
}

hdeopen(dev, flag, otyp)
dev_t dev;
{
	suser();
	if (u.u_error) return(0);
	if (otyp < 0 || otyp >= OTYPCNT) {
	} else
	if (otyp == OTYP_LYR) {
		hdeotyp[OTYP_LYR]++;
	} else {
		hdeotyp[otyp] = 1;
	}
	hdeosum = 1;
	while (hdeclinp)
		sleep((caddr_t) &hdeclinp, HDEOPRI);
}

hdeclose(dev, flag, otyp)
dev_t dev;
{
	register int i;
	register struct hdeedd *edp;
	register int osum;

	if (otyp == OTYP_LYR) {
	if (otyp < 0 || otyp >= OTYPCNT) {
	} else
		hdeotyp[OTYP_LYR]--;
	} else {
		hdeotyp[otyp] = 0;
	}
	for (osum = 0, i = 0; i < OTYPCNT; i++)
		osum |= hdeotyp[i];
	if (hdeosum == osum) return(0);
	hdeclinp = u.u_procp->p_pid;
	for (edp = hdeeqdt, i = 0; i < hdeeduc; edp++, i++) {
		if (edp->hdepid) {
			(*cdevsw[major(edp->hdeedev)].d_close)
				(edp->hdeedev, FREAD|FWRITE, OTYP_LYR);
			edp->hdepid = 0;
		}
		edp->hdeflgs &= ~HDEWANTD;
	}
	hdelipid = 0;
	hdelipct = 0;
	hdefixl = hdefixw = hdefixp = 0;
	hdeerwt = 0;
	hdeclinp = 0;
	wakeup((caddr_t)&hdeclinp);
}

hdeexit()
{
	register int i;
	register struct hdeedd *edp;
	register int mypid;
	register int fixwflg;

	fixwflg = 0;
	mypid = u.u_procp->p_pid;
	for (edp = hdeeqdt, i = 0; i < hdeeduc; edp++, i++) {
		if (edp->hdepid == mypid) {
			(*cdevsw[major(edp->hdeedev)].d_close)
				(edp->hdeedev, FREAD|FWRITE, OTYP_LYR);
			edp->hdepid = 0;
			fixwflg = 1;
			if (edp->hdeflgs & HDEWANTD) {
				edp->hdeflgs &= ~HDEWANTD;
				wakeup ((caddr_t)edp);
			}
		}
	}
	if (mypid == hdefixp)
		hdeunfix();
	else if (mypid == hdelipid) {
		hdelipid = 0;
		hdelipct = 0;
		fixwflg = 1;
	}
	if (fixwflg && hdefixw)
		wakeup ((caddr_t) &hdefixw);
	if (hdeclinp == mypid) {
		hdeclinp = 0;
		wakeup((caddr_t) &hdeclinp);
	}
}

hdeunfix()
{
	register int i;
	register struct hdeedd *edp;

	hdefixl = hdefixw = hdefixp = 0;
	for (edp = hdeeqdt, i = 0; i < hdeeduc; edp++, i++) {
		if (edp->hdeflgs & HDEWANTD && edp->hdepid == 0) {
			edp->hdeflgs &= ~HDEWANTD;
			wakeup ((caddr_t)edp);
		}
	}
	if (hdeerwt) {
		hdeerwt = 0;
		wakeup ((caddr_t)&hdeerwt);
	}
}

hdediso()
{
	register int i;
	register struct hdeedd *edp;
	register int my_pid;

	my_pid = u.u_procp->p_pid;
	for (edp = hdeeqdt, i = 0; i < hdeeduc; edp++, i++) {
		if (edp->hdepid && edp->hdepid != my_pid) return(1);
	}
	return(0);
}

hdefind(ddev)
register dev_t ddev;
{
	register int i;
	register struct hdeedd *edp;
	for (edp = hdeeqdt, i = 0; i < hdeeduc; edp++, i++) {
		if (edp->hdeedev == ddev) return(i);
	}
	return(-1);
}

hdehdio(iocmd, uap, kap)
struct hdearg *uap, *kap;
{
	register caddr_t uadr;
	dev_t ddev;
	register int i;
	struct io_arg khdarg;

	uadr = (caddr_t) &(uap->hdebody.hdargbuf);
	if ((i = hdefind(kap->hdebody.hdedskio.hdeddev))
		< 0) {
		kap->hderval = HDE_NODV;
		u.u_error = EINVAL;
		return(-1);
	}
	if (hdeeqdt[i].hdepid != u.u_procp->p_pid) {
		kap->hderval = HDE_NOTO;
		u.u_error = EINVAL;
		return(-1);
	}
	khdarg.retval = 0;
	khdarg.sectst = kap->hdebody.hdedskio.hdedaddr;
	khdarg.memaddr = (int) (kap->hdebody.hdedskio.hdeuaddr);
	khdarg.datasz = kap->hdebody.hdedskio.hdebcnt;
	copyout((caddr_t) &khdarg, uadr, sizeof(struct io_arg));
	if (u.u_error) return(-1);
	ddev = kap->hdebody.hdedskio.hdeddev;
	(*cdevsw[major(ddev)].d_ioctl)
		(ddev, iocmd, uadr, FREAD|FWRITE);
	if (u.u_error) {
		kap->hderval = HDE_IOE;
		return(-1);
	}
	copyin(uadr, (caddr_t) &khdarg, sizeof(struct io_arg));
	if (khdarg.retval) {
		kap->hderval = HDE_IOE;
		u.u_error = EIO;
		return(-1);
	}
	return(0);
}

hdeioctl(dev, command, uarg, flag)
{
	struct hdearg karg;
	struct hdedata kers;
	register int uadr, i;
	register int my_pid;
	register int j, k;

	copyin((caddr_t)uarg, (caddr_t)&karg, sizeof (struct hdearg));
	if (u.u_error) return(0);
	karg.hderval = 0;
	karg.hdersize = 0;
	my_pid = u.u_procp->p_pid;
	switch (command)
	{
		case HDEGEDCT:
			karg.hdersize = hdeeduc;
			break;
		case HDEGEQDT:
			karg.hdersize = hdeeduc;
			if (karg.hdebody.hdegeqdt.hdeeqdct < hdeeduc) {
				karg.hderval = HDE_SZER;
				u.u_error = EINVAL;
				break;
			}
			uadr = (int) (karg.hdebody.hdegeqdt.hdeudadr);
			for (i = 0; i < hdeeduc; i++) {
				copyout((caddr_t)&hdeeqdt[i],
					(caddr_t)uadr, sizeof(struct hdeedd));
				uadr += sizeof(struct hdeedd);
			}
			break;
		case HDEOPEN:
			if ((i = hdefind(karg.hdebody.hdedskio.hdeddev))
				< 0) {
				karg.hderval = HDE_NODV;
				u.u_error = EINVAL;
				break;
			}
			if (hdeeqdt[i].hdepid == my_pid)
				break;
			while (hdeeqdt[i].hdepid ||
			    (hdefixl && hdefixp != my_pid)) {
				hdeeqdt[i].hdeflgs |= HDEWANTD;
				sleep(&hdeeqdt[i], HDEOPRI);
			}
			hdeeqdt[i].hdepid = my_pid;
			(*cdevsw[major(karg.hdebody.hdedskio.hdeddev)].d_open)
				(karg.hdebody.hdedskio.hdeddev,
				FREAD|FWRITE, OTYP_LYR);
			if (u.u_error) {
				hdeeqdt[i].hdepid = 0;
				karg.hderval = HDE_BADO;
				break;
			}
			break;
		case HDEGETSS:
			hdehdio(V_GETSSZ, uarg, &karg);
			break;
		case HDERDPD:
			hdehdio(V_PDREAD, uarg, &karg);
			break;
		case HDEWRTPD:
			hdehdio(V_PDWRITE, uarg, &karg);
			break;
		case HDERDISK:
			hdehdio(V_PREAD, uarg, &karg);
			break;
		case HDEWDISK:
			hdehdio(V_PWRITE, uarg, &karg);
			break;
		case HDECLOSE:
			if ((i = hdefind(karg.hdebody.hdedskio.hdeddev))
				< 0) {
				karg.hderval = HDE_NODV;
				u.u_error = EINVAL;
				break;
			}
			if (hdeeqdt[i].hdepid != my_pid) {
				karg.hderval = HDE_NOTO;
				u.u_error = EINVAL;
				break;
			}
			(*cdevsw[major(karg.hdebody.hdedskio.hdeddev)].d_close)
				(karg.hdebody.hdedskio.hdeddev,
				FREAD|FWRITE, OTYP_LYR);
			hdeeqdt[i].hdepid = 0;
			if (hdefixw)
				wakeup ((caddr_t)&hdefixw);
			if (hdeeqdt[i].hdeflgs & HDEWANTD) {
				hdeeqdt[i].hdeflgs &= ~HDEWANTD;
				wakeup ((caddr_t)&hdeeqdt[i]);
			}
			break;
		case HDEMLOGR:
			uadr = (int)karg.hdebody.hdeelogr.hdeuladr;
			for (i = karg.hdebody.hdeelogr.hdelrcnt; i > 0; --i) {
				copyin((caddr_t)uadr, (caddr_t)&kers,
					sizeof(struct hdedata));
				if (u.u_error) {
					karg.hdersize = karg.hdebody.hdeelogr.hdelrcnt
							- i;
					goto endofsw;
				}
				hdelog(&kers);
			}
			karg.hdersize = karg.hdebody.hdeelogr.hdelrcnt;
			break;
		case HDEGERCT:
			while (hdefixl && my_pid != hdefixp) {
				hdeerwt = 1;
				sleep ((caddr_t)&hdeerwt, HDEOPRI);
			}
			karg.hdersize = hdeqcnt();
			break;
		case HDEERSLP:
			while ((hdefixl && my_pid != hdefixp)
				|| hdeqcnt() == 0) {
				hdeerwt = 1;
				sleep ((caddr_t)&hdeerwt, HDEOPRI);
			}
			karg.hdersize = hdeqcnt();
			break;
		case HDEGEREP:
			while (hdefixl && my_pid != hdefixp) {
				hdeerwt = 1;
				sleep ((caddr_t)&hdeerwt, HDEOPRI);
			}
			if (hdeqcnt() == 0) break;
			if (my_pid != hdefixp) {
				if (u.u_procp->p_ppid != 1) {
					/* only demon or hdefix can do it */
					karg.hderval = HDE_NOTD;
					u.u_error = EINVAL;
					break;
				}
				if (hdelipct && my_pid != hdelipid) {
					karg.hderval = HDE_TWOD;
					u.u_error = EINVAL;
					break;
				}
			}
			i = min(hdeqcnt(), karg.hdebody.hdeelogr.hdelrcnt);
			karg.hdersize = i;
			uadr = (int)karg.hdebody.hdeelogr.hdeuladr;
			for (j = hdeqondx, k = 0; k < i; j = hdeqincr(j), k++) {
				copyout ((caddr_t)&hdeerq[j], (caddr_t)uadr,
					sizeof(struct hdedata));
				if (u.u_error) {
					karg.hdersize = 0;
					goto endofsw;
				}
				uadr += sizeof(struct hdedata);
			}
			if (my_pid != hdefixp) {
				hdelipid = u.u_procp->p_pid;
				if (i > hdelipct) hdelipct = i;
			}
			break;
		case HDECEREP:
			if (my_pid != hdefixp && my_pid != hdelipid) {
				karg.hderval = HDE_CANT;
				u.u_error = EINVAL;
				break;
			}
			i = karg.hdebody.hdeelogr.hdelrcnt;
			uadr = (int) (karg.hdebody.hdeelogr.hdeuladr);
			while (i-- > 0) {
				copyin((caddr_t) uadr, (caddr_t) &kers,
					sizeof(struct hdedata));
				if (u.u_error) {
					goto endofsw;
				}
				for (j = hdeqondx; j != hdeqnndx; j = hdeqincr(j)) {
					if (hdecmp((caddr_t) &kers,
						(caddr_t) &hdeerq[j],
						sizeof(struct hdedata)))
							continue;
					karg.hdersize++;
					for (k = j; k != hdeqondx;) {
						if (--k < 0) k = HDEQSIZE-1;
						hdeerq[j] = hdeerq[k];
						j = k;
					}
					hdeqondx = hdeqincr(hdeqondx);
					if (my_pid == hdelipid &&
						--hdelipct <= 0) {
						hdelipct = 0;
						hdelipid = 0;
						if (hdeerwt) {
							hdeerwt = 0;
							wakeup((caddr_t) &hdeerwt);
						}
						if (hdefixw)
							wakeup((caddr_t) &hdefixw);
					}
					goto gotit;
				}
				karg.hderval = HDE_BADR;
				u.u_error = EINVAL;
				goto endofsw;
gotit:
				uadr += sizeof(struct hdedata);
			}
			break;
		case HDEFIXLK:
			if (hdefixl || hdefixw) {
				karg.hderval = HDE_TWOF;
				u.u_error = EINVAL;
				break;
			}
			hdefixw = 1;
			while (hdelipct || hdediso()) {
				sleep ((caddr_t)&hdefixw, HDEOPRI);
			}
			hdefixl = 1;
			hdefixp = my_pid;
			hdefixw = 0;
			break;
		case HDEFIXUL:
			if (my_pid != hdefixp) {
				karg.hderval = HDE_NOTF;
				u.u_error = EINVAL;
				break;
			}
			hdeunfix();
			break;
		default:
			karg.hderval = HDE_UNKN;
			u.u_error = EINVAL;
			break;
	}
endofsw:
	copyout((caddr_t)&karg, (caddr_t)uarg, sizeof (struct hdearg));
}

hdeqcnt()
{
	register int i;

	i = hdeqnndx - hdeqondx;
	if (i < 0) i += HDEQSIZE;
	return(i);
}

hdecmp(cp1,cp2,sz)
register char *cp1, *cp2;
 register int sz;
{
	while (sz--)
		if (*cp1++ != *cp2++)
			return(1);
	return(0);
}
