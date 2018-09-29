/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:boot/lboot/dfcstand.c	10.3"

/*
 * This module, dfcstand, is the most machine dependant of all
 * lboot modules. The first part of this program contains the 3b5 code,
 * while the second half contains the 3b2 code. There is no shared
 * code in this module.
 */

#ifdef u3b5
/*
 *	          #####  #       #######
 *	         #     # #       #
 *	               # #       #
 *	 #    #        # # ###   #####
 *	 #    #    ####  ##   #       #
 *	 #    #        # #    #        #
 *	 #    #        # #    #        #
 *	 #   ##  #     # ##   #  #    #
 *	  ### #   #####  # ###    ####
 */

#include <sys/types.h>
#include <sys/dir.h>
#include "lboot.h"
#include <sys/param.h>
#include "io.h"
#include <sys/idfc.h>
#include <sys/iobuf.h>
#include <sys/elog.h>
#include <sys/dfdrv.h>
#include <sys/sysgdb.h>
#include <sys/firmware.h>
#include "error.h"

#if lint
/*
 * These declarations prevent complaints from lint
 */
struct inode { int junk; };
struct eblock { int junk; };
struct dinode { int junk; };
struct filsys { int junk; };
#endif

#define DELAYMAX	2000
#define SET		1
#define BNJRQE		2
#define BNJCQE		2

/*
 * Static function declarations for this file
 */
static long    cksum();
static boolean io_complete();
static boolean reset_complete();


/*
 * Constants which specify the disk containing the boot file system
 */
unsigned char	Smd=0;		/* SMD # */
unsigned char	Dsk=0;		/* Disk # on SMD */

/*
 * File system offset from beginning of disk.  This magic number is entirely
 * dependent upon the size of the boot partition!
 */
long		Fso=FSO_OFFSET;


/*
 * Template for stand-alone command block which is copied to IDFC RAM
 */
static union sa_cmd standalone_commandblock;


/*
 * Pointer to IDFC PIR
 */
static struct pir16 *pirp;



static struct df_jrqe  request_q[BNJRQE];		/* job request queue */
static struct df_jrqe *rq_loadp = request_q;		/* job request queue load pointer */
static struct df_jrqe *rq_unloadp = request_q;		/* job request queue unload pointer */

static struct df_jcqe  completion_q[BNJCQE];		/* job completion queue */
static struct df_jcqe *cq_loadp = completion_q;		/* job completion queue load pointer */
static struct df_jcqe *cq_unloadp = completion_q;	/* job completion queue unload pointer */



/*
 * Sysgen data block for boot program LBOOT.
 */
static struct sgdb
	{
	struct	sgcom	sdbc;	/* sysgen db header */
	struct	sgjqd	jrqd;	/* request Q descriptor */
	struct	sgjqd	jcqd;	/* job completion queue desc. */
	long	sgcksum;	/* sysgen db checksum */
	}
	sysgen_datablock = {
		0xb2L,					/* sysgen job id */
		sizeof(struct sgdb)/sizeof(long),	/* sgdb size in words */
		D_PSYSG,				/* BOOT sysgen opcode */
		1,					/* number of job request queues */
		1,					/* number of job completion queues */

		(paddr_t *) request_q,			/* address of request queue */
		(paddr_t *) &rq_loadp,			/* address of request queue load pointer */
		(paddr_t *) &rq_unloadp,		/* address of request queue unload pointer */
		sizeof(request_q)/sizeof(long),		/* size of request queue in words (BNJRQE entries) */
		0,

		(paddr_t *) completion_q,		/* address of comp queue */
		(paddr_t *) &cq_loadp,			/* address of comp queue load pointer */
		(paddr_t *) &cq_unloadp,		/* address of comp queue unload pointer */
		sizeof(completion_q)/sizeof(long),	/* size of comp queue in words (BNJCQE entries) */
		0,

		0					/* sysgen datablock checksum */
		};

/*
 * Reset and sysgen the IDFC
 *
 * Return	TRUE  - successful completion
 *		FALSE - failure
 */
 boolean
