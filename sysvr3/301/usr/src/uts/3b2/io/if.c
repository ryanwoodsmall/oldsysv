/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/if.c	10.12"
/* 
 *		Copyright 1984 AT&T
 *
 *		3B2 Computer UNIX Integral Floppy Disk Driver
 *
 */

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sbd.h"
#include "sys/dma.h"
#include "sys/csr.h"
#include "sys/iu.h"
#include "sys/immu.h"
#include "sys/dir.h"
#include "sys/sysmacros.h"
#include "sys/conf.h"
#include "sys/signal.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/region.h"
#include "sys/user.h"
#include "sys/proc.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/elog.h"
#include "sys/iobuf.h"
#include "sys/systm.h"
#include "sys/if.h"
#include "sys/cmn_err.h"
#include "sys/vtoc.h"
#include "sys/open.h"
#include "sys/file.h"

/*
 * The following definitions belong in sys/if.h; they
 * should be moved as soon as is practical.
 */
#define	IFSETTLE	4		/* iflag: awaiting write settle */
#define	IFCOPC		0xf0		/* Mask for command opcode */
#define	IFCMOD		0x0f		/* Mask for command modifiers */
#undef	IFMAXXFER			/* ...because if.h definition differs */
#define	IFMAXXFER	2		/* Transfer retries (hw does five) */

/*
 * Partitioning.
 */
struct	{
	daddr_t nblocks;	/* number of blocks in disk partition */
	int 	cyloff;		/* starting cylinder # of partition */
} if_sizes[8] = {
	990,   24,	/* partition 0 -	cyl 24-78 (root)      */
	810,   34,	/* partition 1 -	cyl 34-78 	      */
	612,   45,	/* partition 2 -	cyl 45-78 	      */
	414,   56,	/* partition 3 -	cyl 56-78 	      */
	216,   67,	/* partition 4 -	cyl 67-78 	      */
	1404,  1,	/* partition 5 -	cyl 1-78  (init)      */
	1422,  0,	/* partition 6 -	cyl 0-78  (full disk) */
	18,    0 	/* partition 7 -	cyl 0     (boot)      */
} ;

struct ifccmd ifccmd;

struct iobuf iftab;		/* drive and controller info */
struct iotime ifstat;		/* drive status info */

int	ifcsrset;		/* Drive motor is enabled */
int	ifslow;			/* Drive motor is not yet up to speed */
extern int sbdwcsr;

int ifspurint;		/* counter for spurious interrupts */
extern paddr_t if_addr[];	/* local bus addr of disk controller */



#define acts io_s1		/* space for driver save info */
#define cylin b_resid		/* bytes not transferred on error */
#define ccyl jrqsleep		/* process sleep counter on jrq full */


#define ifslice(x) (x&7)
#define ifformatdev(x) ((x>>3)&1)
#define ifnodev(x) ((x>>4)&1)

#define IFNODEV		0x10
#define IFPDBLKNO	1422
#define IF_PREAD	4
#define IF_PWRITE	5
#define IF_PDREAD	6
#define IF_PDWRITE	7

#define NULL 0
#define SET  1


extern int ifloppy;
#define IF ((struct ifdev *) &ifloppy)

extern int duart;
#define CONS ((struct duart *) &duart)

#define IFOQUIET 0x00
#define IFOWAIT  0x01
#define IFOGOOD 0x02
#define IFOBAD 0x03

int ifopenst;		/* flag for determining drive ready at open */
			/* with above defined flags	*/


int	iftimeo;	/* ifspindn() timeout ID (NULL when no timeout) */
int	ifisopen;	/* Drive open or awaiting end of I/O */
int	ifclosed;	/* Drive closed */
int iflag;		/* FLAG FOR COMMAND INTERRUPT INTERPRETATION */
int ifotyp[OTYPCNT];
int ifisroot;		/* is this floppy the root device */
unsigned int ifstate;	/* used for formatting-I/O contention */


int ifskcnt;		/* JOB RETRY COUNTERS */
int ifxfercnt;
int iflstdcnt;


extern char u400;
int ifsidesw;
int ifwdside;

struct ifsave {
	caddr_t b_addr;
	daddr_t b_blkno;
	unsigned int b_bcount;
}ifsvaddr;

struct  {
	unsigned char buf[512];
}ifcache[2];

unsigned int ifcacheaddr;
/*
 * The following guarantees alignment of fmat_buf.
 * Since this buffer is used for dma, this 
 * alignment may be necessary.
 */
