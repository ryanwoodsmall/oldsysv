/*	csi.c 1.8 of 3/30/82
	@(#)csi.c	1.8
 */



#include "sys/param.h"
#include "sys/types.h"
#include "sys/signal.h"
#include "sys/errno.h"
#include "sys/dir.h"
#include "sys/user.h"
#ifdef vax
#include "sys/page.h"
#endif
#include "sys/seg.h"
#include "sys/map.h"
#include "sys/systm.h"
#include "sys/buf.h"
#include "sys/csi.h"
#include "sys/csihdw.h"
#include "sys/csierrs.h"
#include "sys/sysmacros.h"
#include "sys/uba.h"

#define CSIINTR 6
#define NOARG 0
#define KMCS 4

extern struct csibd *csideq();
extern int	kmc_cnt;
extern int	csi_cnt;
extern int	csibnum;
extern struct csi csi_csi[];
extern struct csibuf *csibpt[];
struct csik csik[KMCS];
#ifdef pdp11
struct buf ubuf;
#endif

#define ROUND(X) ((X+3)&~3)
#define CSIDEV(X)  (X&077)
#define KMCDEV(X)  (((unsigned char)X>>6)&03)







/*	CSIATTACH- connect the protocol driver to a synchronous line.
 *	Returns an index into the CSI structure for use in CSI calls.
 *	A negative return code indicates an error as follows:
 *                                                           
 *		NOIDEV:	no csi structure available
 *		ATTCH:  the device is already attached
 *		BADDEV: invalid PCD
 *
 *	The device specified is first checked as valid for this system.
 *	Then it is checked to see if device is already attached to another
 *	protocol driver.
 */


csiattach(dev, rint, pindex)

dev_t	dev;		/* Major/Minor device numbers */
int	pindex;		/* Protocol driver index */
int	(*rint)();	/* Protocol driver "interrupt" routine */
{

	register struct csi *cp;
	register int csislot;
	dev_t mdev;


	mdev = (((char)dev<<6)|((unsigned char)dev>>5))&0377;
	if((mdev>>6) >= kmc_cnt || (mdev&077) > 7)
		return(BADDEV);

	csislot = -1;
	for(cp=csi_csi; cp <= &csi_csi[csi_cnt-1]; cp++) {
		if (cp->state&C_ATTACH) {
			if(mdev==cp->mdev)
				return(ATTCH);
		} else {
			if(csislot < 0)
				csislot = cp - csi_csi;
		}
	}
	if (csislot < 0)
		return(NOIDEV);
	cp = &csi_csi[csislot];
	cp->mdev = mdev;
	cp->rint = rint;
	cp->pindex = pindex;
	cp->state |= C_ATTACH;
	return(csislot);
}


/*
 *	CSIDETACH - Disconnect the protocol driver from the line.
 *	Error returns are as follows:
 *
 *	ATTCH:	The device was not attached.
 *	STERR:	The device is in the start state.
 *
 *	The state is first checked for validity, then csiclean
 *	is called, state reset and 0 returned.
 */

csidetach(idev)

int	idev;		/* The CSI interface index */
{
	register struct csi *cp;

	cp = &csi_csi[idev];

	if (!(cp->state & C_ATTACH))
		return(ATTCH);
	if (cp->state & C_START)
		return(STERR);

	csiclean(idev);
	cp->state = 0;
	return(0);
}


/*
 *	CSISTART - Start the protocol script running in the device.
 *	Returns STERR if the specified device
 *	is in the started state, otherwise the device is marked as
 *	started.
 */

csistart(idev, flagp)

int	idev;		/* The CSI interface index */
struct csiflags *flagp;	/* The start time flags */
{
	register struct csi *cp;
	register struct csik *kp;
	short	opts;
	extern csiok();
	extern csirint();
	register union csirpt *rp;
	struct unsolrpt unsolrpt;
	union csirpt csirpt;




	rp = &csirpt;
	if (idev >= csi_cnt)
		return(NOIDEV);
	if ((cp = &csi_csi[idev])->state & C_START || (cp->state & C_ATTACH) == 0)
		return(STERR);
	if (((kp = &csik[KMCDEV(cp->mdev)])->state & KMCRUN) == 0) {
		if (kmcset(cp->mdev, CSIINTR, csirint)) {
			return(STERR);
		}
		if ((kmcload(cp->mdev & 0300, BASEIN, NOARG, NOARG)) == -1) {
			return(STERR);
		}
		kp->state |= KMCRUN;
		timeout(csiok, KMCDEV(cp->mdev), 5*HZ);
	}


	cp->xmax = 1;
	cp->rmax = 1;
	cp->flags.options = flagp->options;
	cp->flags.intopts = flagp->intopts;
	cp->flags.window = flagp->window;
	cp->flags.timer0 = flagp->timer0;
	cp->flags.timer1 = flagp->timer1;

	opts = ((short)cp->flags.options<<8)|((short)cp->flags.intopts&0377);