dfinit()
	{PROGRAM(dfinit)

	register struct idfc_wcsr *wcsrp = (struct idfc_wcsr *) (((long) BOOT_DEVP->boot_addr) | OCSR);
	register unsigned short *local_sacbp = (unsigned short *) &standalone_commandblock;
	register unsigned short *idfc_sacbp = (unsigned short *)(((long) BOOT_DEVP->boot_addr)| OSACB);
	register int cnt;


	pirp = (struct pir16 *) (((long) BOOT_DEVP->boot_addr) | OPIR);



	/*
	 * RESET the boot IDFC
	 */

	wcsrp->req_reset = SET;
	cnt = DELAYMAX;
	while (--cnt >= 0)
		continue;

	if (! reset_complete((struct idfc_rcsr *)wcsrp))
		{
		/* IDFC reset failed! */
		error(ER8);
		return(FALSE);
		}

	/*
	 * SYSGEN the boot IDFC
	 */
	sysgen_datablock.sgcksum = cksum((long*) &sysgen_datablock, sizeof(sysgen_datablock)/sizeof(long));

	/*
	 * build the stand-alone command block and copy it to IDFC RAM
	 */
	((union sa_cmd *)local_sacbp)->sysgen.cmdcode = D_PSYSG;
	((union sa_cmd *)local_sacbp)->sysgen.sysgdp = (char *)&sysgen_datablock;

	for (cnt=4; cnt; --cnt)
		*idfc_sacbp++ = *local_sacbp++;

	/*
	 * send sysgen pir to IDFC
	 */
	pirp->pir01 = SET;

	if (io_complete(D_PSYSG))
		return(TRUE);

	/* IDFC sysgen failed! */
	error(ER9);
	return(FALSE);
	}

/*
 * This routine is used as a software timer to time the IDFC reset
 *
 * Return	TRUE  - successful completion
 *		FALSE - failure
 */
 static
 boolean
reset_complete(csrp)
	register struct idfc_rcsr *csrp;
	{PROGRAM(reset_complete)

	register int	delay = 512*DELAYMAX;

	while (--delay >= 0)
		{
		if (csrp->rcsr3 & RESET_COMPL)	/* is reset complete? */
			{
			/*
			 * Reset is complete
			 */
			return(TRUE);
			}
		}

	return(FALSE);      /* no - return error */
	}

/*
 * The read dfc routine will generate a request in the IDFC
 * job request queue and properly update the load pointer.
 */
 void
dfcread(bp)
	register struct buf *bp;
	{PROGRAM(dfcread)

	/* load the job request queue entry */
	rq_loadp->jid = bp;
	rq_loadp->dsknum = Dsk;
	rq_loadp->smdnum = Smd;
	rq_loadp->opc = D_READ;
	rq_loadp->sma = (paddr_t) bp->b_un.b_addr;
	rq_loadp->sdba = (bp->b_blkno + Fso) << 9;
	rq_loadp->bcnt = bp->b_bcount;
	rq_loadp->cksum = cksum((long*) rq_loadp, sizeof(struct df_jrqe)/sizeof(long));

#if DEBUG1
	if (prt[_DFCSTAND])
		{
		printf("\nRequest:\n");
		printf("     rq_loadp  = 0x%lX\n",rq_loadp);
		printf("     rq_unloadp= 0x%lX\n",rq_unloadp);
		printf("     jid    = 0x%lX\n",rq_loadp->jid);
		printf("     jcqid  = 0x%X\n",rq_loadp->jcqid);
		printf("     opc    = 0x%X\n",rq_loadp->opc);
		printf("     pmta   = 0x%lX\n",rq_loadp->pmta);
		printf("     sma    = 0x%lX\n",rq_loadp->sma);
		printf("     sdba   = %D\n",rq_loadp->sdba>>9);
		printf("     bcnt   = 0x%lX\n",rq_loadp->bcnt);
		printf("     cksum  = 0x%lX\n",rq_loadp->cksum);
		}
#endif

	if (++rq_loadp == &request_q[BNJRQE]) /* check for end of queue */
		rq_loadp = request_q;

	pirp->pir04 = SET;	/* send work pending PIR to IDFC */

	if (! io_complete(D_READ))
		{
		bp->b_flags |= B_ERROR;

		/* I/O ERROR:	id=<cq_unloadp->jid> block=<bp->b_blkno+Fso> count=<bp->b_bcount>
				jstat=<cq_unloadp->jstat> erstat=<cq_unloadp->erstat> xerstat=<cq_unloadp->xerstat > */
		error(ER10, cq_unloadp->jid, bp->b_blkno+Fso, bp->b_bcount,
				cq_unloadp->jstat, cq_unloadp->erstat, cq_unloadp->xerstat);

		if (++cq_unloadp == &completion_q[BNJCQE])
			cq_unloadp = completion_q;
		}

	bp->b_flags |= B_DONE;
	}

/*
 * This routine is used as a software timer to time disk requests
 *
 * Return	TRUE  - successful completion; cq_unloadp is updated
 *		FALSE - failure; cq_unloadp is not updated
 */
 static
 boolean
