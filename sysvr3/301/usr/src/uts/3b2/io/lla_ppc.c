/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:io/lla_ppc.c	10.7"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/errno.h"
#include "sys/signal.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/fs/s5dir.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/tty.h"
#include "sys/pp_dep.h"
#include "sys/queue.h"
#include "sys/cmn_err.h"
#include "sys/cio_defs.h"
#include "sys/ppc_lla.h"
#include "sys/lla_ppc.h"
#include "sys/ppc.h"
#include "sys/inline.h"
#ifdef STANDALONE
#include "sys/firmware.h"


#define printf	PRINTF
#endif


/*
 * 3B2 PPC Kernel Driver (PORTS) LLA Interface Functions
 */


#define ENTRY	0x01		/* compile-time debug flags	*/
#define EXIT	0x02
#define RDUMP	0x04
#define CDUMP	0x08
#define INTER	0x10
#define VTOP	0x20

#define SIXTEENK	(16*1024)

#define IS_CEXPRESS(b)	(E_SEQBIT(CQEXPRESS(b)) == csbit[b])

extern int csbit[];		/* express entry sequence bit	*/
extern int maxsavexp;		/* Maximum number of express entries for savee */
extern RENTRY savee[];

static int eload = 0, eunload = 0;


#ifndef STANDALONE
#define cbvtop(c)	(c + pminusv)
#define cbptov(c)	(c - pminusv)

static int pminusv = 0;

extern long vtop();
extern struct cblock *cfree;
#endif


static void lla_express();
static int lla_regular();

#ifdef DEBUG
int ll_debug = DEBUG;

static void dmp_entry();
#endif

extern char *pp_addr[];

int
lla_sysgen(bid, sblk)
register short bid;
register SG_DBLK *sblk;
{
	register int i;
	extern unsigned char getvec();

#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: lla_sysgen(%x,%x)\n", bid, sblk);
#endif

#ifndef STANDALONE
/*
 * Compute the value used in converting virtual addresses to physical
 * addresses
 *
 * This transformation is done through the use of local macros or a
 * system function:
 *	1. cbvtop : Used only when processing cblocks for ppc_option(),
 *	   cbptov	ppc_xmit(), and ppc_recv()
 *	2. vtop	  : Used for all other mainstore addresses
 *
 * Internal macros are used for two reasons:
 *	1. Speed
 *	2. The unavailability of a ptov() function
 */

	pminusv = ((char *) vtop(cfree, NULL)) - ((char *) cfree);

#ifdef DEBUG
if(ll_debug & VTOP)
	printf("	Pminusv: %x = %x - %x\n", pminusv,
		vtop(cfree,NULL), cfree);
#endif
#endif

/*
 * Several error checks are performed:
 *	1. 'bid' is in the proper range
 *	2. The contents of the SYSGEN block are correct
 *	3. The associated REQUEST and COMPLETION queues are empty
 */

	if ((bid < 0)  ||  (bid >= pp_bnbr)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_sysgen: FAIL on bid value\n");
#endif
		u.u_error = ENXIO;
		return(FAIL);
		}

	if ((sblk->request   != (long) &R_QUEUE(bid))		||
	    (sblk->complt    != (long) &C_QUEUE(bid))		||
	    (sblk->req_size  != RQSIZE)				||
	    (sblk->comp_size != CQSIZE)				||
	    (sblk->no_rque   != NUM_QUEUES)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_sysgen: FAIL on sysgen block contents: %x %x %x %x %x\n",
		sblk->request, sblk->complt, sblk->req_size,
		sblk->comp_size, sblk->no_rque);
#endif
		u.u_error = EFAULT;
		return(FAIL);
		}

	for (i = 0; i < NUM_QUEUES; i++)
		if (RQLOAD(bid,i) != RQULOAD(bid,i)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_sysgen: FAIL on RQUEUE pointers\n");
#endif
			u.u_error = EFAULT;
			return(FAIL);
			}
	if ((CQLOAD(bid) != CQULOAD(bid))  ||  IS_CEXPRESS(bid)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_sysgen: FAIL on CQUEUE pointers\n");
#endif
		u.u_error = EFAULT;
		return(FAIL);
		}

/*
 * Complete the SYSGEN block by supplying the interrupt vector
 *
 * Convert the queue addresses from virtual to physical
 */

	sblk->int_vect = getvec((long) pp_addr[bid]);

#ifndef STANDALONE
#ifdef DEBUG
if(ll_debug & VTOP)
	printf("	Translation of queue addr: %x->%x  %x->%x\n",
		sblk->request, vtop(sblk->request, NULL),
		sblk->complt, vtop(sblk->complt, NULL));
#endif

	sblk->request = vtop(sblk->request, NULL);
	sblk->complt  = vtop(sblk->complt, NULL);
#endif

/*
 * Initialize the datastructues for this board:
 *
 *	Reset all CIO load/unload pointers to zero
 */

	for (i = 0; i < NUM_QUEUES; i++)
		RQPTRS(bid,i) = 0;
	CQPTRS(bid) = 0;
	E_SEQBIT(CQEXPRESS(bid)) = 0;
	csbit[bid] = 1;