	if (kmcload(cp->mdev, RUNCMD, opts, idev) == -1) {
		return(STERR);
	}
	cp->state |= (C_START|C_RUN);

#ifdef DEBUG
	csisave(idev, 's', cp->state, cp->pindex);
#endif
	return(0);
}


/*
 *	CSISTOP - Stop the protocol running on the device. Returns
 *	STERR if the device was not in the started state, otherwise
 *	marks it as stopped .
 */

csistop(idev)

int	idev;		/* The CSI interface index */
{
	register struct csi *cp;
	register int	sps;
	register union csirpt *rp;
	struct unsolrpt unsolrpt;
	union csirpt csirpt;

	rp = &csirpt;
	cp = &csi_csi[idev];
#ifdef DEBUG
	csisave(idev, 'Z', cp->state, CSIXQL(idev));
#endif
	if ((cp->state & C_START) == 0)
		return(STERR);
	cp->state &= ~C_START;
	if ((cp->state&C_RUN) == 0)
		return(0);
	sps = spl5();
	if (kmcload(cp->mdev, HALTCMD, NOARG, NOARG) == -1) {
		rp->unsolp = &unsolrpt;
		rp->unsolp->excode = CSINONAPP;
		rp->unsolp->code = PCDCMD;
		(*cp->rint)(idev, cp->pindex, CSITERM, rp);
		csiclean(idev);
	}
	splx(sps);
#ifdef DEBUG
	csisave(idev, 't', cp->state, cp->xqkmc.l_ql);
#endif
	return(0);
}





/*	CSIRINT - interrupt routines initiated from codes
 *	passed up from the KMC's
 */