io_complete(opcode)
	unsigned char opcode;
	{PROGRAM(io_complete)

	register int	delay = 512*DELAYMAX;

	while (--delay >= 0)
		{
		if (cq_loadp != cq_unloadp)	/* is job done? */
			{
			/*
			 * Job is done, check the results of the I/O
			 */
#if DEBUG1
			if (prt[_DFCSTAND])
				{
				printf("\nCompletion:\n");
				printf("     cq_loadp  = 0x%lX\n",cq_loadp);
				printf("     cq_unloadp= 0x%lX\n",cq_unloadp);
				printf("     jid    = 0x%lX\n",cq_unloadp->jid);
				printf("     jstat  = 0x%x\n",cq_unloadp->jstat);
				printf("     opc    = 0x%x\n",cq_unloadp->opc);
				printf("     erstat = 0x%lX\n",cq_unloadp->erstat);
				printf("     xerstat= 0x%lX\n",cq_unloadp->xerstat);
				}
#endif

			if (cq_unloadp->opc == opcode && cq_unloadp->jstat == 0)
				{
				/*
				 * Update unload pointer, and return success
				 */
				if (++cq_unloadp == &completion_q[BNJCQE])
					cq_unloadp = completion_q;
				return(TRUE);
				}
			else
				/*
				 * Do not update unload pointer, and return failure
				 */
				return(FALSE);
			}
		}

	/* IDFC timed out! */
	error(ER11);
	return(FALSE);
	}

/*
 * Generate an exclusive-or checksum over size-1 words beginning
 * at addr
 */
 static
 long
cksum(addr, size)
	register long *addr;
	register int size;		/* number of longs + 1 */
	{PROGRAM(cksum)

	register long sum = 0;

	while (--size > 0)
		sum ^= *(addr++);

	return(sum);
	}
#endif

#ifdef u3b2
/*
 *	          #####  #        #####
 *	         #     # #       #     #
 *	               # #             #
 *	 #    #        # # ###        #
 *	 #    #    ####  ##   #    ###
 *	 #    #        # #    #   #
 *	 #    #        # #    #  #
 *	 #   ##  #     # ##   #  #
 *	  ### #   #####  # ###   #######
 */

#include <sys/types.h>
#include <sys/dir.h>
#include "lboot.h"
#include <sys/param.h>
#include "io.h"
#include <sys/iobuf.h>
#include <sys/vtoc.h>
#include <sys/if.h>
#include "error.h"


/*
 * Default system devices obtained from VTOC
 */

char	*VTOC_devname = NULL;	/* driver name associated with boot device */
int	VTOC_major;		/* board slot of boot device (major number) */
int     VTOC_minor; 		/* device number on controller */
int	VTOC_root = -1;		/* partition number */
int	VTOC_swap = -1;		/* partition number */
int	VTOC_nswap;		/* partition size */

/*
 * File system offset from beginning of disk.  This magic number is entirely
 * dependent upon the size of the boot partition!
 */
long		Fso = -1;

/*
 * access routines for disk
 */
static boolean 	(*access)();
static boolean	idread();
static boolean	ifread();
static boolean  pdevread();

static void	findboot();

#define	MYPHYS	((struct pdinfo *)(BOOTADDR + (2 * BSIZE)))

/*
 * Initialize disk access:
 *
 *	- determine boot device (integral hard disk or floppy or peripheral)
 *	- assign disk access routine
 *	- read vtoc to determine root file system (if applicable)
 *	- search vtoc to get system device defaults
 */
 boolean
dfinit()
	{PROGRAM(dfinit)

	register i;

	/*
	 * VTOC block
	 */
	union	{
		char		buffer[BSIZE];
		struct vtoc	vtoc;
		}
		block1;

	switch (P_CMDQ->b_dev)
		{
	case HARDDISK0:
	case HARDDISK1:
		access = idread;

		if (! (*access)(PHYS_INFO[P_CMDQ->b_dev-HARDDISK0].logicalst+1,&block1))
			{
			/* VTOC read failed */
			error(ER8);
			return(FALSE);
			}

		if (block1.vtoc.v_sanity != VTOC_SANE)
			{
			/* VTOC does not exist or is damaged */
			error(ER9);
			return(FALSE);
			}

		VTOC_minor = (P_CMDQ->b_dev & 0x0F) -1;
#if DEBUG1
		if (prt[_DFCSTAND]) {
		printf("VTOC_minor = 0x%X\n", VTOC_minor);
		}
#endif

		/*
		 * determine the name and board slot of the boot device
		 */
		edtscan(EDT_START, 0, findboot);

		/*
		 * search the VTOC for root file system and default system devices
		 */
		for (i=0; i<V_NUMPAR; ++i)
			{
			if (VTOC_root == -1 && block1.vtoc.v_part[i].p_tag == V_ROOT)
				{
				VTOC_root = i;
				if (Fso == -1)
					Fso = PHYS_INFO[P_CMDQ->b_dev-HARDDISK0].logicalst + block1.vtoc.v_part[i].p_start;
				}

			if (VTOC_swap == -1 && block1.vtoc.v_part[i].p_tag == V_SWAP)
				{
				VTOC_swap = i;
				VTOC_nswap = block1.vtoc.v_part[i].p_size;
				}
			}

		if (Fso == -1)
			/*
			 * if no root partition, use FSO_OFFSET
			 */
			Fso = FSO_OFFSET;

		break;

	case FLOPDISK:
		access = ifread;

		if (Fso == -1)
			/*
			 * ROOT file system begins at cylinder 1
			 */
			Fso = IFNUMSECT * 2;
		break;
	default:

		access = pdevread;

		if (! (*access)(MYPHYS->logicalst+1,&block1))
			{
			/* VTOC read failed */
			error(ER8);
			return(FALSE);
			}

		if (block1.vtoc.v_sanity != VTOC_SANE)
			{
			/* VTOC does not exist or is damaged
			error(ER9);
			return(FALSE); */
			}

		VTOC_minor = (P_CMDQ->b_dev & 0x0F);
		
		/*
		 * determine the name and board slot of the boot device
		 */
		edtscan(EDT_START, 0, findboot);

		/*
		 * search the VTOC for root file system and default system devices
		 */
		for (i=0; i<V_NUMPAR; ++i)
			{
			if (VTOC_root == -1 && block1.vtoc.v_part[i].p_tag == V_ROOT)
				{
				VTOC_root = i;
				if (Fso == -1)
					Fso = MYPHYS->logicalst + block1.vtoc.v_part[i].p_start;
				}

			if (VTOC_swap == -1 && block1.vtoc.v_part[i].p_tag == V_SWAP)
				{
				VTOC_swap = i;
				VTOC_nswap = block1.vtoc.v_part[i].p_size;
				}
			}

		if (Fso == -1)
			/*
			 * if no root partition, use FSO_OFFSET
			 */
			Fso = FSO_OFFSET;

		break;

		}
	return(TRUE);
	}

