/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/region.h	10.5"

/*
 * Per region descriptor.  One is allocated for
 * every active region in the system.
 */

typedef	struct	region	{
	short	r_flags;	/* Various flags defined below. */
	short	r_listsz;	/* Number of page tables allocated */
				/* to r_list.	*/
	short	r_pgsz;		/* size in pages */
	short	r_nvalid;	/* number of valid pages in region */
	short	r_refcnt;	/* number of users pointing at region */
	short	r_type;		/* type of region */
	short	r_waitcnt;	/* number of processes waiting for load */
	short	r_noswapcnt;	/* Count of nbr of processes which have	*/
				/* requested that this region not be	*/
				/* swapped out.				*/
	int	r_filesz;	/* Size in bytes of section of file	*/
				/* from which this region is loaded.	*/
	pde_t	**r_list;	/* Pointer to a list of pointers to	*/
				/* page tables and dbd's.		*/
	struct inode   *r_iptr;	/* pointer to inode where blocks are */
	struct region  *r_forw;	/* forward link */
	struct region  *r_back;	/* backward link */

} reg_t;

/*
 * Region flags
 */
#define	RG_NOFREE	0x0001	/* Don't free region on last detach */
#define	RG_DONE		0x0002	/* Region is initialized */
#define	RG_NOSHARE	0x0004	/* Don't share the region. */
#define RG_LOCK		0x0008	/* Region is locked */
#define RG_WANTED	0x0010	/* Wakeup rp after clearing RG_LOCK */
#define RG_WAITING	0x0020	/* Wakeup rp->r_flags when RG_DONE is set */
#define XREMOTE		0x0040	/* remote region entry */

/*
 * Region types
 */
#define	RT_UNUSED	0	/* Region not being used.	*/
#define	RT_PRIVATE	1	/* Private (non-shared) region. */
#define RT_STEXT	2	/* Shared text region */
#define RT_SHMEM	3	/* Shared memory region */

/*	Each process has a number of pregions which describe the
 *	regions which are attached to the process.
 */

typedef struct pregion {
	struct region	*p_reg;		/* Pointer to the region.	*/
	caddr_t		p_regva;	/* User virtual address of	*/
					/* region.			*/
	short		p_flags;	/* Flags.			*/
	short		p_type;		/* Type.			*/
} preg_t;

/*	Pregion flags.
 */

#define	PF_RDONLY	0x0001		/* Read-only attach.		*/

/*	Pregion types.
 */

#define	PT_UNUSED	0x00		/* Unused region.		*/
#define	PT_TEXT		0x01		/* Text region.			*/
#define	PT_DATA		0x02		/* Data region.			*/
#define	PT_STACK	0x03		/* Stack region.		*/
#define	PT_SHMEM	0x04		/* Shared memory region.	*/
#define	PT_DMM		0x05		/* Double mapped memory.	*/
#define	PT_LIBTXT	0x06		/* Shared library text region.	*/
#define	PT_LIBDAT	0x07		/* Shared library data region.	*/

extern preg_t	nullpregion;		/* A null preg_t. */
extern int	rlist_lock;		/* Lock for the region list.	*/
extern int	pregpp;			/* Number of pregions per	*/
					/* process including null one.	*/

extern reg_t	region[];	/* Global array of regions */
extern reg_t	ractive;	/* List of active regions */
extern reg_t	rfree;		/* List of free regions */
extern reg_t	sysreg;		/* System region. */
extern reg_t	nullregion;	/* A null region. */

reg_t		*allocreg();	/* region allocator */
void		freereg();	/* region free routine */
preg_t		*attachreg();	/* Attach region to process. */
void		detachreg();	/* Detach region from process. */
reg_t		*dupreg();	/* Duplicate region (fork). */
int		growreg();	/* Grow region. */
int		loadreg();	/* Load region from file. */
int		mapreg();	/* Map region to 413 file. */
reg_t		*findreg();	/* Find region from virtual address. */
preg_t		*findpreg();	/* Find pregion of given type. */
void		chgprot();	/* Change protection for region. */
void		reginit();	/* Initialize the region table. */


/*	The page table entries are followed by a list of disk block 
 *	descriptors which give the location on disk where a
 *	copy of the corresponding page is found.  If the page
 *	is on swap, it is always a single block.  However, if
 *	it is on a regular file, a single page may correspond
 *	to a number of non-consecutive disk blocks.
 */

typedef struct dbd {
	uint	dbd_type  :  4;	/* The values for this field are given	*/
				/* below.				*/
	uint	dbd_swpi  :  4;	/* The index into swaptab for the	*/
				/* device this page is on if dbd_type	*/
				/* is DBD_SWAP.				*/
	uint	dbd_blkno : 24;	/* The block number or i_map index.	*/
} dbd_t;

#define	DBD_NONE	0	/* There is no copy of this page on 	*/
				/* disk.				*/
#define	DBD_SWAP	1	/* A copy of this page is on block nbr	*/
				/* dbd_blkno of the swap file		*/
				/* swptbl[dbd_swpi].			*/
#define	DBD_FILE	2	/* A copy of this page is on the file	*/
				/* described by the inode r_iptr.  The	*/
				/* dbd_blkno field is an index into the	*/
				/* i_map list pointed to by the inode.	*/
				/* It is the start of a list of block	*/
				/* number which contain a copy of the	*/
				/* page.				*/
#define DBD_LSTFILE	3	/* Same as DBD_FILE except that		*/
				/* this is the last page of the region	*/
#define	DBD_DZERO	4	/* This is a demand zero page.  No	*/
				/* space is allocated now.  When a 	*/
				/* fault occurs, allocate a page and	*/
				/* initialize it to all zeros.		*/
#define	DBD_DFILL	5	/* This is a demand fill page.  No	*/
				/* space is allocated now.  When a	*/
				/* fault occurs, allocate a page and	*/
				/* do not initialize it at all.  It	*/
				/* will be initialized by reading in 	*/
				/* data from disk.			*/