/*
 * Have the board enter its "pre-sysgen" state. Note that this
 * ASSUMES that any board reset, if necessary, has already been
 * done
 *
 * Note the use of a "read" to trigger the interrupt, rather then a
 * "write"
 */

#ifdef DEBUG
if(ll_debug & INTER)
	printf("BOARD %x INT0: %x\n", bid, (pp_addr[bid] + 1));
#endif

	i = *((char *) (pp_addr[bid]  + 1));
#ifndef SYSTEST
	for (i = 0; i < 10000; i++)
		;
#endif

/*
 * Put the pointer to the SYSGEN block in its appropriate place and
 * notify the board
 */

#ifndef STANDALONE
#ifdef DEBUG
if(ll_debug & VTOP)
	printf("	Translation of sysgen block: %x->%x\n",
		sblk, vtop(sblk, NULL));
#endif

	*SG_PTR = vtop(sblk, NULL);
#else
	*SG_PTR = (long) sblk;
#endif

	lla_attn(bid);

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_sysgen: PASS\n");
#endif

	return(PASS);
}

int
lla_dlm(bid, mda, pda, mds)
register short bid;
register char *mda, *pda;
register unsigned short mds;
{
	char seqbit;

#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: lla_dlm(%x,%x,%x,%x)\n", bid, mda, pda, mds);
#endif

/*
 * Several error checks are performed:
 *	1. 'bid' is in the proper range
 *	2. 'mda' is non-zero
 *	3. 'pda' is block-aligned
 *	4. 'mds" does not go outside of defined PPC RAM
 */

	if ((bid < 0)  ||  (bid >= pp_bnbr)  ||
	    ((BSTATE(bid) & ISSYSGEN) == 0)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_dlm: FAIL on bid value/sysgen\n");
#endif
		u.u_error = ENXIO;
		return(FAIL);
		}

	if ((((int) mda) == 0)					     || 
	    ((((int) pda) & 255) != 0)  || (((int) pda) < RAM_START) ||
	    ((((int) pda)+mds) > RAM_END)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_dlm: FAIL on argument values\n");
#endif
		u.u_error = EFAULT;
		return(FAIL);
		}
	seqbit = ((int) pda) >= SIXTEENK;

/*
 * DLM requests go by way of the EXPRESS queue
 *
 * Notification of the board is automatic for EXPRESS entries
 */

#ifndef STANDALONE
#ifdef DEBUG
if(ll_debug & VTOP)
	printf("	Translation of mda: %x->%x\n",
		mda, vtop(mda,NULL));
#endif

	lla_express(bid, (unsigned short) (mds-1), seqbit, ((int) pda)/256, DLM, vtop(mda,NULL), 0);
#else
	lla_express(bid, (unsigned short) (mds-1), seqbit, ((int) pda)/256, DLM, (long) mda, 0);
#endif

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_dlm: PASS\n");
#endif

	return(PASS);
}

int
lla_fcf(bid, pda)
register short bid;
register char *pda;
{
#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: lla_fcf(%x,%x)\n", bid, pda);
#endif

/*
 * Several error checks are performed:
 *	1. 'bid' is in the proper range
 *	3. 'pda' is in range
 */

	if ((bid < 0)  ||  (bid >= pp_bnbr)  ||
	    ((BSTATE(bid) & ISSYSGEN) == 0)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_fcf: FAIL on bid value/sysgen\n");
#endif
		u.u_error = ENXIO;
		return(FAIL);
		}

	if ((((int) pda) < RAM_START)  ||  (((int) pda) > RAM_END)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_fcf: FAIL on arugment value\n");
#endif
		u.u_error = EFAULT;
		return(FAIL);
		}

/*
 * FCF requests go by way of the EXPRESS queue
 *
 * Notification of the board is automatic for EXPRESS entries
 */

	lla_express(bid, 0, 0, 0, FCF, (long) pda, 0);

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_fcf: PASS\n");
#endif

	return(PASS);
}

int
lla_dos(bid)
register short bid;
{
#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: lla_dos(%x)\n", bid);
#endif

/*
 * Several error checks are performed:
 *	1. 'bid' is in the proper range
 */

	if ((bid < 0)  ||  (bid >= pp_bnbr)  ||
	    ((BSTATE(bid) & ISSYSGEN) == 0)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_dos: FAIL on bid value/sysgen\n");
#endif
		u.u_error = ENXIO;
		return(FAIL);
		}

/*
 * DOS requests go by way of the EXPRESS queue
 *
 * Notification of the board is automatic for EXPRESS entries
 */

	lla_express(bid, 0, 0, 0, DOS, 0L, 0);

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_dos: PASS\n");
#endif

	return(PASS);
}

