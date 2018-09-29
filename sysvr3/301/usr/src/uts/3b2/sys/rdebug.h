/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/rdebug.h	10.7"
/* Debugging flags turned on and off by rdebug */

#define DB_SYSCALL	0x001	/* remote system calls */
#define DB_SD_RD	0x002	/* send and receive descriptors */
#define DB_SERVE	0x004	/* servers and server-control */
#define DB_MNT_ADV	0x008	/* advertising and remote mounts */
#define DB_RFSTART	0x010	/* rfstart, rfstop */
#define DB_RECOVER	0x020	/* recovery */
#define DB_RDUSER	0x040	/* rd_user structures */
#define DB_RDWRI	0x080	/* remote readi/writei */
#define DB_PROT		0x100	/* protocol */
#define DB_COMM		0x200	/* communication interface */
#define DB_GDP		0x400	/* GDP */
#define DB_PACK		0x800	/* PACK stream interface */
#define DB_RMOVE	0x1000	/* rmove */
#define DB_RSC		0x2000	/* RSC ACK and NACK */
#define DB_RFSYS	0x4000	/* miscellaneous sys call */
#define DB_SIGNAL       0x8000  /* signal */
#define DB_GDPERR       0x10000  /* GDP temporary error */
#define NO_RETRANS      0x20000  /* turn off retransmission */
#define NO_RECOVER      0x40000  /* turn off recovery */
#define NO_MONITOR      0x80000  /* turn off monitor */
#define DB_CANON     	0x100000  /* canonical form conversion */
#define	DB_FSS		0x200000  /* dufst debugging */


extern	long	dudebug;

#if DUDEBUG == YES
#define	DUPRINT1(x,y1) if (dudebug & x) printf(y1);
#define	DUPRINT2(x,y1,y2) if (dudebug & x) printf(y1,y2);
#define	DUPRINT3(x,y1,y2,y3) if (dudebug & x) printf(y1,y2,y3);
#define	DUPRINT4(x,y1,y2,y3,y4) if (dudebug & x) printf(y1,y2,y3,y4);
#define	DUPRINT5(x,y1,y2,y3,y4,y5) if (dudebug & x) printf(y1,y2,y3,y4,y5);
#define DUPRINT6(x,y1,y2,y3,y4,y5,y6) if (dudebug & x) printf(y1,y2,y3,y4,y5,y6);
#else
#define	DUPRINT1(x,y1)
#define	DUPRINT2(x,y1,y2)
#define	DUPRINT3(x,y1,y2,y3)
#define	DUPRINT4(x,y1,y2,y3,y4)
#define	DUPRINT5(x,y1,y2,y3,y4,y5)
#define	DUPRINT6(x,y1,y2,y3,y4,y5,y6)
#endif
