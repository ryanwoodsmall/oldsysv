/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/id.c	10.10"
/*
 * 3B2 UNIX Integral Winchester Disk Driver
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sbd.h"
#include "sys/id.h"
#include "sys/if.h"
#include "sys/dma.h"
#include "sys/immu.h"
#include "sys/fs/s5dir.h"
#include "sys/sysmacros.h"
#include "sys/signal.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/elog.h"
#include "sys/iobuf.h"
#include "sys/systm.h"
#include "sys/firmware.h"
#include "sys/cmn_err.h"
#include "sys/vtoc.h"
#include "sys/hdelog.h"
#include "sys/open.h"
#include "sys/inline.h"

/* pointer to disk controller */
extern int idisk;
#define	ID ((struct iddev *) &idisk)

extern struct dma	dmac;
#define	DMAC (&dmac)

#define DIS_DMAC	0x04	/*  disables dmac chan for requested chan */

struct vtoc idvtoc[IDNDRV];

struct idsave idsvaddr[IDNDRV];

/* first block for current job converted to phyical blkno */
daddr_t blk1[IDNDRV];

/* defect maps */
extern struct defstruct iddefect[];

/* bad block stuff */
struct hdedata idelog[IDNDRV];
extern hdelog ();

extern int idiskmaj;	/* defined in master.d file for idisk */
/* physical description table */
struct pdsector	idsect0[IDNDRV];

/* seek parameter structure */
struct idseekstruct idseekparam[IDNDRV];

/* controller initialization */
struct idspecparam idspec_s;
struct idspecparam idspec_f;

/* transfer parameter sturcture */
struct idxferstruct idxferparam[IDNDRV];


struct {
	unsigned char noparam;
} idnoparam;

/* drive status byte */
struct idstatstruct idstatus[IDNDRV];

/* temporary buffer for jobs which cross 64-Kbyte boundaries */
struct {
	unsigned int buf[128];
} idcache[IDNDRV+1];
unsigned int idpcacheaddr[IDNDRV];

struct	iobuf	idtab[IDNDRV];		/* drive information */
struct	iotime	idtime[IDNDRV];		/* drive status information */
extern time_t	time;			/* system time in seconds */

int	idspurintr;		/* spurious interrupt counter */
extern	paddr_t id_addr[];	/* local bus base address of disk controller */
extern unsigned ifstate;	/* floppy driver state register */

/* rename buf structure variables */
#define	acts	io_s1
#define	cylin	b_resid
#define ccyl	jrqsleep

idcopy (faddr, taddr, count)
unsigned int *faddr;
unsigned int *taddr;
unsigned int count;
{
	unsigned int *fptr;
	unsigned int *tptr;
	int i;

	tptr = taddr;
	fptr = faddr;
	for (i=0; i<(count/4); i++)
		*tptr++ = *fptr++;
}

unsigned char idscanflag;
unsigned char idnoscan;
idscan ()
{
	int s;
	s = spl6();
	if ((idtab[0].b_actf != IDNULL) || (idtab[1].b_actf != IDNULL)) {
		if (idscanflag == IDNULL)
			idrecal (IDNOUNIT);
		else
			idscanflag = IDNULL;
	}
	splx (s);
	timeout (idscan, 0, (10*HZ));
}
	
idsetblk (bufhead, cmd, blkno, dev) 
struct buf *bufhead;
unsigned char cmd;
daddr_t blkno;
dev_t dev;
{
	clrbuf (bufhead);
	bufhead->b_flags |= cmd;
	bufhead->b_blkno = blkno;
	bufhead->b_dev = (dev|IDNODEV);
	bufhead->b_proc = 0x00;
	bufhead->b_flags &= ~B_DONE;
	if (cmd == B_WRITE)
		bufhead->b_bcount = idsect0[iddn(minor(dev))].pdinfo.bytes;
}

/* Set up the initial values for pdsector and clear defect map */
idsetdef(unit)
int unit;
{
	register int j;

	/* initialize sector 0 */
	idsect0[unit].pdinfo.driveid = 0x02;
	idsect0[unit].pdinfo.sanity = 0x00;
	idsect0[unit].pdinfo.version = 0x01;
	idsect0[unit].pdinfo.cyls = 1;
	idsect0[unit].pdinfo.tracks = 1;
	idsect0[unit].pdinfo.sectors = 18;
	idsect0[unit].pdinfo.bytes = 512;

	/* initialize mach defect management tables */
	for (j=0; j<(IDDEFSIZ/8); j++) {
		iddefect[unit].map[j].bad.full = 0xffffffff;
		iddefect[unit].map[j].good.full = 0xffffffff;
	}
}

/* idopen - on first call reads in physical description, vtoc, 
   and defect info */
