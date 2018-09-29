/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*
 * SYS16 Disk Dump
 */

#include "sys/types.h"
#include "sys/disk.h"
#include "sys/dcu.h"

extern dumpdev;
daddr_t dumplo = 0;
extern physmem;
extern struct disktab dc_tbl[];
extern short dcinit;

#define NBLK	30
int dd_buf[NBLK];

struct dcu_iocb dd_iocb;

static slice, drive;

dcdump()
{
	drive = dumpdev>>3;
	slice = dumpdev & 07;
	printf("\nDisk Dump\n");
	printf("Drive %d, slice %d, offset %d\n", drive, slice, dumplo);
	printf("%d memory blocks\n%d blocks per record\n", physmem, NBLK);
	if (drive > 7 || (dcinit & (1<<drive)) == 0) {
		printf("Bad drive %d\n", drive);
		return;
	}

	if (dwall())	/* write out memory */
		printf("dump failed\n");
	else
		printf("dump completed successfully\n");
}

static
dwall()
{
	register struct disktab *tp;
	register struct dcu_iocb *ip;
	register num = physmem;
	register i, base;
	int ntrk, nsec, nspc, cyloff;
	daddr_t nblocks, dblk;

	dblk = dumplo;
	tp = &dc_tbl[drive];
	ip = &dd_iocb;
	nblocks = tp->dt_part[slice].nbl;
	cyloff = tp->dt_part[slice].cyloff;
	ntrk = tp->dt_ntr;
	nsec = tp->dt_nsec;
	nspc = ntrk * nsec;

	if (num + dumplo > nblocks) {
		printf("too many blocks for dump slice\n");
		return(1);
	}
	base = 0;
	while (num > 0) {
		int blk, cn, sn, tn;
		daddr_t bn;

		blk = num > NBLK ? NBLK : num;
		bn = dblk;
		cn = bn/nspc + cyloff;
		sn = bn%nspc;
		if (blk > nspc-sn)
			blk = nspc-sn;
		tn = sn/nsec;
		sn = sn%nsec;
		for (i=0; i < blk; i++) {
			dd_buf[i] = (base+i) << 9;
		}

		ip->dc_opcode = DC_WRITE;
		ip->dc_count = blk;
		ip->dc_cylinder = cn;
		ip->dc_drhdsec = (drive<<13) | (tn<<8) | (sn);
		ip->dc_buffer = (int)dd_buf;
		ip->dc_chain = 0;
		ip->dc_sense = 0;
		ip->dc_ecc = 0;
		ip->dc_status = 0;
		if (dc_poll(DC_START, &ip->dc_opcode) != 0) {
			printf("disk I/O error %x\n", ip->dc_sense);
			return(1);
		}

		base += blk;
		num -= blk;
		dblk += blk;
	}
	return(0);
}
