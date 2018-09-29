/*	nc.h 1.5 of 3/23/82
	@(#)nc.h	1.5
 */

#define STDPKTSZ	128

#define NC		('N'<<8)
#define NCPVCI		(NC|1)
#define NCPVCR		(NC|2)
#define NCSTART		(NC|3)
#define NCSTOP		(NC|4)
#define NCPVCSTAT	(NC|5)
#define NCLNKSTAT	(NC|6)
#define NCATTACH	(NC|7)
#define NCBKATTACH	(NC|8)
#define NCBKSTART	(NC|9)
#define NCDETACH	(NC|10)
#define NCBKDETACH	(NC|11)
#define NCCHNGE		(NC|12)
#define NCBKSTOP	(NC|13)

/* Argument structure for NCPVCI ioctl command */
struct ncpvc {
	unsigned short slot;
	unsigned short chno;
	unsigned short linkno;
	unsigned short options;
};

/* Argument structure for NCSTART ioctl command */
struct ncstart {
	unsigned short linkno;
	unsigned short pktsz;
	struct csiflags csiflags;
};

/* Argument structure for NCLNKSTAT ioctl command */
struct nclnkstat {
	struct x25link x25link;
	struct csiinfo prinfo;
	struct csiinfo bkinfo;
	ushort linkno;
	ushort prxmtq;
	ushort bkxmtq;
	ushort premptq;
	ushort bkemptq;
};

/* Argument structure for NCPVCSTAT ioctl command */
struct ncpvcstat {
	struct x25slot x25slot;
	struct x25tab x25tab;
	ushort mdevno;
};

/* Argument structure for NCATTACH and NCBKATTACH ioctl commands */
struct ncattch {
	ushort linkno;
	dev_t	dev;
};
