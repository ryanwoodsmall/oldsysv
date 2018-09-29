/* @(#)gtdump.c	6.3 */
/*
 * Dump core to magtape on either TM03 (TE16) or TM78 (TU78)
 * Assumes memory mapping has been disabled
 * and IPL is set high
 */

extern short cputype;

struct mba_regs {
	int	csr, cr, sr, var, bcr;
};

struct device {
	int	cs1, ds1, er1, mr1;
	int	as, fc, dt, ds2;
	int	sn, mr2, mr3, ndc;
	int	ndt[4];
	int	ia, id;
};
#define	dtc	ds1	/* TM78 */
#define	fm	er1	/* TM78 */
#define	ds78	ds2	/* TM78 */
#define	ds03	ds1	/* TM03 */
#define	tc	mr2	/* TM03 */

#define	gtslave(X)	(X & 01)
#define	gtdrive(X)	((X >> 1) & 01)

extern dump_addr;
extern dumpdev;
static phys_addr;
static struct device *taddr;
static tm03;

#define	NBLK	16

#define	DTABT	0x1000
#define	DTCMP	0x2000
#define	INTC	0x3f
#define	CDONE	01
#define	GO	01
#define	WCOM	060
#define	CFILE	040
#define	WEOF	026
#define	SREV	032
#define	ERASE	024
#define	REW	06
#define	DCLR	010
#define	P800	01300		/* 800 + pdp11 mode */
#define	P1600	02300		/* 1600 + pdp11 mode */
#define	DRY	0200
#define	EOT	02000
#define	PES	040
#define	CORC	0x8400

#define	GTTM03	050
#define	GTTM78	0100

#define MBAINIT 1

gtdump()
{
	extern physmem;

	phys_addr = (cputype == 780) ? 0x20000000 + 0x2000 * dump_addr : 0xf28000 + 0x2000 * (dump_addr - 8);
	taddr = (struct device *)(phys_addr + 0x400 + gtdrive(dumpdev) * 0x80);
	tm03 = ((taddr->dt) & 0770) == GTTM03;

	printf("\nTape Dump on %s\n", tm03 ? "TU16" : "TU78");
	printf("%d memory blocks\n%d blocks per record\n", physmem, NBLK);
	printf("1600 bpi\n");
	if (tm03)
		taddr->tc = P1600;
	twall((char *)0, physmem);	/* write out memory */
	if (tm03) {
		tcom(WEOF);
		tcom(WEOF);
	} else
		tcom(CFILE);
	tcom(REW);
	((struct mba_regs *)phys_addr)->cr = MBAINIT;
}

static
twall(start, num)
register char *start;
register num;
{
	while (num) {
		twrite(start);
		start += 512*NBLK;
		num -= NBLK;
	}
}

static
twrite(buf)
register char *buf;
{
	register struct device *rp = taddr;
	register struct mba_regs *cp;
	register i, pte, *mp;
	int	err;

	pte = (((int)buf)>>9) | 0x80000000;
	mp = (int *)(phys_addr + 0x800);
	for (i = 0; i < NBLK; i++)
		*mp++ = pte++;
	cp = (struct mba_regs *)(phys_addr);

	if (!tm03)
		tinit78();

	for (i = 0; i < 10; i++) {
		cp->sr = -1;
		cp->bcr = -512 * NBLK;
		rp->as = rp->as;
		if (tm03) {
			rp->tc = P1600 | gtslave(dumpdev);
			rp->fc = -512 * NBLK;
		} else {
			rp->fc = 512 * NBLK;
			rp->fm = (1 << 2) | gtslave(dumpdev);
		}
		cp->var = 0;
		rp->cs1 = WCOM | GO;
		if (tm03) {
			while (!(rp->ds1 & DRY));
			err = rp->er1 & 0xffff;
			if (rp->ds1 & PES)
				err &= ~CORC;
			if (err == 0)
				cp->sr = DTABT;
			rp->cs1 = DCLR | GO;
			if (!(cp->sr & DTABT)) {
				cp->sr = cp->sr;
				return;
			}
			cp->sr = cp->sr;
			rp->fc = -1;
			rp->cs1 = SREV | GO;
			if (i & 02)
				tcom(ERASE);
			while (!(rp->ds1 & DRY));
		} else {
			while ((cp->sr & DTCMP) == 0);
			if (!(cp->sr & DTABT)) {
				cp->sr = cp->sr;
				return;
			}
		}
	}
	printf("err %x %x\n", buf, err);
}

static
tcom(cmd)
{
	register struct device *rp = taddr;

	if (tm03) {
		while (!(rp->ds1 & DRY));
		rp->cs1 = cmd | GO;
	} else {
		tinit78();
		rp->ndt[gtslave(dumpdev)] = cmd | GO;
	}
}

	/* Initialize or clear TU78 drive */
static
tinit78 ()
{
	register struct device *rp = taddr;
	register struct mba_regs *cp;

	cp = (struct mba_regs *)(phys_addr);

	rp->as = rp->as;
	cp->sr = cp->sr;
	rp->ndt[gtslave(dumpdev)] = DCLR | GO;
	while ((rp->ndc & INTC) != CDONE);
}
