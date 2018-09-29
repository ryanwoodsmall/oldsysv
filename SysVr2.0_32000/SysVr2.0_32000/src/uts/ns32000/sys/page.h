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
#define PFM_HACK	/* User RO doesnt work if MOD bit set */
/*
 * NS page table entry
 */

struct pt_entry {
	unsigned
		pg_v : 1,	/* Valid */
		pg_prot : 2,	/* Protection */
		pg_ref : 1,	/* Referenced bit */
		pg_m : 1,	/* Modified bit */
		pg_res : 2,	/* Reserved for NS */
		pg_ndref : 1,	/* need reference */
		pg_disk : 1,	/* page on disk */
		pg_zero : 1,	/* actually low bit of pfn */
		pg_pfn : 14,	/* Page frame number */
		pg_lock : 1,	/* page locked */
		pg_cw : 1,	/* copy on write */
		pg_none : 6;	/* not used */
	unsigned
		pg_v2 : 1,	/* Valid */
		pg_prot2 : 2,	/* Protection */
		pg_ref2 : 1,	/* Referenced bit */
		pg_m2 : 1,	/* Modified bit */
		pg_res2 : 4,
		pg_one : 1,	/* actually low bit of pfn */
		pg_pfn2 : 14,	/* Page frame number */
		pg_none2 : 8;	/* not used */
};

#define PG_PNSH	10
#define PFNMASK	0xfffe00
#define PG_2ND	0x200
#define PG_PE2(X)	(X|PG_2ND)

#define	PG_V	0x01
#define	PG_PROT	0x06
#define PG_REF	0x08
#ifdef PFM_HACK
#define PG_MOD	0x10		/* real hardware mod bit */
#define PG_MX	0x4000000	/* pseudo mod bit */
#define PG_M	(PG_MOD|PG_MX)	/* fake mod info */
#else
#define	PG_M	0x10
#endif
#define PG_NDREF 0x80
#define PG_NDR	0x80
#define	PG_LOCK	0x1000000
#define PG_CW	0x2000000

#define	PG_KR	0x0
#define	PG_KW	0x2
#define PG_UR	0x4
#define PG_UW	0x6

#define svtoc(X)	((int)(X) >> PG_PNSH)
#define ctosv(X)	((int)(X) << PG_PNSH)
#define svtop(X)	((int)(X) >> PG_PNSH)
#define ptosv(X)	((int)(X) << PG_PNSH)
#define	ctopv(X)	((int)(X) << PG_PNSH)
#define svtos(X)	((int)(X) >> 16)
#define NPPL	16384
#define ptoptp(X)	((X+(NPPL-1))/NPPL)
#define	svtopte(X)	(kpte2 + svtoc(X))
#define mapa(X)		(upte2 + svtoc(X))

typedef union pte {

/*
 *	-------------------------------------------------------------
 *	| none |cw|l|		pfn		|d|nr|res|m|r|prot|1|
 *	-------------------------------------------------------------
 *	   6    1  1		15		 1 1   2  1 1  2   1
 */
	struct {
	unsigned
		pg_v : 1,	/* Valid */
		pg_prot : 2,	/* Protection */
		pg_ref : 1,	/* Referenced bit */
		pg_m : 1,	/* Modified bit */
		pg_res : 2,	/* Reserved for NS */
		pg_ndref : 1,	/* need reference */
		pg_disk : 1,	/* page on disk */
		pg_zero : 1,	/* actually low bit of pfn */
		pg_pfn : 14,	/* Page frame number */
		pg_lock : 1,	/* page locked */
		pg_cw : 1,	/* copy on write */
		pg_none : 6;	/* not used */
	unsigned
		pg_v2 : 1,	/* Valid */
		pg_prot2 : 2,	/* Protection */
		pg_ref2 : 1,	/* Referenced bit */
		pg_m2 : 1,	/* Modified bit */
		pg_res2 : 4,
		pg_one : 1,	/* actually low bit of pfn */
		pg_pfn2 : 14,	/* Page frame number */
		pg_none2 : 8;	/* not used */
	} pgm;


/*
 *	-------------------------------------------------------------
 *	| iord |	devblock		|1|nr|res|m|r|prot|0|
 *	-------------------------------------------------------------
 *	   4		   19			 1 1   2  1 1  2   1
 */
	struct {
	unsigned
		: 9,
		pg_blk : 19,	/* secondary storage block */
		pg_iord : 4;	/* inode index */
	} pgd;

	/* 
	 * For fast mask operations when needed
	 * (a single mask operation beats setting
	 * several individual fields to constant values).
	 */
	struct {
		int	pg_pte;
		int	pg_pte2;
	} pgi;
} pte_t;

#define NPGPT	64	/* number of (double) ptes in a segment */
#define NPG_PG	128	/* number of (double) ptes in a (double) page */
#define NL_PG	1	/* number of pages per r_list item */
#define NL1_SEG	256	/* number of segments in an address space */
#define NSG_PG	2	/* number of segments mapped by a pte page */

typedef union pte	spte_t;
typedef union pte	seg_t[NPGPT];		/* segment table type */
typedef seg_t		*as_t[NL1_SEG];		/* address space type */

#define	PTE_KR		0
#define	PTE_KW		1
#define	PTE_UR		2
#define	PTE_UW		3


#define PT_VAL		1		/* page valid			*/
#define PT_LOCK		1		/* page locked			*/
#define PT_COPYW	1		/* copy on write		*/
#define PT_MOD		1		/* modify bit			*/
#define PT_RES		1		/* resident bit			*/
#define PT_REF		1		/* reference bit		*/
#define PT_NDREF	1		/* need reference bit		*/

extern spte_t *kpte2;
extern pte_t *upte2;
