/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/ct.c	10.9"
/*
 *
 * File:	ct.c
 *
 * Description:	3B2 UNIX/Driver interface for the Cartridge Tape Controller
 *              Peripheral.  See ct_lla.c for Driver/FW interface routines.
 *
 * Written By:	James D. Jennings, IW 45155, 1Z-219, x7093, May 1984
 *
 * Supporting Documents:
 *            - 3B2 Computer Feature Requirements: Cartridge Tape Controller,
 *              S. A. James
 *            - AT&T 3B2 System Feature Requirements: Cartridge Tape
 *		Controller (CTC) Driver, J. D. Jennings
 *            - AT&T 3B2 Computer Cartridge Tape Controller (CTC) Firmware
 *              Requirements, K. P. Sherwin
 *
 * Contents:	ctcbad()      - board failed to respond to SYSGEN.
 *		ctchk_req()   - check request parameters.
 *		ctchk_tuse()  - check tape pass count vs. tape max. count
 *		ctclose()     - take sub-device out of service.
 *		ctclr_brd()   - reset board, clear flags, reset queues.
 *		ctcontig()    - check contiguous memory in paged env.
 *		ctdet_err()   - determine UNIX error type from FW R/W result.
 *		ctflg_tim()   - timeout routine for stream open flag request.
 *		ctgtqe()      - get CT driver job queue entry from pool.
 *		ctimjob()     - job timeout error function.
 *		ctinit()      - initialize driver global variables.
 *		ctint()       - driver interrupt handler.
 *		ctioctl()     - driver misc. I/O commands.
 *		ctlnkwq()     - link job to UNIX/driver work queue.
 * 		ctopen()      - bring sub-device into service.
 *		ctpmpctl()    - copy arg to/from ctpump() to/from ctioctl().
 *		ctprint()     - common driver print routine.
 *		ctpump()      - provide UNIX pump functions for CTC board.
 *		ctread()      - character read from sub-device.
 *		ctrtqe()      - return CT driver job queue entry to free pool.
 *		ctsetup()     - determine FW parameters for RW/WRT.
 *		ctsingusr()   - determine if only user on board.
 *		ctstrategy()  - block read/write to/from sub-device.
 *		ctstrt_ques() - Initialize all CTC work queues.
 *		ctud_ctim()  - update controller time usage.
 *		ctwrite()    - character write to sub-device.
 *
 * Global Data:
 *   - Allocated in master.d/ctc file
 *              ct_addr[]     = address of CTC(s) in system
 *		ct_board[]    = CT board control block - see ct_lla.h.
 *		ctc_cnt       = Total No. of ctc's in system.
 *		ctc_maj       = Major device no. assigned to ctc by self-config.
 *		ctslotno      = Array for storing slot location(s).
 *              ctlla_state[] = ct low level access state.
 *   - Allocated in ct.c
 *		ct_sgdblk   = sysgen data block.
 *		ctpmpdata[] = scratch area for ct download used by pump routine.
 *		ctf_sizes[] = default floppy drive partition configuration.
 *
 * Driver Particulars:
 *   - sleep() assignments
 *
 *        Processes                Assignment                 Detection
 *
 *   Single lla functions ct_board[brd_id].brd_flgs.brd_state    none
 *   UNIX rd/wrt functions   (struct ctjobstat *) ctjob          none
 *   Full Request Queue       ct_board.full_q[brd_id]    ct_board.full_q[brd_id]
 *   CTOPEN Command          (struct ctjobstat *) ctjob          none
 *   CTCLOSE Command         (struct ctjobstat *) ctjob          none
 *   CTFORMAT Command        (struct ctjobstat *) ctjob          none
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/fs/s5dir.h>
#include <sys/sysmacros.h>
#include <sys/signal.h>
#include <sys/psw.h>
#include <sys/pcb.h>
#include <sys/user.h>
#include <sys/errno.h>
#include <sys/buf.h>
#include <sys/elog.h>
#include <sys/iobuf.h>
#include <sys/systm.h>
#include <sys/cmn_err.h>
#include <sys/pump.h>
#include <sys/ct_dep.h>
#include <sys/queue.h>
#include <sys/cio_defs.h>
#include <sys/if.h>
#include <sys/vtoc.h>
#include <sys/diskette.h>
#include <sys/ct.h>
#include <sys/open.h>
#include <sys/ct_lla.h>
#include <sys/immu.h>
#include <sys/nvram.h>
#include <sys/file.h>
#include <sys/region.h>
#include <sys/proc.h>


#define	FWPASS		0	/* CTC FW pass/fail return values */

#define	DEF_FPART	8	/* Default current floppy # of partitions */

#define	LINK2Q		1	/* Link job to working queue */
#define	UNLINKQ		2	/* Unlink completed job from work queue */

/* Extern kernel routines */

paddr_t	vtop();			/* Virtual to physical address translation */
extern unsigned char getvec();
extern int spl6();		/* Raise priority level to > 12 */
extern rnvram();		/* Routine to read controller pass count */
extern char MAJOR[];		/* Array of kernel major device no.s */

/* Local Driver Variables */
/* Allocated in /etc/master.d/ctc by self-configuration */

extern int ct_addr[];
extern struct ctboard ct_board[];
extern int ctc_cnt;
extern int ctc_maj;
extern int ctslotno[];
extern int ctlla_state[];

/* Local to driver */

SG_DBLK	ct_sgdblk;
long	ctpmpdata[PU_LBLOCK];
struct dfd_conf ctf_sizes[8] = {
	990,	24,	/* partition 0 -	cyl 24-78 (root)	 */
	810,	34,	/* partition 1 -	cyl 34-78		 */
	612,	45,	/* partition 2 -	cyl 45-78		 */
	414,	56,	/* partition 3 -	cyl 56-78		 */
	216,	67,	/* partition 4 -	cyl 67-78		 */
	1404,	1,	/* partition 5 -	cyl 1-78  (init)	 */
	1422,	0,	/* partition 6 -	cyl 0-78  (full disk)	 */
	18,	0	/* partition 7 -	cyl 0     (boot)	 */
};

/*
 * Name:        ctinit()
 *
 * Abstract:    Driver initialization function.
 *
 * Use:         Called only by the KERNEL at power up.
 *
 * Parameters:  None.
 *
 * Externals:
 *
 * Returns:     None.
 *
 * Description: This function initializes those variables which are used to
 *              track the perform the the CTC board(s).  UNIX calls this
 *              function prior to any other function making use of the CTC(s).
 *
 * Calls:       rnvram(), cmn_err(), vtop(), ctstrt_ques()
 *
 * Macros:      UNX_NVR, CTIMEUSE(), BRD_FLGS(), VTOC(), PDSECT(), SDEV_FLGS()
 */

ctinit()
{
	int i;
	int b;
	register char brd_id;

		/*
		 * Determine slot(s) occupied by ctc's.
		 * Used for accessing nvram controller(s) pass count
		 * stored in nvram.  After determined, initialize
		 * controller pass counts in ct_board structure.
		 */
	
	for (i = 0, b = 0; ((i < 12) && (b < ctc_cnt)); i++) {

		if (MAJOR[i] == ctc_maj) {
			ctslotno[b] = i;

			if (rnvram(&UNX_NVR->ioslotinfo[i], &CTIMEUSE(b),
			      sizeof(CTIMEUSE(b))) != PASS) {
				cmn_err(CE_WARN,
	    "ctinit: Invalid controller pass count for CTC %d in NVRAM\n", b+1);
				CTIMEUSE(b) = 0;
			}
			b++;
		}
	}

	for (brd_id = 0;brd_id < ctc_cnt; brd_id++) {

		ctstrt_ques(brd_id);	     /* Initialize all working queues */
						/* Mark driver as initialized */
		BRD_FLGS(brd_id).bits.isinit = SET;
					      /* Mark board as up if sysgen'd */
		if (BRD_FLGS(brd_id).bits.issysgen == SET) {
			BRD_FLGS(brd_id).bits.is_up = SET;
		}
/*
 * Assume default floppy config. for sub-device 1.  Floppies currently do
 * not support a VTOC, however, rumor has it they will soon ... if and when
 * they do, this must be changed.
 */
		for (i = 0; i < DEF_FPART; i++) {
			VTOC(brd_id, FLPY_DEV).v_part[i].p_flag = 0;
			VTOC(brd_id, FLPY_DEV).v_part[i].p_start =
							ctf_sizes[i].sblock;
			VTOC(brd_id, FLPY_DEV).v_part[i].p_size = 
							ctf_sizes[i].nblocks;
		}
		VTOC(brd_id, FLPY_DEV).v_nparts = (ushort)(DEF_FPART-1);
		VTOC(brd_id, FLPY_DEV).v_sanity = VTOC_SANE;
		SDEV_FLGS(brd_id, FLPY_DEV).bits.vtoc_pres = SET;
		PDSECT(brd_id, FLPY_DEV).pdinfo.cyls = 1;
		PDSECT(brd_id, FLPY_DEV).pdinfo.tracks = IFNTRAC;
		PDSECT(brd_id, FLPY_DEV).pdinfo.sectors = IFNUMSECT;
		PDSECT(brd_id, FLPY_DEV).pdinfo.bytes = IFBYTESCT;
		PDSECT(brd_id, FLPY_DEV).pdinfo.sanity = VALID_PD;
	}
}

/*
 * Name:        ctopen()
 *
 * Abstract:    CTC Driver Open function
 *
 * Use:         This funciton is called indirectly by the user via the UNIX OS
 *              OPEN(2) function.
 *
 * Parameters:  dev   = device number of the device file.
 *              flags = flags described in the oflag field of the open
 *                      system call (see UNIX man OPEN(2)).
 *              otyp  = type of open request.
 * Externals:
 *
 * Returns:     Error condition in the user structure u.u_error variable if an
 *              error is detected, otherwise, nothing.
 *
 * Description: 
 *
 * Calls:       vtop(), ct_open(), cmn_err(), sleep(), ctinit(), ctchk_tuse(),
 *              wakeup(), ct_assert(), timeout(), untimeout(), ctgtqe(),
 *              ctrtqe(), spl6(), splx()
 *
 * Macros:      minor(), eminor(), CT_BRD(), SUB_DEV(), PARTNUM(), BRD_FLGS(),
 *              VTOC(), PDSECT(), SDEV_FLGS(), CT_OTYP(), PMNT_CHK()
 */