csirint(dev, code, sel4, sel6)
{
	register struct csi *cp;
	register struct csik *kp;
	register struct csibd *bdp, *bdp1;
	extern csiok();
	paddr_t addr;
	register union csirpt *rp;
	struct solrpt solrpt;
	struct unsolrpt unsolrpt;
	struct trrpt trrpt;
	union csirpt csirpt;
	extern csiclean();
	int	i;



	rp = &csirpt;
	cp = &csi_csi[CSIDEV(dev)];

	switch (code) {
	case RRTNXBUF:
		bdp = (struct csibd *)ubmrev((int)sel4, (int)sel6);
		if (csirmv(bdp, &cp->xqkmc) == 0) {
			if (kmcload(cp->mdev, HALTCMD, NOARG, NOARG) == -1) {
				rp->unsolp = &unsolrpt;
				rp->unsolp->excode = CSINONAPP;
				rp->unsolp->code = PCDCMD;
				(*cp->rint)(CSIDEV(dev), cp->pindex, CSITERM, rp);
				csiclean(CSIDEV(dev));
			}
			csisave(CSIDEV(dev), 'x', cp->state, 0);
			return;
		}
		while ((cp->xqkmc.l_ql < cp->xmax) && (bdp1 = csideq(&cp->xmtq))) {
			addr = ubmdata(bdp1);
			if (kmcload(cp->mdev, XBUFINCMD, loword(addr), hiword(addr)) == -1) {
				rp->unsolp = &unsolrpt;
				rp->unsolp->excode = CSINONAPP;
				rp->unsolp->code = PCDCMD;
				(*cp->rint)(CSIDEV(dev), cp->pindex, CSITERM, rp);
				csiclean(CSIDEV(dev));
			}
			csienq(bdp1, &cp->xqkmc);
		}
		rp->solp = &solrpt;
		rp->solp->bdp = bdp;
		rp->solp->retcode = CSIXMT;
		(*cp->rint)(CSIDEV(dev), cp->pindex, CSIRXBUF, rp);
		break;
	case RRTNRBUF:
		bdp = (struct csibd *)ubmrev((int)sel4, (int)sel6);
		if (csirmv(bdp, &cp->eqkmc) == 0) {
			if (kmcload(cp->mdev, HALTCMD, NOARG, NOARG) == -1) {
				rp->unsolp = &unsolrpt;
				rp->unsolp->excode = CSINONAPP;
				rp->unsolp->code = PCDCMD;
				(*cp->rint)(CSIDEV(dev), cp->pindex, CSITERM, rp);
				csiclean(CSIDEV(dev));
			}
			csisave(CSIDEV(dev), 'y', cp->state, 0);
			return;
		}
		while ((cp->eqkmc.l_ql < cp->rmax) && (bdp1 = csideq(&cp->emptq))) {
			addr = ubmdata(bdp1);
			if (kmcload(cp->mdev, RBUFINCMD, loword(addr), hiword(addr)) == -1) {
				rp->unsolp = &unsolrpt;
				rp->unsolp->excode = CSINONAPP;
				rp->unsolp->code = PCDCMD;
				(*cp->rint)(CSIDEV(dev), cp->pindex, CSITERM, rp);
				csiclean(CSIDEV(dev));
			}
			csienq(bdp1, &cp->eqkmc);
		}
		rp->solp = &solrpt;
		rp->solp->bdp = bdp;
		rp->solp->retcode = CSIRCV;
		(*cp->rint)(CSIDEV(dev), cp->pindex, CSIRRBUF, rp);
		break;
	case ITRACE:
		csisave(CSIDEV(dev), 'I', sel4, sel6);
		break;
	case RTRACE:
		rp->trp = &trrpt;
		rp->trp->scloc = sel4;
		rp->trp->info[0] = sel6 & 0377;
		rp->trp->info[1] = (sel6 >> 8) & 0377;
		rp->trp->info[2] = 0;
		rp->trp->info[3] = 0;
		(*cp->rint)(CSIDEV(dev), cp->pindex, CSITRACE, rp);
		break;
	case RTNSNAP:
		rp->trp = &trrpt;
		rp->trp->scloc = 0;
		rp->trp->info[0] = sel4 & 0377;
		rp->trp->info[1] = (sel4 >> 8) & 0377;
		rp->trp->info[2] = sel6 & 0377;
		rp->trp->info[3] = (sel6 >> 8) & 0377;
		(*cp->rint)(CSIDEV(dev), cp->pindex, CSITRACE, rp);
		break;
	case RTNSRPT:
		rp->trp = &trrpt;
		rp->trp->scloc = 0;
		rp->trp->info[0] = sel4 & 0377;
		rp->trp->info[1] = (sel4 >> 8) & 0377;
		rp->trp->info[2] = sel6 & 0377;
		rp->trp->info[3] = (sel6 >> 8) & 0377;
		(*cp->rint)(CSIDEV(dev), cp->pindex, CSISRPT, rp);
#ifdef DEBUG
		csisave(CSIDEV(dev), 'P', sel4, sel6);
#endif
		break;
	case RTNER1:
		for (i = 0; i < 2; i++) {
			if (cp->errors[i] > 0) {
				if ((cp->errors[i] = cp->errors[i] - (sel4 >> (8 * (1 - i))) & 0377) <= 0) {
					cp->errors[i] = 0;
					rp->unsolp = &unsolrpt;
					rp->unsolp->excode = CSINONAPP;
					rp->unsolp->code = i;
					(*cp->rint)(CSIDEV(dev), cp->pindex, CSIERR, rp);
				}
			}
		}
		for (i = 0; i < 2; i++) {
			if (cp->errors[i] > 0) {
				if ((cp->errors[i+2] = cp->errors[i+2] - (sel6 >> (8 * (1 - i))) & 0377) <= 0) {
					cp->errors[i] = 0;
					rp->unsolp = &unsolrpt;
					rp->unsolp->excode = CSINONAPP;
					rp->unsolp->code = i + 2;
					(*cp->rint)(CSIDEV(dev), cp->pindex, CSIERR, rp);
				}
			}
		}
		break;
	case RTNER2:
		for (i = 0; i < 2; i++) {
			if (cp->errors[i] > 0) {
				if ((cp->errors[i+4] = cp->errors[i+4] - (sel4 >> (8 * (1 - i))) & 0377) <= 0) {
					cp->errors[i] = 0;
					rp->unsolp = &unsolrpt;
					rp->unsolp->excode = CSINONAPP;
					rp->unsolp->code = i + 4;
					(*cp->rint)(CSIDEV(dev), cp->pindex, CSIERR, rp);
				}
			}
		}
		for (i = 0; i < 2; i++) {
			if (cp->errors[i] > 0) {
				if ((cp->errors[i+6] = cp->errors[i+6] - (sel6 >> (8 * (1 - i))) & 0377) <= 0) {
					cp->errors[i] = 0;
					rp->unsolp = &unsolrpt;
					rp->unsolp->excode = CSINONAPP;
					rp->unsolp->code = i + 6;
					(*cp->rint)(CSIDEV(dev), cp->pindex, CSIERR, rp);
				}
			}
		}
		break;

	case RTNACK:
		rp->solp = &solrpt;
		rp->solp->bdp = 0;
		rp->solp->retcode = 0;
		(*cp->rint)(CSIDEV(dev), cp->pindex, CSICMDACK, rp);
		break;
	case RTNOK:
		kp = &csik[KMCDEV(dev)];
		kp->state |= KMCOK;
		kp->pdata.p_lbolt = lbolt;
		trsave(2, KMCDEV(dev), (char *) & kp->pdata, sizeof(kp->pdata));
		addr = ubmdata( & kp->pdata);
		if (kmcload(dev & 0300, OKCMD, loword(addr), hiword(addr)) == -1) {
			rp->unsolp = &unsolrpt;
			rp->unsolp->excode = CSINONAPP;
			rp->unsolp->code = PCDCMD;
			(*cp->rint)(CSIDEV(dev), cp->pindex, CSITERM, rp);
			csiclean(CSIDEV(dev));
		}
		break;
	case STARTUP:
		cp->xmax = sel4 & 0377;
		cp->rmax = (sel4 >> 8) & 0377;
		if ((cp->state & C_RESET) == 0) {
			rp->solp = &solrpt;
			rp->solp->bdp = 0;
			rp->solp->retcode = sel6 & 0377;
			(*cp->rint)(CSIDEV(dev), cp->pindex, CSISTART, rp);
			if (rp->solp->retcode != 0) {
				csiclean(CSIDEV(dev));
			}
		} else {
			if ((sel6 & 0377) != 0) {
				rp->unsolp = &unsolrpt;
				rp->unsolp->excode = CSINONAPP;
				rp->unsolp->code = sel6 & 0377;
				(*cp->rint)(CSIDEV(dev), cp->pindex, CSITERM, rp);
				csiclean(CSIDEV(dev));
			} else {
				cp->state &= ~C_RESET;
			}
		}
#ifdef DEBUG
		csisave(CSIDEV(dev), 'B', sel4, sel6);
#endif
		break;
	case RTNSTOP:
		rp->solp = &solrpt;
		rp->solp->bdp = 0;
		rp->solp->retcode = sel6 & 0377;
		(*cp->rint)(CSIDEV(dev), cp->pindex, CSISTOP, rp);
		csiclean(CSIDEV(dev));
		break;
	case ERRTERM:
		rp->unsolp = &unsolrpt;
		rp->unsolp->code = sel6 & 0377;
		rp->unsolp->excode = (sel6 >> 8) & 0377;
		(*cp->rint)(CSIDEV(dev), cp->pindex, CSITERM, rp);
#ifdef DEBUG
		csisave(CSIDEV(dev), 'E', sel4, sel6);
#endif
		csiclean(CSIDEV(dev));
		break;
	case BASEACK:
		kp = &csik[KMCDEV(dev)];
		if (kp->state & KMCRESET) {
			kp->state |= KMCRUN;
			kp->state &= ~KMCRESET;
			kp->state |= KMCOK;
			kp->pdata.p_lbolt = lbolt;
			trsave(2, KMCDEV(dev), (char *) & kp->pdata, sizeof(kp->pdata));
			addr = ubmdata( & kp->pdata);
			if (kmcload(dev & 0300, OKCMD, loword(addr), hiword(addr)) == -1) {
				rp->unsolp = &unsolrpt;
				rp->unsolp->excode = CSINONAPP;
				rp->unsolp->code = PCDCMD;
				(*cp->rint)(CSIDEV(dev), cp->pindex, CSITERM, rp);
				csiclean(CSIDEV(dev));
			}

		}
		break;
	}
}