int
lla_dsd(bid, mda)
register short bid;
register char *mda;
{
#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: lla_dsd(%x,%x)\n", bid, mda);
#endif

/*
 * Several error checks are performed:
 *	1. 'bid' is in the proper range
 *	2. 'mda' is non-zero
 */

	if ((bid < 0)  ||  (bid >= pp_bnbr)  ||
	    ((BSTATE(bid) & ISSYSGEN) == 0)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_dsd: FAIL on bid value/sysgen\n");
#endif
		u.u_error = ENXIO;
		return(FAIL);
		}

	if ((int) mda == 0) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_dsd: FAIL on zero mda value\n");
#endif
		u.u_error = EFAULT;
		return(FAIL);
		}

/*
 * DSD requests go by way of the EXPRESS queue
 *
 * Notification of the board is automatic for EXPRESS entries
 */

#ifndef STANDALONE
#ifdef DEBUG
if(ll_debug & VTOP)
	printf("	Translation of mda: %x->%x\n",
		mda, vtop(mda,NULL));
#endif

	lla_express(bid, 0, 0, 0, DSD, vtop(mda,NULL), 0);
#else
	lla_express(bid, 0, 0, 0, DSD, (long) mda, 0);
#endif

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_dsd: PASS\n");
#endif

	return(PASS);
}

int
lla_attn(bid)
short bid;
{
	register int i;

#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: lla_attn(%x)\n", bid);
#endif

/*
 * Send an "ATTENTION" interrupt (INT1) to a board
 */

#ifdef DEBUG
if(ll_debug & INTER)
	printf("BOARD %x ATTENTION (INT1) INTERRUPT: %x\n", bid,
		(pp_addr[bid] + 3));
#endif

/*
 * Note the use of a "read" to trigger the interrupt, rather then a
 * "write"
 */

	i = *((char *) (pp_addr[bid] + 3));

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_attn: PASS\n");
#endif

	return(PASS);
}

int
lla_reset(bid)
short bid;
{
	register int i;

#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: lla_reset(%x)\n", bid);
#endif

/*
 * Perform a software RESET of a board
 */

#ifndef SYSTEST
#ifdef DEBUG
if(ll_debug & INTER)
	printf("BOARD %x RESET: %x\n", bid, (pp_addr[bid] + 5));
#endif

/*
 * Note the use of a "cpu-hog" for loop, to give the board time to
 * set up its basic environment
 */

	i = *((char *) (pp_addr[bid]  + 5));

	for (i = 0; i < 10000; i++)
		;
#endif

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_reset: PASS\n");
#endif

	return(PASS);
}

int
lla_cqueue(bid, eptr)
register short bid;
register CENTRY *eptr;
{
	register int i;

#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: lla_cqueue(%x,%x)\n", bid, eptr);
#endif

/*
 * Return a COMPLETION QUEUE entry for board 'bid'
 *
 ***** NOTE ***** NOTE ***** NOTE ***** NOTE ***** NOTE *****
 *
 * It is assumed that this function is being called ONLY from within
 * a high-priority interrupt handler
 *
 * For that reason, it does NOT alter the priority level
 *
 ***** NOTE ***** NOTE ***** NOTE ***** NOTE ***** NOTE *****
 */

	if (IS_CEXPRESS(bid)) {
		/*
		 * An EXPRESS entry is returned before any of the
		 * regular entries
		 */
		*eptr = CQEXPRESS(bid);
		BSTATE(bid) &= ~EXPRESS;

#ifdef DEBUG
if(ll_debug & CDUMP)
	dmp_entry("EXPRESS COMPLETION", eptr);
#endif

		/*
		 * Toggle the interal sequence bit value, so that the
		 * next entry can be recognized
		 */
		csbit[bid] ^= 1;

		/*
		 * If an EXPRESS entry has been queued for output,
		 * send it now
		 */
		if (eload != eunload) {
#ifdef DEBUG
if(ll_debug & RDUMP) {
	printf("	Sending queued EXPRESS entry %d\n", eunload);
	dmp_entry("EXPRESS REQUEST", &savee[eunload]);
	}
#endif
			BSTATE(bid) |= EXPRESS;

			RQEXPRESS(bid) = savee[eunload++];
			if (eunload == maxsavexp)
				eunload = 0;
#ifdef DEBUG
if(ll_debug & INTER)
	printf("BOARD %x EXPRESS (INT0) INTERRUPT: %x\n", bid,
		(pp_addr[bid] + 1));
#endif
			i = *((char *) (pp_addr[bid]  + 1));
			}

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_cqueue PASS\n");
#endif

		return(PASS);
		}

/*
 * This function fails if there are not entries to give the user
 */

	if (CQLOAD(bid) == CQULOAD(bid)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_cqueue: FAIL\n");
#endif
		return(FAIL);
		}

	i = CQULOAD(bid) / sizeof(CENTRY);

	*eptr = CQENTRY(bid,i);

#ifdef DEBUG
if(ll_debug & CDUMP)
	dmp_entry("REQULAR COMPLETION", eptr);
#endif

	if (++i == CQSIZE)
		i = 0;
	CQULOAD(bid) = i * sizeof(CENTRY);

/*
 * A COMPLETION QUEUE entry from the PPC can mean that the
 * corresponding REQUEST QUEUE has an available slot
 */

	switch ((int) EP_OPCODE(eptr)) {
	case CFW_CONFIG:
	case CFW_IREAD:
	case CFW_IWRITE:
	case CFW_WRITE:
		/*
		 * All CFW-xxx functions refer to the REQUEST QUEUE
		 * for port 0
		 */
		i = 0;
		break;
	case PPC_OPTIONS:
	case PPC_XMIT:
	case PPC_CONN:
	case PPC_DISC:
	case PPC_BRK:
		/*
		 * These PPC-xxx functions specify specifically the
		 * port they reference
		 */
		i = EP_SUBDEV(eptr);

		/*
		 * These functions pass a cblock address to the
		 * board.  Convert this address back from physical to
		 * virtual
		 */
#ifndef STANDALONE
#ifdef DEBUG
if(ll_debug & VTOP)
	printf("	Translation of cblock: %x->%x\n",
		EP_ADDR(eptr), cbptov(EP_ADDR(eptr)));
#endif

		EP_ADDR(eptr) = cbptov(EP_ADDR(eptr));
#endif
		break;
	case PPC_RECV:
		/*
		 * The PPC-RECV function always refers to the one
		 * common "supply buffer" queue
		 */
		i = SUPPLYBUF;

		/*
		 * This function passes a cblock address to the
		 * board.  Convert this address back from physical to
		 * virtual
		 */
#ifndef STANDALONE
#ifdef DEBUG
if(ll_debug & VTOP)
	printf("	Translation of cblock: %x->%x\n",
		EP_ADDR(eptr), cbptov(EP_ADDR(eptr)));
#endif

		EP_ADDR(eptr) = cbptov(EP_ADDR(eptr));
#endif
		break;
	default:
		/*
		 * These functions do not "relate" to any REQUEST QUEUE
		 * entry
		 */
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_cqueue: PASS\n");
#endif
		return(PASS);
	}

	PNBR(bid,i)--;

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: lla_cqueue: PASS\n");
#endif

	return(PASS);
}