ctopen(dev, flags, otyp)
dev_t dev;
int flags, otyp;
{
	int frtrn_stat;				/* function return status */
	int time_id;				/* local timeout parameter */
	paddr_t vtoc_add;			/* Physical addrs of vtoc */
	paddr_t pdsect_add;			/* Physical addrs of pdsect */
	register int cur_pri;
	register struct ctjobstat *ctjob;	/* timeout job entry */

	struct ctjobstat *ctgtqe();
	extern ctimjob();

	char fwflag = NORM_OPN;			    /* FW flag for Streaming */
	char min_dev = minor(dev);		    /* Minor Device number */
	char part_num = PARTNUM(min_dev);	    /* Sub-device part. no. */
	register char brd_id = CT_BRD(min_dev);   /* Board id */
	register char sub_dev = SUB_DEV(min_dev); /* Sub-device number */

#ifdef SEEFUNCT
	printf("ctopen(0x%x, 0x%x, 0x%x)", dev, flags, otyp);
	printf(" min=0x%x  emaj=0x%x emin=0x%x\n",
		min_dev, emajor(dev), eminor(dev));
	seestates();
#endif
						/* Valid board id number ? */
	if (brd_id > (ctc_cnt-1) || eminor(dev) > CTMAX_MINOR) {
		u.u_error = ENXIO;		 /* No such device or address */
		return;
	}

	if ((flags & O_PUMP) == O_PUMP) {	    /* Check for pump request */

		if (u.u_uid != 0) {			      /* Super-user ? */
			u.u_error = EPERM;		   /* Nope! Has to be */
			return;
		}
		ctclr_brd(brd_id);	      /* Re-init board - Driver reset */
		BRD_FLGS(brd_id).bits.pumping = SET;      /* Proceed w/pump */
		return;
	}				       /* Check to see if board is up */

	if (BRD_FLGS(brd_id).bits.is_up != SET) {
						    /* board failed to sysgen */
		if (BRD_FLGS(brd_id).bits.issysgen != SET) {
			u.u_error = ENOSGEN;
			return;
		}				     /* ctinit() exec'd yet ? */
		else if (BRD_FLGS(brd_id).bits.isinit != SET) {
			ctinit();
		}				      /* Board marked as down */
		else {
			u.u_error = EBRDDWN;
			return;
		}
	}			    /* Check for open request pending already */

	if (BRD_FLGS(brd_id).bits.opnpending == SET) {

		do {
			BRD_FLGS(brd_id).bits.opn_sleep = SET;

			if (sleep(&BRD_FLGS(brd_id).brd_state, (PSLEP | PCATCH)) > 0) {
				BRD_FLGS(brd_id).bits.opn_sleep = CLEAR;
				u.u_error = EINTR;
				return;
			}
			else {
				BRD_FLGS(brd_id).bits.opn_sleep = CLEAR;
			}
		}
		while (BRD_FLGS(brd_id).bits.opnpending == SET);
	}
	BRD_FLGS(brd_id).bits.opnpending = SET;   /* Set open pending flag */
					   /* Check for prev. open timing out */
	if (BRD_FLGS(brd_id).bits.is_up != SET) {
		u.u_error = EBRDDWN;
		BRD_FLGS(brd_id).bits.opnpending = CLEAR;
		return;
	}				  /* Check for ctioctl() open request */

	if ((flags & O_CTSPECIAL) == O_CTSPECIAL ||
	    (flags & O_FORMAT) == O_FORMAT) {

		if (ctsingusr(brd_id) == ENTSUSR) {
			BRD_FLGS(brd_id).bits.opnpending = CLEAR;
			u.u_error = ENTSUSR;	   /* Already special opened! */
			return;
		}
		else {
			SDEV_FLGS(brd_id, sub_dev).bits.specl_opn = SET;
			BRD_FLGS(brd_id).bits.opnpending = CLEAR;
			return;
		}
	}			       /* Check for special open case already */

	if (SDEV_FLGS(brd_id, sub_dev).bits.specl_opn == SET) {
		BRD_FLGS(brd_id).bits.opnpending = CLEAR;
		u.u_error = EUSRSPL;
		return;
	}					 /* Process streaming request */

	if (BRD_FLGS(brd_id).bits.streaming == SET) {

		if (sub_dev != TAPE_DEV) {
			BRD_FLGS(brd_id).bits.opnpending = CLEAR;
			u.u_error = EINVAL;
			return;
		}
		else if (ctsingusr(brd_id) == ENTSUSR) {
			BRD_FLGS(brd_id).bits.opnpending = CLEAR;
			u.u_error = ENTSUSR;
			return;
		}
		untimeout(TIME_ID(brd_id));

					/* Check for correct stream direction */

		if ((flags & FREAD) == FREAD && (flags & FWRITE) == FWRITE) {
			BRD_FLGS(brd_id).brd_state &= ~(STREAMING | OPENPNDING);
			u.u_error = EBDOFLG;
			return;
		}
		else if ((flags & FREAD) == FREAD) {
			fwflag = STREAM_RD;
		}
		else if ((flags & FWRITE) == FWRITE) {
			fwflag = STREAM_WT;
		}
		else {
			BRD_FLGS(brd_id).brd_state &= ~(STREAMING | OPENPNDING);
			u.u_error = EBDOFLG;
			return;
		}
	}				     /* Check for device already open */

	if (SDEV_FLGS(brd_id, sub_dev).bits.subopen == SET) {

		if (otyp < 0 || otyp >= OTYPCNT) {      /* Mark type of open */
			;
		}
		else if (otyp == OTYP_LYR) {
			CT_OTYP(brd_id, sub_dev)[ OTYP_LYR ]++;
		}
		else {
			CT_OTYP(brd_id, sub_dev)[ otyp ] |= (1 << part_num);
		}
		BRD_FLGS(brd_id).bits.opnpending = CLEAR;
		return;
	}

	if ((vtoc_add = vtop(&VTOC(brd_id, sub_dev), NULL)) == NULL) {
		BRD_FLGS(brd_id).bits.opnpending = CLEAR;
		cmn_err(CE_WARN,"ctopen: Bad vtop on ct_board.ct_vtoc - %x\n",
			&VTOC(brd_id, sub_dev));
		u.u_error = EIO;
		return;
	}

	if ((pdsect_add = vtop(&PDSECT(brd_id, sub_dev), NULL)) == NULL) {
		BRD_FLGS(brd_id).bits.opnpending = CLEAR;
		cmn_err(CE_WARN,"ctopen: Bad vtop on ct_board.pdsect - %x\n",
			&PDSECT(brd_id, sub_dev));
		u.u_error = EIO;
		return;
	}
	cur_pri = spl6();			/* Block I/O Interrupts */
						/* Put job into work queues */
	while ((ctjob = ctgtqe(brd_id)) == NULL) {
		IOBUF(brd_id).jrqsleep++;

		if (sleep(&IOBUF(brd_id).jrqsleep, PCATCH | PSLEP) > 0) {
			IOBUF(brd_id).jrqsleep--;
			u.u_error = EINTR;
			splx(cur_pri);
			BRD_FLGS(brd_id).bits.opnpending = CLEAR;
			return;
		}
		IOBUF(brd_id).jrqsleep--;
	}
	splx(cur_pri);
	ctjob->jobstat.bits.opcode  = CTOPEN;	/* Init. job queue entry vars */
	ctjob->jobstat.bits.brd_id  = brd_id;
	ctjob->jobstat.bits.sub_dev = sub_dev;
						 /* Issue ct_lla open request */
	frtrn_stat = ct_open(ctjob, vtoc_add, pdsect_add, fwflag);

	if (frtrn_stat == FULL_Q) {		      /* Check for full queue */

		do {				  /* Sleep until req. honored */
			ct_board[ brd_id ].full_q++;

			if (sleep((caddr_t) &ct_board[ brd_id ].full_q, (PSLEP|PCATCH)) > 0) {
				ct_board[ brd_id ].full_q--;
				BRD_FLGS(brd_id).bits.opnpending = CLEAR;
				cur_pri = spl6();
				ctrtqe(ctjob);
				splx(cur_pri);
				u.u_error = EINTR;
				return;
			}
			else {
				ct_board[ brd_id ].full_q--;
			}
		}
		while ((frtrn_stat = ct_open(ctjob, vtoc_add, pdsect_add, fwflag)) == FULL_Q);
	}

	if (frtrn_stat != PASS) {
		BRD_FLGS(brd_id).bits.opnpending = CLEAR;
		cur_pri = spl6();
		ctrtqe(ctjob);
		splx(cur_pri);
		u.u_error = EFWCBAD;
		return;
	}
	cur_pri = spl6();
	time_id = timeout(ctimjob, ctjob, CTJOB_TIME);
	ct_assert(brd_id);		    /* Send INT to FW for job pending */
	sleep((caddr_t) ctjob, PZERO);		    /* Wait on FW int */
	untimeout(time_id);
	splx(cur_pri);
						   /* Check completion status */
	if (ctjob->jobstat.bits.err_code != FWPASS) {

		switch (ctjob->jobstat.bits.err_code) {

		case CTNOMEDIA:
			u.u_error = ENOMED;
			break;

		case CTNVTOC:
			u.u_error = EBDVTOC;
			break;

		case NOTRDY:
			u.u_error = ENOTRDY;
			break;

		case ETIMOUT:
			u.u_error = ETIMOUT;
			break;

		case ISOPEN:
					    	       /* DRVR-FW out of sync */
			if (SDEV_FLGS(brd_id, sub_dev).bits.subopen != SET) {
				cmn_err(CE_NOTE,
	     "ctopen: Driver-Firmware have confused open states on CTC %d (%d)",
					brd_id, sub_dev);
			}
			break;

		default:
			u.u_error = EIO;
			break;
		}
		BRD_FLGS(brd_id).bits.opnpending = CLEAR;

		if (BRD_FLGS(brd_id).bits.streaming == SET) {
			BRD_FLGS(brd_id).bits.streaming = CLEAR;
		}
	}				         /* FW passed ok but did we ? */
					      /* Check for VTOC/PDSECT errors */
	else if (sub_dev == TAPE_DEV) {

		if (VTOC(brd_id, sub_dev).v_sanity != VTOC_SANE) {
			u.u_error = EBDVTOC;
		}
		else if (PDSECT(brd_id, sub_dev).pdinfo.sanity != VALID_PD) {
			u.u_error = EBDPSEC;
		}
		else if (PMNT_CHK(brd_id, sub_dev, part_num) == V_UNMNT &&
							   otyp == OTYP_MNT) {
			 u.u_error = EACCES;
		}
		else {					     /* Everything ok */
			ctchk_tuse(ctjob);
			SDEV_FLGS(brd_id, sub_dev).bits.vtoc_pres = SET;
			goto opnpass;
		}			  /* All VTOC/PDSECT errors come here */
					/* FW sub_dev opened & must be closed */
		ctjob->jobstat.lng_val &= CTCLRSBITS;
		frtrn_stat = ct_close(ctjob);

		if (frtrn_stat == FULL_Q) {

			do {
				ct_board[ brd_id ].full_q++;

				if (sleep((caddr_t) &ct_board[ brd_id ].full_q, (PSLEP | PCATCH)) > 0) {
					ct_board[ brd_id ].full_q--;
					u.u_error = EINTR;
				}
			}
			while ((frtrn_stat = ct_close(ctjob)) == FULL_Q);
		}
		ct_assert(brd_id);
		sleep((caddr_t) ctjob, PZERO);
		BRD_FLGS(brd_id).bits.opnpending = CLEAR;
		return;
	}
	else {
opnpass:
		SDEV_FLGS(brd_id, sub_dev).bits.subopen = SET;

		if (otyp < 0 || otyp >= OTYPCNT) {      /* Mark type of open */
			;
		}
		else if (otyp == OTYP_LYR) {
			CT_OTYP(brd_id, sub_dev)[ OTYP_LYR ]++;
		}
		else {
			CT_OTYP(brd_id, sub_dev)[ otyp ] |= (1 << part_num);
		}
	}
	BRD_FLGS(brd_id).bits.opnpending = CLEAR;

	if (BRD_FLGS(brd_id).bits.opn_sleep == SET) {
		wakeup(&BRD_FLGS(brd_id).brd_state);
	}
	cur_pri = spl6();
	ctrtqe(ctjob);
	splx(cur_pri);
#ifdef SEEFUNCT
	printf("ctopen() completed\n");
	seestates();
#endif
}

/*
 * Name:        ctclose().
 *
 * Abstract:    CTC Driver Close Function.
 *
 * Use:         This function is called indirectly by the user via the UNIX OS
 *              function CLOSE(2).
 *
 * Parameters:  dev   = device number of the device file.
 *              flags = flags described in the oflag field of the open/close
 *                      system call (see UNIX man OPEN(2)).
 *              otyp  = type of close request.
 *
 * Externals:
 *
 * Returns:     Error condition in the user structure u.u_error varialbe if an
 *              error is detected, otherwise nothing.
 *
 * Description:
 *
 * Calls:       ctchk_req(), ct_close(), sleep(), wakeup(),
 *              ct_assert(), timeout(), untimeout(), ctgtqe(), ctrtqe()
 *
 * Macros:      minor(), CT_BRD(), SUB_DEV(), PARTNUM(), VTOC(), PDSECT(),
 *              SDEV_FLGS(), CT_OTYP(), CT_VERIFY()
 */

ctclose(dev, flags, otyp)
dev_t dev;
int flags, otyp;
{
	int i, osum;
	int frtrn_stat;				    /* function return status */
	int time_id;				   /* local timeout parameter */
	register int cur_pri;
	register struct ctjobstat *ctjob;		 /* timeout job entry */

	struct ctjobstat *ctgtqe();
	extern ctimjob();

	char min_dev = minor(dev);		       /* Minor device number */
	char part_num = PARTNUM(min_dev);		  /* Partition number */
	register char brd_id = CT_BRD(min_dev); 	   /* Board id number */
	register char sub_dev = SUB_DEV(min_dev);      /* Sub-device number */

#ifdef SEEFUNCT
	printf("ctclose(0x%x, 0x%x, 0x%x)", dev, flags, otyp);
	printf("\tminor = 0x%x\n", min_dev);
	seestates();
#endif
					      /* Check brd and sub_dev status */

	if ((frtrn_stat = ctchk_req(dev, brd_id, sub_dev)) > 0) {

		if (SDEV_FLGS(brd_id, sub_dev).bits.specl_opn == SET) {
			SDEV_FLGS(brd_id, sub_dev).bits.specl_opn = CLEAR;
			return;
		}				       /* Device was pumped ? */
		else if (frtrn_stat != ENXIO && 
		     		      BRD_FLGS(brd_id).bits.pumping == SET) {

			BRD_FLGS(brd_id).bits.pumping = CLEAR;
			return;
		}
		else {
			u.u_error = frtrn_stat;
			return;
		}
	}					/* Process close request type */

	if (otyp < 0 || otyp >= OTYPCNT) {
		;
	}
	else if (otyp == OTYP_LYR) {
		CT_OTYP(brd_id, sub_dev)[ OTYP_LYR ]--;
	}
	else {
		CT_OTYP(brd_id, sub_dev)[ otyp ] &= ~(1 << part_num);
	}

	for (osum = i = 0; i < OTYPCNT; osum |= CT_OTYP(brd_id, sub_dev)[i++])
		;

	if (osum > 0) {
		return;
	}
	cur_pri = spl6();
						/* Put job into work queues */
	while ((ctjob = ctgtqe(brd_id)) == NULL) {
		IOBUF(brd_id).jrqsleep++;

		if (sleep(&IOBUF(brd_id).jrqsleep, PCATCH | PSLEP) > 0) {
			IOBUF(brd_id).jrqsleep--;
			u.u_error = EINTR;
			splx(cur_pri);
			return;
		}
		IOBUF(brd_id).jrqsleep--;
	}
	splx(cur_pri);
	ctjob->jobstat.bits.opcode = CTCLOSE;	/* Init. job queue entry vars */
	ctjob->jobstat.bits.brd_id = brd_id;
	ctjob->jobstat.bits.sub_dev = sub_dev;
	frtrn_stat = ct_close(ctjob);

	if (frtrn_stat == FULL_Q) {		      /* Check for full queue */

		do {				  /* Sleep until req. honored */
			ct_board[ brd_id ].full_q++;

			if (sleep((caddr_t) &ct_board[ brd_id ].full_q, (PSLEP | PCATCH)) > 0) {
				ct_board[ brd_id ].full_q--;
				u.u_error = EINTR;
				cur_pri = spl6();
				ctrtqe(ctjob);
				splx(cur_pri);
				return;
			}
			else {
				ct_board[ brd_id ].full_q--;
			}
		}
		while ((frtrn_stat = ct_close(ctjob)) == FULL_Q);
	}

	if (frtrn_stat != PASS) {
		u.u_error = EFWCBAD;
		cur_pri = spl6();
		ctrtqe(ctjob);
		splx(cur_pri);
		return;
	}
	cur_pri = spl6();
	time_id = timeout(ctimjob, ctjob, CTJOB_TIME);
	ct_assert(brd_id);
	sleep((caddr_t) ctjob, PZERO);		    /* Wait on FW int */
	untimeout(time_id);
	splx(cur_pri);

						 /* Process completion status */
	switch (ctjob->jobstat.bits.err_code) {

	case FWPASS:					     /* Everything ok */
		    break;
						    /* Stream prev. R/W error */
	case CTSRWER: 				       /* Close was a success */
	case RWERROR:
		u.u_error = ERWERR;
		break;

	case NOTOPN:
		u.u_error = ENOTOPN;
		break;

	case ETIMOUT:
		u.u_error = ETIMOUT;
		break;

	default:
		u.u_error = EIO;
		break;
	}
	SDEV_FLGS(brd_id, sub_dev).bits.subopen = CLEAR;

	if (sub_dev == TAPE_DEV) {
		VTOC(brd_id, sub_dev).v_sanity = 0;
		PDSECT(brd_id, sub_dev).pdinfo.sanity = 0;
		SDEV_FLGS(brd_id, sub_dev).bits.vtoc_pres = CLEAR;
	}
				   /* Remove verify flag if set for partition */
	if (((CT_VERIFY(brd_id, sub_dev) >> part_num) & SET) == SET) {
		CT_VERIFY(brd_id, sub_dev) &= ~(1 << part_num);
	}
	cur_pri = spl6();
	ctrtqe(ctjob);
	splx(cur_pri);
#ifdef SEEFUNCT
	printf("ctclose() - completed\n");
	seestates();
#endif
}