/*	CSICLEAN - This routine returns all buffers to the protocol
 *	driver along with the status of the buffers. It then resets
 *	the state of the CSI structure.
 */

csiclean(idev)
{
	register struct csi *cp;
	register union csirpt *rp;
	struct solrpt solrpt;
	union csirpt csirpt;


	cp = &csi_csi[idev];
	rp = &csirpt;
	rp->solp = &solrpt;



	while (rp->solp->bdp = csideq(&cp->xmtq)) {
		rp->solp->retcode = CSINXMT;
		(*cp->rint)(idev, cp->pindex, CSIRXBUF, rp);
	}
	while (rp->solp->bdp = csideq(&cp->xqkmc)) {
		rp->solp->retcode = CSIMAYBE;
		(*cp->rint)(idev, cp->pindex, CSIRXBUF, rp);
	}
	while (rp->solp->bdp = csideq(&cp->emptq)) {
		rp->solp->retcode = CSIEMPTY;
		(*cp->rint)(idev, cp->pindex, CSIRRBUF, rp);
	}
	while (rp->solp->bdp = csideq(&cp->eqkmc)) {
		rp->solp->retcode = CSIEMPTY;
		(*cp->rint)(idev, cp->pindex, CSIRRBUF, rp);
	}
	cp->state &= (C_ATTACH | C_START);
	rp->solp->bdp = NULL;
	rp->solp->retcode = 0;
	(*cp->rint)(idev, cp->pindex, CSICLEANED, rp);
	return(0);
}




/*
 *	CSISCMD - Sends a command to the protocol script.
 *	If the device specified has a scmd routine it is
 *	called and returns the number of bytes transferred.
 */

csiscmd(idev, cmd, cnt)

int	idev, 		/* The CSI interface index */
cnt;		/* Number of bytes in the command */

char	*cmd;		/* Pointer to the command */
{
	register struct csi *cp;
	short	sel4, sel6;
	register union csirpt *rp;
	struct unsolrpt unsolrpt;
	union csirpt csirpt;


	if ((cnt < 1)||(cnt > 4))
		return(0);
	rp = &csirpt;
	cp = &csi_csi[idev];
	sel4 = 0;
	sel6 = 0;
	switch(cnt) {
	case 1:
		sel4 |= cmd[0];
		break;
	case 2:
		sel4 = ((short)cmd[1]<<8)|cmd[0];
		break;
	case 3:
		sel4 = ((short)cmd[1]<<8)|cmd[0];
		sel6 |= cmd[2];
		break;
	case 4:
		sel4 = ((short)cmd[1]<<8)|cmd[0];
		sel6 = ((short)cmd[3]<<8)|cmd[2];
		break;
	}
	if (kmcload(cp->mdev, SCRIPTCMD, sel4, sel6) == -1) {
		rp->unsolp = &unsolrpt;
		rp->unsolp->excode = CSINONAPP;
		rp->unsolp->code = PCDCMD;
		(*cp->rint)(idev, cp->pindex, CSITERM, rp);
		csiclean(idev);
	}
	return(cnt);
}