asm(".align 8");
struct iftrkfmat fmat_buf;	/* a waste of memory */
struct ifformat kifmat;
struct io_arg kifargs;

ifcopy(faddr, taddr, count)
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

unsigned char ifalive;
unsigned char ifnoscan;

/*
 * ifscan()
 *
 * Check for lost interrupts.
 */
ifscan()
{
	int	s;

	s = spl6();
	if (iftab.b_actf != NULL)
		if (ifalive == NULL)
			ifflush();
		else
			ifalive = NULL;
	splx(s);
	timeout(ifscan, 0, 10*HZ);
}

ifsetblk(bufhead, cmd, blkno, dev)
struct buf *bufhead;
unsigned char cmd;
daddr_t blkno;
dev_t dev;
{
	clrbuf(bufhead);
	bufhead->b_flags |= cmd;
	bufhead->b_blkno = blkno;
	bufhead->b_dev = (dev|IFNODEV);
	bufhead->b_proc = 0x00;
	bufhead->b_flags &= ~B_DONE;
	if (cmd == B_WRITE)
		bufhead->b_bcount = 512;
}

/* FLOPPY - OPEN ROUTINE */
ifopen(dev, flag, otyp)
unsigned int	otyp;
{
	int iplsave;

	if (((minor(dev)) & 0x7f) >= 8) {
		u.u_error = ENXIO;
		return;
	}
	if (ifnoscan == SET) {
		ifnoscan = NULL;
		ifscan ();
	}

	iplsave = splhi();

	/* make sure the disk drive is ready */

	while (ifopenst != IFOQUIET){
		sleep(&ifopenst,PRIBIO);
	}
	if (ifisopen == NULL){
		ifopenst = IFOWAIT;
		CONS->scc_sopbc = F_LOCK;
		CONS->scc_sopbc = F_SEL;
		ifspinup(0);
		while (ifopenst == IFOWAIT){
			sleep (&ifopenst,PRIBIO);
		}
		if((flag & FWRITE) && (IF->statcmd & IFWRPT))
		{
			ifspindn(0);
			u.u_error = EROFS;
		}
		else
		if (ifopenst != IFOGOOD){
			ifspindn(0);
			u.u_error = ENXIO;
		} else
			ifisopen = SET;
		ifopenst = IFOQUIET;
		wakeup(&ifopenst);
	}
	if (ifisopen) {
		ifclosed = NULL;
		if (otyp == OTYP_LYR)
			++ifotyp[OTYP_LYR];
		else if (otyp < OTYPCNT)
			ifotyp[otyp] |= 1 << (dev & 0x7f);
	}
	splx(iplsave);
}

/* FLOPPY - CLOSE ROUTINE */
ifclose(dev, flag, otyp)
unsigned int	otyp;
{
	register int	i;
	register int	s;

	s = spl6();
	if (otyp == OTYP_LYR)
		--ifotyp[OTYP_LYR];
	else if (otyp < OTYPCNT)
		ifotyp[otyp] &= ~(1 << (dev & 0x7f));
	if (!ifisroot) {
		for (i = 0; i < OTYPCNT && ifotyp[i] == 0; ++i)
			;
		if (i == OTYPCNT)
			ifclosed = SET;
	}
	if (iftab.b_actf == NULL)
		ifspindn(0);
	splx(s);
}

/* FLOPPY INITIALIZATION OF POINTERS UPON KERNEL REQUEST */
ifinit()
{
	ifopenst = IFOQUIET;
	ifwdside=NULL;
	iftimeo = NULL;
	ifslow = NULL;
	ifstate=IFIDLEF;

	iftab.io_addr = (paddr_t)&ifloppy;
	iftab.io_start = NULL;
	iftab.b_actf = NULL;
	iftab.b_actl = NULL;
	iftab.qcnt = NULL;
	iftab.b_forw = NULL;
	iftab.b_forw = NULL;
	ifisopen = NULL;
	ifclosed = SET;
	ifnoscan = SET;
	ifalive = SET;
	/* assign physical address of temporary cache */
	ifcacheaddr = (unsigned int) vtop(&ifcache[0], 0);
	if (((ifcacheaddr&MSK64K)+0x200)>BND64K)
		ifcacheaddr = (unsigned int) vtop(&ifcache[1], 0);

}