idopen(dev,flag,otyp)
{
	struct buf *geteblk();
	struct buf *bufhead;
	register int unit, i, j;
	int defcnt, defaddr;

	if (idnoscan == IDSET) {
		idnoscan = IDNULL;
		idscan ();
	}
	unit = iddn(minor(dev));
	if (idstatus[unit].equipped == IDNULL) {
		/* no disk out there */
		u.u_error = ENXIO;
		return;
	}
	while (idstatus[unit].open == IDOPENING) {
		sleep(&idstatus[unit],PZERO);
	}
	if (idstatus[unit].open == IDNOTOPEN) {
		idstatus[unit].open = IDOPENING;

		/* set up default values in the pdsector */
		idsetdef(unit);

		/* read physical description sector */
		bufhead = geteblk();
		idsetblk (bufhead, B_READ, IDPDBLKNO, dev);
		idstrategy(bufhead);
		iowait(bufhead);
		if (bufhead->b_flags & B_ERROR) {
			cmn_err(CE_WARN,"\nhard disk:  cannot read sector 0 on drive %d\n",unit);
			goto badopen;
		}
		idcopy (bufhead->b_un.b_addr, &idsect0[unit], sizeof(struct pdsector));


 		if (idsect0[unit].pdinfo.sanity == VALIDINFO) {
 			idsetdef(unit);
 			cmn_err(CE_WARN,"\nhard disk:  Drive %d is in the 1.0 layout.  It can not be used until the conversion is made to the current layout.\n",unit);
 			goto opendone;
 		}else if (idsect0[unit].pdinfo.sanity != VALID_PD) {
			cmn_err(CE_WARN,"\nhard disk:  Bad sanity word on drive %d.\n",unit);
			goto badopen;
		}

		/* read the defect map */
		if (idsect0[unit].pdinfo.defectsz > IDDEFSIZ) {
			cmn_err (CE_WARN, "\nhard disk: too little space allocated in driver for defect table on drive %d\n", unit);
			goto badopen;
		}
		for (defcnt = 0; defcnt < (idsect0[unit].pdinfo.defectsz/idsect0[unit].pdinfo.bytes);defcnt++) {
			idsetblk (bufhead, B_READ, idsect0[unit].pdinfo.defectst+defcnt, dev);
			idstrategy(bufhead);
			iowait(bufhead);
			if (bufhead->b_flags & B_ERROR) {
				cmn_err(CE_WARN,"\nhard disk:  Cannot read defect map on drive %d\n",unit);
				goto badopen;
			}
			defaddr = ((int)&iddefect[unit])+(defcnt*idsect0[unit].pdinfo.bytes);
			idcopy (bufhead->b_un.b_addr, defaddr, idsect0[unit].pdinfo.bytes);
		}

		/* read in the vtoc */
		idsetblk (bufhead,B_READ, idsect0[unit].pdinfo.logicalst+IDVTOCBLK, dev);
		idstrategy(bufhead);
		iowait(bufhead);
		if (bufhead->b_flags & B_ERROR) {
			cmn_err(CE_WARN,"\nhard disk:  Cannot read the VTOC on drive %d\n",unit);
			goto badopen;
		}
		idcopy (bufhead->b_un.b_addr, &idvtoc[unit], sizeof(struct vtoc));
		if (idvtoc[unit].v_sanity != VTOC_SANE) {
			cmn_err(CE_WARN,"\nhard disk: Bad sanity word in VTOC on drive %d.\n",unit);
			goto opendone;
		}

		/* open is complete - wakeup sleeping processes and return buffer */
		
		idstatus[unit].open = IDISOPEN;
		goto opendone;
badopen:
		u.u_error = ENXIO;
opendone:

		if (idstatus[unit].open != IDISOPEN)
			idstatus[unit].open = IDNOTOPEN;
		wakeup(&idstatus[unit]);
		brelse(bufhead);

	}
}

/* idclose is provided as a null function */
idclose(dev)
{
}

/* reset and initialize controller, initialize controller table */
/* check for drive ready and recalibrate drives */
idinit()
{
	register int i, j;
	int iplsave;
 	dev_t ddev;
	extern int hdeeduc, hdeedct;

	/* ENTER CRITICAL REGION */
	iplsave = spl6();

	/*  controller initialization - specify parameter structure  */
	idspec_s.mode = 0x18;
	idspec_s.dtlh = 0xe2;
	idspec_s.dtll = 0x00;
	idspec_s.etn = 0xef; 
	idspec_s.esn = 0x11;
	idspec_s.gpl2 = 0x0d;
	idspec_s.rwch = 0x00;
	idspec_s.rwcl = 0x80;

	idspec_f.mode = 0x1f;
	idspec_f.dtlh = 0xe2;
	idspec_f.dtll = 0x00;
	idspec_f.etn = 0xef;
	idspec_f.esn = 0x11;
	idspec_f.gpl2 = 0x0d;
	idspec_f.rwch = 0x00;
	idspec_f.rwcl = 0x80;

	for (i=0, j=0; i < IDNDRV; i++, j++) { 
		/* initialize drive state */
		idstatus[i].open = IDNOTOPEN;
		idstatus[i].state = IDIDLE;
		idstatus[i].equipped = IDNULL;
		idpcacheaddr[i] = (unsigned int) vtop(&idcache[j],IDNULL);
		if (((idpcacheaddr[i] & MSK64K)+0x200)>BND64K) {
			j++;
			idpcacheaddr[i]=(unsigned int)vtop(&idcache[j],IDNULL);
		}
	}
	idrecal (IDNOUNIT);
	idnoscan = IDSET;
	idscanflag = IDSET;
	/* EXIT CRITICAL REGION */
	splx (iplsave);
 	for (j = 0; j < 128; j++)
 		if (MAJOR[j] == idiskmaj) break;
 	for (i = 0; i < IDNDRV; i++)
 		if (idstatus[i].equipped != IDNULL) {
 			ddev = makedev(j, idmkmin(i));
 			hdeeqd(ddev, IDPDBLKNO, EQD_ID);
 		}
} /* end idinit */


/*
 *	Break up the request that came from physio into chunks of
 *	contiguous memory so we can get around the DMAC limitations
 *	We must be sure to pass at least 512 bytes (one sector) at a
 *	time (except for the last request).   */

idbreakup(bp)
register struct buf *bp;
{
	dma_breakup(idstrategy, bp);
}  /* end idbreakup */