/*
 *	CSISETERRS - Sets the device error counters. The routine returns
 *                   the number of error counters set.
 */

csiseterrs(idev, errcnt, cnt)

int	idev, 		/* The CSI interface index */
cnt;		/* Number of error counters */

short	errcnt[];	/* Error counters */
{
	register struct csi *cp;
	int	i, sps;


	cp = &csi_csi[idev];
	if (cnt <= 0 || cnt > NOERRS)
		cnt = NOERRS;
	sps = spl5();
	for (i = 0; i < cnt; i++)
		cp->errors[i] = errcnt[i];
	splx(sps);
	return(cnt);


}


/*
 *	CSIGETERRS - Gets the device error counters. The rountine returns
 *	             the number of error counters returned.
 */

csigeterrs(idev, errcnt, cnt)

int	idev, 		/* The CSI interface index */
cnt;		/* Number of error counters */

short	errcnt[];	/* Error counters */
{
	register struct csi *cp;
	int	i, sps;

	cp = &csi_csi[idev];
	if (cnt <= 0 || cnt > NOERRS)
		cnt = NOERRS;
	sps = spl5();
	for (i = 0; i < cnt; i++)
		errcnt[i] = cp->errors[i];
	splx(sps);
	return(cnt);


}


/*
 *	CSIALLOC - Allocates a block of at least nbytes of memory.
 *	Csibufp is a pointer to a csibuf structure which is allocated
 *	by the protocol driver. NULL is returned if the space can't
 *	be allocated. The map is initialized and the memory is "freed"
 *	into the map. The size of the block and address are saved,
 *	and the free buffer descriptor pointer is initialized to NULL.
 */

csialloc(csibufp, nbytes)
register struct csibuf *csibufp;	/* Ptr to csibuf structure */
int	nbytes;		/* Size to be allocated */
{
#ifdef pdp11
	register int	segp;
#else
	register char	*segp;
#endif
	register int	ubmd;
	register int    i;

#ifdef pdp11
	if ((segp = malloc(coremap,(ushort)btoc(nbytes))) == NULL)
		return(NULL);
#else
	if ((segp = (char *)sptalloc(btoc((int)nbytes), PG_V | PG_KW, 0)) == NULL)
		return(NULL);
#endif
	mapinit(csibufp->csimap, CSIMAP);
	mfree(csibufp->csimap, nbytes, 1);
#ifdef pdp11
	ubuf.b_flags |= B_PHYS;
	ubuf.b_bcount = nbytes;
	ubuf.b_paddr = ctob((paddr_t)segp);
	ubmd = ubmalloc((unsigned)nbytes);
	csibufp->bubm = ubmaddr(&ubuf,ubmd);
	csibufp->segp = ctob((paddr_t)segp);
#else
	ubmd = ubmalloc(nbytes,0);
	csibufp->bubm = ubmvad((paddr_t)segp,ubmd);
	csibufp->segp = segp;
#endif
	csibufp->ubmd = ubmd;
	csibufp->csibsz = nbytes;
	csibufp->freebdp = NULL;
#ifdef pdp11
	return(nbytes);
#else
	for (i=0; i<csibnum; i++)
		if (csibpt[i] == NULL) {
			csibpt[i] = csibufp;
			return(nbytes);
		}
	csifree(csibufp);
	return(NULL);
#endif
}


/*
 *	CSIFREE - Free (release) the block of memory indicated
 *	by the csibuf pointer. If the block cannot be freed as
 *	a single chunk, someone still owns some. In this case a
 *	message is printed and the space is freed in chunks.
 *	Freeing is done by allocating all the memory in the map
 *	and then releasing the block with sptfree.
 */

csifree(csibufp)
register struct csibuf *csibufp;	/* Ptr to csibuf structure */
{
	register int	i = 0;
	register int    j, flag;

	if (malloc(csibufp->csimap, csibufp->csibsz) == 0) {
		printf("csifree: Can't free block\n");
		for (i = CSIBSZ; i; i >>= 1)
			while (malloc(csibufp->csimap, i))
				;
		i = -1;
	}
#ifdef pdp11
	mfree(coremap,btoc((int)csibufp->csibsz),btoc(csibufp->segp));
	ubmfree(csibufp->ubmd,csibufp->csibsz);
#else
	sptfree(csibufp->segp, btoc((int)csibufp->csibsz), 1);
	ubmfree(csibufp->ubmd);
	for (j=0,flag=0;(j<csibnum)&&(flag==0); j++) {
		if (csibpt[i] == csibufp) {
			csibpt[i]= NULL;
			flag=1;
		}
	}
#endif
	csibufp->csibsz = 0;
	return(i);
}


