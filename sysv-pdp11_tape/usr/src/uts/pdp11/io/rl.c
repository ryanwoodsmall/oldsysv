/* @(#)rl.c	1.2 */
/*
 *  RL disk driver
 */

#include "sys/param.h"
#include "sys/types.h"
#include "sys/sysmacros.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/buf.h"
#include "sys/elog.h"
#include "sys/iobuf.h"
#include "sys/systm.h"

/*
 *	specify block sizes for rl01 and rl02 respectively.
 *	otherwise, the two are identical
 */

#define	NBLK1	10240
#define	NBLK2	20480
#define	RLCYLSZ	10240
#define	RLSECSZ	256
#define	NRL	4	/* number of rl's per controller */
#define	NBKTR	20	/* number of unix blocks per track */

/*
 *	define status codes
 */

#define	RL02		0200	/* bit 7 indicates an rl02 present if set */
#define	CRDY		0200	/* controller ready */
#define	OPI	       02000	/* operation incomplete */
#define	DE	      040000	/* drive error */
#define	CSERR	      036000	/* error masck for rlcs */
#define	ERETRY		  10	/* number of retries to attempt */
#define	VC	      010000	/* volume check */

/*
 *	define rl commands
 */

#define	GETSTAT	  04		/* getstat command for rlcs */
#define	SEEK	  06		/* seek command */
#define	RDHDR	 010		/* read header command */
#define	WCOM	 012		/* write command */
#define	RCOM	 014		/* read command */
#define	IENABLE	0100		/* enable interrupts (rlcs) */
#define	RESET	 013		/* rlda value during getstat - clear err reg */
#define	STAT	  03		/* rlda value during getstat - leave err reg */
#define	SEEKHI	  05		/* rlda during a seek command - move toward spindle */
#define	SEEKLO	  01		/* rlda during a seek command - away from spindle */

/*
 *	drive states
 */

#define	ERRST	0		/* need a drive error reset */
#define	LOCST	2		/* trying to find where heads are */
#define	R_W_ST	8		/* on the right track - perform command */

/*
 *	location of head status
 *	this is used to attempt to avoid spurious volume checks.
 *	it is only partially successful.
 *	a volume check occurs when the drive heads move away from
 *	the home position. therefore, it will always occur on an
 *	initial acces to a powered up drive.
 *	however, there is no way to determine this condition, since
 *	a drive may be powered down and powered up at any time.
 *	using UNKNOWN will avoid the volume check of the first access
 *	to a drive, since system startup.
 *	One might have thought that resetting the current track to
 *	UNKNOWN in the close might have solved this problem, but this
 *	would lose much of the advantages of knowing where the drive
 *	at all times, except during an actual error.
 *	Because the first access to the powered down/up drive will
 *	cause a volume check, this error will be reported as a
 *	recoverable error, but otherwise will function correctly.
 */
#define	UNKNOWN	(unsigned)-1	/* location of head is unknown */

/*
 *	define queuing information - only used for optimized version
 */
#define	DOOPT	1	/* enable multiple seeks if more than one drive */

#ifdef	DOOPT
#define	IOPRE	10	/* number of times an IO buffer can be preempted */
#define	trk	av_back	/* space to save the track information per buffer */
#define	actdn	io_s2	/* specifiy the active drive number */
#endif
/*
 *	device information
 */
struct device
{
	short rlcs;	/* control and status reg */
	short rlba;	/* bus address */
	short rlda;	/* disk address */
	short rlmp;	/* multpurpose register, word count and status */
};

struct	device	*rl_addr[];
short	rl_cnt;
/*
 *	x  is used to quicky sequence through the drives
 *	with the initial drive being an arbitrary drive
 *	number.
 */

#ifdef	DOOPT
static char x[8] = {0,1,2,3,0,1,2,3};
#endif

/*
 *	drive information kept for each active drive.
 *	optimally, this should only inlude the number of drives actually
 *	configured on the system. however, it is the max. number per
 *	controller.
 */
struct 	rlstruc
{
	int	nblk;		/* number of blocks - 10240 or 20480 */
	unsigned short	curtrk;		/* location of head - cyl + surface */
	unsigned bleft;		/* bytes left to be transferred */
	unsigned bpart;		/* number of bytes transferred */
#ifdef	DOOPT
	int direc;		/* direction of head motion. 0 is descending */
#endif
	char	drivestate;	/* drive state - c.d. rowles */
}	rl[NRL];

struct	iotime	rlstat[NRL];

struct	iobuf	rltab =	tabinit(RL0,0);
#ifdef	DOOPT
#define	sa(X)	tabinit(RL0,&rlstat[X].ios)
struct	iobuf	rlutab[NRL] = { sa(0), sa(1), sa(2), sa(3) } ;
#endif