/*
 * Name:        ctstrategy()
 *
 * Abstract:    Driver entry point to submit read/write jobs to the CTC
 *              Firmware.
 *
 * Use:         This function is called indirectly by the user via the UNIX OS
 *              system calls READ(2)/WRITE(2).
 *
 * Parameters:  bp = buffer header containing information about transfer to be
 *                   performed.  See sys/buf.h.
 *
 * Externals:
 *
 * Returns:     Error condition in bp->b_flags and bp->b_error.  Otherwise
 *              nothing.
 *
 * Description:
 *
 * Calls:       iodone(), cmn_err(), ctsetup(), ctlnkwq(), ct_rdwrt(),
 *              sleep(), ctchk_req(), spl6(), splx(), copyin(), copyout(),
 *              ctgtqe(), timeout(), untimeout(), ctrtqe(), ct_assert()
 *
 * Macros:      minor(), CT_BRD(), SUB_DEV(), PARTNUM(), VTOC(), IOTIME(),
 *              IOBUF()
 */

#define INIT     0x00	/* No problem, no block jobs pending */
#define BLKJOB   0x01   /* this job should be a block job */
#define UNLD_BLK 0x02	/* last job was a block job, unload the cache */
#define ERR_LUP  0x03   /* break out of all loops */
#define RESET_BLK 0x04  /* reset pointers after a block transfer */
#define FINISH   0x05	/* Insure whole block is written if in block mode */
		        /* this ensures that is break is hit on top of a */
			/* 128K boundary that the whole 1024 block is written */

#define TOUSER    1
#define TOCACHE   0

/* macro to find out if */
/* a user process attached or is this a delayed write */
#define USER_PROC ((bp->b_flags & (B_READ | B_PHYS | B_AGE)) != 0)

#define USER_SLEEP (PSLEP | PCATCH)  /* sleep state for user processes  */
				     /* so break will work properly */
#define DELAY_SLEEP (PZERO)          /* sleep state for delayed writes */

/* set u.u_error if this is a user attached process */
#define TSTU_ERROR { \
		if (sleepstate == USER_SLEEP) { \
			u.u_error = bp->b_error; \
		} \
	}
	

ctstrategy(bp)
register struct buf *bp;
{
	int lastblk;				 /* last block on partition */
	int frtrn_stat;				 /* function return status */
	int time_id;
	int streaming, rawmode;
	int cur_pri, j;
	register struct ctjobstat *ctjob;
	struct lla_rwcmd lla_cmd;	 	 /* low level r/w access cmnd */
	register int job_bytes;
	register unsigned long vbuf_addr;

	struct ctjobstat *ctgtqe();
	extern ctimjob();

	long sleepstate;
	unsigned long blkno;
						  /* ONE28K segment variables */
	unsigned long seg_bndry;       /* temporary variable for calculations */
	unsigned long taddr;   /* temporary storage for address while caching */
	char     cache[ IFBYTESCT ];		  /* cache for splitting jobs */
  	register int tot_bytes;			 /* total bytes for this call */
	register char flag;   /* flag showing current state of block transfer */

	char part_num;					  /* Partition Number */
	char min_dev = minor(bp->b_dev);	       /* Minor device number */
	char brd_id = CT_BRD(min_dev);		   /* Board id number */
	char sub_dev = SUB_DEV(min_dev);		 /* Sub-device number */

#ifdef DEBUG
	printf ("CTCSTRAT: b_cnt=%d u_cnt=%d u_base=0x%x u_offset=%d\n",
		bp->b_bcount, u.u_count, u.u_base, u.u_offset);
#endif
	streaming = 0;
	rawmode = 0;

	if (USER_PROC) {
		sleepstate = USER_SLEEP;
	}
	else {
		sleepstate = DELAY_SLEEP;
	}
	j = 0;

	if ((frtrn_stat = ctchk_req(bp->b_dev, brd_id, sub_dev)) > 0) {
		bp->b_flags |= (B_ERROR | B_STALE | B_AGE);
		bp->b_error = frtrn_stat;
		TSTU_ERROR;
		iodone(bp);			/* Return buffer to UNIX */
		return;
	}
	part_num = PARTNUM(min_dev);
					      /* Whole tape only worked by SA */
	if (part_num == 6 && sub_dev == TAPE_DEV) {

		if (u.u_uid != 0) {
			bp->b_flags |= (B_ERROR | B_STALE | B_AGE);
			bp->b_error = EPERM;
			TSTU_ERROR;
			iodone(bp);
			return;
		}
	}				     /* Check for read only partition */

	if ((VTOC(brd_id, sub_dev).v_part[ part_num ].p_flag & V_RONLY) == V_RONLY &&
	    (bp->b_flags & B_READ) != B_READ) {
		cmn_err(CE_WARN,
	     "ctstrategy: partition %d on board %d sub_dev %d marked read only",
			part_num, brd_id, sub_dev);
		bp->b_flags |= (B_ERROR | B_STALE | B_AGE);
		bp->b_error = EROPART;
		TSTU_ERROR;
		iodone(bp);
		return;
	}
	lastblk = VTOC(brd_id, sub_dev).v_part[ part_num ].p_size;

					       /* Check for partition overrun */

	if ((bp->b_blkno < 0) || (bp->b_blkno >= lastblk) ||
		((bp->b_blkno + (bp->b_bcount / IFBYTESCT)) > lastblk)) {

		if ((bp->b_blkno == lastblk) && (bp->b_flags & B_READ)) {
			bp->b_resid = bp->b_bcount;
		}
		else {
			bp->b_flags |= (B_ERROR | B_STALE | B_AGE);
			bp->b_error = EPRTOVR;
		}
		iodone(bp);
		return;
	}		 

	if (BRD_FLGS(brd_id).bits.streaming == SET && sub_dev == TAPE_DEV) {

		++streaming;
#ifdef DEBUG
		printf("STRAT: streaming b_cnt=%d u_cnt=%d b_addr=0x%x\n",
				bp->b_bcount, u.u_count, bp->b_un.b_addr);
#endif
		if ((bp->b_bcount % CTSEG_SIZE > 0)) {
				
			bp->b_flags |= (B_ERROR | B_STALE | B_AGE);
			bp->b_error = EBDJSIZ;
			TSTU_ERROR;
			iodone(bp);
			return;
		} 
	}
	else if (bp->b_flags & B_PHYS) {
#ifdef DEBUG
		printf("STRAT: rawmode\n");
#endif
		++rawmode;
	}
	cur_pri = spl6();
						/* Put job into work queues */
	while ((ctjob = ctgtqe(brd_id)) == NULL) {
		IOBUF(brd_id).jrqsleep++;
		sleep(&IOBUF(brd_id).jrqsleep, PZERO);
		IOBUF(brd_id).jrqsleep--;
	}
	ctjob->jobstat.bits.opcode = CTREAD;   /* Gee-whiz opcode for ctimjob */
	ctjob->jobstat.bits.brd_id = brd_id;
	ctjob->jobstat.bits.sub_dev = sub_dev;
	ctjob->procid = (unsigned long) bp;

	ctlnkwq(ctjob, LINK2Q);
	splx(cur_pri);

	lla_cmd.brd_id = brd_id;
	lla_cmd.sub_dev = sub_dev;
	lla_cmd.jqaddr = (daddr_t) ctjob;

	blkno = bp->b_blkno;
	vbuf_addr = (unsigned long) bp->b_un.b_addr;
	tot_bytes = bp->b_bcount;

	/*
	 * outer loop checks for jobs > 128 K and passes job "chunks" to the
	 * inner loop.  Chunks are, in order, amount up to block boundary
	 * nearest to ONE28K,  block across ONE28K into cache, cache to user,
	 * amount above ONE28K boundary then on to next ONE28K
	 * boundary - - and awaaay we go!
	 */

	flag = INIT;				 /* initialize the state flag */
							/* total bytes in job */
	while ((tot_bytes > 0) && (flag != ERR_LUP)) {

						   /* switch to correct state */
	    switch (flag) {

	    case BLKJOB:		     /* last case was up to a ONE28K, */
		job_bytes = IFBYTESCT;			/* do a BLOCK JOB now */
		taddr = vbuf_addr;		      /* save the old address */
						 /* access block in the cache */
		vbuf_addr = (unsigned long) cache;
						 /* sort out reads and writes */

		if ((bp->b_flags & B_READ) != B_READ) {

	         /* in case the whole job ends at the edge of a 128K boundary */
						 /* move block into the cache */

		    if (tot_bytes < IFBYTESCT) {
			copyin(taddr, cache, tot_bytes);
		    }
		    else {
			copyin(taddr, cache, IFBYTESCT);
		    }
		    flag = RESET_BLK;		     /* get out of block mode */
		}
		else {
		    flag = UNLD_BLK;		/* next state to unload block */
		}
		break;

	    case UNLD_BLK:	   /* finished read block transfer, unload it */
				     /* in case the whole job ends at or near */
					       /* the edge of a 128K boundary */
					/* move the partial blk to user space */

		if (tot_bytes < IFBYTESCT) {
		     copyout(cache, taddr, tot_bytes); 
		}
		else {
		    copyout(cache, taddr, IFBYTESCT); 
		}

	    case RESET_BLK: /* just finished block transfer, go to init state */
		tot_bytes -= IFBYTESCT;
						   /* if done get out of loop */
		if (tot_bytes <= 0) {
		    tot_bytes = 0;
		    continue;
		}
		vbuf_addr = IFBYTESCT + taddr;
		flag = INIT;		 /* no break here so the next segment */
					/* may be checked for ONE28K crossing */

	    case INIT:
	    case FINISH:
	    default:					/* does it cross 128K */

		if (rawmode > 0) {
		    job_bytes = ctcontig(bp->b_proc, vbuf_addr, tot_bytes);
#ifdef DEBUG
		printf ("STRAT: ctcontig: jbytes=%d tbytes=%d vaddr=0x%x\n",
				job_bytes, tot_bytes, vbuf_addr);
#endif

		    if ((job_bytes & (IFBYTESCT-1)) && (job_bytes != tot_bytes)) {
			job_bytes &= ~(IFBYTESCT-1);
			flag = BLKJOB;
		    }
			/*
			 * forces a full (1024) block to be written if you are
			 * in block mode and hit break
			 *	or
			 * a little remnant if you hit break in raw mode
			 */

		    else if ((tot_bytes - job_bytes) <= IFBYTESCT) {
			flag = FINISH;
		    }
		    tot_bytes -= job_bytes;

			 /*
			  * else
			  * leave flag at init so only a two chunk move because
			  * 512 and 128K boundarys align
			  */
		}
		else if (streaming > 0) {
		    flag = INIT;
		    job_bytes = CTSEG_SIZE;
		    tot_bytes -= CTSEG_SIZE;
		}
		else {						/* Block mode */
		    flag = INIT;
		    job_bytes = tot_bytes;
		    tot_bytes = 0;
		}
	    }

	    while (job_bytes > 0) {

		if (streaming > 0) {
		    lla_cmd.nobytes = ctcontig(bp->b_proc, vbuf_addr, job_bytes);

		    if (tot_bytes > 0) {
			ctjob->jobstat.bits.splitjob = 1;
		    }
		    else {
			ctjob->jobstat.bits.splitjob = 0;
		    }
		}
		else if (job_bytes > CTSEG_SIZE) {
		    lla_cmd.nobytes = CTSEG_SIZE;
		    job_bytes -= CTSEG_SIZE;
		    ctjob->jobstat.bits.splitjob = 1;
		}
		else {
		    lla_cmd.nobytes = job_bytes;
		    ctjob->jobstat.bits.splitjob = 0;
		}
			       /* Determine parameters for FW read/write call */

#ifdef DEBUG
	printf ("STRAT: ctsetup: vaddr=0x%x tbytes=%d jbytes=%d llnobytes=%d\n",
		vbuf_addr, tot_bytes, job_bytes, lla_cmd.nobytes);
#endif
		if (ctsetup(&lla_cmd, bp, part_num, blkno, vbuf_addr) == FAIL) {
		    bp->b_flags |= (B_ERROR | B_STALE | B_AGE);
		    bp->b_error = EIO;
		    TSTU_ERROR;
		    IOTIME(brd_id, sub_dev).io_cnt--;
		    IOTIME(brd_id, sub_dev).io_bcnt -= btoc(bp->b_bcount);
		    IOBUF(brd_id).qcnt--;
		    IOTIME(brd_id, sub_dev).io_qc--;

		    cur_pri = spl6();
		    ctrtqe(ctjob);
		    ctlnkwq(ctjob, UNLINKQ);
		    splx(cur_pri);

		    if (flag == RESET_BLK || flag == UNLD_BLK) {
			bp->b_resid = tot_bytes;
		    }
		    else {
			bp->b_resid = tot_bytes + job_bytes;
		    }
		    iodone(bp);
		    return;
		}
					       /* Issue FW Read/Write request */
		frtrn_stat = ct_rdwrt(&lla_cmd, brd_id);

		if (frtrn_stat == FULL_Q) {	      /* Check for full queue */

		    do {			  /* Sleep until req. honored */
		        ct_board[ brd_id ].full_q++;

		        if ((sleep((caddr_t) &ct_board[ brd_id ].full_q, sleepstate) > 0) && (flag != FINISH)) {

			    ct_board[ brd_id ].full_q--;
			    bp->b_flags |= (B_ERROR | B_STALE | B_AGE);
			    bp->b_error = EINTR;

				/*
				 * This is redendant check since only time you
				 * can ever get here is if sleepstate is
				 * PSLEP | PCATCH, but it is consistent and
				 * non-performance impacting in an error leg
				 */

			    TSTU_ERROR;
			    IOTIME(brd_id, sub_dev).io_cnt--;
			    IOTIME(brd_id, sub_dev).io_bcnt -=
							btoc(bp->b_bcount);
			    IOBUF(brd_id).qcnt--;
			    IOTIME(brd_id, sub_dev).io_qc--;

			    cur_pri = spl6();
			    ctrtqe(ctjob);
			    ctlnkwq(ctjob, UNLINKQ);
			    splx(cur_pri);

			    if (flag == RESET_BLK || flag == UNLD_BLK) {
				bp->b_resid = tot_bytes;
			    }
			    else {
				bp->b_resid = tot_bytes + job_bytes;
			    }
			    iodone(bp);
			    return;
		        }
		        ct_board[ brd_id ].full_q--;
		    }
		    while ((frtrn_stat = ct_rdwrt(&lla_cmd, brd_id)) == FULL_Q);
		}

		if (frtrn_stat != PASS) {
		    bp->b_flags |= (B_ERROR | B_STALE | B_AGE);
		    bp->b_error = EFWCBAD;
		    TSTU_ERROR;

					 /* Adjust UNIX controller statistics */

		    IOTIME(brd_id, sub_dev).io_cnt--;
		    IOTIME(brd_id, sub_dev).io_bcnt -= btoc(bp->b_bcount);
		    IOBUF(brd_id).qcnt--;
		    IOTIME(brd_id, sub_dev).io_qc--;

						/* Unlink job fm work queues */

		    cur_pri = spl6();
		    ctrtqe(ctjob);
		    ctlnkwq(ctjob, UNLINKQ);
		    splx(cur_pri);

		    if (flag == RESET_BLK || flag == UNLD_BLK) {
			bp->b_resid = tot_bytes;
		    }
		    else {
			bp->b_resid = tot_bytes + job_bytes;
		    }
		    iodone(bp);
		    return;
		}

		if (streaming > 0 && (job_bytes -= lla_cmd.nobytes) != 0) {
		    vbuf_addr += lla_cmd.nobytes;
		    blkno += (lla_cmd.nobytes / IFBYTESCT);
		    continue;
		}
		cur_pri = spl6();
		time_id = timeout(ctimjob, ctjob, CTJOB_TIME);
		ct_assert(brd_id);

		while (ctjob->jobstat.bits.io_done != SET) {

		    if ((sleep((caddr_t) ctjob, sleepstate) > 0) &&
			(flag != FINISH)) {

			if (ctjob->jobstat.bits.io_done != 1 &&
				   bp->b_error != ETIMOUT) {

			    ctjob->jobstat.bits.err_code = EINTR;
			    sleep((caddr_t) ctjob, PZERO);
			    bp->b_flags |= (B_ERROR | B_STALE | B_AGE);
			    bp->b_error = EINTR;
			    TSTU_ERROR;
			    splx(cur_pri);
			    break;
			}
		    }

		    if (bp->b_error == ETIMOUT) {
			bp->b_flags |= (B_ERROR | B_STALE | B_AGE);
			TSTU_ERROR;

			if (flag == RESET_BLK || flag == UNLD_BLK) {
			    bp->b_resid = tot_bytes;
			}
			else {
			    bp->b_resid = tot_bytes + job_bytes;
			}
			splx(cur_pri);
			iodone(bp);
			return;
		    }
		}
		untimeout(time_id);
		splx(cur_pri);
					      /* Error detected in prev. job */

		if (ctjob->jobstat.bits.err_code > 0) {
		    flag = ERR_LUP;
		    break;
		}
	/*
	 * only modify blkno and vbuf_addr by SEG SIZE if a split job, else
	 * modify by real value so the 128K splitting will work
	 */

		if (ctjob->jobstat.bits.splitjob == SET) {
		    blkno += (unsigned long) CTSEG_BLKS;
		    vbuf_addr += (unsigned long) CTSEG_SIZE;
#ifdef DEBUG
		printf ("STRAT: SPLITJOB: blkno=%d vaddr=0x%x\n",
				blkno, vbuf_addr);
#endif
		}
		else {		       /* add left-overs less than CTSEG_SIZE */
		    vbuf_addr += job_bytes;
		    blkno += (job_bytes / IFBYTESCT);
		    job_bytes = 0;
#ifdef DEBUG
		printf ("STRAT: end of inner blkno=%d vaddr=0x%x\n",
				blkno, vbuf_addr);
#endif
		}
		ctjob->jobstat.lng_val &= CTCLRSBITS;

	    }

	}

	IOBUF(brd_id).qcnt--;
	IOTIME(brd_id, sub_dev).io_qc--;

	if (ctjob->jobstat.bits.err_code == CLEAR) {
	    IOTIME(brd_id, sub_dev).io_resp += (lbolt - bp->b_start);
	}
	else {

	    if (flag == RESET_BLK || flag == UNLD_BLK) {
		bp->b_resid = tot_bytes;
	    }
	    else {
		bp->b_resid = tot_bytes + job_bytes;
	    }
	}
	cur_pri = spl6();
	ctlnkwq(ctjob, UNLINKQ);
	ctrtqe(ctjob);
	splx(cur_pri);

	TSTU_ERROR;
	iodone(bp);
}