idstrategy (bufhead)
register struct buf *bufhead;
{
	register struct iobuf *drvtab;	/* drive status pointer */
	daddr_t	lastblk;		/* last block in partition */
	register int unit;		/* drive unit id */
	int	partition;		/* drive partition number */
	int	iplsave;		/* saved interrupt level */
 	int	sectoff;		/* start sector of this partition */

	/* initialize local variables */
	partition = idslice (minor (bufhead->b_dev));
	unit = iddn (minor (bufhead->b_dev));
	if (idstatus[unit].equipped == IDNULL)
		goto diskerr;

	if (idnodev (minor (bufhead->b_dev))) {
		lastblk = (idsect0[unit].pdinfo.sectors * idsect0[unit].pdinfo.tracks * idsect0[unit].pdinfo.cyls);
		sectoff = 0x00;
	}else	{
		/* check for invalid VTOC */
		if (idvtoc[unit].v_sanity != VTOC_SANE) {
			goto diskerr;
		}
		/* check for read only partition */
		if (((idvtoc[unit].v_part[partition].p_flag & V_RONLY)==V_RONLY)&&((bufhead->b_flags & B_READ)!=B_READ)) {
			u.u_error = ENXIO;
			cmn_err (CE_WARN, "\nhard disk: partition %d on drive %d is marked read only\n", partition, unit);
			goto diskerr;
		}
		lastblk = idvtoc[unit].v_part[partition].p_size;
 		sectoff = (idvtoc[unit].v_part[partition].p_start 
 			+ idsect0[unit].pdinfo.logicalst);
	}
	drvtab = &idtab[unit];

	/* if the requested block does not exist within requested partition */
	if ((bufhead->b_blkno >= lastblk) ||
	    (bufhead->b_blkno < IDFRSTBLK)  ||
	    (bufhead->b_blkno+((bufhead->b_bcount-1)/idsect0[unit].pdinfo.bytes) >= lastblk)) {
		if ((bufhead->b_blkno == lastblk) && (bufhead->b_flags&B_READ)) {
			/* this case is here to help read ahead */
			bufhead->b_resid = bufhead->b_bcount;
			iodone(bufhead);
			return;
		}
		goto diskerr;
	}


	/* ENTER CRITICAL REGION */
	iplsave = spl6();
 	bufhead->cylin	= ((bufhead->b_blkno+sectoff)
 			/(idsect0[unit].pdinfo.sectors
 			*idsect0[unit].pdinfo.tracks)); 
	bufhead->b_start = lbolt; /* time stamp request */
	idtime[unit].io_cnt++;    /* inc operations count */
	/*	Increment disk block count */

	idtime[unit].io_bcnt +=
		(bufhead->b_bcount + NBPSCTR-1) >> SCTRSHFT;
	drvtab->qcnt++;		  /* inc drive current request count */


	/* link buffer header to drive worklist */
	bufhead->av_forw = IDNULL;
	if (drvtab->b_actf == IDNULL) {
		idscanflag = IDSET;
		drvtab->b_actf = bufhead;
		drvtab->b_actl = bufhead;
		drvtab->acts = (int)bufhead;
		idsetup (unit);
		idseek (unit);
	} else {
 		register struct buf *ap, *cp;
 		if (((int)idtime[unit].io_cnt&0x0f) == 0)
 			drvtab->acts = (int)drvtab->b_actl;
 		for (ap = (struct buf *)drvtab->acts; cp = ap->av_forw; ap = cp) {
 			int s1, s2;
 			if ((s1 = ap->cylin - bufhead->cylin)<0)
 				s1 = -s1;
 			if ((s2 = ap->cylin - cp->cylin)<0)
 				s2 = -s2;
 			if (s1 < s2)
 				break;
 		}
 		ap->av_forw =  bufhead;
 		if ((bufhead->av_forw = cp) == NULL)
 			drvtab->b_actl = bufhead;
 		bufhead->av_back = ap;
	}
	/* EXIT CRITICAL REGION */
	splx (iplsave);
	return;

diskerr:
	bufhead->b_flags |= B_ERROR;
	bufhead->b_error = ENXIO;
	iodone (bufhead);
	return;
}