int
lla_ldeuld(bid,pid)
short bid, pid;
{
	RQLOAD(bid,pid) = RQULOAD(bid,pid);
	PNBR(bid,pid) = 0;
	return(PASS);
}

int
lla_luequal(bid, pid)
short bid, pid;
{
	return(RQLOAD(bid,pid) - RQULOAD(bid,pid));
}

int
lla_xfree(bid, pid)
short bid, pid;
{
/*
 * Return TRUE iff the REQUEST queue for port (bid,pid) has an available
 * entry in it
 */

	return(PNBR(bid,pid) < NUM_ELEMENTS);
}

int
lla_rfree(bid)
short bid;
{
/*
 * Return TRUE iff the SUPPLYBUF queue for board bid has an available
 * entry in it
 */

	return(PNBR(bid,SUPPLYBUF) < (RQSIZE-1));
}

int
cfw_config(bid, opt)
register short bid;
short opt;
{
#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: cfw_config(%x,%x)\n", bid, opt);
#endif

/*
 * Several error checks are performed:
 *	1. 'bid' is in the proper range
 */

	if ((bid < 0)  ||  (bid >= pp_bnbr)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: cfw_config: FAIL on bid value\n");
#endif
		u.u_error = ENXIO;
		return(FAIL);
		}

/*
 * CONFIG requests go by way of the regular REQUEST queue
 *
 * Notification of the board is automatic for this function
 */

	lla_regular(bid, TRUE, 0, 0, CFW_CONFIG, (long) opt, 0, 0);

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: cfw_config: PASS\n");
#endif

	return(PASS);
}

int
cfw_iread(bid)
register short bid;
{
#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: cfw_iread(%x)\n", bid);
#endif

/*
 * Several error checks are performed:
 *	1. 'bid' is in the proper range
 */

	if ((bid < 0)  ||  (bid >= pp_bnbr)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: cfw_iread: FAIL on bid value\n");
#endif
		u.u_error = ENXIO;
		return(FAIL);
		}

/*
 * IREAD requests go by way of the regular REQUEST queue
 *
 * Notification of the board is automatic for this function
 */

	lla_regular(bid, TRUE, 0, 0, CFW_IREAD, 0L, 0, 0);

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: cfw_iread: PASS\n");
#endif

	return(PASS);
}

int
cfw_iwrite(bid, value, count)
register short bid;
long value;
short count;
{
#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: cfw_iwrite(%x,%x,%x)\n", bid, value, count);
#endif

/*
 * Several error checks are performed:
 *	1. 'bid' is in the proper range
 *	2. 'count' is in the interval [1,4]
 */

	if ((bid < 0)  ||  (bid >= pp_bnbr)  ||
	    (count < 1)  ||  (count > 4)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: cfw_iwrite: FAIL on argument values\n");
#endif
		u.u_error = ENXIO;
		return(FAIL);
		}

/*
 * IWRITE requests go by way of the regular REQUEST queue
 *
 * Notification of the board is automatic for this function
 */

	lla_regular(bid, TRUE, (unsigned short) (count-1), 0, CFW_IWRITE, value, 0, 0);

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: cfw_iwrite: PASS\n");
#endif

	return(PASS);
}