/*
 * bytes to the next page boundary
 */

#define pgbnd(a)	(NBPP - ((NBPP - 1) & (int)(a)))

ctcontig (proc, vaddr, maxcnt)
struct proc *proc;
register unsigned int vaddr;
int maxcnt;
{
	register int physaddr, nphysaddr;
	register int contig, bytes_left;

	if ((contig = pgbnd (vaddr)) >= maxcnt) {
		return(maxcnt);
	}

	bytes_left = maxcnt - contig;

	if ((physaddr = vtop (vaddr, proc)) == NULL) {
		cmn_err(CE_PANIC, "ctcontig: vtop failed");
	}
	vaddr += contig;

	if ((nphysaddr = vtop (vaddr, proc)) == NULL) {
		cmn_err(CE_PANIC, "ctcontig: vtop failed");
	}

	if ((physaddr + contig) != nphysaddr) {
		return(contig);
	}

		/*
		 * vaddr will now always point to the beginning of the next page
		 */

	while (contig < maxcnt) {
		physaddr = nphysaddr;

		if (bytes_left <= NBPP) {
			return(contig + bytes_left);
		}
		vaddr += NBPP;

		if ((nphysaddr = vtop (vaddr, proc)) == NULL) {
			cmn_err(CE_PANIC, "ctcontig: vtop failed");
		}

		if ((physaddr + NBPP) != nphysaddr) {
			return(contig);
		}
		bytes_left -= NBPP;
		contig += NBPP;
	}
		/* "this can't happen" */

	cmn_err(CE_PANIC, "ctcontig: vtop is insane");
}

/*
 * Name:        ctlnkwq()
 *
 * Abstract:    CTC Driver Link/Unlink Work Queue function.
 *
 * Use:         ctlnkwq(ctjob, lnktyp);
 *
 * Parameters:  ctjob = 
 *              lnktyp  = LINK2Q  = 1 = add to work list
 *                        UNLINKQ = 2 = remove from work list
 * Externals:
 *
 * Returns:     None.
 *
 * Description: This function is responsible for the linking and unlinking of
 *              jobs.  Jobs are linked via the bp->av_forw, bp->av_back and
 *              ct_board struct iobuf b_actf and b_actl variables.
 *
 * Calls:       cmn_err()
 *
 * Macros:      IOBUF(), IOTIME()
 */

ctlnkwq(ctjob, lnktyp)
register struct ctjobstat *ctjob;
register lnktyp;
{
	register struct buf *tempbp;
	register struct buf *fp, *rp;

	switch (lnktyp) {

	case LINK2Q:

	    if (IOBUF(CTJOB_BRDID).b_actf == NULL) {

			/*
			 * link buf to fwd list pointer, link buf to back list
			 * pointer, point buf back to list, point buf back to
			 * list head.
			 */

		IOBUF(CTJOB_BRDID).b_actf = (struct buf *) ctjob->procid;
		IOBUF(CTJOB_BRDID).b_actl = (struct buf *) ctjob->procid;
		((struct buf *) ctjob->procid)->av_back = 
				      (struct buf *) &IOBUF(CTJOB_BRDID);
	    }
	    else {	/*
			 * get addr of last buf in list, link buf to dev back
			 * list pointer, link buf in next to 1st buf fwd ptr,
			 * point to previous last buffer.
			 */

		tempbp = IOBUF(CTJOB_BRDID).b_actl;
		IOBUF(CTJOB_BRDID).b_actl = (struct buf *) ctjob->procid;
		tempbp->av_forw = (struct buf *) ctjob->procid;
		((struct buf *) ctjob->procid)->av_back = tempbp;
	    }
				    /* mark as last buf in device work list */
	    ((struct buf *) ctjob->procid)->av_forw = NULL;

		 /*
		  * Update job request, controller and drive status data, inc
		  * operations count, click count, jobs outstanding count and
		  * jobs outstanding for sub-device count
		  */

	    IOTIME(CTJOB_BRDID, CTJOB_SUBDEV).io_cnt++;
	    IOTIME(CTJOB_BRDID, CTJOB_SUBDEV).io_bcnt +=
			      btoc(((struct buf *) ctjob->procid)->b_bcount);
	    IOBUF(CTJOB_BRDID).qcnt++;
	    IOTIME(CTJOB_BRDID,  CTJOB_SUBDEV).io_qc++;
				    /* time stamp start of request process */
	    ((struct buf *) ctjob->procid)->b_start = lbolt;
	    break;

	case UNLINKQ:				/* Unlink job from work list */
	    fp = (struct buf *) ((struct buf *) ctjob->procid)->av_forw;
	    rp = (struct buf *) ((struct buf *) ctjob->procid)->av_back;

			       	       /* Check for only one job on work list */

	    if (fp == NULL &&
		rp == (struct buf *) &IOBUF(CTJOB_BRDID)) {

		IOBUF(CTJOB_BRDID).b_actf = NULL;
		IOBUF(CTJOB_BRDID).b_actl = NULL;

		if (IOBUF(CTJOB_BRDID).qcnt != NULL) {
		    cmn_err(CE_NOTE, "CTC driver queue count wrong on CTC %d!",
			CTJOB_BRDID);
		}
	    }				 /* Check for job at end of work list */

	    if (fp == NULL) {		 /* mark next to last as last on list */
		rp->av_forw = NULL;
		IOBUF(CTJOB_BRDID).b_actl = rp;
	    }
	    else {
		rp->av_forw = fp;	   /* link prev. fwd ptr to next buf. */
		fp->av_back = rp;	/* link nxt buf back ptr to prev. buf */
	    }
	    break;
	}
}

/*
 * Name:        ctsetup()
 *
 * Abstract:    Determine the proper FW command parameters based upon request
 *              data (i.e. type, size, block number, etc.).
 *
 * Use:         if (ctsetup(&(struct lla_rwcmd variable), part_no) != PASS) {
 *                     FAIL;
 *              }
 *
 * Parameters:  Pointer to structure type lla_rwcmd - see ct_lla.h
 *              part_no = partition number requested within sub-device.
 *
 * Externals:
 *
 * Returns:     PASS or FAIL as defined in ???.h.
 *
 * Description: Determine parameters for issuance of FW read/write command
 *              (ct_rdwrt()).  Load the lla_cmd structure for transfer to
 *              ct_rdwrt() lla function.
 *
 * Calls:       cmn_err()
 *
 * Macros:      VTOC(), PDSECT(), BPPTR, CT_VERIFY()
 */

#define	LLA_BRDID	lla_cmd->brd_id
#define	LLA_SUBDEV	lla_cmd->sub_dev

