/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/extbus.h	1.1"
/*
 *	EXTBUS.H -	Common Extended Bus definitions
 */

/* driver ioctl commands */

#define BIOC		('B'<<8)	/* For BUS ioctl() commands */
#define	B_GETTYPE	(BIOC|1)	/* Get bus and driver name */
#define	B_GETDEV	(BIOC|2)	/* Get device for pass through */
#define B_EDSD		(BIOC|3)	/* Regenerate and return the Extended DSD structure */
#define B_REDT		(BIOC|4)	/* Read Extended EDT */
#define B_WEDT		(BIOC|5)	/* Write Extended EDT */

#define	NAME_LEN	10

struct bus_type {
	char	bus_name[NAME_LEN];	/* Name of the driver's bus */
	char	drv_name[NAME_LEN];	/* Driver prefix */
};

/* Extended Bus Sanity Word */
#define BEDT_SANE	0XABCDDCBA

/* TC peripheral device types - tc_type in B_TC structure 	   */
/* A TC may have more then one of the following bits on 	   */
#define harddisk	1	/* non-removable hard disk devices */
#define floppy_disk	2	/* floppy disk devices             */
#define rmdisk		3	/* removable disk devices          */
#define ldensity	4	/* tape density of 800 bpi.        */
#define mdensity	5	/* tape density of 1600 bpi.       */
#define bdensity	6	/* tape density of 3200 bpi.       */
#define hdensity	7	/* tape density of 6250 bpi.       */
#define sdensity	8	/* tape density of 12500 bpi.      */
#define printer		9	/* pdtype of the printer devices   */
#define processor	10	/* pdtype of the precessor devices */

/* LOGICAL UNIT STRUCTURE */
typedef struct {
	unsigned char	lu_id;		/* Logical Unit Number                */
	unsigned char	pdtype;		/* Periphal Device Type		      */
	unsigned 	dev_type: 7;	/* Device type qualifier	      */
	unsigned 	rm_mdm	: 1;	/* One for removable medium	      */
	unsigned char	pad;		/* Pad for 32 bit boundary	      */
	unsigned long	rsrv1;		/* Reserved longs	      	      */
	unsigned long	rsrv2;		/* Reserved longs	      	      */
} B_LU;

/* TARGET CONTROLLER STRUCTURE */
typedef struct {
	unsigned long	maj;		/* Major number			      */
	unsigned char	name[NAME_LEN];	/* Pointer to AT&T device name        */
	unsigned short	tc_type;	/* TC type qualifier                  */
	unsigned char	equip_stat;	/* Equip status, one = equipped       */
	unsigned char	n_lus;		/* # of equipped LU's for this TC     */
	unsigned char	max_lu;		/* Maximum # of LU's for this TC      */
	unsigned char	pad;		/* 32 bit boundry pad                 */
	unsigned short 	lu_ptr;		/* index into LU area for TC	      */
	unsigned short	rsrv1;     	/* Reserved area per controller       */
	unsigned long	rsrv2;     	/* Reserved area per controller       */
	unsigned long	rsrv3;     	/* Reserved area per controller       */
	unsigned long	rsrv4;     	/* Reserved area per controller       */
	unsigned long	rsrv5;     	/* Reserved area per controller       */
} B_TC;

/* EXTENDED BUS EQUIPPED DEVICE TABLE */
typedef struct {
	unsigned long	sanity;		/* Sanity word for SCSI_EDT           */
	unsigned char	version;	/* Version of the EDT	              */
	unsigned char	ha_slot;	/* Slot # of HA (for HA Driver)       */
	unsigned char	max_tc;		/* Maximum # of target controllers    */
	unsigned char	tc_size;	/* Size of TC structure               */
	unsigned char	max_lu;		/* Maximum # of LU's for all TC's     */
	unsigned char	lu_size;	/* Size of LU structure               */
	unsigned char	n_tcs;		/* Number of equipped TC's	      */
	unsigned char	pad;		/* 32 bit boundry pad                 */
	B_TC  		tc[1]; 		/* TC array 			      */
} B_EDT;

/* STRUCTURE USED TO OBTAIN THE SIZE OF THE EDT */
struct bedt_szinfo{
	unsigned long	sanity;		/* sanity word for extended EDT	*/
	unsigned char	version;	/* version of the EDT 		*/
	unsigned char	ha_id;		/* ID of HA (for HA Driver) 	*/
	unsigned char	max_tc;		/* max # of target controllers 	*/
	unsigned char	tc_size;	/* size of TC structure 	*/
	unsigned char	max_lu;		/* max # of LU's for all TC's 	*/
	unsigned char	lu_size;	/* size of LU structure 	*/
	unsigned char	n_tcs;		/* # of equipped TC's		*/
	unsigned char	pad;		/* 32 bit boundry pad 		*/
};


/* Macros for accessing Extended EDT structures */
#define TC_MAJ(t)	tc[t].maj
#define	TC_EQ(t)	tc[t].equip_stat
#define	TC_NLU(t)	tc[t].n_lus
#define	TC_MAXLU(t)	tc[t].max_lu
#define	TC_TYPE(t)	tc[t].tc_type

#define	LU_PTR(t,ptr)		(B_LU *)((char *)ptr+ptr->tc[t].lu_ptr)

#define LU_ID(p,t,l)		(LU_PTR(t,p))[l].lu_id
#define LU_PDTYPE(p,t,l)	(LU_PTR(t,p))[l].pdtype
#define LU_PDQ(p,t,l)		(LU_PTR(t,p))[l].dev_type
#define LU_RMV(p,t,l)		(LU_PTR(t,p))[l].rm_mdm