ifstrategy(bp)
register struct buf *bp;
{
	register struct iobuf *dp;

	daddr_t lastblk;		/* last block in partition */
	int part;		/* partition number */
	int iplsave;		/* save interrupt priority */
	int ifbytecnt;

	part = ifslice(minor(bp->b_dev));
	lastblk = if_sizes[part].nblocks;
 	bp->cylin = bp->b_blkno/(IFNUMSECT*IFNTRAC)+if_sizes[part].cyloff;
	ifbytecnt = bp->b_bcount;

	if ((ifformatdev(minor(bp->b_dev)) == SET) || (ifnodev(minor(bp->b_dev)) == SET)){
		lastblk = (IFNUMSECT * IFTRACKS);
		ifbytecnt = (IFBYTESCT*IFNUMSECT);
	}

	dp = &iftab;

	/* CHECK FOR PARTITION OVERRUN I.E. BLOCK OUT OF BOUNDS */
	if ((bp->b_blkno<0)||
	(bp->b_blkno>=lastblk)||
	(bp->b_blkno+(ifbytecnt/IFBYTESCT))>lastblk){
		if ((bp->b_blkno == lastblk) && (bp->b_flags & B_READ))
			bp->b_resid = bp->b_bcount;
		else{
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
		}
		iodone(bp);	/* JOB TERMINATION */
		return;
	}


	iplsave = spl6();	/* save previous IPL */

	bp->b_start = lbolt;
	ifstat.io_cnt++;
	ifstat.io_bcnt += (bp->b_bcount + BSIZE-1) >> BSHIFT; /* inc disk block count */
	dp->qcnt++;

	bp->av_forw = NULL;		/* mark request as last on list */
	if (dp->b_actf == NULL){		/* if no request for drive */
		dp->b_actf = bp;	/* link to front of worklist */
		dp->b_actl = bp;
		dp->acts = (int)bp;
		ifalive = SET;
		ifspinup(0);
 	} else {				/* link to end of list */
 		register struct buf *ap, *cp;
 		if (((int)ifstat.io_cnt&0x0f) == 0)
 			dp->acts = (int) dp->b_actl;
 		for (ap = (struct buf*)dp->acts; cp = ap->av_forw; ap = cp) {
 			int s1, s2;
 			if ((s1 = ap->cylin - bp->cylin) < 0)
 				s1 = -s1;
 			if ((s2 = ap->cylin - cp->cylin) < 0)
 				s2 = -s2;
 			if (s1 < s2)
 				break;
 		}
 		ap->av_forw = bp;
 		if ((bp->av_forw = cp) == NULL)
 			dp->b_actl = bp;
 		bp->av_back = ap;
	}
	/* RETURN TO ORIGINAL IPL */
	splx(iplsave);



}	
/*
 *	Break up the request that came from physio into chunks of
 *	contiguous memory so we can get around the DMAC limitations
 *	We must be sure to pass at least 512 bytes (one sector) at a
 *	time (except for the last request).   */

ifbreakup(bp)
register struct buf *bp;
{
	dma_breakup(ifstrategy, bp);
}  /* end ifbreakup */

ifrest()	/* THIS FUNCTION IS SIGNIFICANT ONLY TO SEEK ERRORS */
{
	iflag = IFRESTORE;		/* INTERRUPT ON RESTORE FLAG */
	ifxfercnt = 0;
	IF->statcmd = (IFREST|IFSTEPRATE);	/* RESTORE HEADS TO TRACK 00 */
}

/*
 * ifspinup()
 *
 * Spin up the floppy. Waits two seconds for the
 * drive speed to stabilize.
 */
ifspinup(timed)
int		timed;
{
	register int	s;

	if (timed) {
		s = spl6();
		ifslow = NULL;
	}
	if (iftimeo) {
		untimeout(iftimeo);
		iftimeo = NULL;
	}
	if (ifcsrset) {
		if (ifslow)
			;
		else if (ifopenst == IFOWAIT)
			ifrest();
		else {
			ifsetup();
			ifseek();
		}
	} else {
		Wcsr->s_flop = SET;
		ifslow = ifcsrset = SET;
		timeout(ifspinup, 1, 2 * HZ);
	}
	if (timed)
		splx(s);
}

/*
 * ifspindn()
 *
 * Spin down the drive. Waits two seconds for additional
 * I/O before actually turning off the motor.
 */
ifspindn(timed)
int		timed;
{
	register int	i;
	register int	s;

	if (timed) {
		s = spl6();
		ifcsrset = iftimeo = NULL;
		Wcsr->c_flop = NULL;
		splx(s);
	} else {
		if (ifclosed) {
			ifisopen = NULL;
			CONS->scc_ropbc = F_LOCK;
			CONS->scc_ropbc = F_SEL;
		}
		if (ifcsrset && iftimeo == NULL)
			iftimeo = timeout(ifspindn, 1, 2 * HZ);
	}
}

