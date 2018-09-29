/* @(#)gddump.c	6.1 */
/* This is a dump to disk routine... changing the dump entry in the
	dfile to something like:
		dump	disk	05
	will cause the system dump device to be the disk, drive 0 / slice 5.
	until config handles this properly, gddump will always dump starting
	at block zero of the slice.  the offset may be kludged using
	the block offset "dumplo" (which would normally be set by config).
	Also the mba address for the comet is fixed, and some magic should
	be done to calculate it like the 780.
				any problems, let me know    ... JJP
*/
#include <sys/types.h>
#include <sys/gdisk.h>
/*
 * Dump core to disk
 * Assumes memory mapping has been disabled
 * and IPL is set high
 */

extern short cputype;

struct mba_regs {
	int	csr, cr, sr, var, bcr;
};

#define	INIT	01
#define	DTABT	0x1000
#define	DTCMP	0x2000

extern dump_addr;
extern dumpdev;
daddr_t dumplo;
static phys_addr;
static struct device *daddr;
static int drive;
static int slice;
static daddr_t dblk;
static int pos;

#define	NBLK	16

gddump()
{
	extern physmem;

	if (cputype == 780)
		phys_addr = 0x20000000 + 0x2000*dump_addr;
	else
		/* this is a kludge, something smart needs to be done here */
		phys_addr = 0xf28000;
	daddr = (struct device *)(phys_addr + 0x400 + gddrive(dumpdev) * 0x80);
	drive = gddrive(dumpdev);
	slice = gdslice(dumpdev);
	pos = gdpos(dumpdev);

	if (gdtype[pos] == 0) {
		printf("unknown disk type / drive not initialized\n");
		return(1);
	}

	printf("\nDisk Dump on %s\n", gdloc(pos).name);
	printf("Drive %d, slice %d, offset %d\n", drive, slice, dumplo);
	printf("%d memory blocks\n%d blocks per record\n", physmem, NBLK);
	if (dwall((char *)0, physmem))	/* write out memory */
		printf("dump failed\n");
	else
		printf("dump completed successfully\n");
}

static
dwall(start, num)
char *start;
int num;
{
	register struct mba_regs *mba;
	register i, pte, *mbamap;
	int	err;
	int ntrk, nsec, nspc, cyloff;
	daddr_t nblocks;

	dblk = dumplo;
	pte = (((int)start)>>9) | 0x80000000;
	mba = (struct mba_regs *)(phys_addr);
	mba->cr = INIT;	/* init the mba */
	daddr->cs1 = DCLR | GO;
	if ((daddr->ds & VV) == 0) {
		daddr->cs1 = PRESET | GO;
		daddr->of = FMT22;
	} else daddr->of &= ~SSEI;
	nblocks = gdloc(pos).sizep[slice].nblocks;
	cyloff = gdloc(pos).sizep[slice].cyloff;
	ntrk = gdloc(pos).trk;
	nsec = gdloc(pos).sec;
	nspc = ntrk * nsec;

	if (num > nblocks) {
		printf("too many blocks for dump slice\n");
		return(1);
	}
	while (num > 0) {
		int blk, cn, sn, tn;
		daddr_t bn;

		mbamap = (int *)(phys_addr + 0x800);
		blk = num > NBLK ? NBLK : num;
		bn = dblk;
		cn = bn/nspc + cyloff;
		sn = bn%nspc;
		tn = sn/nsec;
		sn = sn%nsec;
		daddr->dc = cn;
		daddr->da = (tn << 8) + sn;
		for (i=0; i < blk; i++) {
			*mbamap++ = pte++;
		}
		mba->sr = -1;
		mba->var = 0;
		mba->bcr = -(blk*512);
		daddr->cs1 = WCOM | GO;

		while ((mba->sr & DTCMP) == 0) ;

		if (mba->sr & DTABT) {
			printf("disk I/O error %x\n", daddr->ds);
			return(1);
		}
		start += blk*512;
		num -= blk;
		dblk += blk;
	}
	return(0);
}