/*
 *	the argument dev is assumed to be the minor device number
 *	if this should change in the future, all references to dev
 *	just should be changed to minor(dev)&03
 */

rlopen(dev)
{
	struct iobuf *dp;
	register struct device *rp;
	register i;

	if(dev>=rl_cnt) {
		u.u_error = ENXIO;
		return;
	}
	rp = rl_addr[dev>>2];
/*
 *	once only operations
 *	do ubmap allocation (only applies if a 70)
 *	ALSO, on first open, initialize all the drives to the number
 *	of blocks - 10240 for rl01, 20480 for rl02.
 *	all drives are initialized to the size for the root device.
 *	this is done because it is impossible to guarantee the
 *	controller being idle on an arbitrary open.
 */
	if ((rltab.b_flags&B_ONCE)==0) {
		rltab.b_flags |= B_ONCE;
#ifdef pdp11
		rltab.io_s1 = ubmalloc(0-1);
#else
		rltab.io_s1 = ubmalloc(ctob(125),1);
#endif
		spl5();
		for(i=0; i<NRL; i++) {
			rp->rlda = STAT;
			rp->rlcs = ((i&03)<<8) | GETSTAT;
			while ((rp->rlcs & CRDY) == 0)
				;
			rl[i].nblk = (rp->rlmp & RL02) ? NBLK2 : NBLK1;
			rl[i].curtrk = UNKNOWN;
			rl[i].drivestate = ERRST;
		}
		spl0();
	}
#ifdef	DOOPT
	dp = &rlutab[dev];
#else
	dp = &rltab;
#endif
	dp->io_addr = (physadr)rl_addr[dev>>2];
/*
 *	NOTE: macro NDEVREGS has been replaced by explicit statement.
 *	all rl registers are 16 bit only.
 */

	dp->io_nreg = (sizeof(struct device)/sizeof(short));
}

rlclose(dev)
{
}

rlstrategy(bp)
register struct buf *bp;
{
	struct	iobuf	*dp;
#ifdef	DOOPT
	register struct	buf	*ioq1, *ioq2;
	short t;
	short dn = minor(bp->b_dev)&03;
	struct rlstruc *rlp = &rl[dn];
#else
	struct rlstruc *rlp = &rl[minor(bp->b_dev)&03];
#endif

	if(bp->b_blkno >= rlp->nblk) {
		if(bp->b_blkno == rlp->nblk && bp->b_flags&B_READ) {
			bp->b_resid = bp->b_bcount;
		} else {
			bp->b_flags |= B_ERROR;
			bp->b_error = ENXIO;
		}
		iodone(bp);
		return;
	}
	bp->b_start = lbolt;
#ifdef	DOOPT
	bp->b_error = IOPRE;	/* set up initial preempt value */
	bp->trk = (struct buf *)(bp->b_blkno/NBKTR);	/* calculate track no. of bp */
	dp = &rlutab[dn];
	spl5();
	if ( ( ioq1 = dp->b_actf) == NULL ) {
		dp->b_actf = bp;
		bp->av_forw = NULL;
		dp->b_actl = bp;
	} else {
		ioq2 = ioq1->av_forw;
		while (ioq2) {
			if ( !ioq2->b_error)
				ioq1 = ioq2;	/* a preempted bp. start */
						/* search here */
			ioq2=ioq2->av_forw;
		}
		t = rlp->direc;
		for(; ioq2 = ioq1->av_forw; ioq1=ioq2)
			if ( ioq1->trk <= bp->trk
			    && bp->trk <= ioq2->trk
			    || ioq1->trk >= bp->trk
			    && bp->trk >= ioq2->trk ) {
				break;		/* found spot -- exit */
			} else {
				if ( t ) {	/* current direction ascend */
					if (ioq2->trk < ioq1->trk)
						if (bp->trk > ioq1->trk)
							break;
						else
							t=0;
				} else {	/* current direction descend */
					if (ioq2->trk > ioq1->trk)
						if (bp->trk < ioq1->trk)
							break;
						else
							t++;
				}
			}
		if ( ( bp->av_forw = ioq2 ) == NULL )
			dp->b_actl = bp;
		ioq1->av_forw = bp;
		while ( ioq2 ) {
			if ( ioq2->b_error )
				ioq2->b_error--;
			ioq2=ioq2->av_forw;
		}
	}
#else
	dp = &rltab;
	spl5();
	bp->av_forw = NULL;
	if ( dp->b_actf == NULL )
		dp->b_actf = bp;
	else
		dp->b_actl->av_forw = bp;
	dp->b_actl = bp;
#endif

	if (rltab.b_active == 0 ) {
#ifdef	DOOPT
		rltab.actdn = dn;
#endif
		rlstart();
	}
	spl0();
}

