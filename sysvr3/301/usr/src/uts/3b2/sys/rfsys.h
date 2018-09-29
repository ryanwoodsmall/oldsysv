/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/rfsys.h	10.4"

#ifdef INKERNEL
extern	char	Domain[];
#endif

/*  opcodes for rfsys system call */

#define RF_FUMOUNT	1	/* forced unmount */
#define RF_SENDUMSG	2	/* send buffer to remote user-level */
#define RF_GETUMSG	3	/* wait for buffer from remote user-level */
#define RF_LASTUMSG	4	/* wakeup from GETUMSG */
#define RF_SETDNAME	5	/* set domain name */
#define RF_GETDNAME	6	/* get domain name */
#define RF_SETIDMAP	7
#define RF_FWFD		8
#define RF_VFLAG	9
#define RF_DISCONN	10	/* return value for link down */
#define RF_VERSION	11


/* defines for VFLAG option	*/
#define V_CLEAR 0
#define V_SET	1
#define V_GET	2

/* defines for the VERSION option	*/
#define VER_CHECK	1
#define VER_GET		2