int
cfw_write(bid, mda, mds)
register short bid;
register char *mda;
register unsigned short mds;
{
#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: cfw_write(%x,%x,%x)\n", bid, mda, mds);
#endif

/*
 * Several error checks are performed:
 *	1. 'bid' is in the proper range
 *	2. 'mda' is non-zero
 */

	if ((bid < 0)  ||  (bid >= pp_bnbr)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: cfw_write: FAIL on bid value\n");
#endif
		u.u_error = ENXIO;
		return(FAIL);
		}

	if ((int) mda == 0) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: cfw_write: FAIL on zero mda value\n");
#endif
		u.u_error = EFAULT;
		return(FAIL);
		}

/*
 * WRITE requests go by way of the regular REQUEST queue
 *
 * Notification of the board is automatic for this function
 */

#ifndef STANDALONE
#ifdef DEBUG
if(ll_debug & VTOP)
	printf("	Translation of mda: %x->%x\n",
		mda, vtop(mda,NULL));
#endif

	lla_regular(bid, TRUE, (unsigned short) (mds-1), 0, CFW_WRITE, vtop(mda,NULL), 0, 0);
#else
	lla_regular(bid, TRUE, (unsigned short) (mds-1), 0, CFW_WRITE, (long) mda, 0, 0);
#endif

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: cfw_write: PASS\n");
#endif

	return(PASS);
}

int
ppc_clr(bid)
register short bid;
{
#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: ppc_clr(%x)\n", bid);
#endif

/*
 * This is a request for a borad to immediately shut itself down
 *
 * It has one use: It is sent on a power fail condition
 *
 * Because of this:
 *	1. There is no error checking - the request is sent out as soon
 *		as possible
 *	2. Any previous EXPRESS entry not yet acknowledged is
 *		overwritten
 *
 *
 * CLR requests go by way of the EXPRESS queue
 *
 * Notification of the board is automatic for EXPRESS entries
 */

	BSTATE(bid) &= ~EXPRESS;

	lla_express(bid, 0, 0, 0, PPC_CLR, 0L, 0);

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_clr: PASS\n");
#endif

	return(PASS);
}

int
ppc_option(bid, pid, mda, mds)
register short bid, pid;
register char *mda;
short mds;
{
#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: ppc_options(%x,%x,%x,%x)\n", bid, pid, mda, mds);
#endif

/*
 * Several error checks are performed:
 *	1. 'bid' and 'pid' are in the proper range
 *	2. 'mda' is non-zero
 */

	if ((bid < 0)  ||  (bid >= pp_bnbr)  ||
	    (pid < 0)  ||  (pid > 5)	     ||
	    ((BSTATE(bid) & ISSYSGEN) == 0)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_options: FAIL on bid value/sysgen\n");
#endif
		u.u_error = ENXIO;
		return(FAIL);
		}

	if ((((int) mda) == 0) ||
	    (mds < 0)  ||  (mds > sizeof(Options))) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_options: FAIL on argument values\n");
#endif
		u.u_error = EFAULT;
		return(FAIL);
		}

/*
 * OPTIONS requests go by way of the regular REQUEST queue
 *
 * Notification of the board is automatic for this function
 */

#ifndef STANDALONE
#ifdef DEBUG
	{
	register int i;

	if (ll_debug & VTOP)
		printf("	Translation of cblock: %x->%x\n",
			mda, cbvtop(mda));

	i = lla_regular(bid, TRUE, (unsigned short) mds, pid, PPC_OPTIONS, (long) cbvtop(mda) ,0, 0);

	if (ll_debug & EXIT)
		printf(" EXIT: ppc_options: %s\n", (i == PASS) ? "PASS" : "FAIL");

	return(i);
	}
#else
	return(lla_regular(bid, TRUE, (unsigned short) mds, pid, PPC_OPTIONS, (long) cbvtop(mda), 0, 0));
#endif
#else
	lla_regular(bid, TRUE, (unsigned short) mds, pid, PPC_OPTIONS, (long) mda, 0, 0);

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_options: PASS\n");
#endif

	return(PASS);
#endif
}