idsetup (unit)	/* This routine fills the drive command buff from buff head */
unsigned int unit;
{
	register int	blkcnt;		/* number of blocks this job */
	register struct buf *bufhead;	/* buffer header */
	register struct iobuf *drvtab;	/* head of drive worklist */
	register unsigned char *user, *driver;
	union diskaddr daddress;	/* disk address for current job */
	int vaddress;			/* virtual memory address */
	int paddress;			/* physical memory address */
	int	sectoff;		/* sector offset into drive */
	int 	sectno;			/* sector number on cylinder */
	int	partition;		/* drive partition number */
	struct defect *deftab;		/* pointer to the defect table */
	union diskaddr lastsect;	/* last sector for this job */
	int lsectoff;			/* offset of the last sector in job */
	int i;
	int defcnt, bytes;
	unsigned char partial;
	
	/* initialize local variables */
	drvtab = &idtab[unit];
	bufhead = drvtab->b_actf;
	deftab = iddefect[unit].map;
	partition = idslice (minor (bufhead->b_dev));
	if (idnodev (minor (bufhead->b_dev)))
		sectoff = 0x00;
	else
		sectoff = (idvtoc[unit].v_part[partition].p_start + idsect0[unit].pdinfo.logicalst);

	/* if no work on worklist */
	if (bufhead == IDNULL) 
		return(IDFAIL);

	/* if this is the first time this job has come through, time stamp it 
	   and save buffer header information */
	if (drvtab->b_active == 0) {
		idsvaddr[unit].b_addr = bufhead->b_un.b_addr;
		idsvaddr[unit].b_blkno = bufhead->b_blkno;
		idsvaddr[unit].b_bcount = bufhead->b_bcount;
		drvtab->io_start = lbolt;
	}

	blk1[unit] = idsvaddr[unit].b_blkno + sectoff;

	/* increase activity count */
	drvtab->b_active++;

	/* clear result information */
	idstatus[unit].retries = IDRETRY;
	idstatus[unit].reseeks = IDRESEEK;


	/* compute disk address */
	bufhead->cylin = ((idsvaddr[unit].b_blkno+sectoff)/(idsect0[unit].pdinfo.sectors*idsect0[unit].pdinfo.tracks)); /* start cylinder */
	sectno = (idsvaddr[unit].b_blkno+sectoff)%(idsect0[unit].pdinfo.sectors*idsect0[unit].pdinfo.tracks);	/* offset into start cylinder */

	/* load disk address */
	daddress.part.pcnh = (bufhead->cylin>>8)&0xff;
	daddress.part.pcnl = bufhead->cylin&0xff;
	daddress.part.phn = sectno/idsect0[unit].pdinfo.sectors;
	daddress.part.psn = sectno%idsect0[unit].pdinfo.sectors;

	/* get physical address from buffer header */
	vaddress = (int) idsvaddr[unit].b_addr;
	paddress = vtop(vaddress, bufhead->b_proc);
	if (paddress==IDNULL) 
		cmn_err(CE_PANIC,"\nhard disk: Bad address returned by VTOP\n");
	/* blocks to do this job */
 	blkcnt = idsvaddr[unit].b_bcount/idsect0[unit].pdinfo.bytes;

	/* chop the job up */
	/* make sure we don't overrun track boundary */
	if ((daddress.part.psn+blkcnt) > idsect0[unit].pdinfo.sectors)
		blkcnt = idsect0[unit].pdinfo.sectors - daddress.part.psn;

	/* check for 64K-byte boundary overrun  or partial sector r/w */
	partial = 0;
 	if (idsvaddr[unit].b_bcount < idsect0[unit].pdinfo.bytes)
		partial = 1;
	if ((((paddress & MSK64K)+(blkcnt*idsect0[unit].pdinfo.bytes)) > BND64K)
 	|| (partial)) {
		blkcnt = (BND64K - (paddress & MSK64K)) / idsect0[unit].pdinfo.bytes;
		/* if sector r/w crosses 64-Kbyte boundary or partial sector */
		if ((blkcnt == 0) || partial) {
			blkcnt = 1;
			/* if its a write to disk, copy form user to driver */
			if ((bufhead->b_flags&B_READ) != B_READ) {
				bytes = idsect0[unit].pdinfo.bytes;
				if (partial) {
					register unsigned int *zp;
 					bytes = idsvaddr[unit].b_bcount;
					zp = (unsigned int *)idpcacheaddr[unit];
					for (i=0; i<128; i++)
						*zp++ = 0x00000000;
				}
				user = (unsigned char *) paddress;
				driver = (unsigned char *) idpcacheaddr[unit];
				for (i=0; i<bytes; i++)
					*driver++ = *user++;
			}
			paddress = idpcacheaddr[unit];
		}
	}

	/* look for any defective sectors in this job */
	for (defcnt=0;(defcnt<(IDDEFSIZ/8))&&(daddress.full>deftab->bad.full); defcnt++) {
		deftab++;
	}
	/* determine the address of the last sector for this job */
	lastsect.part.pcnh = daddress.part.pcnh;
	lastsect.part.pcnl = daddress.part.pcnl;
	lastsect.part.phn = (sectno+blkcnt-1)/(idsect0[unit].pdinfo.sectors);
	lastsect.part.psn = (sectno+blkcnt-1)%(idsect0[unit].pdinfo.sectors);

	if (lastsect.full >= deftab->bad.full) {
		if (daddress.full == deftab->bad.full) {
			daddress.full = deftab->good.full;
			blkcnt=1;
		} else {
			lsectoff = (deftab->bad.part.phn*(idsect0[unit].pdinfo.sectors))+deftab->bad.part.psn;
			blkcnt = lsectoff-sectno;
		}
	}
			
		
	/* load seek parameters */
	idseekparam[unit].pcnh = daddress.part.pcnh;
	idseekparam[unit].pcnl = daddress.part.pcnl;

	/* load transfer parameters */
	idxferparam[unit].phn = daddress.part.phn;
	idxferparam[unit].lcnh = ~daddress.part.pcnh;
	idxferparam[unit].lcnl = daddress.part.pcnl;
	idxferparam[unit].lhn = daddress.part.phn;
	idxferparam[unit].lsn = daddress.part.psn;
	idxferparam[unit].scnt = blkcnt;
	idxferparam[unit].bcnt = blkcnt*idsect0[unit].pdinfo.bytes;
	idxferparam[unit].necop = (bufhead->b_flags&B_READ) ? IDREAD:IDWRITE;
	idxferparam[unit].dmacop = (bufhead->b_flags&B_READ) ? WDMA:RDMA;
	idxferparam[unit].b_addr = paddress;
	idxferparam[unit].unitno = unit;
	/* if the head number is greater than 7 */
	if (idxferparam[unit].phn >= IDMAXHD) 
		idxferparam[unit].unitno += IDADDEV;
	
	/* adjust remaining byte count and start address */
	if (idxferparam[unit].b_addr != idpcacheaddr[unit]) {
		idsvaddr[unit].b_bcount -= (blkcnt*idsect0[unit].pdinfo.bytes);
		idsvaddr[unit].b_addr += (blkcnt*idsect0[unit].pdinfo.bytes);
	}
	idsvaddr[unit].b_blkno = idsvaddr[unit].b_blkno + blkcnt;
	return(IDPASS);
} /* end idsetup */