ctsetup(lla_cmd, bp, part_no, blkno, vbuf_addr)
register struct lla_rwcmd *lla_cmd;
struct buf *bp;
register char part_no;
register unsigned long blkno;
register unsigned long vbuf_addr;
{
	register unsigned long sectoff;

	if (LLA_SUBDEV == TAPE_DEV) {		 /* Determine tape parameters */
	    sectoff = (unsigned long)
	     VTOC(LLA_BRDID, LLA_SUBDEV).v_part[ part_no ].p_start +
			     PDSECT(LLA_BRDID, LLA_SUBDEV).pdinfo.logicalst;
						         /* Physical block no */
	    lla_cmd->blkno = sectoff + blkno;
	}
	else {					  /* Determine floppy param.s */
	    lla_cmd->blkno =
	        VTOC(LLA_BRDID, LLA_SUBDEV).v_part[ part_no ].p_start + blkno;
	}
	lla_cmd->bufaddr = vtop((caddr_t) vbuf_addr, bp->b_proc);

	if (lla_cmd->bufaddr == NULL) {
	    cmn_err(CE_WARN, "ctsetup: Bad vtop buf. addr. on CTC %d!",
		LLA_BRDID);
	    return(FAIL);
	}
	lla_cmd->utime = lbolt;				/* Time stamp the job */
	lla_cmd->opcode =
		((bp->b_flags & B_READ) == B_READ) ? CTREAD : CTWRITE;

	if (((CT_VERIFY(LLA_BRDID, LLA_SUBDEV) >> part_no) & SET) == SET) {
	    lla_cmd->opcode = CTVWRITE;
	}
#ifdef TEST
	printf("\nsu - %s", ((lla_cmd->opcode == CTREAD) ? "R" : "W"));

	printf(" FW: %d %d 0x%x",
		lla_cmd->blkno, lla_cmd->nobytes, lla_cmd->bufaddr);

	printf("\nUNIX: 0x%x %d 0x%x %d",
	      bp, bp->b_blkno, bp->b_flags, bp->b_bcount);
#endif
	return(PASS);
}

/*
 * Name:        ctint()
 *
 * Abstract:    CTC Driver Interrupt Handler.
 *
 * Use:         None - called by detection of interrupt from ctc board.
 *
 * Parameters:  brd_id = board number generating interrupt request to KERNEL.
 *
 * Externals:
 *
 * Returns:     None.
 *
 * Description:
 *
 * Calls:       ct_ccq(), cmn_err(), ctclr_brd(), wakeup(), ctlnkwq(),
 *              ctud_ctim()
 *
 * Macros:      BRD_FLGS(), SDEV_FLGS(), VTOC(), CTIMEUSE(),
 *              CHK_VERIFY()
 */

ctint(brd_id)
register int brd_id;
{
	register int cq_stat;		      /* return status of lla cq exam */
	CENTRY c_entry;	      		      /* completion queue entry read */
	register char sub_dev;
	register unsigned char lla_rcode;		   /* lla return code */
	register struct ctjobstat *ctjob;

		     /* Check for spurious completion interrupt via CQ status */

	cq_stat = ct_ccq(brd_id, &c_entry);

	if (cq_stat != PASS) {

	    switch (cq_stat) {
						 /* Completion queue is empty */
	    case EMPTY_Q:
		cmn_err(CE_WARN,"ctint: completion queue empty on CTC %d!\n",
		      brd_id);
		break;
					 /* Board never SYSGEN'd via ctpump() */
	    case UN_INIT:
		cmn_err(CE_WARN,"ctint: case not_init on CTC %d!\n", brd_id);
		BRD_FLGS(brd_id).bits.issysgen = CLEAR;
		break;

	    default:
		cmn_err(CE_WARN, "ctint: unknown value from cq_stat on CTC %d! (%d)\n",
				brd_id, cq_stat);
		break;
	    }
	    return;
	}

	lla_rcode = c_entry.common.codes.bytes.opcode;
	sub_dev = c_entry.common.codes.bits.subdev;

	if (lla_rcode == FAULT) {		     /* Check for fault first */
	    cmn_err(CE_WARN, "ctint: CTC %d faulted, taking off line!\n",
		brd_id);
	    ctclr_brd(brd_id);
	    return;
	}
#ifdef TEST
	if (BRD_FLGS(brd_id).bits.pumping != SET) {
	    printf("\nci - %d %d %d",
			 c_entry.common.codes.bytes.bytcnt, sub_dev, lla_rcode);

	    printf(" 0x%x 0x%x %d\n",
		     c_entry.common.addr, brd_id, c_entry.appl.command);
#ifdef SEEFUNCT
	    seestates();
#endif
	}
#endif
	if (lla_rcode == SYSGEN) {			  /* Check for SYSGEN */
	    BRD_FLGS(brd_id).bits.issysgen = SET;
	    BRD_FLGS(brd_id).bits.ciotype = CLEAR;

	    if (BRD_FLGS(brd_id).bits.isinit == SET) {
		BRD_FLGS(brd_id).bits.is_up = SET;
	    }
	    wakeup(&BRD_FLGS(brd_id).brd_state);
	    return;
	}
						/* Otherwise use appl.command */
	switch (c_entry.appl.command) {

	case CTWRITE:
	case CTVWRITE:
	case CTREAD:
	    ctjob = (struct ctjobstat *) c_entry.common.addr;

	    if (ctjob == NULL) {
		cmn_err(CE_WARN, "ctint: NULL ctjob on CTC %d! (R/W - %d)",
				brd_id, sub_dev);
		return;
	    }

	    if (ctjob->procid == NULL) {
		cmn_err(CE_WARN, "ctint: NULL ctjob->procid on CTC %d! (R/W - %d)",
				brd_id, sub_dev);
		return;
	    }
	    ctjob->jobstat.lng_val |= (CTIO_DONE | lla_rcode);
					      /* Check for failing job status */
	    if (ctjob->jobstat.bits.err_code != FWPASS) {
		ctdet_err(ctjob, c_entry.common.codes.bytes.bytcnt);
	    }
	    else if (ctjob->jobstat.bits.splitjob != 1) {
		((struct buf *) (ctjob->procid))->b_resid = 0;
	    }
	    wakeup((caddr_t) ctjob);
	    break;

	case CTOPEN:
	    ctjob = (struct ctjobstat *) c_entry.common.addr;

	    if (ctjob == NULL) {
		cmn_err(CE_WARN, "ctint: NULL ctjob on CTC %d! (Open - %d)",
				brd_id, sub_dev);
		return;
	    }
	    ctjob->jobstat.lng_val |= (CTIO_DONE | lla_rcode);
	    wakeup((caddr_t) ctjob);
	    break;

	case CTCLOSE:
	    ctjob = (struct ctjobstat *) c_entry.appl.jio_start;

	    if (ctjob == NULL) {
		cmn_err(CE_WARN, "ctint: NULL ctjob on CTC %d! (Close - %d)",
				brd_id, sub_dev);
		return;
	    }
	    ctjob->jobstat.lng_val |= (CTIO_DONE | lla_rcode);

				     /* Update controller pass count in nvram */
	    if (sub_dev == TAPE_DEV) {
		CTIMEUSE(brd_id) += c_entry.common.addr;
		ctud_ctim(brd_id);
	    }

	    if (BRD_FLGS(brd_id).bits.streaming == SET) {
	        BRD_FLGS(brd_id).bits.streaming = CLEAR;
	    }
	    wakeup((caddr_t) ctjob);
	    break;

	case CTFORMAT:
	    ctjob = (struct ctjobstat *) c_entry.appl.jio_start;

	    if (ctjob == NULL) {
		cmn_err(CE_WARN, "ctint: NULL ctjob on CTC %d! (Format - %d)",
				brd_id, sub_dev);
		return;
	    }
	    ctjob->jobstat.lng_val |= (CTIO_DONE | lla_rcode);

	    if (sub_dev == TAPE_DEV) {
		CTIMEUSE(brd_id) += c_entry.common.addr;
		ctud_ctim(brd_id);
	    }
	    wakeup((caddr_t) ctjob);
	    break;

	case DLM:			   	       /* CIO download memory */
	case FCF:			 	   /* CIO force function call */
	    BRD_FLGS(brd_id).bits.ciotype = CLEAR;
	    wakeup(&BRD_FLGS(brd_id).brd_state);
	    break;

	default:			     /* Undetectable interrupt reason */
	    cmn_err(CE_WARN, "ctint: unknown opcode(%d) on CTC %d!",
			c_entry.appl.command, brd_id);
	    break;
	}

	if (ct_board[ brd_id ].full_q > 0) {
	    wakeup(&ct_board[ brd_id ].full_q);
	}
}

/*
 * Name:        ctdet_err()
 *
 * Abstract:    Determine error type to report back to user via UNIX buffer
 *              pointer variables bp->b_error and bp->b_flags.
 *
 * Use:         Internal to driver only.
 *              Call is ctdet_err(ctjob, bytes);
 *
 * Parameters:  
 *              bytes = Number of bytes transferred in buffer before error.
 *
 * Externals:
 *
 * Returns:     Appropriate error report in bp->b_error and bp->b_flags var.s.
 *
 * Description: Error was detected in completion report received in ctint().
 *              This function determines the error type based upon those
 *              valid error conditions defined in the header file ct_dep.h and
 *              reports back to UNIX, via bp->b_error and bp->b_flags, the
 *              error conditions to be returned to the user as found in ct.h.
 *
 * Calls:       cmn_err().
 *
 * Macros:      None.
 */

ctdet_err(ctjob, bytes)
register struct ctjobstat *ctjob;
register int bytes;
{
	((struct buf *) ctjob->procid)->b_flags |= (B_ERROR | B_STALE | B_AGE);
	((struct buf *) ctjob->procid)->b_resid =
			((struct buf *) ctjob->procid)->b_bcount - bytes;

	switch (ctjob->jobstat.bits.err_code) {

	case INVDEVN:			      /* Invalid device no. specified */
		((struct buf *) ctjob->procid)->b_error = ENODEV;
		break;

	case NCONFDEV:			  /* Device not configured on ctc brd */
		((struct buf *) ctjob->procid)->b_error = ENOCONF;
		break;

	case RWERROR:					  /* Read/Write Error */
		((struct buf *) ctjob->procid)->b_error = ERWERR;
		cmn_err(CE_NOTE,
	      "CTC %d: Access Error: Consult the Error Message Section of the",
			ctjob->jobstat.bits.brd_id);
		cmn_err(CE_CONT,
		  "3B2 Computer Cartridge Tape Utilities Guide(error num=%d)\n",
			ERWERR);
		break;

	case HDWERROR:				/* Hardware Error encountered */
		((struct buf *) ctjob->procid)->b_error = EHDWERR;
		cmn_err(CE_NOTE,
	       "CTC %d: Access Error: Consult the Error Message Section of the",
			ctjob->jobstat.bits.brd_id);
		cmn_err(CE_CONT,
		  "3B2 Computer Cartridge Tape Utilities Guide(error num=%d)\n",
			EHDWERR);
		break;

	case RDONLY:				  /* Media is write protected */
		((struct buf *) ctjob->procid)->b_error = EWRTPRT;
		cmn_err(CE_NOTE,
	       "CTC %d: Access Error: Consult the Error Message Section of the",
			ctjob->jobstat.bits.brd_id);
		cmn_err(CE_CONT,
		  "3B2 Computer Cartridge Tape Utilities Guide(error num=%d)\n",
			EWRTPRT);
		break;

	case NOTRDY:			  /* Device wouldn't go ready in time */
		((struct buf *) ctjob->procid)->b_error = ENOTRDY;
		cmn_err(CE_NOTE,
	       "CTC %d: Access Error: Consult the Error Message Section of the",
			ctjob->jobstat.bits.brd_id);
		cmn_err(CE_CONT,
		  "3B2 Computer Cartridge Tape Utilities Guide(error num=%d)\n",
			ENOTRDY);
		break;

	case NOTOPN:					 /* Device not opened */
		((struct buf *) ctjob->procid)->b_error = ENOTOPN;
		break;

	case CTNOMEDIA:					  /* No media present */
		((struct buf *) ctjob->procid)->b_error = ENOMED;
		cmn_err(CE_NOTE,
	       "CTC %d: Access Error: Consult the Error Message Section of the",
			ctjob->jobstat.bits.brd_id);
		cmn_err(CE_CONT,
		  "3B2 Computer Cartridge Tape Utilities Guide(error num=%d)\n",
			ENOMED);
		break;

	default:
		((struct buf *) ctjob->procid)->b_error = EIO;
		break;
	}
}

/*
 * Name:        ctread()
 *
 * Abstract:    Character read from sub-device.
 *
 * Use:
 *
 * Parameters:  dev = device number of the device file.
 *
 * Externals:
 *
 * Returns:     Error number returned via user structure u.u_error if an error
 *              is encountered, otherwise, nothing.
 *
 * Description: This function allows referencing the sub-devices as character or
 *              raw devices.  I/O requests are usually made in non-block (512)
 *              sizes.  The function checks for a valid board number, checks the
 *              state of the board to make sure it is marked as up, and then
 *              makes sure that the sub-device has been opened.  It then
 *              checks to make sure that the request is found within the
 *              partition indicated via the KERNEL function physck().  If it is
 *              physio() is called which creates a buffer header and passes all
 *              of the I/O information to the ctstrategy() function.
 *
 * Calls:       ctchk_req(), physck(), physio()
 *
 * Macros:      minor(), CT_BRD(), SUB_DEV(), PARTNUM(), VTOC()
 */

ctread(dev)
dev_t dev;
{
	register int frtrn_stat;		/* function return status */
	register char min_dev = minor(dev);
	register char brd_id = CT_BRD(min_dev);
	register char sub_dev = SUB_DEV(min_dev);
	register char part_num = PARTNUM(min_dev);

	if ((frtrn_stat = ctchk_req(dev, brd_id, sub_dev)) > 0) {
		u.u_error = frtrn_stat;
		return;
	}

	if (physck(VTOC(brd_id, sub_dev).v_part[part_num].p_size, B_READ)) {
		physio(ctstrategy, 0, dev, B_READ);
	}
}

