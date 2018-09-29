static char	sccsid[] = "@(#)VAXgt.c	1.2";

/*
 * TM03/TM78 tape driver
 * Handles two tape controllers on one massbus adapter
 * with two slave transports on each controller.
 * minor device classes:
 * bit 0: slave select
 * bit 1: controller select
 * bit 2 off: rewind on close; one: position after first TM
 * If TM78:
 * bit 3 off: 1600 bpi; on: 6250 bpi
 * If TM03:
 * bit 3 off: 800 bpi; on: 1600 bpi
 */

#include "stand.h"

struct	device {
	int	gtcs1, gtds1, gter1, gtmr1;
	int	gtas, gtfc, gtdt, gtds2;
	int	gtsn, gtmr2, gtmr3, gtndc;
	int	gtndt[4];
	int	gtia, gtid;
};
#define	gtdtc	gtds1	/* TM78 */
#define	gtfm	gter1	/* TM78 */
#define	gtds78	gtds2	/* TM78 */
#define	gtds03	gtds1	/* TM03 */
#define	gttc	gtmr2	/* TM03 */

#define	gtslave(X)	(X&01)
#define	gtdrive(X)	((X>>1)&01)
#define	gtpos(X)	(X&03)
#define	NOREW	04
#define	DENS	010
#define	GTTM03	050
#define	GTTM78	0100

#define	GO	01
#define	NOP	0
#define	REW	06
#define	DCLR	010
#define	SENSE	010
#define	WEOF	026
#define	SFORW	030
#define	SREV	032
#define	SFORW78	020
#define	SREV78	022
#define	SFFILE	024
#define	SRFILE	026
#define	ERASE	024
#define	CLFILE	040
#define	CLFGCR	042
#define	SFLEOT	046
#define	WCOM	060
#define	WGCR	062
#define	RCOM	070
#define	XSNS	072

#define	TM	04
#define	CS	02000
#define	COR	0100000
#define	PES	040
#define	ERR	040000
#define	FCE	01000
#define	HARD	064027
#define	MOL	010000
#define ONL	0x2000
#define	PE	0x0800
#define	BOT	0x0400
#define	EOT03	02000
#define EOT78	0x0200
#define	WRL	04000
#define FPT	0x0100

#define	OXCLD	01
#define	OWRIT	02
#define	OLEOT	04
#define	OFATAL	0200

#define	CDONE	01
#define	CTM	02
#define	CEOT	04
#define	CLEOT	05
#define	CREW	07
#define	CFPT	010
#define	CNRDY	011
#define	CNOC	015
#define	CLOREC	020
#define	CSHREC	021
#define	CRETRY	022
#define	CTMFLTB	032
#define	CMBFLT	034
#define	CKEYFLT	077

#define	TMCLR	0x4000

struct mba_regs {
	int mba_csr,
	    mba_cr,
	    mba_sr,
	    mba_var,
	    mba_bcr;
};

#define MBA_MAP 0x800
#define MBA_ERB 0x400

int gt_addr = 0x20012000;	/* mba at nexus 9 on 11/780 */

#define NUNIT	4

static daddr_t	_eof[NUNIT];
short	gttype[NUNIT];

#define GO	01
#define REW	06
#define DCLR	010
#define ERASE	024
#define WEOF	026
#define SFORW	030
#define SREV	032
#define WCOM	060
#define RCOM	070
#define TRE	040000

#define P800	01300		/*  800 + pdp11 mode */
#define P1600	02300		/* 1600 + pdp11 mode */

#define TM	04
#define TM78	04
#define DRY	0200
#define RDY	0x8000		/* TM78 drive ready bit */
#define MOL	010000

#define INIT	01

#define DTABT	0x1000
#define DTCMP	0x2000		/* data transfer complete */
#define INTC	0x3f		/* mask for DT interrupt code */

_gtclose (io)
register struct iob *io; {
	register flag, unit, tm03;
	register struct device *gtaddr;
	register struct mba_regs *mp;
	register dev;

	flag = io->i_flgs;
	dev = io->i_dp->dt_unit;
	unit = gtpos (dev);
	tm03 = (gttype[unit] == GTTM03);

	mp = (struct mba_regs *) gt_addr;
	gtaddr = (struct device *)(((int *) (gt_addr + 0x400) + 32 * unit));
	gtaddr->gtfc = 0;
	if (flag & F_WRITE) {
		if (tm03) {
			_gtstrategy (io, WEOF);
			_gtstrategy (io, WEOF);
		} else
			_gtstrategy (io, (dev & DENS) ? CLFGCR : CLFILE);
	}
	if (dev & NOREW) {
		if (flag & F_WRITE) {
			if (tm03)
				_gtstrategy (io, SREV);
		} else {
			if (!_eof[unit])
				_gtstrategy (io, tm03 ? SFORW : SFLEOT);
		}
	} else {
		_gtstrategy (io, REW);
	}
	_eof[unit] = 0;
}

