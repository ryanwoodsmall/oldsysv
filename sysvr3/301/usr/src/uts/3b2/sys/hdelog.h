/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/hdelog.h	10.2"
/* values for filling in readtype are: */
#define HDECRC		1	/* for CRC data checking */
#define HDEECC		2	/* for Error Correction Code data processing */

/* values for filling in severity are: */
#define HDEMARG		1	/* for marginal blocks */
#define HDEUNRD		2	/* for unreadable blocks */

struct hdedata {
	dev_t	diskdev;	/* the major/minor disk device number */
				/* (major number for character dev) */
	char	dskserno[12];	/* disk pack serial number */
				/* protection of removable media */
	short	pad;		/* to aid 3b2/80186 alignment */
	daddr_t	blkaddr;	/* physical block address
				 * in machine-independent form */
	char	readtype;	/* CRC or EC */
	char	severity;	/* marginal or unreadable */
	char	badrtcnt;	/* number of unreadable tries */
	char	bitwidth;	/* bitwidth of corrected error: 0 if CRC */
	time_t	timestmp;	/* time stamp helps pathological cases*/
};

/* structure for hdeeqdt[] table declared in space.h */
struct hdeedd {
	dev_t	hdeedev;
	short	hdetype;
	short	hdeflgs;
	short	hdepid;
	daddr_t	hdepdsno;
};

#define EQD_ID		0
#define EQD_IF		1
#define EQD_TAPE	2
#define EQD_EHDC	3
#define EQD_EFC		4

/* size of internal log report queue of the hdelog driver: */
#define HDEQSIZE	19
