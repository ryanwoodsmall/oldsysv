/* @(#)hudump.c	6.1 */
/*
 * Dump core to magtape
 * Assumes memory mapping has been disabled
 * and IPL is set high
 */

struct mba_regs {
	int	csr, cr, sr, var, bcr;
};
struct device {
	int	csr, dtc, fm, mr1;
	int	as, fc, dt, ds;
	int	sn, mr2, mr3, ndc;
	int	ndt[4];
};

extern dump_addr;
static phys_addr;
static struct device *taddr;

#define	NBLK	16

#define	DTABT	0x1000
#define	DTCMP	0x2000
#define	GO	01
#define	WCOM	060
#define	CLFILE	040
#define	REW	06

hudump()
{
	extern physmem;

	printf("\nTape Dump\n");
	printf("%d memory blocks\n%d blocks per record\n", physmem, NBLK);
	printf("1600 bpi\n");
	phys_addr = 0x20000000 + 0x2000*dump_addr;
	taddr = (struct device *)(phys_addr + 0x400);
	twall((char *)0, physmem);	/* write out memory */
	tcom(CLFILE);
	tcom(REW);
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

	pte = (((int)buf)>>9) | 0x80000000;
	mp = (int *)(phys_addr + 0x800);
	for (i=0; i<NBLK; i++)
		*mp++ = pte++;
	cp = (struct mba_regs *)(phys_addr);
	for (i=0; i<10; i++) {
		cp->sr = -1;
		cp->bcr = -512*NBLK;
		rp->fm = (1<<2);
		rp->fc = 512*NBLK;
		cp->var = 0;
		rp->csr = WCOM | GO;
		while (!(cp->sr&DTCMP));
		if (!(cp->sr&DTABT)) {
			cp->sr = cp->sr;
			return;
		}
		cp->sr = cp->sr;
	}
	printf("err %x %x\n", buf, rp->dtc);
}

static
tcom(cmd)
{
	register struct device *rp = taddr;

	rp->ndt[0] = cmd | GO;
}