_gtstrategy (io, func)
register struct iob *io; {
	register com, unit, tm03;
	int errcnt = 0;
	int den, dev;
	int ret;
	register struct device *gtaddr;
	register struct mba_regs *mp;
	register i, *mbap, frame;

	dev = io->i_dp->dt_unit;
	unit = gtpos (dev);
	mp = (struct mba_regs *) gt_addr;
	gtaddr = (struct device *) (((int *) (gt_addr + 0x400) + 32 * unit));

	if (!gttype[unit])
		gttype[unit] = gtaddr->gtdt & 0770;
	tm03 = (gttype[unit] == GTTM03);

	gtaddr->gtas = gtaddr->gtas;
	mp->mba_sr = mp->mba_sr;
	if (tm03) {
		gtaddr->gtcs1 = DCLR | GO;
		if ((gtaddr->gtds03 & MOL) == 0) {
			_prs ("TE16 unit not ready\n");
			while ((gtaddr->gtds03 & MOL) == 0);
		}
	} else {
		gtaddr->gtndt[gtslave (unit)] = SENSE | GO;
		while((gtaddr->gtndc & INTC) != CDONE);
		if ((gtaddr->gtds78 & ONL) == 0)
			_prs ("TU78 unit not ready\n");
		while ((gtaddr->gtds78 & ONL) == 0);
	}

	if (func != READ && func != WRITE) {
		if (tm03) {
			gtaddr->gtfc = 0;
			gtaddr->gtcs1 = func | GO;
			while ((gtaddr->gtds03 & DRY) == 0);
		} else {
			gtaddr->gtndt[gtslave (unit)] = func | GO;
			while ((gtaddr->gtndc & INTC) != CDONE);
		}
		gtaddr->gtas = gtaddr->gtas;
		return;
	}

	if (tm03) {
		den = ((dev & 010) ? P1600 : P800) | unit;
		if ((gtaddr->gttc & 03777) != den)
			gtaddr->gttc = den;
		com = (func == READ) ? RCOM : WCOM;
	} else
		com = (func == READ) ? RCOM : ((dev & DENS) ? WGCR : WCOM);

	mbap = (int *) mp;
	mbap += MBA_MAP / 4;
	frame = ((int) io->i_ma) >> 9;
	for (i = ((io->i_cc + 511) >> 9) + 1; i > 0; i--)
		*mbap++ = 0x80000000 | frame++;
	*mbap = 0;	/* invalidate mba entry */
	_eof[unit] = 0;

	do {
		mp->mba_sr = mp->mba_sr;
		mp->mba_var = ((int) io->i_ma) & 0x1ff;
		mp->mba_bcr = -io->i_cc;
		gtaddr->gtas = gtaddr->gtas;
		if (tm03)
			gtaddr->gtfc = -io->i_cc;
		else {
			gtaddr->gtfc = io->i_cc;
			gtaddr->gtfm = (1<<2) | gtslave (unit);
		}
		gtaddr->gtcs1 = com | GO;
		while ((mp->mba_sr & DTCMP) == 0);
		if ((tm03 && (gtaddr->gtds03 & TM)) ||
		   (!tm03 && ((gtaddr->gtdtc & INTC) == CTM))) {
			_eof[unit]++;
			return 0;
		} else
			if (mp->mba_sr & DTABT)
				ret = -1;
		else
			return (gtaddr->gtfc & 0xffff) ?
				(gtaddr->gtfc & 0xffff) : (io->i_cc);

		mp->mba_sr = mp->mba_sr;
		gtaddr->gtas = gtaddr->gtas;
		if (tm03) {
			gtaddr->gtcs1 = DCLR | GO;
			gtaddr->gtfc = -1;
			while ((gtaddr->gtds03 & DRY) == 0);
			_gtstrategy (io, SREV);
			if (com == WCOM) {
				while ((gtaddr->gtds03 & DRY) == 0);
				_gtstrategy (io, ERASE);
			}
		}
	} while (++errcnt < 10);
	errno = EIO;
	return (-1);
}