int
ppc_disc(bid, pid, eflush,dcode)
register short bid, pid;
char eflush; /* number of cblocks to flush from receive entries*/
char dcode; /* code for either to turn terminal ready off, CREAD off
		or turn off nothing */
{
#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: ppc_disc(%x,%x,%x)\n", bid, pid, eflush);
#endif

/*
 * Several error checks are performed:
 *	1. 'bid' and 'pid' are in the proper range
 *	2. 'eflush' < 0 
 */

	if ((bid < 0)  ||  (bid >= pp_bnbr)  ||
	    (pid < 0)  ||  (pid > 5)	     ||
	    ((BSTATE(bid) & ISSYSGEN) == 0)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_conn: FAIL on bid value/sysgen\n");
#endif
		u.u_error = ENXIO;
		return(FAIL);
		}

	if (eflush < 0) 
	{
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_disc: FAIL on argument values\n");
#endif
		u.u_error = EFAULT;
		return(FAIL);
		}

/*
 * PPC_DISC requests go by way of the regular REQUEST queue
 *
 * Notification of the board is automatic for this function
 */

#ifndef STANDALONE
#ifdef DEBUG
	{
	register int i;

	i = lla_regular(bid, TRUE, 0, pid, PPC_DISC, 0, eflush,dcode);

	if (ll_debug & EXIT)
		printf(" EXIT: ppc_disc: %s\n", (i == PASS) ? "PASS" : "FAIL");

	return(i);
	}
#else
	return(lla_regular(bid, TRUE, 0, pid, PPC_DISC, 0, eflush,dcode));
#endif
#else
	return(lla_regular(bid, TRUE, 0, pid, PPC_DISC, 0, eflush,dcode));

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_disc: PASS\n");
#endif

	return(PASS);
#endif
}

int
ppc_conn(bid, pid)
register short bid, pid;
{
#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: ppc_conn(%x,%x,%x)\n", bid, pid);
#endif

/*
 * Several error checks are performed:
 *	1. 'bid' and 'pid' are in the proper range
 */

	if ((bid < 0)  ||  (bid >= pp_bnbr)  ||
	    (pid < 0)  ||  (pid > 5)	     ||
	    ((BSTATE(bid) & ISSYSGEN) == 0)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_conn: FAIL on bid value/sysgen\n");
#endif
		u.u_error = ENXIO;
		return(FAIL);
		}


/*
 * PPC_CONN requests go by way of the regular REQUEST queue
 *
 * Notification of the board is automatic for this function
 */

#ifndef STANDALONE
#ifdef DEBUG
	{
	register int i;

	i = lla_regular(bid, TRUE, 0, pid, PPC_CONN, 0, 0, 0);

	if (ll_debug & EXIT)
		printf(" EXIT: ppc_conn: %s\n", (i == PASS) ? "PASS" : "FAIL");

	return(i);
	}
#else
	return(lla_regular(bid, TRUE, 0, pid, PPC_CONN, 0, 0, 0));
#endif
#else
	return(lla_regular(bid, TRUE, 0, pid, PPC_CONN, 0, 0, 0));

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_conn: PASS\n");
#endif

	return(PASS);
#endif
}

int
ppc_break(bid, pid, arg)
register short bid, pid;
/* arg is TSCBRK if it is zero a break is sent otherwise
it is a noop operation to the peripheral */
{
#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: ppc_break(%x,%x,%x)\n", bid, pid);
#endif

/*
 * Several error checks are performed:
 *	1. 'bid' and 'pid' are in the proper range
 */

	if ((bid < 0)  ||  (bid >= pp_bnbr)  ||
	    (pid < 0)  ||  (pid > 5)	     ||
	    ((BSTATE(bid) & ISSYSGEN) == 0)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_break: FAIL on bid value/sysgen\n");
#endif
		u.u_error = ENXIO;
		return(FAIL);
		}


/*
 * PPC_BRK requests go by way of the regular REQUEST queue
 *
 * Notification of the board is automatic for this function
 */

#ifndef STANDALONE
#ifdef DEBUG
	{
	register int i;

	i = lla_regular(bid, TRUE, 0, pid, PPC_BRK, 0, arg, 0);

	if (ll_debug & EXIT)
		printf(" EXIT: ppc_break: %s\n", (i == PASS) ? "PASS" : "FAIL");

	return(i);
	}
#else
	return(lla_regular(bid, TRUE, 0, pid, PPC_BRK, 0, arg, 0));
#endif
#else
	return(lla_regular(bid, TRUE, 0, pid, PPC_BRK, 0, arg, 0));

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_break: PASS\n");
#endif

	return(PASS);
#endif
}

int
ppc_device(bid, pid, pcf)
register short bid, pid, pcf;
{
#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: ppc_device(%x,%x,%x)\n", bid, pid, pcf);
#endif

/*
 * Several error checks are performed:
 *	1. 'bid' and 'pid' are in the proper range
 *	2. 'pcf' is a legal device command
 */

	if ((bid < 0)  ||  (bid >= pp_bnbr)  ||
	    (pid < 0)  ||  (pid > 5)	     ||
	    ((BSTATE(bid) & ISSYSGEN) == 0)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_device: FAIL on bid value/sysgen");
#endif
		u.u_error = ENXIO;
		return(FAIL);
		}

	switch (pcf) {
	case DR_ENA:
	case DR_DIS:
	case DR_ABR:
	case DR_ABX:
	case DR_BRK:
	case DR_SUS:
	case DR_RES:
	case DR_BLK:
	case DR_UNB:
	case DR_FLU:
		break;
	default:
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_device: FAIL on pfc value\n");
#endif
		u.u_error = EFAULT;
		return(FAIL);
	}

/*
 * DEVICE requests go by way of the EXPRESS queue
 *
 * Notification of the board is automatic for EXPRESS entries
 */

	lla_express(bid, 0, 0, pid, PPC_DEVICE, 0L, pcf);

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_device: PASS\n");
#endif

	return(PASS);
}

