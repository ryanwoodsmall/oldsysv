/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:boot/mboot/mboot.c	10.4"
/* mboot.c MINI-BOOT from hard or floppy disk of UNIX OS ON 3B2 */

#include <sys/firmware.h>
#include <sys/boot.h>
#include <sys/types.h>
#include <sys/vtoc.h>
#include <sys/param.h>

#define FSTART_SECT	1
#define MYPHYS ((struct pdinfo *) (BOOTADDR + 2*BSIZE))
#define MYVTOC ((struct vtoc *) (BOOTADDR + BSIZE))

/* Special magic pointer block which tells us
 * where to load lboot. Any changes here must
 * be coordinated with usr/src/cmd/newboot.c
 */

union	{
	char		buffer[BSIZE];
	struct vtoc	vtoc;
	}
	block1;



main()
{

	register	dev;
	register	memptr;		/* address of where to load into memory */
	register	sect;		/* sector number being read */
	register	lbcount;



	switch (dev = P_CMDQ->b_dev)
		{
	case HARDDISK0:
	case HARDDISK1:
	
		if (!HD_ACS(dev-HARDDISK0, sect=PHYS_INFO[dev - HARDDISK0].logicalst+1, MYVTOC, DISKRD))
			/* Disk read error , return to firmware */
			return;
		memptr = MYVTOC->v_bootinfo[0] - BSIZE;
		lbcount = MYVTOC->v_bootinfo[1];

		do {
			if (!HD_ACS(dev-HARDDISK0, ++sect, memptr += BSIZE, DISKRD))
				return;
		} while ((lbcount -= BSIZE) > 0);
		break;

	case FLOPDISK:
		sect = FSTART_SECT;
		if (!FD_ACS(sect, MYVTOC, DISKRD, FIRST))
			return;		/* error:  return to firmware */

		memptr = MYVTOC->v_bootinfo[0] - BSIZE;
		lbcount = MYVTOC->v_bootinfo[1];

		do {
			if (!FD_ACS(++sect, memptr += BSIZE, DISKRD, NOCHANGE))
					return;
		} while ((lbcount -= BSIZE) > 0);
		break;

	default:
		if (!IOBLK_ACS(P_CMDQ->b_dev, sect=(MYPHYS->logicalst)+1, MYVTOC, DISKRD))
			/* Disk read error , return to firmware */
			return;
		memptr = MYVTOC->v_bootinfo[0] - BSIZE;
		lbcount = MYVTOC->v_bootinfo[1];

		do {
			if (!IOBLK_ACS(P_CMDQ->b_dev, ++sect, memptr += BSIZE, DISKRD))
				return;
		} while ((lbcount -= BSIZE) > 0);
		break;

	}

	/*
	 * Work around "memory corruption upon reset" problem by flagging
	 * memory (to the firmware) as uninitialized. This makes firmware
	 * write to all of memory upon the next boot attempt, effectively
	 * clearing any lurking parity errors. Since firmware does this
	 * during the boot, as opposed to upon reset recovery, there is
	 * no impact to crash dump procedures.
	 */
	MEMINIT = SAFE;

	/* Fix for disk defect mapping problem in prom code */

	if (SERNO->serial0 < 0x23)
		*((char *)&MEMINIT + 5) = 0xff;

	/* Transfer execution control to start of LBOOT */
        (*((int(*)())block1.vtoc.v_bootinfo[0])) ();

}
