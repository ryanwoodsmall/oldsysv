/* @(#)nscdev.h	1.2 */

#ifndef		ATM
#include	"sys/nsc.h"
#endif

/* DR11B - A410 Device Block */

struct	ndb
{
	ushort		db_flags;	/* flags	*/
	short		db_state;	/* driver state */
	dev_t		db_dev;
	short		db_myadr;	/* local address of this minor device */
	struct ctentry *db_ctp;		/* ptr to connection using adapter */
	struct buf *	db_bp;		/* ptr to buffer currently in use */
	struct nmsg	db_msg;		/* input/output NSC message	*/
	struct buf	db_imb;		/* to xfer input message */
	struct buf	db_omb;		/* to xfer output message */
	struct buf	db_sysb;	/* for driver use */
	ushort		db_drstat;	/* DR11-B error status	*/
	short		db_drwc;	/* word count		*/
	ushort		db_drba;	/* buffer address	*/
	ushort		db_drcs;	/* control & status	*/
	ushort		db_drdb;	/* data buffer		*/
	short		db_timeout;
	short		db_ubmap;	/* unibus map/unibus adaptor registers */
	short		db_mytype;	/* tells if dr11b or pi13 */
#	if	NSCSTATS == 1
	struct stats	stats;		/* Statistics */
#	endif
};

/* Connection Table Entry */

struct	ctentry	{
	struct	file	*ct_fp;		/* file table ptr (from file descriptor */
	struct nscgetty ct_getty;	/* driver config parameters */

#define	ct_flags	ct_getty.g_flags
#define	ct_open		ct_getty.g_open
#define	ct_vchan	ct_getty.g_vchan
#define	ct_tonad	ct_getty.g_tonad
#define	ct_acode	ct_getty.g_acode
#define	ct_tmsk		ct_getty.g_tmsk
#define	ct_asmb		ct_getty.g_asmb

	struct	proc	*ct_proc;	/* process table ptr */
	short		ct_pid;		/* connection process id */
	struct	ctrlm	ct_ctrlm;	/* control mode parameters */
	struct	datam	ct_datam;	/* data mode parameters */
	struct adptrst	ct_stat;	/* status from last err in i/o */
#	if	NSCDARD == 1
	char *		ct_bufferp;	/* pointer to receive buffer in system virtual space */
	struct buf	ct_idb;		/* for input data */
#	endif
};

/* Connection Table */

#define	NCTENT	20

struct	ct
{
	struct	ctentry	cte[NCTENT];
};
