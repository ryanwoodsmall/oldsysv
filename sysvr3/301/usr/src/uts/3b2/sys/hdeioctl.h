/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/hdeioctl.h	10.1"
/* Hard Disk Error Log Driver ioctl() interface definitions */

/* User-level call format:
 *	ioctl(fdes, command, argptr)
 *	int fdes, command;
 *	struct hdearg *argptr;
 */

/* command values are: */
#define HDEGEDCT	1	/* get equipped disk count */
#define HDEGEQDT	2	/* get equipped disk table */
#define HDEOPEN		3	/* open hard disk */
#define HDEGETSS	4	/* get sector size of disk */
#define HDERDPD		5	/* read physical description of disk */
#define HDEWRTPD	6	/* write physical description of disk */
#define HDERDISK	7	/* read disk */
#define HDEWDISK	8	/* write disk */
#define HDECLOSE	9	/* close hard disk */
#define HDEMLOGR	10	/* issue manual hdelog() requests */
#define HDEFIXLK	11	/* "hdefix" locks hde log access */
#define HDEFIXUL	12	/* "hdefix" unlocks hde log access */
#define HDEGERCT	13	/* get count of outstanding error reports
				 * does not sleep */
#define HDEGEREP	14	/* get outstanding error reports
				 * does not sleep */
#define HDECEREP	15	/* clear error reports from the queue */
#define HDEERSLP	16	/* wait (sleep) for an error report */

union hdebody {
	struct hdedskio {
		/* used for HDERDPD, HDEWRTPD, HDERDISK, HDEWDISK commands
		 * hdeddev, hdeuaddr only: used for HDEGETSS command
		 * hdeddev only: used for HDEOPEN, HDECLOSE commands
		 */
		dev_t	hdeddev;		/* dev_t for disk */
		char	*hdeuaddr;		/* user buffer address */
		daddr_t	hdedaddr;		/* physical disk address
						 * machine-independent form */
		unsigned long	hdebcnt;	/* byte count */
	} hdedskio;
	struct hdegeqdt {
		/* used for HDEGEQDT command */
		struct hdeedd	*hdeudadr;	/* user dev table address */
		int	hdeeqdct;		/* slots in caller's table */
	} hdegeqdt;
	struct hdeelogr {
		/* used for HDEMLOGR, HDEGEREP, HDECEREP commands */
		struct hdedata	*hdeuladr;	/* user error report table adr */
		int	hdelrcnt;		/* count of reports in table */
	} hdeelogr;
	/* used internally by HDE driver to provide ioctl interface
	 * to hard disk drivers
	 */
	struct io_arg hdargbuf;
	/* The HDEGEDCT, HDEFIXLK, HDEFIXUL, HDEGERCT, and HDEERSLP commands
	 * have no special parameter structures.
	 */
};

struct hdearg {
	int hderval;		/* internal return code */
	int hdersize;		/* size of returned data */
	union hdebody hdebody;	/* command dependent values */
};

/* hderval return codes */
#define HDE_UNKN	1
#define HDE_SZER	2
#define HDE_NODV	3
#define HDE_BADO	4
#define HDE_NOTO	5
#define HDE_NOTD	6
#define HDE_TWOD	7
#define HDE_NOTF	8
#define HDE_TWOF	9
#define HDE_CANT	10
#define HDE_IOE		11
#define HDE_BADR	12