/*
 * Name:        ctwrite()
 *
 * Abstract:    Character write to sub-device.
 *
 * Use:
 *
 * Parameters:  dev = device number of the device file.
 *
 * Externals:
 *
 * Returns:     Error number returned via user structure u.u_error if an error
 *              is encountered, otherwise, nothing.
 *
 * Description: This function allows referencing the sub-devices as character or
 *              raw devices.  I/O requests are usually made in non-block (512)
 *              sizes.  The function checks for a valid board number, checks the
 *              state of the board to make sure it is marked as up, and then
 *              makes sure that the sub-device has been opened.  It then
 *              checks to make sure that the request is found within the
 *              partition indicated via the KERNEL function physck().  If it is
 *              physio() is called which creates a buffer header and passes all
 *              of the I/O information to the ctstrategy() function.
 *
 * Calls:       ctchk_req(), physck(), physio()
 *
 * Macros:      minor(), CT_BRD(), SUB_DEV(), PARTNUM(), VTOC()
 */

ctwrite(dev)
dev_t dev;
{
	register int frtrn_stat;		/* function return status */
	register char min_dev = minor(dev);
	register char brd_id = CT_BRD(min_dev);
	register char sub_dev = SUB_DEV(min_dev);
	register char part_num = PARTNUM(minor(dev));

	if ((frtrn_stat = ctchk_req(dev, brd_id, sub_dev)) > 0) {
		u.u_error = frtrn_stat;
		return;
	}

	if (physck(VTOC(brd_id, sub_dev).v_part[part_num].p_size, B_WRITE)) {
		physio(ctstrategy, 0, dev, B_WRITE);
	}
}

/*
 * Name:        ctprint()
 *
 * Abstract:    Driver print warning message routine.
 *
 * Use:
 *
 * Parameters:  dev = device number of the device file.
 *              str = character string pointer containing message to be
 *                    printed.
 *
 * Externals:
 *
 * Returns:     None.
 *
 * Description: This function is called by the KERNEL to transmit the indicated
 *              error condition found in the str variable argument.
 *
 * Calls:       cmn_err()
 *
 * Macros:      minor(), CT_BRD(), SUB_DEV(), PARTNUM()
 */

ctprint(dev, str)
dev_t dev;
char *str;
{
	short min_dev = minor(dev);

	cmn_err(CE_NOTE, "%s on ctc %d, %s %d\n",
		  str, CT_BRD(min_dev),
  ((SUB_DEV(min_dev) == TAPE_DEV) ? "tape partition" : "floppy disk slice"),
		  PARTNUM(min_dev));
}

/*
 * Name:        ctioctl()
 *
 * Abstract:    CT driver ioctl() function.
 *
 * Use:
 *
 * Parameters:  dev  = device number of the device file.
 *              cmd  = argument used by the function as the type of operation
 *                     to be performed.  See sys/ct.h.
 *              arg  = argument used to pass parameters between a user program
 *                     and the driver.
 *              mode = argument (not used) that contains values set when the
 *                     device was opened.
 *
 * Externals:
 *
 * Returns:     Error condition in the user structure u.u_error variable if an
 *              error is detected.  Otherwise, nothing.
 *
 * Description: This is the universal catch all for all of those special
 *              commands that can not be dealt with via calls to other driver
 *              functions and flag values.  It is used to facilitate the
 *              following operations:
 *               - formating media for the specified sub-device
 *               - telling CTC FW to verify each write command and data written
 *               - pumping the CTC FW onto the CTC hardware board
 *               - notifying CTC FW to attempt to stream all of its operations
 *               - reseting the CTC tape controller time in use maintenance
 *                 variable.
 *
 * Calls:       ctpmpctl(), copyout(), ct_format(), sleep(),
 *              ctud_ctim(), ctgtqe(), ctrtqe(), timeout(), untimeout(),
 *              spl6(), splx()
 *
 * Macros:      CT_BRD(), minor(), eminor(), SUB_DEV(), CT_VERIFY(), TIME_ID()
 *              CHK_OPEN(), VTOC(), PDSECT(), CTIMEUSE(), SDEV_FLGS()
 */

/*
 * Improve readability macros only
 */
#define	USER_VTOC	devdump->dev_vtoc
#define	SIZOFVTOC	sizeof(struct vtoc)
#define	USER_PDSC	devdump->dev_pdsector
#define	SIZOFPDSC	sizeof(struct pdsector)

ctioctl(dev, cmd, arg, mode)
dev_t dev;
int cmd;
int mode;
{
	register char    brd_id;
	register char    sub_dev;
	char             min_dev;
	char             part_num;
	int              cur_pri;
	int              time_id;
	struct dev_dump  *devdump;
	struct fmtstruct *fmtarg;

	register struct ctjobstat *ctjob;
	struct ctjobstat *ctgtqe();

	extern ctimjob();
	extern ctflg_tim();

	brd_id = CT_BRD(minor(dev));

	if (brd_id > (ctc_cnt-1) || eminor(dev) > CTMAX_MINOR) {
		u.u_error = ENXIO;
		return;
	}
					 /* These commands require real opens */
	if (cmd == PUMP) {

		if (BRD_FLGS(brd_id).bits.pumping != SET) {
			u.u_error = EPERM;
			return;
		}
		ctpmpctl((char *) arg);
		return;
	}
	min_dev = minor(dev);
	sub_dev = SUB_DEV(min_dev);
	part_num = PARTNUM(min_dev);

	if (cmd == DUMP) {
		devdump = (struct dev_dump *) arg;

		if (SDEV_FLGS(brd_id, sub_dev).bits.subopen != SET) {
			u.u_error = ENOTOPN;
			return;
		}

		if (sub_dev == TAPE_DEV) {

			if (SDEV_FLGS(brd_id, sub_dev).bits.vtoc_pres == SET) {
			    copyout(&VTOC(brd_id, sub_dev), &USER_VTOC, SIZOFVTOC);
			    copyout(&PDSECT(brd_id, sub_dev), &USER_PDSC, SIZOFPDSC);
			}

			if (PDSECT(brd_id, sub_dev).pdinfo.cyls == 6) {
				devdump->dev_type = CT_TAPE6;
			}
			else {
				devdump->dev_type = CT_TAPE12;
			}
			devdump->hd_cnt = CTIMEUSE(brd_id);
		}
		else {
			u.u_error = EINVAL;
		}
		return;
	}				  /* These commands req. special open */

	if (SDEV_FLGS(brd_id, sub_dev).bits.specl_opn != SET) {
		u.u_error = ENSLOPN;
		return;
	}
					   /* Switch on ct.h internal defines */
	switch (cmd) {

	case VERIFY:		     /* Turn on verify after every write mode */
		CT_VERIFY(brd_id, sub_dev) |= (1 << part_num);
		break;

	case FORMAT:			   /* Format new media for sub-device */
				  /* Only one sub-device active during format */
	case STREAMON:		     /* Big jobs comin thru ... lets lie back */
				         /* Only one person should be lied to */

		if (BRD_FLGS(brd_id).bits.streaming == SET ||
		    BRD_FLGS(brd_id).bits.opnpending == SET) {
			u.u_error = ENTSUSR;
			return;
		}

		if (cmd == STREAMON && sub_dev != TAPE_DEV) {
			u.u_error = EINVAL;
			return;
		}
						       /* Stream handled here */
		if (cmd == STREAMON) {
			BRD_FLGS(brd_id).bits.streaming = SET;
			cur_pri = spl6();
			TIME_ID(brd_id) = timeout(ctflg_tim, brd_id, CTFLG_TIME);
			splx(cur_pri);
			break;
		}
						       /* Format handled here */
		cur_pri = spl6();		      /* Block I/O interrupts */
						  /* Put job into work queues */
		while ((ctjob = ctgtqe(brd_id)) == NULL) {
			IOBUF(brd_id).jrqsleep++;

			if (sleep(&IOBUF(brd_id).jrqsleep, PCATCH | PSLEP) > 0) {
				IOBUF(brd_id).jrqsleep--;
				u.u_error = EINTR;
				splx(cur_pri);
				return;
			}
			IOBUF(brd_id).jrqsleep--;
		}
		splx(cur_pri);
		ctjob->jobstat.bits.opcode = CTFORMAT;
		ctjob->jobstat.bits.brd_id = brd_id;
		ctjob->jobstat.bits.sub_dev = sub_dev;
		fmtarg = (struct fmtstruct *) arg;

		if (ct_format(ctjob, fmtarg->passcnt, fmtarg->mode) != PASS) {
			u.u_error = EFWCBAD;
			cur_pri = spl6();
			ctrtqe(ctjob);
			splx(cur_pri);
			return;
		}
		cur_pri = spl6();
		time_id = timeout(ctimjob, ctjob, CTFMT_TIME);
		ct_assert(brd_id);
		sleep(ctjob, PZERO);			  /* Wait for FW int. */
		untimeout(time_id);
		splx(cur_pri);	     /* Return to previous int. level */
						   /* Check completion status */
		if (ctjob->jobstat.bits.err_code != FWPASS) {

			if (ctjob->jobstat.bits.err_code == ETIMOUT) {
				u.u_error = ETIMOUT;
			}
			else {
				u.u_error = EIO;
			}
		}
		cur_pri = spl6();
		ctrtqe(ctjob);
		splx(cur_pri);
		break;
	
	case GETPASS:			      /* Return controller pass count */
	case RSTPASS:		      /* Reset controller pass count variable */
		if (sub_dev != TAPE_DEV) {
			u.u_error = EINVAL;
		}
		else if (cmd == GETPASS) {
			*((int *) arg) = CTIMEUSE(brd_id);
		}
		else {
			CTIMEUSE(brd_id) = 0;
			ctud_ctim(brd_id);
		}
		break;

	default:					/* Unknow command ? */
		u.u_error = EINVAL;
		break;
	}
}

/*
 * Name:        ctpmpctl()
 *
 * Abstract:    Copy in/out pump data structure argument from ctioctl()
 *              pump request and call ctpump() routine.
 *              Point of routine is to use kernel memory space.
 *
 * Use:         Internal to driver only.  Call is ctpmpctl(arg);
 *
 * Parameters:  arg = user instance of struct pump_st.  See sys/pump.h.
 *
 * Externals:
 *
 * Returns:     Error condition in the user structure u.u_error variable if an
 *              error is detected, otherwise, nothing.
 *
 * Description: This routine copys in an instance of the user's struct pumpst.
 *              It then calls the ctpump() routine, performs the necessary
 *              operations according to the pumpst->cmdcode, and returns the
 *              result in the pumpst->retcode variable.  This routine then
 *              copys the result instance of the struct pumpst back into the
 *              user's instance of the struct pumpst.
 *
 * Calls:       copyin(), ctpump(), copyout()
 *
 * Macros:      None.
 */

ctpmpctl(arg)
char *arg;
{
	struct pump_st pump;

	if (copyin(arg, (char *) &pump, sizeof(struct pump_st))) {
		u.u_error = EFAULT;
		return;
	}

	ctpump(&pump);

	if (copyout((char *) &pump, arg, sizeof(struct pump_st))) {
		u.u_error = EFAULT;
		return;
	}
}

/*
 * Name:        ctpump()
 *
 * Abstract:    This function is called via a UNIX shell script which is
 *              run at power up.  It sets of the CTC hardware into a known
 *              state and then downloads the operational FW onto the CTC board.
 *
 * Use:         Internal to the driver only.  Call is ctpump();.
 *
 * Parameters:  Pointer to an instance of the structure struct pump_st.  See
 *              sys/pump.h.
 *
 * Externals:
 *
 * Returns:     Error condition in the struct pump_st->retcode variable as
 *              defined in sys/pump.h, otherwise, NORMAL as defined in sys/
 *              pump.h.
 *
 * Description: This function is currently invoked by a shell script at the
 *              time that UNIX is brought up.  The current pump() function
 *              issues:
 *               - a request to reset the board (PU_RST) [puts board in a
 *                 known state].
 *               - a request to sysgen the board (PU_SYSGEN) [establishes a
 *                 comm. queue with board's ROM based FW].
 *               - a request to download the operational RAM FW (PU_DLD).
 *               - a force function call (PU_FCF) which starts the operational
 *                 RAM FW.
 *               - a request to sysgen the board (PU_SYSGEN) [establishes a
 *                 comm. queue with the board's operational RAM FW].
 *
 * Calls:       vtop(), cmn_err(), copyin(), spl6(), ct_dld(), splx(),
 *              timeout(), sleep(), untimeout(), getvec(), ct_reset(),
 *              ctsysgen(), ct_fcf()
 *
 * Macros:      CT_BRD(), BRD_FLGS(), TIME_ID()
 */

ctpump(pmpr)
struct pump_st *pmpr;
{
	register int slices;	/* No. of 256 (PU_BLOCK) byte memory slices */
	register long usr_mem;	/* User address pointer pointing to the
				   user memory to be moved to kernel space */
	register long ctaddr;	/* ct address for download */
	register int i;
	register long bsize;	/* block size to be given to CTC */
	register long rem_size;	/* (buffer size) % 256 */
	register int cur_pri;	/* current processor interrupt level */
	unsigned char vector;
	paddr_t msbuffer;	/* Physical address of ctpmpdata */

	extern ctcbad();

	char brd_id = CT_BRD(minor(pmpr->dev));