/*
 * ifflush()
 *
 * Flush the first job on the queue.
 */
ifflush()
{
	register struct buf	*bp;
	register struct buf	*lp;

	iflag = IFNONACTIVE;
	ifstate &= ~(IFBUSYF|IFFMAT1);
	bp = iftab.b_actf;
	iftab.b_active = NULL;
	iftab.b_errcnt = NULL;
	iftab.b_actf = bp->av_forw;
	bp->b_resid = bp->b_bcount;
	bp->b_flags |= B_ERROR;
	iftab.qcnt--;
	if (bp == (struct buf *) iftab.acts)
		iftab.acts = (int) iftab.b_actf;
	if (iftab.b_actl == bp) {
		/*
		 * Should use av_back, but it is not set.
		 */
		if (lp = iftab.b_actf)
			while (lp->av_forw)
				lp = lp->av_forw;
		iftab.b_actl = lp;
	}
	iodone(bp);
	cmn_err(CE_WARN, "\nfloppy disk timeout; request flushed\n");
	if (iftab.b_actf)
		ifspinup(0);
	else
		ifspindn(0);
}

/* FILL COMMAND BUFFER WITH INFORMATION FROM THE BUFFER HEADER */
		/* AND COMPUTE DISK ACCESS ADDRESS */
ifsetup()
{
	register struct ifccmd *cp;
	register struct buf *bp;
	register struct iobuf *dp;
	register unsigned char *if_dmem, *if_umem;
	int bytes;
	int blkcnt;
	int partit,lstblk;
	int cyl;
	int sid;

	dp = &iftab;
	bp = dp->b_actf;
	cp = &ifccmd;

	ifskcnt = NULL;	/* RESET FLAGS FOR RETRIES */
	ifxfercnt = NULL;
	iflstdcnt = NULL;


	if (bp == NULL)	/* VERIFY THERE IS A JOB TO DO */
		return;
	if (dp->b_active == NULL){
		dp->io_start = lbolt;	/* TIME STAMP JOB */
		ifsvaddr.b_bcount = bp->b_bcount;	/* SAVE ADDRESS FOR 64K */
		ifsvaddr.b_blkno = bp->b_blkno;		/*      BOUNDS	        */
		ifsvaddr.b_addr = bp->b_un.b_addr;
	}

	dp->b_active = SET;		/* MARK DRIVE AS ACTIVE */

	if (ifformatdev(minor(bp->b_dev)) == SET){ /* FORMAT / VERIFY ? */
		cp->trknum = ifsvaddr.b_blkno/(IFNUMSECT*IFNTRAC);
		ifsidesw = ((ifsvaddr.b_blkno%(IFNUMSECT*2))/IFNUMSECT)*2;
		cp->baddr = vtop(ifsvaddr.b_addr,bp->b_proc); 
		if (cp->baddr == NULL)
			cmn_err(CE_PANIC,"\nfloppy disk Bad address returned from VTOP\n");
		if ((bp->b_flags & B_READ) == B_READ) {
			cp->c_opc = (IFRDS|IFSLENGRP1|IFMSDELAY|ifsidesw);
			cp->sectnum = (ifsvaddr.b_blkno%IFNUMSECT)+1;
			cp->bcnt = IFBYTESCT;
			ifsvaddr.b_bcount -= IFBYTESCT;
			ifsvaddr.b_blkno++;
			ifsvaddr.b_addr += IFBYTESCT;
		}
		else {
			cp->c_opc = (IFWRTRK|IFMSDELAY|ifsidesw);
			ifsvaddr.b_bcount -= bp->b_bcount;
			cp->bcnt = bp->b_bcount;
		}
	} else {
	    	partit = ifslice(minor(bp->b_dev));
	    	lstblk = if_sizes[partit].nblocks;

		/* find start cylinder in partition */
		if (ifnodev(minor(bp->b_dev)))
			cyl = 0x00;
		else
	    		cyl = if_sizes[partit].cyloff;	
		/* find cylinder offset in partition */
	    	bp->cylin = ifsvaddr.b_blkno/(IFNUMSECT*IFNTRAC)+cyl;
		/* compute sector offset into cylinder  */
	    	cp->sectnum = (ifsvaddr.b_blkno%IFNUMSECT)+1;
		/* compute side */
	    	ifsidesw = ((ifsvaddr.b_blkno%(IFNUMSECT*2))/IFNUMSECT)*2;	

		/* load command buffer */
	    	cp->trknum = bp->cylin;
	    	cp->c_bkcnt = blkcnt = 1;
	    	cp->bcnt = bytes = IFBYTESCT;
	    	cp->c_opc = ((bp->b_flags & B_READ) ? IFRDS : IFWTS)|IFSLENGRP1|IFMSDELAY;
	    	cp->c_opc |= ifsidesw; /* FOR HEAD SWITCH */
	    	cp->baddr = vtop(ifsvaddr.b_addr,bp->b_proc);
	    	if (cp->baddr==NULL) 
		    	cmn_err(CE_PANIC,"\nfloppy disk: Bad address returned by VTOP\n");
		/* crossing 128K-byte boundary */
		sid = secnum((int)ifsvaddr.b_addr);
		if ( (sid == 2) || (sid == 3)  )  {   /* if user address */
			if ((((long)ifsvaddr.b_addr&MSK_IDXSEG)+IFBYTESCT) 
			> (MSK_IDXSEG+1))  {
				ifbufsave(bp,cp);
			}
		}
			
		/* crossing 64K-byte boundary or partial sector transfer */
		if ((((cp->baddr&MSK64K) + IFBYTESCT) > BND64K) 
		|| (ifsvaddr.b_bcount < IFBYTESCT)) {
			ifbufsave(bp,cp);
		}

		/* keep track of byte count, and blk and mem addresses  */
	    	ifsvaddr.b_blkno += blkcnt;
	    	if (cp->baddr != ifcacheaddr) {
		    	ifsvaddr.b_addr += bytes;
			ifsvaddr.b_bcount -= bytes;	
		}
	}
}   

