/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/lboot.h	10.1"



/* Individual I/O buffers. */
extern char	IOBASE[];	/* Base address of I/O buffers. */
extern char	IND3[];		/* 3rd level indirect block */
extern char	DATA[];		/* a data block */
extern char	AHDR[];		/* a.out header block for UNIX */
extern char	IND2[];		/* 2nd level indirect block */
extern char	IND1[];		/* 1st level indirect block */
extern char	INODE[];	/* inode block */
extern char	DIR[];		/* directory block */


#define FFSO	18	/* File system offset on floppy disk */


#define LDPASS 1
#define LDFAIL 0

#define CFAIL 0
#define CPASS 1

#define NOTFOUND 0
#define FOUND    1
#define	DIRFOUND 2

#define BADMAGIC 0xBADD1E

#define DISK_ACS(x,y,z,f) ((P_CMDQ->b_dev == HARDDISK)? HD_ACS(0,x,y,z) : FD_ACS(x,y,z,f))

struct cfinfo {
	unsigned char opt_code;		/* eight bit option code */
	unsigned opt_slot:4;		/* option slot board is resident in */
	unsigned char dev_name[9];	/* ASCII name of device */
};

extern struct inode	Dinode;		/* Inode of file system root directory */
extern struct inode	Fndinode;	/* Inode of file found by findfile() */
extern struct inode	Linode;		/* Inode of last directory found */
extern int		Fso;		/* File system offset */
extern int		fstype;		/* File system type flag */

extern unsigned short findfs(), findfile(), loadprog(), loadfile();
extern readsb();