/*
 *	CSIBGET - Get a buffer and buffer descriptor. Returns pointer
 *	to buffer descriptor or NULL if either a buffer or descriptor
 *	is not available. If a buffer or descriptor can't be obtained
 *	a return or wait is done according to slpflg. Otherwise the
 *	freelist is updated, the size and addr set, and bdp returned.
 */

struct csibd *
csibget(csibufp, nbytes, slpflg)
register struct csibuf *csibufp;	/* Ptr to csibuf structure */
int	nbytes, 			/* Size of buffer to get */
slpflg;			/* Sleep flag */
{
#ifdef pdp11
	ushort	hi;
#endif
	register int 	disp;
	register int	sps;
	register struct csibd *bdp;

	sps = spl5();
	while ((bdp = csibufp->freebdp) == NULL || 
	    (disp = malloc(csibufp->csimap, ROUND(nbytes))) == 0) {
		if (slpflg) {
			sleep((caddr_t)&csibufp->freebdp,CSISLP);
		} else {
			splx(sps);
			return(NULL);
		}
	}
	csibufp->freebdp = bdp->d_next;
	splx(sps);
	bdp->d_size = ROUND(nbytes);
	bdp->d_ct = nbytes;
	bdp->d_adres = nbytes?(csibufp->segp+disp-1):0;
	bdp->d_uadres = nbytes?(csibufp->bubm+disp-1):0;
#ifdef pdp11
	hi = hiword(bdp->d_uadres);
	hiword(bdp->d_uadres) = loword(bdp->d_uadres);
	loword(bdp->d_uadres) = hi;
#endif
	bdp->d_next = NULL;
	return(bdp);
}


/*
 *	CSIBRTN - Returns buffer descriptor and buffer if indicated.
 *	If the address is non-zero the buffer is freed. The descriptor
 *	is linked onto the freelist and a wakeup is done in case
 *	someone was sleeping on a resource.
 */

csibrtn(csibufp, bdp)
register struct csibuf *csibufp;		/* Ptr to csibuf structure */
register struct csibd *bdp;			/* Ptr to bd to return */
{
	register int	sps;

	if (bdp->d_adres && bdp->d_size)
		mfree(csibufp->csimap, ROUND(bdp->d_size),(ushort)(bdp->d_adres-csibufp->segp+1));
	sps = spl5();
	bdp->d_next = csibufp->freebdp;
	csibufp->freebdp = bdp;
	splx(sps);
	wakeup((caddr_t)&csibufp->freebdp);
}


/*
 *	CSIBDLNK - Link a buffer descriptor onto the freelist.
 *	The address field is set to zero and csibrtn called.
 */

csibdlnk(csibufp, bdp)
register struct csibuf *csibufp;		/* Ptr to csibuf structure */
register struct csibd *bdp;			/* Buffer desc. pointer */
{
	bdp->d_adres = 0;
	csibrtn(csibufp, bdp);
}


/*
 *	CSICOPY - Copy cnt bytes from offset in the buffer to
 *	or from user space. 
 */

csicopy(bdp, offset, cnt, rdwr)
struct csibd *bdp;			/* Buffer desc. pointer */
int	offset, 			/* Offset into data buffer */
cnt, 			/* Number of bytes to transfer */
rdwr;			/* Read or write */
{
	if (cnt == 0)
		return(0);
#ifdef pdp11
	pimove((paddr_t)(bdp->d_adres + offset),cnt,rdwr);
#else
	iomove((caddr_t)(bdp->d_adres + offset),cnt,rdwr);
#endif
	if (u.u_error)
		return(-1);
	return(0);
}


/*	CSIXMTQ - Puts a transmitt buffer on the host transmitt queue and 
 *	checks if any transmitt buffers can be put on the KMC transmitt
 *	queue. It returns the length of the total transmitt queue.
 */


csixmtq(idev, bdp)

int	idev;		/* The CSI interface index */
struct csibd *bdp;	/* Buffer descriptor pointer */
{
	register struct csi *cp;
	register int	sps;
	register union csirpt *rp;
	struct unsolrpt unsolrpt;
	union csirpt csirpt;
	paddr_t addr;


	cp = &csi_csi[idev];
	rp = &csirpt;

	if (bdp == NULL)
		return((cp->xmtq.l_ql + cp->xqkmc.l_ql));
	sps = spl5();
	csienq(bdp, &cp->xmtq);
	while ( cp->xqkmc.l_ql < cp->xmax && (bdp = csideq(&cp->xmtq))) {
		addr = ubmdata(bdp);
		if (kmcload(cp->mdev, XBUFINCMD, loword(addr), hiword(addr)) == -1) {
			rp->unsolp = &unsolrpt;
			rp->unsolp->excode = CSINONAPP;
			rp->unsolp->code = PCDCMD;
			(*cp->rint)(idev, cp->pindex, CSITERM, rp);
			csiclean(idev);
		}
		csienq(bdp, &cp->xqkmc);
	}
	splx(sps);
	return(CSIXQL(idev));


}


