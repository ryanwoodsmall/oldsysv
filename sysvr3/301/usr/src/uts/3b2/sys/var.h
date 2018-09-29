/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/var.h	10.2"

/*
 * System Configuration Information
 */
struct var {
	int	v_buf;		/* Nbr of I/O buffers.			*/
	int	v_call;		/* Nbr of callout (timeout) entries.	*/
	int	v_inode;	/* Size of incore inode table.		*/
	char *	ve_inode;	/* Ptr to end of incore inodes.		*/
	int	v_file;		/* Size of file table.			*/
	char *	ve_file;	/* Ptr to end of file table.		*/
	int	v_mount;	/* Size of mount table.			*/
	char *	ve_mount;	/* Ptr to end of mount table.		*/
	int	v_proc;		/* Size of proc table.			*/
	char *	ve_proc;	/* Ptr to next available proc entry	*/
				/* following the last one used.		*/
	int	v_region;	/* Nbr of regions allocated.		*/
	int	v_clist;	/* Nbr of clists allocated.		*/
	int	v_maxup;	/* Max number of processes per user.	*/
	int	v_hbuf;		/* Nbr of hash buffers to allocate.	*/
	int	v_hmask;	/* Hash mask for buffers.		*/
	int	v_pbuf;		/* Nbr of physical I/O buffers.		*/
	int	v_sptmap;	/* Size of system virtual space		*/
				/* allocation map.			*/
	int	v_vhndfrac;	/* Fraction of maxmem to set as limit	*/
				/* for running vhand.  See getpages.c	*/
				/* and clock.c				*/
	int	v_maxpmem;	/* The maximum physical memory to use.	*/
				/* If v_maxpmem == 0, then use all	*/
				/* available physical memory.		*/
				/* Otherwise, value is amount of mem to	*/
				/* use specified in pages.		*/
	int	v_autoup;	/* The age a delayed-write buffer must	*/
				/* be in seconds before bdflush will	*/
				/* write it out.			*/
	int	v_nofiles;	/* Maximum number of open files per	*/
				/* process.				*/

	int	v_nqueue;	/* Nbr of streams queues.		*/
	int	v_nstream;	/* Number of stream head structures.	*/
	int	v_nblk4096;	/* Number of 4096 bytes stream buffers.	*/
	int	v_nblk2048;	/* Number of 2048 bytes stream buffers.	*/
	int	v_nblk1024;	/* Number of 1024 bytes stream buffers.	*/
	int	v_nblk512;	/* Number of 512 bytes stream buffers.	*/
	int	v_nblk256;	/* Number of 256 bytes stream buffers.	*/
	int	v_nblk128;	/* Number of 128 bytes stream buffers.	*/
	int	v_nblk64;	/* Number of 64 bytes stream buffers.	*/
	int	v_nblk16;	/* Number of 16 bytes stream buffers.	*/
	int	v_nblk4;	/* Number of 4 bytes stream buffers.	*/
	int	v_s5inode;	/* Size of s5inode table */
};

extern struct var v;
