/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/pump.h	10.2"

#define	PUMP	(('p'<<8)|8)

#define PU_DLD	1
#define PU_RST	2
#define PU_FCF	3
#define PU_GAD	4
#define PU_SYSGEN	5
#define PU_EQUIP	6
#define O_PUMP	0100


/* failure return code to be inserted in pump_st when no actual
 * contact was made with the PPC
 * other codes are defined by common, they pertain to the
 * situation where contact has been made with PPC
 */

#define PU_OTHER (-1)
#define PU_TIMEOUT (-2)
#define PU_DEVCH (-3)
#define PU_NULL	8

struct pump_st {
	short dev;
	short min;
	short cmdcode;	/* 1 - download
			   2 - reset
			   3 - jump to location to_addr
			   4 - get address
			   5 - sysgen
			   6 - number of equipped slots
			*/
	short options;	/* default is 0		*/
	long bufaddr;
	long to_addr;
	long size; 	
	long numbrd;    /* a return value for the number of boards*/
	long retcode;
};
