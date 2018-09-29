/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/* @(#)region.h	1.2 */
/*
 * Per region descriptor.  One is allocated for
 * every active region in the system.  Beware if you add
 * data elements here:  Dupreg may need to copy them.
 */

typedef	struct	region	{
	short	r_flags;
	short	r_listsz;	/* Number of pages allocated to r_list.	*/
	short	r_pgsz;		/* size in pages */
	short	r_pgoff;	/* offset into region in pages */
	short	r_nvalid;	/* number of valid pages in region */
	short	r_refcnt;	/* number of users pointing at region */
	short	r_incore;	/* number of users pointing at region */
	short	r_type;		/* type of region */
	pte_t	**r_list;	/* Pointer to a list of pointers to	*/
				/* page tables and dbd's.		*/
	int	r_filesz;	/* size in bytes of section of file	*/
				/* from which this region is loaded.	*/
	struct inode   *r_iptr;	/* pointer to inode where blocks are */
	struct region  *r_forw;	/* forward link */
	struct region  *r_back;	/* backward link */
} reg_t;

/*
 * Region flags
 */
#define	RG_NOSWAP	0x0001	/* region cannot be swapped */
#define RG_DEMAND	0x0002	/* region is demand fill from a file */
#define	RG_LOCK		0x0004	/* Region is locked. */
#define	RG_WANTED	0x0008	/* Do wakeup on rp  after clearing */
				/* RG_LOCK. */
#define	RG_NOFREE	0x0010	/* Don't free region on last detach */
#define	RG_DONE		0x0020	/* Region is loaded. */
#define	RG_WAITING	0x0040	/* Do wakeup on &rp->r_flags when */
				/* RG_DONE is set. */
#define RG_AGE		0x0080	/* Age region if set. Temp flag used */
				/* only by vhand */

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

extern preg_t	nullpregion;		/* A null preg_t. */
extern int	pregpp;			/* Number of pregions per	*/
					/* process including null one.	*/


extern reg_t	region[];	/* Global array of regions */
extern reg_t	ractive;	/* List of active regions */
extern reg_t	rfree;		/* List of free regions */
extern reg_t	nullregion;	/* A null region. */

reg_t		*allocreg();	/* region allocator */
void		freereg();	/* region free routine */
preg_t		*attachreg();	/* Attach region to process. */
void		detachreg();	/* Detach region from process. */
reg_t		*dupreg();	/* Duplicate region (fork). */
int		growreg();	/* Grow region. */
int		loadreg();	/* Load region from file. */
int		mapreg();	/* Map region to 413 file. */
preg_t		*findreg();	/* Find region from virtual address. */
preg_t		*findpreg();	/* Find pregion of given type. */
void		chgprot();	/* Change protection for region. */
void		reginit();	/* Initialize the region table. */


/*	The page table entries are followed by a list of disk block
 *	descriptors which give the location on disk where a
 *	copy of the corresponding page is found.
 */

typedef struct dbd {
	uint	dbd_type  :  4;	/* The values for this field are given	*/
				/* below.				*/
	uint	dbd_swpi  :  4;	/* The index into swaptab for the	*/
				/* device this page is on if dbd_type	*/
				/* is DBD_SWAP.				*/
#	define	dbd_adjunct	dbd_swpi
				/* The swpi field is used with DBD_FILE */
				/* entries to record even more info	*/
	uint	dbd_blkno : 24;	/* The block number or i_map index.	*/
	uint	dbd_2nd;	/* make dbd same size as pte */
} dbd_t;

#define	DBD_NONE	0	/* There is no copy of this page on 	*/
				/* disk.				*/
#define	DBD_SWAP	1	/* A copy of this page is on block nbr	*/
				/* dbd_blkno of the swap file		*/
				/* swptbl[dbd_swpi].			*/
#define	DBD_DZERO	3	/* This is a demand zero page.  No	*/
				/* space is allocated now.  When a 	*/
				/* fault occurs, allocate a page and	*/
				/* initialize it to all zeros.		*/
#define	DBD_DFILL	4	/* This is a demand fill page.  No	*/
				/* space is allocated now.  When a	*/
				/* fault occurs, allocate a page and	*/
				/* do not initialize it at all.  It	*/
				/* will be initialized by reading in 	*/
				/* data from disk.			*/
#define DBD_FILE	5	/* This page matches a block on the	*/
				/* file system				*/

/*
 *	ADJUNCT  FIELDS
 *	These are intended to be or'd into the adjunct field for
 *	file mapped pages
 */
#define DBD_FP		0x1	/* marks the first page from a file	*/
				/* system block				*/
				/* DBD_FP set => page matches first	*/
				/* half of a 1024 byte block		*/
				/* DBD_FP clear => second half		*/
#define DBD_LAST	0x2	/* marks the last page of a file mapped */
				/* region				*/
#define	DBD_FIRST	0x4	/* marks the first page of a file 	*/
				/* mapped region			*/