idrecal (unit)
register int unit;
{
	unsigned int i, j;
	unsigned char retval[2];
	register struct buf *bufhead;
	register struct iobuf *drvtab;
	register struct idstatstruct *stat;
	unsigned short cyl;
 	int sects, tracks;

	if (unit != IDNOUNIT)
		idstatus[unit].reseeks--;
	idldcmd(IDRESET, &idnoparam, IDNOPARAMCNT, IDINTON);
	/* wait for controller to reset */
	for (i=0; i < 1000; i++)
	;
	/* re-specify controller characteristics */
	idldcmd(IDSPECIFY, &idspec_s, IDSPECCNT, IDINTOFF);
	/* clear out not-ready interrupts from nonexisting drives */
	for (i=0; i<10000; i++)
	;
	if (ID->statcmd & IDSINTRQ) {
		idldcmd(IDSENSEINT, &idnoparam, IDNOPARAMCNT, IDINTOFF);
		while ((ID->statcmd & IDSINTRQ) != IDSINTRQ)
		;
		idldcmd(IDSENSEINT, &idnoparam, IDNOPARAMCNT, IDINTOFF);
	}
	/* init each drive attached to controller */
	for (i=0; i < IDNDRV; i++) {
		stat = &idstatus[i];
		/* check for drive ready */
		if (idldcmd(IDSENSEUS|i, &idnoparam, IDNOPARAMCNT,IDINTOFF)==IDFAIL) {
			if (stat->equipped == IDSET) {
				stat->equipped = IDNULL;
				idflush (i);
			}
			continue;
		}
		stat->ustbyte = ID->fifo;
		if ((stat->ustbyte & IDREADY) != IDREADY) {
			if (stat->equipped == IDSET) {
				stat->equipped = IDNULL;
				idflush (i);
			}
			continue;
		}
		stat->equipped = IDSET;
		retval[i] = IDFAIL;
		for (j=0; ((retval[i] == IDFAIL) && (j<4)); j++) {
			if (idldcmd(IDRECAL|i|IDBUFFERED, &idnoparam, IDNOPARAMCNT, IDINTOFF) == IDFAIL)
				continue;
			while ((ID->statcmd & IDSINTRQ) != IDSINTRQ)
			;
			if (idldcmd(IDSENSEINT,&idnoparam,IDNOPARAMCNT,IDINTOFF)==IDFAIL)
				continue;
			stat->istbyte = ID->fifo;
			if ((stat->istbyte & (IDSEEKEND|IDSEEKERR)) != IDSEEKEND)
				continue;
			idtab[i].ccyl = 0;
			stat->state = IDIDLE;
			retval[i] = IDPASS;
		}
		if (retval[i] == IDFAIL) {
			stat->equipped = IDNULL;
			cmn_err(CE_WARN,"\nhard disk: cannot recal drive %d\n", i);
			idflush (i);
		}
	}
	idldcmd(IDSPECIFY, &idspec_f, IDSPECCNT, IDINTOFF);
	if (unit != IDNOUNIT) {
		stat = &idstatus[unit];
		drvtab = &idtab[unit];
		if ((stat->reseeks == 0) && (drvtab->b_actf != IDNULL)) {
			bufhead = drvtab->b_actf;
			drvtab->b_active = IDNULL;
			drvtab->b_actf = bufhead->av_forw;
			bufhead->b_flags |= B_ERROR;
			bufhead->b_error |= EIO;
			bufhead->b_resid = 0;
			drvtab->qcnt--;
 			if (bufhead == (struct buf *)drvtab->acts)
 				drvtab->acts = (int)drvtab->b_actf;
			/* update status information */
			idtime[unit].io_resp += lbolt - bufhead->b_start;
			idtime[unit].io_act += lbolt - drvtab->io_start;
	
			stat->state = IDIDLE;
			cyl=((idseekparam[unit].pcnh<<8)|idseekparam[unit].pcnl);
  			idelog[unit].diskdev = bufhead->b_dev & ~(IDNODEV|idslice((-1)));
 			sects = idsect0[unit].pdinfo.sectors;
 			tracks = idsect0[unit].pdinfo.tracks;
  			idelog[unit].blkaddr = 
 				(cyl*sects*tracks) 
 				+ (stat->lhn*sects) 
				+ stat->lsn;
 			idelog[unit].readtype = HDECRC;
 			idelog[unit].severity = HDEUNRD;
 			idelog[unit].bitwidth = 0;
 			idelog[unit].timestmp = time;
			for (i=0;i<12;++i)
				idelog[unit].dskserno[i]=idsect0[unit].pdinfo.serial[i];
 			hdelog (&idelog[unit]);
			cmn_err (CE_WARN,"\nhard disk: cannot access sector %d, head %d, cylinder %d, on drive %d\n",
				stat->lsn, stat->lhn, cyl, unit);
			/* return buffer header to UNIX */
			iodone (bufhead);
			
			if (drvtab->b_actf != IDNULL)
				idsetup (unit);
		}
	}
	for (i=0; i < IDNDRV; i++)
		if (idtab[i].b_actf != IDNULL)
			idseek (i);
}

/* start seek for drive specified */
idseek (unit)
register int unit;
{
	unsigned int other;
	int iplsave;

	other = (unit^1);
	idstatus[unit].state = IDSEEK0;
	iplsave = spl6();
	if ((idstatus[other].state & IDBUSY) == IDBUSY) {
		idstatus[unit].state |= IDWAITING;
		splx(iplsave);
		return;
	}
	idstatus[unit].state |= IDBUSY;
	splx(iplsave);
	if (idtab[unit].ccyl == ((idseekparam[unit].pcnh<<8)|(idseekparam[unit].pcnl))) {
		idxfer (unit); return;
	}
		
	/* set drive current cylinder */
	idtab[unit].ccyl=((idseekparam[unit].pcnh<<8)|(idseekparam[unit].pcnl));
	idldcmd (IDSEEK|unit|IDBUFFERED, &idseekparam[unit], IDSEEKCNT, IDINTON);
}

idtimeout (unit) 
register int unit;
{
	int iplsave;
	iplsave = spl6 ();

	dma_access (CH0IHD, idxferparam[unit].b_addr, idxferparam[unit].bcnt,
		    DMNDMOD, idxferparam[unit].dmacop);
	idldcmd(idxferparam[unit].necop|idxferparam[unit].unitno,&idxferparam[unit],IDXFERCNT,IDINTON);
	splx (iplsave);
}

