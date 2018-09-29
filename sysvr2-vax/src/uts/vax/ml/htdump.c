/* @(#)htdump.c	6.3 */
/*
 * Dump core to magtape
 * Assumes memory mapping has been disabled
 * and IPL is set high
 */

extern short cputype;

struct mba_regs {
	int	csr, cr, sr, var, bcr;
};
struct device {
	int	cs1, ds, er, mr;
	int	as, fc, dt, ck;
	int	sn, tc;
};

extern dump_addr;
static phys_addr;
static struct device *taddr;

#define	NBLK	16

#define	DTABT	0x1000
#define	GO	01
#define	WCOM	060
#define	WEOF	026
#define	SREV	032
#define	ERASE	024
#define	REW	06
#define	DCLR	010
#define P800	01300		/* 800 + pdp11 mode */
#define	P1600	02300		/* 1600 + pdp11 mode */
#define	DRY	0200
#define EOT	02000
#define PES	040
#define	CORC	0x8400

#define MBAINIT 1

htdump()
{
	extern physmem;

	printf("\nTape Dump\n");
	printf("%d memory blocks\n%d blocks per record\n", physmem, NBLK);
	printf("1600 bpi\n");
	phys_addr = (cputype == 780) ? 0x20000000 + 0x2000 * dump_addr : 0xf28000 + 0x2000 * (dump_addr - 8);
	taddr = (struct device *)(phys_addr + 0x400);
	taddr->tc = P1600;
	twall((char *)0, physmem);	/* write out memory */
	tcom(WEOF);
	tcom(WEOF);
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
	for (i=0; i<NBLK; i++)
		*mp++ = pte++;
	cp = (struct mba_regs *)(phys_addr);
	for (i=0; i<10; i++) {
		cp->sr = -1;
		cp->bcr = -512*NBLK;
		rp->fc = -512*NBLK;
		cp->var = 0;
		rp->cs1 = WCOM | GO;
		while (!(rp->ds & DRY));
		err = rp->er&0xffff;
		if (rp->ds&PES)
			err &= ~CORC;
		if (err == 0)
			cp->sr = DTABT;
		rp->cs1 = DCLR | GO;
		if (!(cp->sr&DTABT)) {
			cp->sr = cp->sr;
			return;
		}
		cp->sr = cp->sr;
		rp->fc = -1;
		rp->cs1 = SREV | GO;
		if (i&02)
			tcom(ERASE);
		while (!(rp->ds & DRY));
	}
	printf("err %x %x\n", buf, err);
}

static
tcom(cmd)
{
	register struct device *rp = taddr;

	while (!(rp->ds&DRY));
	rp->cs1 = cmd | GO;
}