/* Routine for coping the user buffer to a driver buffer          */
/* Used for 64K-byte bounds, 128K-byte bounds and partial sectors */
ifbufsave(bufhead,cmdp)
struct buf *bufhead;
struct ifccmd *cmdp;
{
	register unsigned char *if_dmem, *if_umem;
	register unsigned int *zp;
	int i, bytes;
	
	bytes = cmdp->bcnt;
	if ((bufhead->b_flags&B_READ) != B_READ) {
		if (ifsvaddr.b_bcount<IFBYTESCT) {
			bytes = ifsvaddr.b_bcount;
			zp = (unsigned int *) ifcacheaddr;
			for(i=0; i<IFBYTESCT/4; i++)
				*zp++ = 0x00000000;
		}
		if_dmem = (unsigned char *) ifcacheaddr;
		if_umem = (unsigned char *) cmdp->baddr;
		for(i=0; i<bytes; i++)
			*if_dmem++ = *if_umem++;
	} 
	cmdp->baddr = ifcacheaddr;
}


/* ROUTINE FOR SEEKING TO DESIRED TRACK */
ifseek()
{
	register struct ifccmd *cp;
	int i;

	cp = &ifccmd;

	ifskcnt++;
	if (IF->track == cp->trknum){	/* CHECK FOR BEING ON TRACK */
		ifxfer();
		return;
	}
	iflag = IFSEEKATT;		/* INTERRUPT ON SEEK FLAG */
	IF->data = cp->trknum;		/* LOAD DESTINATION TRACK */
	if ((cp->c_opc & IFCOPC) == IFWRTRK)
		IF->statcmd = IFSEEK | IFSTEPRATE;
	else
		IF->statcmd = IFSEEK | IFSTEPRATE | IFVERIFY;
}

ifxfer()	/* DATA TRANSFER IS IMPLEMENTED */
{

	register struct ifccmd *cp;
	unsigned char cmd;
	int i;

	ifxfercnt++;
	cp = &ifccmd;
	iflag = IFXFER;		/* INTERRUPT ON TRANSFER FLAG */
	ifstate |= IFBUSYF;
	if ((ifstate & IFFMAT0) == IFFMAT0)
		ifstate |= IFFMAT1;

	if ((cp->c_opc & IFCOPC) == IFWTS || (cp->c_opc & IFCOPC) == IFWRTRK)
		cmd = RDMA;	/* SET DIRECTION FOR DMA */
	else
		cmd = WDMA;

	/* INITIALIZE DMA FOR TRANSFER */
	dma_access(CH1IFL,cp->baddr,cp->bcnt,SNGLMOD,cmd);
	if ((cp->c_opc & IFCOPC) != IFWRTRK)
		IF->sector = cp->sectnum;	/* LOAD CONTROLLER REGS */
	IF->statcmd = cp->c_opc;	/* WITH SECTOR NO. AND CMD */

}