/*
 * findboot(edtp, lba, elb)
 *
 * Locate the device corresponding to the boot device and assign values to
 * VTOC_devname and VTOC_major
 */
 static
 void
findboot(edtp, lba, elb)
	register struct edt *edtp;
	int lba;
	register int elb;
	{PROGRAM(findboot)

	if (((P_CMDQ->b_dev & 0xF0) >> 4) == elb)
		{
		if (strcmp(edtp->dev_name,"SBD") == 0)
			/*
			 * boot device is the integral disk
			 */
			VTOC_devname = "IDISK";
		else
			VTOC_devname = edtp->dev_name;

		VTOC_major = elb;
		}
	}

/*
 * Disk read routines, one for each type of disk as determined by dfinit().
 * Return TRUE if access was successful, FALSE if access failed.
 */

/*
 *  Integral hard disk read routine
 */
 static
 boolean
idread(block, addr)
	int block;
	address addr;
	{PROGRAM(idread)

	if (HD_ACS(P_CMDQ->b_dev - HARDDISK0, block, addr, DISKRD))
		return(TRUE);
	else
		/*
		 * I/O error
		 */
		return(FALSE);
	}
/*
 * Peripheral access routine
 */

 static
 boolean
pdevread(block, addr)
	int block;
	address addr;
	{PROGRAM(devread)

	if (IOBLK_ACS(P_CMDQ->b_dev, block, addr, DISKRD))
		return(TRUE);
	else
		/*
		 * I/O ERROR
		 */
		return(FALSE);
	}


/*
 *  Integral floppy disk read routine
 */
 static
 boolean
ifread(block, addr)
	int block;
	address addr;
	{PROGRAM(ifread)

	if (FD_ACS(block, addr, DISKRD, NOCHANGE))
		return(TRUE);
	else
		/*
		 * I/O error
		 */
		return(FALSE);
	}

/*
 * Read the disk as specified by the buffer header
 */
 void
dfcread(bp)
	register struct buf *bp;
	{PROGRAM(dfcread)

	register address addr;
	register block, size;

#if DEBUG1
	if (prt[_DFCSTAND])
		{
		printf("\nBuffer header @ 0x%X:\n", bp);
		printf("        b_flags = 0x%X\n", bp->b_flags);
		printf("        b_bcount= %d\n", bp->b_bcount);
		printf("        b_addr  = 0x%X\n", bp->b_un.b_addr);
		printf("        b_blkno = %d\n", bp->b_blkno);
		printf("        b_error = 0x%X\n", bp->b_error);
		}
#endif

	block = bp->b_blkno + Fso;
	addr = (address) bp->b_un.b_addr;
	size = bp->b_bcount;

	while (size > 0)
		{
		if (! (*access)(block,addr))
			{
			/* I/O ERROR:	disk=<P_CMDQ->b_dev> block=<block> count=<BSIZE> */
			error(ER10, (P_CMDQ->b_dev==FLOPDISK)? "IF":"ID", block, BSIZE);

			bp->b_flags |= B_ERROR;
			break;
			}

		block += 1;
		addr += BSIZE;
		size -= BSIZE;
		}

	bp->b_flags |= B_DONE;
	}
#endif