idxfer (unit)
register int unit;
{
	unsigned int other;
	int iplsave;
	unsigned ifcount;
	other = (unit^1);
	idstatus[unit].state = IDXFER;
	iplsave = spl6();
	if ((idstatus[other].state & IDBUSY) == IDBUSY) {
		idstatus[unit].state |= IDWAITING;
		splx(iplsave);
		return;
	}
	idstatus[unit].state |= IDBUSY;
	splx(iplsave);
	idstatus[unit].retries--;
	if (idstatus[unit].retries == 0) {
		idstatus[unit].retries = IDRETRY;
		idrecal (unit);
		return;
	}
	if ((ifstate & IFFMAT1) == IFFMAT1) {
		timeout (idtimeout, unit, (2*HZ)/5);
		return (IDPASS);
	}
	if ((ifstate&IFBUSYF) == IFBUSYF) {
		iplsave = spltty ();
		DMAC->CBPFF = IDNULL;
		ifcount = 0;
		ifcount = DMAC->C1WC;
		ifcount |= (DMAC->C1WC<<8);
		if (ifcount != IFDMACNT) {
			timeout (idtimeout, unit, HZ/22);
			splx (iplsave);
			return (IDPASS);
		}
		splx (iplsave);
	}
		
	/* load the DMAC */
	dma_access (CH0IHD, idxferparam[unit].b_addr, idxferparam[unit].bcnt,
		    DMNDMOD, idxferparam[unit].dmacop);
	if (idldcmd(idxferparam[unit].necop|idxferparam[unit].unitno, &idxferparam[unit],IDXFERCNT,IDINTON) == IDFAIL) 
		return (IDFAIL);
	return (IDPASS);
} /* end idxfer */