int
ppc_xmit(bid, pid, mda, mds)
register short bid, pid;
register char *mda;
short mds;
{
#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: ppc_xmit(%x,%x,%x,%x)\n", bid, pid, mda, mds);
#endif

/*
 * Several error checks are performed:
 *	1. 'bid' and 'pid' are in the proper range
 *	2. 'mda' is non-zero
 *	3. 'mds' is in the interval [0,64]
 */

	if ((bid < 0)  ||  (bid >= pp_bnbr)  ||
	    (pid < 0)  ||  (pid > 5)	     ||
	    ((BSTATE(bid) & ISSYSGEN) == 0)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_xmit: FAIL on bid value/sysgen\n");
#endif
		u.u_error = ENXIO;
		return(FAIL);
		}

	if ((((int) mda) == 0) ||
	    (mds < 0)  ||  (mds > (CLSIZE  - 1))) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_xmit: FAIL on argument values\n");
#endif
		u.u_error = EFAULT;
		return(FAIL);
		}

/*
 * XMIT requests go by way of the regular REQUEST queue
 *
 * Notification of the board is automatic for this function
 */

#ifndef STANDALONE
#ifdef DEBUG
	{
	register int i;

	if (ll_debug & VTOP)
		printf("	Translation of cblock: %x->%x\n",
			mda, cbvtop(mda));

	i = lla_regular(bid, TRUE, (unsigned short) mds, pid, PPC_XMIT, (long) cbvtop(mda),0,0);

	if (ll_debug & EXIT)
		printf(" EXIT: ppc_xmit: %s\n", (i == PASS) ? "PASS" : "FAIL");

	return(i);
	}
#else
	return(lla_regular(bid, TRUE, (unsigned short) mds, pid, PPC_XMIT, (long) cbvtop(mda),0,0));
#endif
#else
	lla_regular(bid, TRUE, (unsigned short) mds, pid, PPC_XMIT, (long) mda, 0, 0);

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_xmit: PASS\n");
#endif

	return(PASS);
#endif
}

int
ppc_recv(bid, mda)
register short bid;
register char *mda;
{
#ifdef DEBUG
if(ll_debug & ENTRY)
	printf("ENTRY: ppc_recv(%x,%x)\n", bid, mda);
#endif

/*
 * Several error checks are performed:
 *	1. 'bid' is in the proper range
 *	2. 'mda' is non-zero
 */

	if ((bid < 0)  ||  (bid >= pp_bnbr)  ||
	    ((BSTATE(bid) & ISSYSGEN) == 0)) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_recv: FAIL on bid value/sysgen\n");
#endif
		u.u_error = ENXIO;
		return(FAIL);
		}

	if ((int) mda == 0) {
#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_recv: FAIL on zero mda value\n");
#endif
		u.u_error = EFAULT;
		return(FAIL);
		}

/*
 * RECV requests go by way of the special "supply buffer" REQUEST queue
 *
 * Notification of the board is automatic for this function
 */

#ifndef STANDALONE
#ifdef DEBUG
	{
	register int i;

	if (ll_debug & VTOP)
		printf("	Translation of cblock: %x->%x\n",
			mda, cbvtop(mda));

	i = lla_regular(bid, TRUE, (unsigned short) CLSIZE - 1, SUPPLYBUF, PPC_RECV, (long) cbvtop(mda), 0, 0);

	if (ll_debug & EXIT)
		printf(" EXIT: ppc_recv: %s\n", (i == PASS) ? "PASS" : "FAIL");

	return(i);
	}
#else
	return(lla_regular(bid, TRUE, (unsigned short) CLSIZE - 1, SUPPLYBUF, PPC_RECV, (long) cbvtop(mda),0,0));
#endif
#else
	lla_regular(bid, TRUE, (unsigned short) CLSIZE - 1, SUPPLYBUF, PPC_RECV, (long) mda, 0,0);

#ifdef DEBUG
if(ll_debug & EXIT)
	printf(" EXIT: ppc_recv: PASS\n");
#endif

	return(PASS);
#endif
}