/* INTERRUPT HANDLER - STATUS - COMMAND INFORMATION INTERPRETER */
ifint(dev)
{
	register struct buf *bp;
	register struct iobuf *dp;
	register struct ifccmd *cp;
	unsigned char dstat;
	register unsigned char *if_dmem, *if_umem;
	unsigned int i;
	int bytes;

	dstat = IF->statcmd;

	if (iflag == IFNONACTIVE) {
		++ifspurint;
		return;
	}

	ifalive = SET;

	if (ifopenst == IFOWAIT) {
		iflag = IFNONACTIVE;
		ifopenst = (dstat & (IFNRDY | IFSKERR)) == 0 ? IFOGOOD : IFOBAD;
		wakeup(&ifopenst);
		return;
	}

	dp = &iftab;
	if ((bp = dp->b_actf) == NULL)
		return;		/* ...should never happen */
	cp = &ifccmd;

	if (dstat & IFNRDY)
		goto diskerr;

	switch (iflag) {
	case IFRESTORE:
		iflag = IFNONACTIVE;
		iutime(12, ifseek);
		return;
	case IFSEEKATT:
		iflag = IFNONACTIVE;
		if ((dstat & IFSKERR) == 0)
			if ((cp->c_opc & IFCOPC) == IFWRTRK)
				iutime(12, ifxfer);
			else
				ifxfer();
		else if (ifskcnt > IFMAXSEEK)
			goto diskerr;
		else
			ifrest();
		return;
	case IFXFER:
		iflag = IFSETTLE;
		ifstate &= ~(IFBUSYF | IFFMAT1);
		if ((cp->c_opc & IFCOPC) == IFWTS)
			iutime(1, ifint);
		else
			ifint();
		return;
	case IFSETTLE:
		iflag = IFNONACTIVE;
		if (dstat & IFWRPT)
			goto diskerr;
		
		if ((dstat & IFCRCERR) && ifxfercnt <= IFMAXXFER) {
			ifxfer();
			return;
		}

		if (dstat & (IFCRCERR | IFRECNF)) {
			if (ifskcnt > IFMAXSEEK)
				goto diskerr;
			else
				ifrest();
			return;
		}

		if (dstat & IFLSTDATA) {
			iflstdcnt++;
			if (iflstdcnt <= IFMAXLSTD){
				ifxfer();
				return;
			}
			iflstdcnt = NULL;
			goto diskerr;
		}
		goto goodend;
		break;
	default:
		cmn_err(CE_PANIC, "ifint");
	}

diskerr:
	cmn_err(CE_NOTE,"\nFloppy Access Error: Consult the Error Message Section");
	cmn_err(CE_CONT,"of the System Administration Utilities Guide");
	cmn_err(CE_CONT,"\n");
	bp->b_flags |= B_ERROR;
	bp->b_error |= EIO;

goodend:
	/* if the data is in the temporary cache */
	if (cp->baddr == ifcacheaddr) {
		bytes = IFBYTESCT;
		if (ifsvaddr.b_bcount < IFBYTESCT)
			bytes = ifsvaddr.b_bcount;
		/* if read, copy out to user */
		if ((bp->b_flags&B_READ) == B_READ) {
			if_dmem = (unsigned char *) ifcacheaddr;
			if_umem = (unsigned char *) vtop(ifsvaddr.b_addr,bp->b_proc);
			for(i=0;i<bytes;i++)
				*if_umem++ = *if_dmem++;
		}
		/* update pointer to user address space */
		ifsvaddr.b_addr += bytes;
		ifsvaddr.b_bcount -= bytes;
	}

	/* if no errors and more to do, then go again */
	if (((bp->b_flags & B_ERROR)==0) && (ifsvaddr.b_bcount != 0)){
		ifsetup();
		ifseek();
		return;
	}

	dp->b_active = NULL;
	dp->b_errcnt = NULL;
	dp->b_actf = bp->av_forw;
	bp->b_resid = NULL;
	dp->qcnt--;

 	if (bp == (struct buf *) dp->acts)
 		dp->acts = (int) dp->b_actf;
	if (dp->b_actl == bp)
		dp->b_actl = NULL;

	ifstat.io_resp += lbolt - bp->b_start;
	ifstat.io_act += lbolt - dp->io_start;
	iodone(bp);

	if (dp->b_actf != NULL){
		ifsetup();
		ifseek();
		return;
	}
	ifspindn(0);
}