idint (dev)
{
register struct buf *bufhead;
register unsigned char statreg;
register unsigned int unit;
register unsigned char *driver, *user;
struct iobuf *drvtab;
int vaddress;	/* virtual memory address of user space */
int istbyte;
unsigned int other;
unsigned int fromaddr;
int i, bytes;

statreg = ID->statcmd;
idscanflag = IDSET;

/* check spurious interrupt */
if ((statreg & (IDSINTRQ|IDENDMASK)) == 0) {
	/* increment spurious interrupt count */
	idspurintr++; return;
}

/* establish unit for command end interrupt */
if ((statreg & IDENDMASK) != 0) {
	if ((idstatus[0].state & IDBUSY) == IDBUSY)
		unit = 0;
	else if ((idstatus[1].state & IDBUSY) == IDBUSY)
		unit = 1;
	else {
		idspurintr++;
		ID->statcmd = IDCLCMNDEND;
		return;
	}
}
if ((statreg & IDSINTRQ) == IDSINTRQ) {
	/* if the controller is busy, mask the interrupt */
	if ((statreg & IDCBUSY) == IDCBUSY) {
		if ((ID->statcmd & IDCBUSY) == IDCBUSY)
			ID->statcmd = IDMASKSRQ;
		return;
	}
	if ((idstatus[0].state & IDBUSY) && (idstatus[1].state & IDSEEK1)) {
		idstatus[1].state |= IDWAITING;
		if ((statreg & IDENDMASK) == 0) {
			ID->statcmd = IDMASKSRQ; return;
		}
	}
	else if ((idstatus[1].state & IDBUSY) && (idstatus[0].state & IDSEEK1)) {
		idstatus[0].state |= IDWAITING;
		if ((statreg & IDENDMASK) == 0) {
			ID->statcmd = IDMASKSRQ; return;
		}
	}
	else if ((idstatus[0].state & IDBUSY) || (idstatus[1].state & IDBUSY)) {
		if ((statreg & IDENDMASK) == 0) {
			ID->statcmd = IDMASKSRQ; return;
		}
	}
	else {
		if (idldcmd(IDSENSEINT,&idnoparam,IDNOPARAMCNT,IDINTOFF)==IDFAIL) {
			idrecal(IDNOUNIT); return;
		}
		istbyte = ID->fifo;
		unit = istbyte & IDUNITADD;
		if (unit >= IDNDRV) {
			idspurintr++; return;
		}
		idstatus[unit].istbyte = istbyte;
		if ((idstatus[unit].istbyte & IDSEEKMSK) != IDSEEKEND) {
			idrecal (unit); return;
		}
		if ((idstatus[unit].state & IDSEEK1) != IDSEEK1) {
			idspurintr++; return;
		}
	}
}

	switch (idstatus[unit].state & (IDSEEK0|IDSEEK1|IDXFER)) {
	/* Driver expected seek complete? */
	case IDSEEK0:
		ID->statcmd = IDCLCMNDEND;
		other = (unit^1);
		if ((statreg & IDENDMASK) != IDCMDNRT) {
			idrecal (unit); return;
		}
		idstatus[unit].state = IDSEEK1;
		if ((idstatus[other].state & IDWAITING) == IDWAITING) {
			if ((idstatus[other].state & IDSEEK0) == IDSEEK0) {
				idseek (other); return;
			}
			if ((idstatus[other].state & IDSEEK1) == IDSEEK1) {
				if (idldcmd(IDSENSEINT,&idnoparam,IDNOPARAMCNT,IDINTOFF)==IDFAIL) {
					idrecal(other); return;
				}
				istbyte = ID->fifo;
				if ((istbyte & IDUNITADD) != other) {
					idspurintr++; return;
				}
				idstatus[other].istbyte = istbyte;
				if ((idstatus[other].istbyte & IDSEEKMSK) != IDSEEKEND) {
					idrecal (other); return;
				}
				idxfer (other); return;
			}
			if ((idstatus[other].state & IDXFER) == IDXFER) {
				idxfer (other); return;
			}
		}
		return;

	case IDSEEK1:
		if ((idstatus[unit].istbyte & IDSEEKMSK) != IDSEEKEND) {
			idrecal (unit); return;
		}
		idxfer (unit); return;

	case IDXFER:
		/* access extended access information */
		idstatus[unit].statreg = statreg;

		/*
		 * There is a bug in the integral disk controller chip. It
		 * generates a spurious DMA request when one reads extended
		 * status after a fault in which a reset is requested. This
		 * hangs the arbiter pending a DMA which will never actually
		 * occur. We work around this by disabling the DMA controller
		 * hard disk channel when the disk controller primary status
		 * shows "reset requested".
		 */
		if (statreg & IDRESETRQ)
			DMAC->WMKR = CH0IHD | DIS_DMAC;

		idstatus[unit].estbyte = ID->fifo;
		idstatus[unit].phn = ID->fifo;
		idstatus[unit].lcnh = ID->fifo;
		idstatus[unit].lcnl = ID->fifo;
		idstatus[unit].lhn = ID->fifo;
		idstatus[unit].lsn = ID->fifo;
		idstatus[unit].scnt = ID->fifo;

		ID->statcmd = IDCLCMNDEND;
		idstatus[unit].state &= ~IDBUSY;
		other = (unit^1);

		/* format controller has lost control of drive ? */
		if ((statreg & IDRESETRQ) || (statreg & IDERROR)) {
			idrecal (unit); return;
		}
		/* command terminated abnormally ? */
		if (statreg&IDCMDABT) {
			if (idstatus[unit].estbyte & (IDDMAOVR|IDEQUIPTC|IDDATAERR)) {
				idxfer (unit); return;
			}
			idrecal (unit); return;
		}
		if ((idstatus[other].state & IDWAITING) == IDWAITING) {
			if ((idstatus[other].state & IDSEEK0) == IDSEEK0) {
				idseek (other); goto wrapup;
			}
			if ((idstatus[other].state & IDSEEK1) == IDSEEK1) {
				if (idldcmd(IDSENSEINT,&idnoparam,IDNOPARAMCNT,IDINTOFF)==IDFAIL) {
					idrecal(other); return;
				}
				istbyte = ID->fifo;
				if ((istbyte & IDUNITADD) != other) {
					idspurintr++; goto wrapup;
				}
				idstatus[other].istbyte = istbyte;
				if ((idstatus[other].istbyte & IDSEEKMSK) != IDSEEKEND) {
					idrecal (other); return;
				}
				idxfer (other);
				goto wrapup;
			}
			idxfer (other);
		}
	}

wrapup:
	drvtab = &idtab[unit];
	bufhead = drvtab->b_actf;

	/* If a write job and the block number is the VTOC block
	   update the memory image of the VTOC */
	if (((bufhead->b_flags&B_READ) != B_READ)
	     && (blk1[unit] <= (idsect0[unit].pdinfo.logicalst + IDVTOCBLK))) {
		if ((blk1[unit] == idsect0[unit].pdinfo.logicalst)
		    && (idxferparam[unit].bcnt > idsect0[unit].pdinfo.bytes)) {
			fromaddr = vtop(bufhead->b_un.b_addr
			           + idsect0[unit].pdinfo.bytes, bufhead->b_proc);
			idcopy(fromaddr, &idvtoc[unit], sizeof(struct vtoc));
		}

		if ((blk1[unit] == (idsect0[unit].pdinfo.logicalst + IDVTOCBLK))
		    && (idxferparam[unit].bcnt >= sizeof(struct vtoc))) {
			fromaddr = vtop(bufhead->b_un.b_addr , bufhead->b_proc);
			idcopy(fromaddr, &idvtoc[unit], sizeof(struct vtoc));
		}
	}

	/* if buffering for 64K-byte boundary crossing or partial sector r/w */
	if (idxferparam[unit].b_addr==idpcacheaddr[unit]) {
		bytes = idsect0[unit].pdinfo.bytes;
		if (idsvaddr[unit].b_bcount < bytes)
			bytes = idsvaddr[unit].b_bcount;
		/* if reading disk, copy out to user space */
		if (idxferparam[unit].necop==IDREAD) {
			vaddress = (int) idsvaddr[unit].b_addr;
			user = (unsigned char *)vtop(vaddress, bufhead->b_proc);
			driver = (unsigned char *)idpcacheaddr[unit];
			for (i=0; i<bytes; i++)
				*user++ = *driver++;
		}
		idsvaddr[unit].b_addr += bytes;
		idsvaddr[unit].b_bcount -= bytes;
	}
	/* if not done with multi-sector job */
	if (idsvaddr[unit].b_bcount > 0) {
		idsetup (unit);
		idseek(unit);
		return;
	}
	
	/* re-initialize drive worklist header information and unlink buffer header */
	idstatus[unit].state = IDIDLE;
	drvtab->b_active = IDNULL;
	drvtab->b_actf = bufhead->av_forw;
	bufhead->b_resid = 0;
	drvtab->qcnt--;
 	if (bufhead == (struct buf *)drvtab->acts)
 		drvtab->acts = (int)drvtab->b_actf;
	/* update status information */
	idtime[unit].io_resp += lbolt - bufhead->b_start;
	idtime[unit].io_act += lbolt - drvtab->io_start;

	/* return buffer header to UNIX */
	iodone (bufhead);
	
 	/* if no active jobs for drive */
	if (drvtab->b_actf != IDNULL) {
		idsetup (unit);	/* load job parameters in command buffer */
		idseek(unit);
	}
}

idflush (unit)
register unsigned unit;
{
	register struct buf *bufhead;
	register struct iobuf *drvtab;
	drvtab = &idtab[unit];
	while (drvtab->b_actf != IDNULL) {
		bufhead = drvtab->b_actf;
		drvtab->b_active = IDNULL;
		drvtab->b_actf = bufhead->av_forw;
		bufhead->b_resid = bufhead->b_bcount;
		bufhead->b_flags |= B_ERROR;
		bufhead->b_error |= EIO;
		drvtab->qcnt--;
 		if (bufhead == (struct buf *)drvtab->acts)
 			drvtab->acts = (int)drvtab->b_actf;
		idstatus[unit].state = IDIDLE;
		/* return buffer header to UNIX */
		iodone (bufhead);
	}
	cmn_err(CE_WARN,"\nhard disk: drive %d out of service\n", unit);
}