static
void
lla_express(bid, bytcnt, seqbit, subdev, opcode, addr, appl)
register short bid;
unsigned short bytcnt;
char seqbit, subdev, opcode;
long addr;
char appl;
{
	int i, splevel;
	RENTRY eentry;

/*
 * Build the REQUEST QUEUE EXPRESS entry
 */

	E_BYTCNT(eentry)  = bytcnt;
	E_SEQBIT(eentry)  = seqbit;
	E_SUBDEV(eentry)  = subdev;
	E_CMDSTAT(eentry) = 0;
	E_OPCODE(eentry)  = opcode;
	E_ADDR(eentry)    = addr;
	E_APPL(eentry,0)  = appl;
	E_APPL(eentry,1)  = 0;
	E_APPL(eentry,2)  = 0;
	E_APPL(eentry,3)  = 0;

#ifdef DEBUG
if(ll_debug & RDUMP)
	dmp_entry("EXPRESS REQUEST", &eentry);
#endif

/*
 * Put an "EXPRESS" entry into the REQUEST QUEUE, and notify the
 * board
 */

	splevel = splpp();
	if (BSTATE(bid) & EXPRESS) {
#ifdef DEBUG
if(ll_debug & RDUMP)
	printf("	EXPRESS queue in use: %d %d\n", eload, eunload);
#endif
		/*
		 * There is already an "active" EXPRESS entry
		 *
		 * Put the new entry on an internal queue, and return
		 *
		 * Watch out for queue overflow: Drop this entry
		 */

		if (eload != eunload)
		{
			if (eload == 0)
				i = maxsavexp - 1;
			else
				i = eload - 1;
			if (E_SUBDEV(eentry) == E_SUBDEV(savee[i])
			  && E_OPCODE(eentry) == E_OPCODE(savee[i])
			  && (E_OPCODE(eentry) != PPC_DEVICE
			    || E_APPL(eentry, 0) == E_APPL(savee[i], 0))) {
				splx(splevel);
				return;
			}
		}

		if ((i = eload + 1) == maxsavexp)
			i = 0;
		if (i == eunload) {
			cmn_err(CE_WARN, "PORTS: EXPRESS QUEUE OVERLOAD: One entry lost\n");
#ifdef DEBUG
if(! (ll_debug & RDUMP))
	dmp_entry("EXPRESS REQUEST", &eentry);
#endif
			}
		else {
			savee[eload] = eentry;
			eload = i;
			}

		splx(splevel);
		return;
		}

	BSTATE(bid) |= EXPRESS;
	RQEXPRESS(bid) = eentry;

	splx(splevel);

/*
 * Send an "EXPRESS" interrupt (INT0) to a board
 */

#ifdef DEBUG
if(ll_debug & INTER)
	printf("BOARD %x EXPRESS (INT0) INTERRUPT: %x\n", bid,
		(pp_addr[bid] + 1));
#endif

/*
 * Note the use of a "read" to trigger the interrupt, rather then a
 * "write"
 */

	i = *((char *) (pp_addr[bid]  + 1));
}

static
int
lla_regular(bid, attn, bytcnt, subdev, opcode, addr,appl0,appl1)
register short bid;
int attn;
unsigned short bytcnt;
register char subdev;
char opcode;
long addr;
char appl0;
char appl1;
{
	int i, splevel;
	RENTRY eentry;

/*
 * Build the REQUEST entry
 */

	E_BYTCNT(eentry)  = bytcnt;
	E_SEQBIT(eentry)  = 0;
	E_SUBDEV(eentry)  = subdev;
	E_CMDSTAT(eentry) = 0;
	E_OPCODE(eentry)  = opcode;
	E_ADDR(eentry)    = addr;
	E_APPL(eentry,0)  = appl0;
	E_APPL(eentry,1)  = appl1;
	E_APPL(eentry,2)  = 0;
	E_APPL(eentry,3)  = 0;

#ifdef DEBUG
if(ll_debug & RDUMP)
	dmp_entry("REQULAR REQUEST", &eentry);
#endif

/*
 * Make sure there is an available REQUEST QUEUE entry available
 *
 * Simply return a FAIL if there is not enough space
 */

	if (PNBR(bid,subdev) == ((subdev == SUPPLYBUF) ? (RQSIZE-1) : NUM_ELEMENTS))
		return(FAIL);

/*
 * Add the new entry to the queue
 *
 * Note the priority level change, to define a non-interruptable
 * critical region
 */

	splevel = splpp();

	i = RQLOAD(bid,subdev) / sizeof(RENTRY);

	RQENTRY(bid,subdev,i) = eentry;

	if (++i == RQSIZE)
		i = 0;
	RQLOAD(bid,subdev) = i * sizeof(RENTRY);

	PNBR(bid,subdev)++;

	splx(splevel);

/*
 * If requested, send an "ATTENTION" interrupt to a board
 */

	if (attn)
		lla_attn(bid);

	return(PASS);
}

#ifdef DEBUG
static 
void
dmp_entry(label, eptr)
char *label;
register CENTRY *eptr;
{
	printf("%s: (%x %x %x %x %x %x) (%x %x %x %x)\n", label,
		EP_BYTCNT(eptr) & 0xffff, EP_CMDSTAT(eptr) & 0x1,
		EP_SEQBIT(eptr) & 0x1,    EP_SUBDEV(eptr) & 0x3f,
		EP_OPCODE(eptr) & 0xff,   EP_ADDR(eptr),
		EP_APPL(eptr,0) & 0xff,   EP_APPL(eptr,1) & 0xff,
		EP_APPL(eptr,2) & 0xff,   EP_APPL(eptr,3) & 0xff);
}
#endif