	switch (pmpr->cmdcode) {
				/* Determine number of equipped slots */
	case PU_EQUIP:
		pmpr->numbrd = ctc_cnt;
		pmpr->retcode = NORMAL;
		return;
						/* download case */
	case PU_DLD:
		if (brd_id > (ctc_cnt-1)) {
			pmpr->retcode = PU_DEVCH;
			return;
		}

		slices = pmpr->size / PU_BLOCK;

		if (slices < 0x3f && (rem_size = pmpr->size % PU_BLOCK) != 0) {
			slices++;
		}
		else if (slices > 0x3f) {
			slices = 0x3f;
			rem_size = 0;
		}
		usr_mem = pmpr->bufaddr;
		ctaddr = pmpr->to_addr;

		if ((msbuffer = vtop(ctpmpdata, NULL)) == NULL) {
			cmn_err(CE_WARN, "CTC %d: Bad vtop on ctpmpdata - %x\n",
				brd_id, ctpmpdata);
			pmpr->retcode = PU_OTHER;
			return;
		}

			/* NOTE: !!!!!!!!!
			 * pump downloads bss as well, which fowls up the
			 * existing CTC PROM bss area; therefore, we WILL
			 * NOT download anything greater than 64K of pump
			 * code.
			 */

		for (i = 1; i <= slices; i++) {

			if ((i == slices) && (rem_size != 0)) {
				bsize = rem_size;
			}
			else {
				bsize = PU_BLOCK;
			}

			if (ctaddr >= SIXTY4_K) {	/* See above note */
				break;
			}
			if (copyin((char *)usr_mem, (char *)ctpmpdata, bsize)) {
				u.u_error = EFAULT;
				cmn_err(CE_WARN,
				    "CTC %d: Pump dld copyin failed! (%x,%x)\n",
					brd_id, ctpmpdata, bsize);
				pmpr->retcode = PU_OTHER;
				return;
			}

			if (BRD_FLGS(brd_id).bits.ciotype == SET) {
				pmpr->retcode = PU_OTHER;
				return;		/* CIO command already in process */
			}

			BRD_FLGS(brd_id).bits.ciotype = SET;
			cur_pri = spl6();

			if (ct_dld(brd_id, msbuffer, bsize, ctaddr) != PASS) {
				splx(cur_pri);
				BRD_FLGS(brd_id).bits.ciotype = CLEAR;
				cmn_err(CE_WARN, "CTC %d: Pump dld call failed! (%x,%x,%x)\n",
					brd_id, msbuffer, bsize, ctaddr);
				pmpr->retcode = PU_OTHER;
				return;
			}
			else {
				TIME_ID(brd_id) = timeout(ctcbad, brd_id, CTCIO_TIME);
				sleep((caddr_t) &BRD_FLGS(brd_id).brd_state, PZERO);
			}

			if (BRD_FLGS(brd_id).bits.ciotype == SET) {
				BRD_FLGS(brd_id).bits.ciotype = CLEAR;
				pmpr->retcode = PU_TIMEOUT;
				splx(cur_pri);
				return;
			}
			else {
				untimeout(TIME_ID(brd_id));
			}
			splx(cur_pri);
			usr_mem += PU_BLOCK;
			ctaddr += PU_BLOCK;
		}
		break;
							/* Reset CTC case */
	case PU_RST:
		if (brd_id > (ctc_cnt-1)) {
			pmpr->retcode = PU_DEVCH;
			return;
		}

		vector = getvec(ct_addr[brd_id]);
		cur_pri = spl6();
		BRD_FLGS(brd_id).bits.issysgen = CLEAR;
		ct_reset(brd_id);

		for (i = 0; i < 50000; i++) ;		/* Wait for CT Reset */

		if (BRD_FLGS(brd_id).bits.ciotype == SET) {
			pmpr->retcode = PU_OTHER;
			return;			/* CIO command already in process */
		}
		BRD_FLGS(brd_id).bits.ciotype = SET;

		if (ctsysgen(brd_id, vector) != PASS) {
			splx(cur_pri);
			BRD_FLGS(brd_id).bits.ciotype  = CLEAR;
			pmpr->retcode = PU_OTHER;
			return;
		}
		else {
			TIME_ID(brd_id) = timeout(ctcbad, brd_id, CTSG_TIME);
			sleep((caddr_t) &BRD_FLGS(brd_id).brd_state, PZERO);
		}

		if (BRD_FLGS(brd_id).bits.issysgen != SET) {
			splx(cur_pri);
			BRD_FLGS(brd_id).bits.ciotype = CLEAR;
			pmpr->retcode = PU_TIMEOUT;
			return;
		}
		else {
			untimeout(TIME_ID(brd_id));
			splx(cur_pri);
		}

		break;
						/* Force Call to Function - */
	case PU_FCF:				/* Jump to location to_addr */
		if (brd_id > (ctc_cnt-1)) {
			pmpr->retcode = PU_DEVCH;
			return;
		}

		cur_pri = spl6();

		if (BRD_FLGS(brd_id).bits.ciotype == SET) {
			splx(cur_pri);
			pmpr->retcode = PU_OTHER;
			return;			/* CIO command already in process */
		}
		BRD_FLGS(brd_id).bits.ciotype = SET;

		if (ct_fcf(brd_id, pmpr->to_addr) != PASS) {
			BRD_FLGS(brd_id).bits.ciotype = CLEAR;
			splx(cur_pri);
			pmpr->retcode = PU_OTHER;
			return;
		}
		else {
			TIME_ID(brd_id) = timeout(ctcbad, brd_id, CTCIO_TIME);
			sleep(&BRD_FLGS(brd_id).brd_state, PZERO);
		}

		if (BRD_FLGS(brd_id).bits.ciotype == SET) {
			splx(cur_pri);
			BRD_FLGS(brd_id).bits.ciotype = CLEAR;
			pmpr->retcode = PU_TIMEOUT;
			return;
		}
		else {
			untimeout(TIME_ID(brd_id));
		}
		splx(cur_pri);
		break;
						/* SYSGEN the CTC case */
	case PU_SYSGEN:
		if (brd_id > (ctc_cnt-1)) {
			pmpr->retcode = PU_DEVCH;
			return;
		}
		vector = getvec(ct_addr[brd_id]);
		cur_pri = spl6();
		BRD_FLGS(brd_id).bits.issysgen = CLEAR;

		if (BRD_FLGS(brd_id).bits.ciotype  == SET) {
			splx(cur_pri);
			pmpr->retcode = PU_OTHER;
			return;
		}
		BRD_FLGS(brd_id).bits.ciotype = SET;

		if (ctsysgen(brd_id, vector) != PASS) {
			splx(cur_pri);
			BRD_FLGS(brd_id).bits.ciotype = CLEAR;
			pmpr->retcode = PU_OTHER;
			return;
		}
		else {
			TIME_ID(brd_id) = timeout(ctcbad, brd_id, CTSG_TIME);
			sleep((caddr_t) &BRD_FLGS(brd_id).brd_state, PZERO);
		}

		if (BRD_FLGS(brd_id).bits.issysgen != SET) {
			splx(cur_pri);
			BRD_FLGS(brd_id).bits.ciotype = CLEAR;
			pmpr->retcode = PU_TIMEOUT;
			return;
		}
		else {
			untimeout(TIME_ID(brd_id));
			splx(cur_pri);
			pmpr->retcode = SYSGEN;
			return;
		}

	default:
		pmpr->retcode = PU_OTHER;
		cmn_err(CE_WARN, "CTC %d: Unknown pump command: %d\n",
			brd_id, pmpr->cmdcode);
		return;
	}

	pmpr->retcode = NORMAL;
}

/*
 * Name:        ctstrt_ques()
 *
 * Abstract:    Initialize the various queues used in this driver.
 *
 * Use:         ctstrt_ques(brd_id);
 *
 * Parameters:  brd_id = controller board number in 3b2 system.
 *
 * Externals:
 *
 * Returns:     None.
 *
 * Description: 
 *
 * Calls:       vtop()
 *
 * Macros:      CTJOBQ_FREEHDR(), CTJOBQ_NODE(), CTJOBQ_ACTVHDR(), IOBUF()
 */

ctstrt_ques(brd_id)
register char brd_id;
{
	register int i;
	register struct ctjobstat *tmpptr;

			       /* Not a UNIX reboot .. Clear all pending jobs */

	if (BRD_FLGS(brd_id).bits.isinit == SET) {

		tmpptr = CTJOBQ_ACTVHDR(brd_id).f_link;

		while (tmpptr != NULL) {
							 /* Pending job found */
			if (tmpptr->jobstat.bits.io_done == CLEAR) {
				tmpptr->jobstat.bits.err_code = EIO;
				wakeup((caddr_t) tmpptr);
			}
			tmpptr = tmpptr->f_link;	    /* Go to next job */
		}
	}
	tmpptr = &CTJOBQ_FREEHDR(brd_id);

	for (i = 0; i < (RQSIZE*2); i++) {
		CTJOBQ_NODE(brd_id, i).b_link = tmpptr;
		CTJOBQ_NODE(brd_id, i).f_link = &CTJOBQ_NODE(brd_id, (i+1));
		tmpptr = &CTJOBQ_NODE(brd_id, i);
	}					   /* NULL end last job entry */
	CTJOBQ_NODE(brd_id, ((RQSIZE*2)-1)).f_link = NULL;

	CTJOBQ_FREEHDR(brd_id).f_link = ct_board[ brd_id ].ctjobs;
	CTJOBQ_FREEHDR(brd_id).b_link = NULL;

	CTJOBQ_ACTVHDR(brd_id).f_link = CTJOBQ_ACTVHDR(brd_id).b_link = NULL;

	BRD_FLGS(brd_id).bits.streaming = CLEAR;    /* Init.stream req. flg */

	IOBUF(brd_id).io_addr = vtop(ct_addr[ brd_id ], NULL);
	IOBUF(brd_id).io_nreg = 0;	       /* No. of regs to log on error */
	IOBUF(brd_id).jrqsleep = 0;		 /* init sleep counter on JRQ */
	IOBUF(brd_id).io_start = lbolt;	       /* time CTC intialized */
	IOBUF(brd_id).b_actf = NULL;			  /* Nothing in queue */
	IOBUF(brd_id).b_actl = NULL;
	IOBUF(brd_id).qcnt = NULL;		       /* No outstanding jobs */

}

/*
 * Name:        ctchk_req()
 *
 * Abstract:    Check the request for correct parameters.
 *
 * Use:         if (ctchk_req(dev, brd_id, sub_dev) > 0) { error }
 *
 * Parameters:  dev     = device number of the device file.
 *              brd_id  = board number (0-8).
 *              sub_dev = sub-device number (0-1).
 *
 * Externals:
 *
 * Returns:     0 if request is ok, otherwise, error condition (see ct.h).
 *
 * Description:
 *
 * Calls:       None.
 *
 * Macros:      eminor()
 */

ctchk_req(dev, brd_id, sub_dev)
register dev_t dev;
register char brd_id, sub_dev;
{
	if (brd_id > (ctc_cnt-1) || eminor(dev) > CTMAX_MINOR) {
		return(ENXIO);
	}
	else if (BRD_FLGS(brd_id).bits.is_up != SET) {
		return(EBRDDWN);
	}
	else if (SDEV_FLGS(brd_id, sub_dev).bits.subopen != SET) {
		return(ENOTOPN);
	}
	else {
		return(0);
	}
}

/*
 * Name:        ctud_ctim()
 *
 * Abstract:    Update tape drive time in use variable stored in NVRAM.
 *
 * Use:         Internal to driver only.  Call is ctud_ctim();.
 *
 * Parameters:  brd_id = board number (0-8).
 *
 * Externals:
 *
 * Returns:     None.
 *
 * Description: This function is used to update the alloted NVRAM location
 *              with the new time in use amount and it performs a check to
 *              if the new result has exceeded the maximum amount of time
 *              since the last maintenance was performed on the tape drive
 *              heads.  The usage of the cartridge tape controller is
 *              measured in 25ms intervals.  Once the value has reached
 *              MACPASS (~20 Hrs. of operational time), a preventive
 *              maintenance message is issued to the system console each time
 *              this function is called.  The SA should perform a ioctl() call
 *              to reset the controller pass count once maintenance has been
 *              performed.
 *
 * Calls:       wnram(), cmn_err()
 *
 * Macros:      CTIMEUSE(), UNX_NVR
 */

ctud_ctim(brd_id)
register char brd_id;
{
	if (wnvram(&CTIMEUSE(brd_id), &UNX_NVR->ioslotinfo[ ctslotno[ brd_id ] ],
		     sizeof(CTIMEUSE(brd_id))) != PASS) {
		cmn_err(CE_WARN,
	"ctud_ctim: Can't update NVRAM controller pass count on CTC %d\n",
			brd_id);
	}
		/* See if time to clean controller */

	if (CTIMEUSE(brd_id) >= MAXCPAS) {
		cmn_err(CE_NOTE,
	"Schedule preventive maintenance for CTC %d -- CLEAN TAPE DRIVE.",
			brd_id);
		cmn_err(CE_CONT,
		"Failure to clean the tape drive will lead to data loss.\n");
	}
}

/*
 * Name:        ctchk_tuse()
 *
 * Abstract:    Check the total amount of passes accumulated on this tape
 *              vs. the maximum amount of passes allowed before the tape
 *              should be replaced.
 *
 * Use:         Internal to the driver.  Call is ctchk_tuse();.
 *
 * Parameters:  brd_id  = board number (0-8)
 *
 * Externals:
 *
 * Returns:     None.
 *
 * Description: See the abstract.  This function is called to alert the
 *              user that the useful life of the tape has expired.
 *
 * Calls:       cmn_err()
 *
 * Macros:      PDSECT()
 */