rlstart()
{

	register struct device *rp;
	struct iobuf *dp;
	register struct buf *bp;
	register struct rlstruc *rlp;
	short dn;
#ifdef	DOOPT
	short t, i;
	short sect[NRL];
#else
	short sect;
#endif
	short trk;
	paddr_t addr;
	short cmd;
	unsigned short seekaddr;
	short startblk;
#ifdef	DOOPT
	t = rltab.actdn;
/*
 *	start looking at the active drive -
 *	if it's in locst, have to read the header info.
 *	else want to set any drives seeking that need seeking
 *	NOTE: on the optimized driver, loop is performed for
 *	NRL drives, even if fewer are configured.
 */
	for (i=0; i<=3; i++) {	/* see def of x[] above */
		dn= (short)x[i+t];
		dp = &rlutab[dn];
#else
		dp = &rltab;
#endif
		if ( !(bp = dp->b_actf) )
#ifdef	DOOPT
			continue;
#else
			return;
		dn = minor(bp->b_dev)&03;
#endif
		if ( dp->b_active == 0 )
			dp->io_start = lbolt;
		dp->b_active++;
		rp = rl_addr[dn>>2];
		rlp = &rl[dn];
#ifdef	DOOPT
		bp->b_error = 0;	/* insure preempt value is zeroed */
#endif
		switch (rlp->drivestate) {
		case LOCST:
			rlp->curtrk = rp->rlmp & 0177700;
			/*
			 *	must read 2 scratch words of silo. are ignored.
			 */
			startblk = rp->rlmp;
			startblk = rp->rlmp;
			rlp->drivestate = R_W_ST;
		case R_W_ST:
			startblk = (bp->b_blkno + (rlp->bpart>>9));
			trk = startblk/NBKTR;
#ifdef	DOOPT
			sect[dn] = startblk%NBKTR;
#else
			sect = startblk%NBKTR;
#endif
			if ((seekaddr = trk<<6) != rlp->curtrk) {
				if (seekaddr > rlp->curtrk) {
#ifdef	DOOPT
					rlp->direc = 1;
#endif
					trk = ((seekaddr & 0177600)-(rlp->curtrk & 0177600)) | SEEKHI;
				} else {
#ifdef	DOOPT
					rlp->direc = 0;
#endif
					trk = ((rlp->curtrk & 0177600)-(seekaddr & 0177600)) | SEEKLO;
				}
				rp->rlda = trk | ((seekaddr &0100) >>2);
				rlp->curtrk = seekaddr;
				rlstat[dn].ios.io_misc++;
				rp->rlcs = SEEK | (dn << 8);
				/*
				 * since it takes so long to field an
				 * interrupt, and the seek interrupts
				 * immediately, just wait for it, to
				 * start the overlapping.
				 */
				while ((rp->rlcs&CRDY) == 0)
					;
			}
			break;
		case ERRST:
			rp->rlda = RESET;
			rlstat[dn].ios.io_misc++;
			rp->rlcs = (dn<<8) | GETSTAT;
			while((rp->rlcs&CRDY) == 0)
				;
			blkacty |= (1<<RL0);
#ifdef	DOOPT
			rltab.actdn = dn;
#endif
			rltab.b_active++;
			rlstat[dn].ios.io_misc++;
			rlp->drivestate = LOCST;
			rp->rlcs = RDHDR | IENABLE | dn<<8;
			return;
		}
#ifdef	DOOPT
	}
#endif
#ifdef	DOOPT
/*
 *	for the optimizing driver:
 *	this section is reached after all drives have been set seeking.
 *	the scheduling is strictly round robin, since there is no
 *	fast way of determining which seek will complete
 *	first.
 *	for the non-optimizing driver, this section is done when it
 *	is determined that the bp at head of queue is ready for I/O
 */
	for ( i=0; i<=3; i++) {	/* skip current active drive */
					/* to initiate the round robin */
		dn= (short)x[i+t+1];
		dp = &rlutab[dn];
		rlp = &rl[dn];
		if ( !(bp=dp->b_actf) || (rlp->drivestate != R_W_ST) )
			continue;
		/* found a drive ready to go */
		addr=ubmaddr(bp,rltab.io_s1);
		addr += rlp->bpart;
		if((rlp->bleft = (NBKTR-sect[dn])<<9) >
			(startblk = (bp->b_bcount)-rlp->bpart))
				rlp->bleft = startblk;
		if(bp->b_flags & B_READ)
			cmd = RCOM;
		else
			cmd = WCOM;
		rp->rlba = loword(addr);
		rp->rlmp = -(rlp->bleft >> 1);
			/* rlmp is a word counter */
		rp->rlda = rlp->curtrk | (sect[dn] << 1);
		blkacty |= (1<<RL0);
		rltab.actdn = dn;
		rltab.b_active++;
		rlstat[dn].io_cnt++;
		rlstat[dn].io_bcnt += btoc(bp->b_bcount);
		rp->rlcs = cmd | (hiword(addr) << 4) | IENABLE | (dn << 8);
		return;
	}
#else
		/* found a drive ready to go */
		addr=ubmaddr(bp,rltab.io_s1);
		addr += rlp->bpart;
		if((rlp->bleft = (NBKTR-sect)<<9) >
			(startblk = (bp->b_bcount)-rlp->bpart))
				rlp->bleft = startblk;
		if(bp->b_flags & B_READ)
			cmd = RCOM;
		else
			cmd = WCOM;
		rp->rlba = loword(addr);
		rp->rlmp = -(rlp->bleft >> 1);
			/* rlmp is a word counter */
		rp->rlda = rlp->curtrk | (sect << 1);
		blkacty |= (1<<RL0);
		rltab.b_active++;
		rlstat[dn].io_cnt++;
		rlstat[dn].io_bcnt += btoc(bp->b_bcount);
		rp->rlcs = cmd | (hiword(addr) << 4) | IENABLE | (dn << 8);
		return;
#endif
}