idread(dev)
{
	if (physck(idvtoc[iddn(minor (dev))].v_part[idslice(minor (dev))].p_size, B_READ))
		physio(idbreakup, 0, dev, B_READ);
}

idwrite(dev)
{
	if (physck(idvtoc[iddn(minor (dev))].v_part[idslice(minor (dev))].p_size, B_WRITE))
		physio(idbreakup, 0, dev, B_WRITE);
}

idprint (dev,str)
char *str;
{
	cmn_err(CE_NOTE,
		"%s on integral hard disk %d, partition %d\n",
		str, iddn(minor(dev)), idslice(dev));
}

/* routine to load hard disk controller */
idldcmd(command, params, paramcnt, intopt)

unsigned char command; 	/* command opcode */
unsigned char *params; 	/* pointer to first parameter in parameter list */
unsigned char intopt; 	/* interrupt option */
short paramcnt; 	/* number of parameters for this command */
{
	while (ID->statcmd & IDCBUSY) /* wait for controller not busy */
	;
	ID->statcmd = IDCLFIFO;		/* clear parameter fifo */
	while (paramcnt > 0) {		/* load parameters into controller */
		ID->fifo = *params++;
		paramcnt--;
	}
	ID->statcmd = command;	/* load command opcode into controller */
	if (intopt)
		return (IDPASS);
	/* wait for command end from controller */
	while (ID->statcmd & IDCBUSY)
	;
	if ((ID->statcmd & IDENDMASK) != IDCMDNRT) {
		ID->statcmd = IDCLCMNDEND;
		return (IDFAIL);
	}
	ID->statcmd = IDCLCMNDEND;
	return (IDPASS);
}

ididle()
{
	register int i;
	for (i=0;i<IDNDRV;i++)
		if (idtab[i].b_actf != IDNULL)
			return(1);
	return(0);
}

idioctl(dev,cmd,args,flag)
struct io_arg *args;
{
	struct buf *geteblk();
	struct buf *bufhead;
	int errno, i, xfersz;
	register int unit;
	unsigned int block, mem, count, numbytes, defblock;
	struct io_arg karg;	/* copy of user arg struct */

	if(copyin((char *)args, (char *)&karg, sizeof(struct io_arg))) {
		errno = V_BADREAD;
		suword(&args->retval, errno);
		return;
	}


	unit = iddn (minor (dev));

	switch (cmd) {

	case V_PREAD:
		bufhead = geteblk();
		block = karg.sectst;
		mem = karg.memaddr;
		count = karg.datasz;
		while (count) {
			idsetblk (bufhead, B_READ, block, dev);
			idstrategy(bufhead);
			iowait(bufhead);
			if (bufhead->b_flags & B_ERROR) {
				errno = V_BADREAD;
				suword (&args->retval,errno);
				goto ioctldone;
			}
			xfersz = min (count, bufhead->b_bcount);
			if (copyout(bufhead->b_un.b_addr, mem, xfersz) != 0) {
				errno = V_BADREAD;
				suword (&args->retval,errno);
				goto ioctldone;
			}
			block+=2;
			count -= xfersz;
			mem += xfersz;
		}
		break;

	case V_PWRITE:
		bufhead = geteblk();
		block = karg.sectst;
		mem = karg.memaddr;
		count = karg.datasz;
		defblock = idsect0[unit].pdinfo.defectst;
		numbytes = 0;
		while (count) {
			idsetblk (bufhead, B_WRITE, block, dev);
			xfersz = min (count, bufhead->b_bcount);
			if (copyin (mem, bufhead->b_un.b_addr, xfersz) != 0) {
				errno = V_BADWRITE;
				suword(&args->retval, errno);
				goto ioctldone;
			}
			idstrategy(bufhead);
			iowait(bufhead);
			if (bufhead->b_flags & B_ERROR) {
				errno = V_BADWRITE;
				suword(&args->retval, errno);
				goto ioctldone;
			}

			/* update memory image if special data */
			if (bufhead->b_blkno ==  IDPDBLKNO) {
				idcopy (bufhead->b_un.b_addr, &idsect0[unit], xfersz);
				defblock = idsect0[unit].pdinfo.defectst;
			}
			if (bufhead->b_blkno == defblock) {
				defblock++;
				idcopy (bufhead->b_un.b_addr, (((unsigned int) &iddefect[unit]) + numbytes), xfersz);
				numbytes += xfersz;
			} 

			block+=1;
			count -= xfersz;
			mem += xfersz;
		}
		break;

	case V_PDREAD:
		bufhead = geteblk();
		idsetblk (bufhead, B_READ, IDPDBLKNO, dev);
		idstrategy(bufhead);
		iowait(bufhead);
		if (bufhead->b_flags & B_ERROR) {
			errno = V_BADREAD;
			suword (&args->retval,errno);
			goto ioctldone;
		}
		if (copyout(bufhead->b_un.b_addr, karg.memaddr, idsect0[unit].pdinfo.bytes) != 0) {
			errno = V_BADREAD;
			suword (&args->retval,errno);
			goto ioctldone;
		}
		break;

	case V_PDWRITE:
		bufhead = geteblk();
		idsetblk (bufhead, B_WRITE, IDPDBLKNO, dev);
		if (copyin (karg.memaddr, bufhead->b_un.b_addr, idsect0[unit].pdinfo.bytes) != 0) {
			errno = V_BADWRITE;
			suword(&args->retval, errno);
			goto ioctldone;
		}
		idstrategy(bufhead);
		iowait(bufhead);
		if (bufhead->b_flags & B_ERROR) {
			errno = V_BADWRITE;
			suword(&args->retval, errno);
			goto ioctldone;
		}
		break;

	case V_GETSSZ:
		suword(karg.memaddr, idsect0[unit].pdinfo.bytes);
		return;

	default:
		u.u_error = EIO;
		return;
	}

ioctldone:
	brelse(bufhead);
}