ctchk_tuse(ctjob)
register struct ctjobstat *ctjob;
{
	ct_board[ CTJOB_BRDID ].tapepas =
		PDSECT(CTJOB_BRDID, CTJOB_SUBDEV).devsp[CTPASS];

	if (ct_board[ CTJOB_BRDID ].tapepas >= PDSECT(CTJOB_BRDID, CTJOB_SUBDEV).devsp[MTPASS]) {
		cmn_err(CE_NOTE,"The cartridge tape in drive %d is wearing out.",
			CTJOB_BRDID);
		cmn_err(CE_CONT,"Please replace it as soon as possible.");
		cmn_err(CE_CONT,"\nIt has an estimated life of 2 more backups.");
		cmn_err(CE_CONT,"\nFailure to replace the tape cartridge will");
		cmn_err(CE_CONT," lead to data loss.\n");
	}
}

/*
 * Name:        ctimjob()
 *
 * Abstract:    This function is called by the KERNEL when a call to timeout()
 *              has expired from a ctopen(), ctclose(), ctstrategy(), and/or
 *              ctioctl() has been performed.  It's principle purpose is to
 *              protect against insane firmware on the controller board.
 *
 * Use:         Internal to the driver.  Call is performed using the KERNEL
 *              function time_id = timeout(ctimjob, ctjob, TIME_OUT_VALUE).
 *
 * Parameters:  ctjob = a local job queue entry from board doubly-linked
 *              job queue entry list.
 *
 * Externals:
 *
 * Returns:     None.
 *
 * Description:
 *
 * Calls:       wakeup(), cmn_err(), ctclr_brd()
 *
 * Macros:      VTOC(), SDEV_FLGS()
 */

ctimjob(ctjob)
register struct ctjobstat *ctjob;
{
	int errflg = 0;

	switch (ctjob->jobstat.bits.opcode) {

	case CTOPEN:
	case CTFORMAT:
		break;

	case CTCLOSE:

		if (BRD_FLGS(CTJOB_BRDID).bits.streaming == SET) {
			BRD_FLGS(CTJOB_BRDID).bits.streaming = CLEAR;
		}
		break;
	
	case CTREAD:
		if (ctjob->procid != NULL) {

			((struct buf *) (ctjob->procid))->b_flags =
						(B_ERROR | B_STALE | B_AGE);
			((struct buf *) (ctjob->procid))->b_error = ETIMOUT;
		}
		break;
	default:
		errflg = 1;
		break;
	}
	ctjob->jobstat.bits.err_code = ETIMOUT;
	ctjob->jobstat.bits.io_done = SET;	/* This job timed out ! */
	wakeup((caddr_t) ctjob);
					   /* Check for prev. timeouts/faults */
					      /* Board wouldn't be up if true */
	if (BRD_FLGS(CTJOB_BRDID).bits.is_up == SET) {
		cmn_err(CE_WARN,
		     "ctimjob: CTC %d is insane, taking off line! (0x%x, %d)\n",
			CTJOB_BRDID, ctjob, ctjob->jobstat.bits.opcode);

		if (errflg > 0) {
			cmn_err(CE_CONT,
			     "unable to determine opcode on timeout failure!");
		}
		ctclr_brd(CTJOB_BRDID);
	}
}

/*
 * Name:        ctflg_tim()
 *
 * Abstract:    This function is called by the KERNEL when a call to timeout()
 *              has expired.  Specifically, this will happen if a user has, via
 *              an ctioctl() set the stream request bit for the sub-device and
 *              has failed to use it (detected by an ctopen() request).
 *
 * Use:         Internal to the driver.  Call is performed using the KERNEL
 *              function time_id = timeout(ctflg_tim, brd_id, TIME_OUT_VALUE).
 *
 * Parameters:  brd_id = board number (0-8).
 *
 * Externals:
 *
 * Returns:     None.
 *
 * Description: User has failed to use the specified sub-device within the
 *              alloted time value (detected by an absense of an ctopen()
 *              request within the time specified) so the stream request flag
 *              is reset so that further access to the board is not blocked.
 *
 * Calls:       None.
 *
 * Macros:      None.
 */

ctflg_tim(brd_id)
char brd_id;
{
	BRD_FLGS(brd_id).bits.streaming = CLEAR;
}

/*
 * Name:        ctcbad()
 *
 * Abstract:    This function is called by the KERNEL when a call to timeout()
 *              has expired.
 *
 * Use:         Internal to the driver.  Call is performed using the KERNEL
 *              function time_id = timeout(ctcbad, brd_id, TIME_OUT_VALUE);.
 *
 * Parameters:  brd_id = board number (0-8).
 *
 * Externals:
 *
 * Returns:     None.
 *
 * Description: See abstract.  This function is used to detect when a common
 *              I/O command has not responded back to the driver within the
 *              allotted amount of time, as defined in the CIOTIME #define.
 *              See sys/ct_lla.h.
 *
 * Calls:       wakeup()
 *
 * Macros:      BRD_FLGS()
 */

ctcbad(brd_id)
char brd_id;
{
	wakeup(&BRD_FLGS(brd_id).brd_state);
}

/*
 * Name:        ctgtqe()
 *
 * Abstract:    Return the next free job entry queue pointer from the
 *              drivers free job queue entry doubly-linked list.
 *
 * Use:         struct ctjobstat *centry = ctgtqe(brd_id);
 *
 * Parameters:  brd_id = board number (0-8).
 *
 * Externals:
 *
 * Returns:	NULL pointer for exhausted queue, otherwise next free entry.
 *
 * Description:
 *
 * Calls:       sleep(), wakeup()
 *
 * Macros:      CTJOBQ_BUSY(), CTJOBQ_FREEHDR(), CTJOBQ_WANTED(), 
 *              CTJOBQ_ACTVHDR()
 */

struct ctjobstat *
ctgtqe(brd_id)
register char brd_id;
{
	register struct ctjobstat *freentry;
	int j = 0;

	while (CTJOBQ_BUSY(brd_id) > 0) {
		CTJOBQ_WANTED(brd_id)++;

		sleep(&CTJOBQ_FREEHDR(brd_id), PZERO);

		CTJOBQ_WANTED(brd_id)--;
	}
	CTJOBQ_BUSY(brd_id)++;

	if (CTJOBQ_FREEHDR(brd_id).f_link == NULL) {
		CTJOBQ_BUSY(brd_id) = 0;

		if (CTJOBQ_WANTED(brd_id) > 0) {
			wakeup(&CTJOBQ_FREEHDR(brd_id));
		}
		return(NULL);
	}
	else {
		freentry = CTJOBQ_FREEHDR(brd_id).f_link;
		CTJOBQ_FREEHDR(brd_id).f_link = freentry->f_link;

		if (freentry->f_link != NULL) {
			(freentry->f_link)->b_link = &CTJOBQ_FREEHDR(brd_id);
		}
		freentry->b_link = &CTJOBQ_ACTVHDR(brd_id);
		freentry->f_link = CTJOBQ_ACTVHDR(brd_id).f_link;
		CTJOBQ_ACTVHDR(brd_id).f_link = freentry;

		if (freentry->f_link != NULL) {
			(freentry->f_link)->b_link = freentry;
		}
		freentry->procid = NULL;	     /* Clear prev. use stuff */
		freentry->jobstat.bits.opcode = 0;
		freentry->jobstat.bits.brd_id = 0;
		freentry->jobstat.bits.sub_dev = 0;
		freentry->jobstat.lng_val &= CTCLRSBITS;

		CTJOBQ_BUSY(brd_id) = 0;

		if (CTJOBQ_WANTED(brd_id) > 0) {
			wakeup(&CTJOBQ_FREEHDR(brd_id));
		}
		return(freentry);
	}
}

/*
 * Name:        ctrtqe()
 *
 * Abstract:    Return a used job queue entry node to the job queue entry
 *              free list.
 *
 * Use:         ctrtqe(ctjob);
 *
 * Parameters:  ctjob = used job entry node to be added to free list.
 *
 * Externals:
 *
 * Returns:     None.
 *
 * Description:
 *
 * Calls:       sleep(), wakeup()
 *
 * Macros:      CTJOBQ_BUSY(), CTJOBQ_WANTED(), CTJOBQ_FREEHDR(), IOBUF()
 */

ctrtqe(ctjob)
register struct ctjobstat *ctjob;
{
	int j = 0;

	while (CTJOBQ_BUSY(CTJOB_BRDID) > 0) {
		CTJOBQ_WANTED(CTJOB_BRDID)++;

		sleep(&CTJOBQ_FREEHDR(CTJOB_BRDID), PZERO);

		CTJOBQ_WANTED(CTJOB_BRDID)--;
	}
	CTJOBQ_BUSY(CTJOB_BRDID)++;

	(ctjob->b_link)->f_link = ctjob->f_link;

	if (ctjob->f_link != NULL) {
		(ctjob->f_link)->b_link = ctjob->b_link;
	}
	ctjob->f_link = CTJOBQ_FREEHDR(CTJOB_BRDID).f_link;
	ctjob->b_link = &CTJOBQ_FREEHDR(CTJOB_BRDID);

	if (CTJOBQ_FREEHDR(CTJOB_BRDID).f_link != NULL) {
		(CTJOBQ_FREEHDR(CTJOB_BRDID).f_link)->b_link = ctjob;
	}
	CTJOBQ_FREEHDR(CTJOB_BRDID).f_link = ctjob;
	CTJOBQ_BUSY(CTJOB_BRDID) = 0;
					    /* Wake any pending access sleeps */
	if (CTJOBQ_WANTED(CTJOB_BRDID) > 0) {
		wakeup(&CTJOBQ_FREEHDR(CTJOB_BRDID));
	}
				     /* Wake any pending request entry sleeps */
	if (IOBUF(CTJOB_BRDID).jrqsleep > 0) {
		wakeup(&IOBUF(CTJOB_BRDID).jrqsleep);
	}
}

/*
 * Name:        ctclr_brd()
 *
 * Abstract:    Reset the board, clear all possible sub-device state flags,
 *              and re-initialize all queues.
 *
 * Use:         ctclr_brd(brd_id);
 *
 * Parameters:  brd_id = board number (0-8).
 *
 * Externals:
 *
 * Returns:     None.
 *
 * Description:
 *
 * Calls:       ct_reset(), ctstrt_ques()
 *
 * Macros:      BRD_FLGS(), SDEV_FLGS()
 */

ctclr_brd(brd_id)
register char brd_id;
{
	register int i;

#ifndef FWDEBUG
	ct_reset(brd_id);
#endif
			/* Only bits left set are ISINIT & OPN_SLEEP */

	BRD_FLGS(brd_id).brd_state &= (ISINIT | OPN_SLEEP);
	ctlla_state[ brd_id ] = 0;

	for (i = 0; i < NUM_DEVS; i++) {
		SDEV_FLGS(brd_id, i).sub_state &= ~(SUBOPEN | SPECL_OPN);

		if (i == TAPE_DEV) {
			SDEV_FLGS(brd_id, i).bits.vtoc_pres = CLEAR;
		}
	}
						/* Clear all open type counts */
	for (i = 0; i < OTYPCNT; i++) {
		CT_OTYP(brd_id, TAPE_DEV)[i] = 0;
		CT_OTYP(brd_id, FLPY_DEV)[i] = 0;
	}
	ctstrt_ques(brd_id);				/* Re-init all queues */
}

/*
 * Name:        ctsingusr()
 *
 * Abstract:    Check to insure this request is only user active on board.
 *
 * Use:         if (ctsingusr(brd_id) == ENTSUSR) {
 * 			Not single user on board.
 *              }
 *
 * Parameters:  brd_id = board number (0-8).
 *
 * Externals:
 *
 * Returns:     None.
 *
 * Description:
 *
 * Calls:       None.
 *
 * Macros:      CT_OTYP(), SDEV_FLGS()
 */

ctsingusr(brd_id)
register char brd_id;
{
	register int i, osum;

	for (osum = i = 0; i < OTYPCNT; osum |= CT_OTYP(brd_id, TAPE_DEV)[i++])
		;
	
	for (i = 0; i < OTYPCNT; osum |= CT_OTYP(brd_id, FLPY_DEV)[i++])
		;

	if (osum > 0) {
		return(ENTSUSR);
	}

	if (SDEV_FLGS(brd_id, TAPE_DEV).bits.specl_opn == SET) {
		return(ENTSUSR);
	}
	else if (SDEV_FLGS(brd_id, FLPY_DEV).bits.specl_opn == SET) {
		return(ENTSUSR);
	}
	else if (BRD_FLGS(brd_id).bits.pumping == SET) {
		return(ENTSUSR);
	}
	else {
		return(0);
	}
}

#ifdef	SEEFUNCT
seestates()
{
	char brd;
	char sub;
	char otyp;

	for (brd = 0; brd < ctc_cnt; brd++) {
	    printf("BS: 0x%x SS: 0-0x%x 1-0x%x ",
		ct_board[ brd ].brd_flgs.brd_state,
		ct_board[ brd ].ct_sub[ TAPE_DEV ].sdev_flgs.sub_state,
		ct_board[ brd ].ct_sub[ FLPY_DEV ].sdev_flgs.sub_state);
	}
	printf("\n");

	for (brd = 0; brd < ctc_cnt; brd++) {
		for (sub = 0; sub < NUM_DEVS; sub++) {
			for (otyp = 0; otyp < OTYPCNT; otyp++) {
				printf("[%d] = 0x%x ",
					otyp, CT_OTYP(brd, sub)[otyp]);
			}
			printf("\n");
		}
	}
	printf("\n");
}
#endif