rlintr()
{
	register struct buf *bp;
	register struct iobuf *dp;
	register struct device *rp;
	struct rlstruc *rlp;
	short status;
	short dn;
	extern rlprint();

#ifdef	DOOPT
	dn = rltab.actdn;	/* pick up the active device */
	dp = &rlutab[dn];	/* get the associated iobuf */
	bp = dp->b_actf;
#else
	dp = &rltab;
	bp = dp->b_actf;
	dn = minor(bp->b_dev)&03;
#endif
	rlp = &rl[dn];
	rp = rl_addr[dn>>2];
	if (rltab.b_active == NULL) {
		logstray(rp);
		return;
	}
	blkacty &= ~(1<<RL0);
	if (rp->rlcs < 0) {		/* error bit */
		if (rp->rlcs & CSERR) {
#ifndef	DOOPT
			dp->io_stp = &rlstat[dn].ios;
#endif
			fmtberr(dp,0);
			if(dp->b_errcnt > 2)
				prcom(rlprint, bp, (unsigned) rp->rlcs, (unsigned) rp->rlda);
		}
		if (rp->rlcs & DE) {
			rp->rlda = STAT;
			rlstat[dn].ios.io_misc++;
			rp->rlcs = (dn << 8) | GETSTAT;
			while ((rp->rlcs & CRDY) == 0)
				;
			status = rp->rlmp;
			if (!((rlp->curtrk == UNKNOWN) && (status & VC)) ) {
#ifndef	DOOPT
				dp->io_stp = &rlstat[dn].ios;
#endif
				fmtberr(dp,0);
				if(dp->b_errcnt > 2)
					prcom(rlprint, bp, (unsigned) status, (unsigned) rp->rlda);
			}
		}
		rlp->drivestate = ERRST;
		if (++dp->b_errcnt <= ERETRY) {
			rlstart();
			return;
		}
		bp->b_flags |= B_ERROR;
	} else {
		if ( rlp->drivestate == R_W_ST)
			rlp->bpart += rlp->bleft;
	}

	if ( dp->b_errcnt > ERETRY || ((rlp->drivestate == R_W_ST) &&
		( rlp->bpart - bp->b_bcount == 0 ))) {
			if(dp->io_erec) {
				logberr(dp,bp->b_flags&B_ERROR);
			}
			bp->b_resid = bp->b_bcount - rlp->bpart;
			rlp->bpart = 0;
			dp->b_errcnt = 0;
			dp->b_actf = bp->av_forw;
			rltab.b_active = 0;
			dp->b_active = 0;
			rlstat[dn].io_resp += lbolt - bp->b_start;
			rlstat[dn].io_act += lbolt - dp->io_start;
			iodone(bp);
	}
	rlstart();
}

rlread(dev)
{

	if (physck((daddr_t)rl[dev].nblk, B_READ))
		physio(rlstrategy, 0, dev, B_READ);
}

rlwrite(dev)
{

	if (physck((daddr_t)rl[dev].nblk, B_WRITE))
		physio(rlstrategy, 0, dev, B_WRITE);
}

rlclr()
{
	rltab.b_active = 0;
	rlstart();
}

rlprint(dev, str)
char *str;
{
	printf("%s on RL01/2 drive %d\n", str, dev&03);
}