/* READ DEVICE ROUTINE */

ifread(dev)
{
	if (physck(if_sizes[minor(dev)&07].nblocks, B_READ))
		physio(ifbreakup, 0, dev, B_READ);
}

/* WRITE DEVICE ROUTINE */

ifwrite(dev)
{
	if (physck(if_sizes[minor(dev)&07].nblocks, B_WRITE))
		physio(ifbreakup, 0, dev, B_WRITE);
}

/* LOCAL PRINT ROUTINE */

ifprint(dev,str)
char *str;
{
	cmn_err(CE_NOTE,"%s on floppy drive, slice %d\n", str, dev&7);
}

ifidle()
{
	if (iftab.b_actf != NULL)
		return(1);
	return(0);
}
ifioctl(dev,cmd,arg,mode)
unsigned int dev, cmd, arg, mode;
{

	
	switch(cmd){
		case IFBCHECK:{

			struct ifformat ifmat;
			paddr_t ifbaddr;

			if (copyin((struct ifformat *)arg, &ifmat, sizeof(struct ifformat))) {
				u.u_error = EFAULT;
				return;
			}
			ifbaddr = vtop(ifmat.data, u.u_procp);
			if (ifbaddr == 0){
				cmn_err(CE_WARN,"\nfloppy disk: Bad address returned from VTOP\n");
				u.u_error = EFAULT;
				return;
			}
			if (((ifbaddr & MSK64K)+ifmat.size) > BND64K){
				u.u_error = EFAULT;
				return;
			}
			break;
		}
		case IFFORMAT:{

			register struct buf *bp;
			struct iftrkfmat *trkpt;
			struct ifformat *ifmat;
			int bpbcount;
			caddr_t bpbaddr;

			trkpt = (struct iftrkfmat *)arg;
			if (copyin(trkpt, &fmat_buf, sizeof(struct iftrkfmat)) != 0) {
				u.u_error = EFAULT;
				return;
			}
			bp = geteblk();
			bp->b_error = 0;
			bp->b_dev = (dev | IFPTN);
			bpbcount = bp->b_bcount;
			bpbaddr= bp->b_un.b_addr;
			bp->b_bcount = sizeof(struct iftrkfmat);
			bp->b_proc = u.u_procp;
			bp->b_un.b_addr =  ((caddr_t)&fmat_buf);
			bp->b_flags = (B_BUSY | B_WRITE);
			bp->b_blkno = ((fmat_buf.dsksct[0].TRACK*(IFNUMSECT*2))+(fmat_buf.dsksct[0].SIDE*IFNUMSECT));
			ifstate |= IFFMAT0;
			ifstrategy(bp);
			iowait(bp);
			bp->b_bcount = bpbcount;
			bp->b_un.b_addr = bpbaddr;
			brelse(bp);
			ifstate &= ~IFFMAT0;
			break;
		}
		case IFCONFIRM:{
			register struct buf *bp;
			struct iftrkfmat *trkpt;
			struct ifformat *ifmat;
			struct ifformat *argpt;
			int bpbcount;
			paddr_t ifbaddr;
				
			caddr_t bpbaddr;

			argpt = (struct ifformat *)arg;
			if (copyin(argpt, &kifmat, sizeof(struct ifformat))) {
				u.u_error = EFAULT;
				return;
			}
			trkpt = (struct iftrkfmat *)(kifmat.data);
			bp = geteblk();
			bp->b_error = 0;
			bp->b_flags = (B_BUSY | B_READ);
			bp->b_dev = (dev | IFPTN);
			bpbcount = bp->b_bcount;
			bpbaddr = bp->b_un.b_addr;
			bp->b_bcount = (IFNUMSECT*IFBYTESCT);
			bp->b_proc = u.u_procp;
			bp->b_un.b_addr = ((caddr_t) &fmat_buf);
			bp->b_blkno = ((kifmat.iftrack*(IFNUMSECT*2))+(kifmat.ifside*IFNUMSECT));
			ifstrategy(bp);
			iowait(bp);
			bp->b_bcount = bpbcount;
			bp->b_un.b_addr = bpbaddr;
			brelse(bp);
			if (copyout(&fmat_buf, trkpt, sizeof (struct iftrkfmat)) != 0) {
				u.u_error = EFAULT;
			}
			break;
			
		}
	case V_PREAD:{
		struct io_arg *ifargs;
		struct buf *geteblk();
		struct buf *bufhead;
		int errno, xfersz;
		unsigned int block, mem, count; 

		ifargs = (struct io_arg *)arg;
		if (copyin(ifargs, &kifargs, sizeof(struct io_arg))) {
			u.u_error = EFAULT;
			return;
		}
		block = kifargs.sectst;
		mem = kifargs.memaddr;
		count = kifargs.datasz;
		bufhead = geteblk();
		while (count) 	{
			ifsetblk (bufhead, B_READ, block, dev);
			ifstrategy(bufhead);
			iowait(bufhead);
			if (bufhead->b_flags & B_ERROR)	{
				errno = V_BADREAD;
				suword(&ifargs->retval,errno);
				goto preaddone;
			}
			xfersz = min(count,bufhead->b_bcount);
			if (copyout(bufhead->b_un.b_addr, mem, xfersz) != 0)	{
				errno = V_BADREAD;
				suword(&ifargs->retval,errno);
				goto preaddone;
			}
			block+=2;
			count -= xfersz;
			mem += xfersz;
		}
preaddone:
		bufhead->b_bcount = BSIZE;
		brelse(bufhead);
		break;
	}
	case V_PWRITE:{
		struct io_arg *ifargs;
		struct buf *geteblk();
		struct buf *bufhead;
		int errno, xfersz;
		unsigned int block, mem, count; 

		if (!suser())
			return;
		ifargs = (struct io_arg *)arg;
		if (copyin(ifargs, &kifargs, sizeof(struct io_arg))) {
			u.u_error = EFAULT;
			return;
		}
		block = kifargs.sectst;
		mem = kifargs.memaddr;
		count = kifargs.datasz;
		bufhead = geteblk();
		while (count) {
			ifsetblk(bufhead,B_WRITE,block,dev);
			xfersz = min(count,bufhead->b_bcount);
			if (copyin(mem,bufhead->b_un.b_addr,xfersz) != 0) {
				errno = V_BADWRITE;
				suword(&ifargs->retval,errno);
				goto pwritedone;
			}
			ifstrategy(bufhead);
			iowait(bufhead);
			if (bufhead->b_flags & B_ERROR)	{
				errno = V_BADWRITE;
				suword(&ifargs->retval,errno);
				goto pwritedone;
			}
			block +=1;
			count -= xfersz;
			mem += xfersz;
		}
pwritedone:
		bufhead->b_bcount = BSIZE;
		brelse(bufhead);
		break;
	}
	case V_PDREAD:{
		struct io_arg *ifargs;
		struct buf *geteblk();
		struct buf *bufhead;
		int errno, xfersz;
		unsigned int block, mem, count; 

		ifargs = (struct io_arg *)arg;
		if (copyin(ifargs, &kifargs, sizeof(struct io_arg))) {
			u.u_error = EFAULT;
			return;
		}
		bufhead = geteblk();
		ifsetblk (bufhead, B_READ, IFPDBLKNO, dev);
		ifstrategy(bufhead);
		iowait(bufhead);
		if (bufhead->b_flags & B_ERROR)	{
			errno = V_BADREAD;
			suword(&ifargs->retval,errno);
			goto pdrddone;
		}
		if (copyout(bufhead->b_un.b_addr, kifargs.memaddr, IFBYTESCT)  != 0)	{
			errno = V_BADREAD;
			suword(&ifargs->retval,errno);
			goto pdrddone;
		}
pdrddone:
		bufhead->b_bcount = BSIZE;
		brelse(bufhead);
		break;
	}
	case V_PDWRITE:{
		struct io_arg *ifargs;
		struct buf *geteblk();
		struct buf *bufhead;
		int errno;

		if (!suser())
			return;
		ifargs = (struct io_arg *)arg;
		if (copyin(ifargs, &kifargs, sizeof(struct io_arg))) {
			u.u_error = EFAULT;
			return;
		}
		bufhead = geteblk();
		ifsetblk(bufhead,B_WRITE,IFPDBLKNO,dev);
		if (copyin(kifargs.memaddr,bufhead->b_un.b_addr,IFBYTESCT) != 0) {
			errno = V_BADWRITE;
			suword(&ifargs->retval,errno);
			goto pdwrtdone;
		}
		ifstrategy(bufhead);
		iowait(bufhead);
		if (bufhead->b_flags & B_ERROR)	{
			errno = V_BADWRITE;
			suword(&ifargs->retval,errno);
			goto pdwrtdone;
		}
pdwrtdone:
		bufhead->b_bcount = BSIZE;
		brelse(bufhead);
		break;
	}
	default:
		u.u_error = EIO;
		return;
	}
}