/*	CSIEMPTQ - Place an empty recieve buffer on the host empty 
 *	recieve queue and the check if any empty recieve buffers
 *	can go on the KMC empty recieve queue. It returns the total
 *	length of the empty recieve queue.
 */

csiemptq(idev, bdp)
int	idev;
struct csibd *bdp;
{
	register struct csi *cp;
	register int	sps;
	register union csirpt *rp;
	struct unsolrpt unsolrpt;
	union csirpt csirpt;
	paddr_t addr;

	cp = &csi_csi[idev];
	rp = &csirpt;
	if (bdp == NULL)
		return(cp->emptq.l_ql + cp->eqkmc.l_ql);
	sps = spl5();
	csienq(bdp, &cp->emptq);
	while (cp->eqkmc.l_ql < cp->rmax && (bdp = csideq(&cp->emptq))) {
		addr = ubmdata(bdp);
		if (kmcload(cp->mdev, RBUFINCMD, loword(addr), hiword(addr)) == -1) {
			rp->unsolp = &unsolrpt;
			rp->unsolp->excode = CSINONAPP;
			rp->unsolp->code = PCDCMD;
			(*cp->rint)(idev, cp->pindex, CSITERM, rp);
			csiclean(idev);
		}
		csienq(bdp, &cp->eqkmc);
	}
	splx(sps);
	return(CSIEQL(idev));


}


/*
 *	CSIENQ - Queues bdp on the llp linked list. if bdp is NULL
 *	the queues length is returned, otherwise the bdp is linked
 *	onto the end and the new length is returned.
 */

csienq(bdp, llp)
struct csibd *bdp;			/* Buffer descriptor pointer */
struct csillist *llp;			/* List pointer */
{
	int	sps;

	sps = spl5();
	if (bdp) {
		if (bdp->d_next != NULL) {
			splx(sps);
			return(-1);
		}
		if (llp->l_ql)
			llp->l_last->d_next = bdp;
		else
			llp->l_first = bdp;
		llp->l_last = bdp;
		++llp->l_ql;
	}
	splx(sps);
	return(llp->l_ql);
}


/*
 *	CSIDEQ - Returns the first bdp on the llp linked list.
 *	Returns NULL if the list is empty.
 */

struct csibd *
csideq(llp)
struct csillist *llp;
{
	register struct csibd *bdp;
	int	sps;

	sps = spl5();
	if (llp->l_ql == 0) {
		splx(sps);
		return(NULL);
	}
	bdp = llp->l_first;
	llp->l_first = bdp->d_next;
	if (llp->l_first == NULL)
		llp->l_last = NULL;
	bdp->d_next = NULL;
	--llp->l_ql;
	splx(sps);
	return(bdp);
}


/*
 *	CSIRMV - Removes the buffer descriptor specified from the
 *	linked list specified. Returns NULL if bdp is not found.
 */

csirmv(bdp, llp)
register struct csibd *bdp;
struct csillist *llp;
{
	register struct csibd *prvpt, *pt;
	int	sps;

	prvpt = (struct csibd *)NULL;
	sps = spl5();
	for (pt = llp->l_first; pt != NULL; pt = pt->d_next) {
		if (pt == bdp) {
			if ( pt == llp->l_first) {
				llp->l_first = pt->d_next;
				if (llp->l_first == NULL)
					llp->l_last = NULL;
			} else {
				if (pt == llp->l_last) {
					llp->l_last = prvpt;
					prvpt->d_next = NULL;
				} else 
					prvpt->d_next = pt->d_next;
			}
			bdp->d_next = NULL;
			--llp->l_ql;
			splx(sps);
			return(1);
		}
		prvpt = pt;
	}
	splx(sps);
	return(0);
}



/*
 *	CSISTAT - Returns status information about this CSI line.
 */

csistat(idev, infop)
int	idev;
register struct csiinfo *infop;
{
	register struct csi *cp;

	cp = &csi_csi[idev];
	infop->mdev = cp->mdev;
	infop->devtype = KMCDEV(cp->mdev);
	infop->options = cp->flags.options;
	infop->intopts = cp->flags.intopts;
	infop->window = cp->flags.window;
	infop->timer0 = cp->flags.timer0;
	infop->timer1 = cp->flags.timer1;
}


/*
 *	CSISAVE - Creates an event record and uses trsave to pass
 *	it to the trace driver.
 */

