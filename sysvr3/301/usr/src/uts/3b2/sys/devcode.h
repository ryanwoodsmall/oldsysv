/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/devcode.h	10.2"
/********************************************************************/
/*	devcode.h - header file for device and subdevice codes      */
/********************************************************************/


/* device code definitions */

#define D_NULL	0	/* null device */
#define D_SBD	1	/* system board */
#define D_NI	2	/* ethernet network interface */
#define D_PORTS 3	/* ports board */



/* subdevice code definitions */

#define SD_NULL 0	/* null subdevice */
#define SD_IF	1	/* internal floppy disk */
#define SD_ID10	2	/* internal 10 Mb winchester disk */
#define SD_ID30	3	/* internal 30 Mb winchester disk */