csisave(idev, type, word1, word2)
int	idev, word1, word2;
unsigned char	type;
{
	static int	csiseqn;
	register sps;
	struct {
		short	e_seqn;
		char	e_type;
		char	e_dev;
		short	e_word1;
		short	e_word2;
	} csient;

	sps = spl5();
	if (csiseqn >= 077777)
		csiseqn = 0;
	csient.e_seqn = ++csiseqn;
	csient.e_type = type;
	csient.e_dev = idev;
	csient.e_word1 = word1;
	csient.e_word2 = word2;
	trsave(0, idev, (char *) & csient, sizeof(csient));
	splx(sps);
}



/*	CSIOK - does the ok checking (VAX version only) with the KMC's.
 *	If not ok it then terminates all processes using that devive.
 */


csiok(dev)
{
	register struct csik *kp;
	register struct csi *cp;
	register int	idev;
	register union csirpt *rp;
	struct unsolrpt unsolrpt;
	union csirpt csirpt;
	int	sps;


	rp = &csirpt;
	kp = &csik[dev];
	if(kp->state&KMCRESET) {
		timeout(csiok,dev,5*HZ);
		return;
	}
	if((kp->state&KMCRUN)==0)
		return;
	if (kp->state & KMCOK) {
		kp->state &= ~KMCOK;
		timeout(csiok,dev,5 * HZ);
	} else {
		kp->state &= ~KMCRUN;
		kmcreset(dev);
		for (cp = &csi_csi[0], idev = 0; idev < csi_cnt; ++cp, ++idev) {
			if ((cp->state & C_START) && KMCDEV(cp->mdev) == dev) {
				rp->unsolp = &unsolrpt;
				rp->unsolp->code = OKFAIL;
				rp->unsolp->excode = CSINONAPP;
				sps = spl5();
				(*cp->rint)(idev, cp->pindex, CSITERM, rp);
				csisave(idev, 'O', dev, kp->state);
				csiclean(idev);
				splx(sps);
			}
		}
	}
}




/* 	CSICLR - Does the power fail restart sequence for the 
 *	CSI interface. 
 */

csiclr()
{
	register dev;
	register n;
	register struct csi *cp;
	register struct csik *kp;
	register struct csibuf *csibp;
	register union csirpt *rp;
	struct solrpt solrpt;
	struct unsolrpt unsolrpt;
	union csirpt csirpt;
	short	opts;
	int 	i;


	rp = &csirpt;
#ifdef vax
	for (i=0; i<csibnum; i++) {
		if((csibp = csibpt[i]) != NULL)
			csibp->bubm = ubmvad((paddr_t)csibp->segp,csibp->ubmd);
	}
#endif
	for (dev = 0; dev < KMCS; ++dev)
		(&csik[dev])->state &= ~KMCRESET;
	for (dev = 0; dev < csi_cnt; ++dev) {
		cp = &csi_csi[dev];
		if (cp->state&C_ATTACH) {
			kp = &csik[KMCDEV(cp->mdev)];
			if ((kp->state & KMCRUN) && (kp->state & KMCRESET) == 0) {
				kmcdclr(KMCDEV(cp->mdev));
				if (kmcset(cp->mdev, CSIINTR, csirint)) {
					rp->unsolp = &unsolrpt;
					rp->unsolp->excode = CSINONAPP;
					rp->unsolp->code = PCDCMD;
					(*cp->rint)(dev, cp->pindex, CSITERM, rp);
					csiclean(dev);
				}
				if (kmcload(cp->mdev & 0300, BASEIN, NOARG, NOARG) == -1) {
					rp->unsolp = &unsolrpt;
					rp->unsolp->excode = CSINONAPP;
					rp->unsolp->code = PCDCMD;
					(*cp->rint)(dev, cp->pindex, CSITERM, rp);
					csiclean(dev);
				}
				kp->state |= KMCRESET;
				kp->state &= ~KMCRUN;
				csisave(dev, 'R', kp->state, cp->state);
			}
			if (cp->state & C_START) {
				cp->state |= C_RESET;
				opts = ((short)cp->flags.options << 8) | ((short)cp->flags.intopts & 0377);
				if (kmcload(cp->mdev, RUNCMD, opts, dev) == -1) {
					rp->unsolp = &unsolrpt;
					rp->unsolp->excode = CSINONAPP;
					rp->unsolp->code = PCDCMD;
					(*cp->rint)(dev, cp->pindex, CSITERM, rp);
					csiclean(dev);
				}
				for (n = csienq((struct csibd *)NULL, &cp->xqkmc); n > 0; --n) {
					rp->solp = &solrpt;
					rp->solp->bdp = csideq(&cp->xqkmc);
					rp->solp->retcode = CSIXMT;
					(*cp->rint)(dev, cp->pindex, CSIRXBUF, rp);
				}
				for (n = csienq((struct csibd *)NULL, &cp->eqkmc); n > 0; --n) {
					csiemptq(dev, csideq(&cp->eqkmc));
				}
			}
		}
	}
}








struct csibd *
csidexmtq(idev)
{
	register struct csi *cp;

	cp = &csi_csi[idev];

	return(csideq(&cp->xmtq));
}


