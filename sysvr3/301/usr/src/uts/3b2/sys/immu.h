/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/immu.h	10.5"

/*
 * Page Descriptor (Table) Entry Definitions
 */

typedef union pde {    /*  page descriptor (table) entry  */
/*    	                                                */
/*  +---------------------+---+--+--+--+-+--+-+-+-+-+  */
/*  |        pfn          |lck|nr|  |  |r|cw| |l|m|p|  */
/*  +---------------------+---+--+--+--+-+--+-+-+-+-+  */
/*             21            1  1  1  2 1  1 1 1 1 1   */
/*                                                      */
	struct {
		uint pg_pfn	: 21,	/* Physical page frame number */
		     pg_lock	:  1,	/* Lock in core (software) */
		     pg_ndref	:  1,	/* Needs reference (software).	*/
				:  1,	/* Unused software bit.		*/
				:  2,	/* Reserved by hardware.	*/
		     pg_ref	:  1,	/* Page has been referenced */
		     pg_cw	:  1,	/* Copy on write (fault) */
		     		:  1,	/* Reserved by hardware.	*/
		     pg_last	:  1,	/* Last (partial) page in segment */
		     pg_mod	:  1,	/* Page has been modified */
		     pg_pres	:  1;	/* Page is present in memory */
	} pgm;

	struct {
		uint	pg_pde;		/* Full page descriptor (table) entry */
	} pgi;
} pde_t;

#define pg_v	pg_pres		/* present bit = valid bit */

/*
 *	Page Table
 */

#define NPGPT		64		/* Nbr of pages per page table (seg). */
typedef union ptbl {
	int page[NPGPT];
} ptbl_t;

/* Page descriptor (table) entry dependent constants */

#define	NBPP		2048		/* Number of bytes per page */
#define	NBPPT		256		/* Number of bytes per page table */
#define	BPTSHFT		8 		/* LOG2(NBPPT) if exact */
#define NDPP		4		/* Number of disk blocks per page */
#define DPPSHFT		2		/* Shift for disk blocks per page. */

#define PNUMSHFT	11		/* Shift for page number from addr. */
#define PNUMMASK	0x3F		/* Mask for page number in segment. */
#define POFFMASK        0x7FF		/* Mask for offset into page. */
#define	PNDXMASK	0x7FFFF		/* Mask for page index into section.*/
#define PGFNMASK	0x1FFFFF	/* Mask page frame nbr after shift. */

#define	NPTPP		8		/* Nbr of page tables per page.	*/
#define	NPTPPSHFT	3		/* Shift for NPTPP. */

/* Page descriptor (table) entry field masks */

#define PG_ADDR		0xFFFFF800	/* physical page address */
#define PG_LOCK		0x00000400	/* page lock bit (software) */
#define PG_NDREF	0x00000200	/* need reference bit (software) */
#define PG_REF		0x00000020	/* reference bit */
#define PG_COPYW	0x00000010	/* copy on write bit */
#define PG_LAST		0x00000004	/* Last page bit */
#define PG_M		0x00000002	/* modify bit */
#define PG_P		0x00000001	/* page present bit */

#define ONEPAGE		255
#define DZ_PAGE		(unsigned char) 8

#define PTSIZE		256		/* page table size in bytes */
#define PTSZSHFT	8		/* page table size shift count */


/* byte addr to virtual page */

#define pnum(X)   ((uint)(X) >> PNUMSHFT) 

/* page offset */

#define poff(X)   ((uint)(X) & POFFMASK)

/*	The page number within a section.
*/

#define pgndx(X)	(((X) >> PNUMSHFT) & PNDXMASK)

/* Round up page table address */

#define ptround(p)	((int *) (((int)p + PTSIZE-1) & ~(PTSIZE-1)))

/* Round down page table address */

#define ptalign(p)	((int *) ((int)p & ~(PTSIZE-1)))

/*	Disk blocks (sectors) and pages.
*/

#define	ptod(PP)	((PP) << DPPSHFT)
#define	dtop(DD)	(((DD) + NDPP - 1) >> DPPSHFT)
#define dtopt(DD)	((DD) >> DPPSHFT)

/*	Disk blocks (sectors) and bytes.
*/

#define	dtob(DD)	((DD) << SCTRSHFT)
#define	btod(BB)	(((BB) + NBPSCTR - 1) >> SCTRSHFT)
#define	btodt(BB)	((BB) >> SCTRSHFT)

/*	Page tables (64 entries == 256 bytes) to pages.
*/

#define	pttopgs(X)	((X + NPTPP - 1) >> NPTPPSHFT)
#define	pttob(X)	((X) << BPTSHFT)
#define	btopt(X)	(((X) + NBPPT - 1) >> BPTSHFT)

union ptbl *getptbl();		/* page table allocator */

extern int		nptalloced;
extern int		nptfree;

/* Form page descriptor (table) entry from modes and page frame
** number
*/

#define	mkpde(mode,pfn)	(mode | ((pfn) << PNUMSHFT))

/*	The following macros are used to check the value
 *	of the bits in a page descriptor (table) entry 
 */

#define pg_isvalid(pde) 	((pde)->pgm.pg_pres)
#define pg_islocked(pde)	((pde)->pgm.pg_lock)

/*	The following macros are used to set the value
 *	of the bits in a page descrptor (table) entry 
 *
 *	Atomic instruction is available to clear the present bit,
 *	other bits are set or cleared in a word operation.
 */

#define pg_setvalid(P)	((P)->pgi.pg_pde |= PG_P) /* Set valid bit.	*/
#define pg_clrvalid(P)	((P)->pgi.pg_pde &= ~PG_P) /* Clear valid bit.	*/

#define pg_setndref(P)	((P)->pgi.pg_pde |= PG_NDREF)
						/* Set need ref bit.	*/
#define pg_clrndref(P)	((P)->pgi.pg_pde &= ~PG_NDREF)
						/* Clr need ref bit.	*/

#define pg_setlock(P)	((P)->pgi.pg_pde |= PG_LOCK)	
						/* Set lock bit.	*/
#define pg_clrlock(P)	((P)->pgi.pg_pde &= ~PG_LOCK) /* Clear lock bit.	*/

#define pg_setmod(P)	((P)->pgi.pg_pde |= PG_M)	
						/* Set modify bit.	*/
#define pg_clrmod(P)	((P)->pgi.pg_pde &= ~PG_M)	
						/* Clear modify bit.	*/

#define pg_setcw(P)	((P)->pgi.pg_pde |= PG_COPYW) /* Set copy on write.*/
#define pg_clrcw(P)	((P)->pgi.pg_pde &= ~PG_COPYW) /* Clr copy on write.*/

#define pg_setref(P)	((P)->pgi.pg_pde |= PG_REF) 	/* Set ref bit.	*/
#define pg_clrref(P)	((P)->pgi.pg_pde &= ~PG_REF) /* Clear ref bit.	*/

#define pg_setprot(P,b)	

/*
 * Segment Descriptor (Table) Entry Definitions
 */

typedef struct sde {    /*  segment descriptor (table) entry  */
/*                                                                            */
/*  +--------+--------------+--+--------+ +--------------------------------+  */
/*  |  prot  |     len      |  |  flags | |             address            |  */
/*  +--------+--------------+--+--------+ +--------------------------------+  */
/*       8          14        2     8                      32                 */
/*                                                                            */
/*					  +--------------------------+-+-+-+  */
/*			   (V0):	  |                          |N|W|S|  */
/*					  +--------------------------+-+-+-+  */
/*						       29             1 1 1   */
/*                                                                            */
	uint seg_prot	:  8,	/* Segment protection bits */
	     seg_len	: 14,	/* Segment length in doublewords */
			:  2,	/* Reserved */
	     seg_flags	:  8;	/* Segment descriptor flags */
	union {
		uint address;	/* Address of PDT or physical segment (cont.) */
		struct {
			uint		: 29,	/* Not used */
			     wanted	:  1,	/* "N" bit  */
			     shmswap	:  1,	/* "W" bit  */
			     sanity	:  1;	/* "S" bit  */
		} V0;
	} wd2;
} sde_t;

/*  access modes  */

#define KNONE  (unsigned char)  0x00
#define KEO    (unsigned char)  0x40	/* KRO on WE32000	*/
#define KRE    (unsigned char)  0x80
#define KRWE   (unsigned char)  0xC0	/* KRW on WE32000	*/

#define UNONE  (unsigned char)  0x00
#define UEO    (unsigned char)  0x01	/* URO on WE32000	*/
#define URE    (unsigned char)  0x02
#define URWE   (unsigned char)  0x03	/* URW on WE32000	*/

#define UACCESS (unsigned char) 0x03
#define KACCESS (unsigned char) 0xC0

#define SEG_RO	(KRWE|URE)
#define SEG_RW	(KRWE|URWE)

/* descriptor bits */

#define SDE_I_bit	(unsigned char) 0x80
#define SDE_V_bit	(unsigned char) 0x40
#define SDE_T_bit	(unsigned char) 0x10
#define	SDE_C_bit	(unsigned char) 0x04
#define	SDE_P_bit	(unsigned char) 0x01
#define SDE_flags	(unsigned char) 0x41   /*  sets V_bit and P_bit  */
#define SDE_kflags	(unsigned char) 0x45   /*  sets V_bit, C_bit and */
					       /*  P_bit.		 */

#define SDE_SOFT        7

#define isvalid(sde)    ((sde)->seg_flags & SDE_V_bit)
#define indirect(sde)   ((sde)->seg_flags & SDE_I_bit)
#define	iscontig(sde)   ((sde)->seg_flags & SDE_C_bit)
#define	istrap(sde)     ((sde)->seg_flags & SDE_T_bit)
#define segbytes(sde)   (int)((((sde)->seg_len) + 1) << 3)
#define lastpg(sde)     (((sde)->seg_len) >> 8)
#define u_access(sde)   (((sde)->seg_prot) & UACCESS)
#define k_access(sde)   (((sde)->seg_prot) & KACCESS)
#define segindex(addr)	((int)addr & MSK_IDXSEG)

#define sde_clrvalid(sde)	((sde)->seg_flags &= ~SDE_V_bit)

/*	Segment descriptor (table) dependent constants.	*/

#define NBPS		0x20000 /* Number of bytes per segment */
#define SNUMSHFT	17	/* Shift for segment number from address. */
#define SNUMMASK	0x1FFF	/* Mask for segment number after shift. */
#define SOFFMASK	0x1FFFF	/* Mask for offset into segment. */
#define PPSSHFT		6	/* Shift for pages per segment. */

#define snum(X)   (((uint)(X) >> SNUMSHFT) & SNUMMASK)
#define soff(X)   ((uint)(X) & SOFFMASK)


/*
 * Memory Management Unit Definitions
 */

typedef struct _VAR {	/*  virtual address     */
	uint v_sid	:  2,	/*  section number      */
	     v_ssl	: 13,	/*  segment number      */
	     v_psl	:  6,	/*  page number         */
	     v_byte	: 11;	/*  offset within page  */
} VAR;

/*  masks to extract portions of a VAR  */

#define MSK_IDXSEG  0x1ffff  /*  lower 17 bits == PSL || BYTE  */

typedef struct  _FLTCR { /*  fault code register  */
	uint		: 21,
	     reqacc	:  4,
	     xlevel	:  2,
	     ftype	:  5;
} FLTCR;

/*  access types */

#define	AT_SPOPWRITE	 1
#define AT_SPOPFTCH	 3
#define	AT_ADFTCH	 8
#define	AT_OPFTCH	 9
#define	AT_WRITE	11
#define	AT_IFTCH	13
#define	AT_IFAD		12
#define	AT_IPFTCH	14
#define	AT_MT		 0

/*  access execution level */

#define	XLVL_K		0
#define	XLVL_U		3

/*  Fault types  */

#define F_NONE       0x0
#define F_MPROC	     0x1
#define F_RMUPDATE   0x2
#define F_SDTLEN     0x3
#define F_PWRITE     0x4
#define F_PDTLEN     0x5
#define F_INVALID    0x6
#define F_SEG_N_P    0x7
#define F_OBJECT     0x8
#define F_PDT_N_P    0x9
#define F_P_N_P      0xa
#define F_INDIRECT   0xb
#define F_ACCESS     0xd
#define F_OFFSET     0xe
#define F_ACC_OFF    0xf
#define F_D_P_HIT    0x1f

typedef struct _CONFIG { /*  configuration register  */
	uint		: 30,
	     ref	:  1,
	     mod	:  1;
} CR;

typedef int SRAMA;		/* Segment descriptor table address */

typedef struct _SRAMB {		/* SRAMB Area		*/

/*
 *	+---------+-------------+----------+
 *	| reserve | # segs      | reserve  |
 *	+---------+-------------+----------+
 *	    9		13	   10
 */
	unsigned int		:  9;	/* reserved */
	unsigned int	SDTlen	: 13;	/* number of segments in section */
	unsigned int		: 10;	/* reserved */
} SRAMB;

/* Virtual start address of sections */

#define VSECT0		0x00000000
#define VSECT1		0x40000000
#define VSECT2		0x80000000
#define VSECT3		0xC0000000

#define SRAMBSHIFT	10
#define MAXSDTSEG	8192	/* Maxmum number of SDT entries */

#define OFF 0
#define ON  1

/*  MMU-specific addresses  */

extern char *mmusrama, *mmusramb, *mmufltcr, *mmufltar, *mmucr, *mmuvar;

#define srama  ((SRAMA *) (&mmusrama))
#define sramb  ((SRAMB *) (&mmusramb))
#define fltcr  ((FLTCR *) (&mmufltcr))
#define fltar  ((int *)   (&mmufltar))
#define crptr  ((CR *)    (&mmucr))
#define varptr ((int *)   (&mmuvar))

/*
 * Peripheral mode functions
 */

#define flushaddr(vaddr)	(*((int *)(varptr)) = (int)(vaddr))
#define flushsect(sectno)	(*((SRAMA *)(srama + (sectno))) = \
					(*(SRAMA *)(srama + (sectno))))

/*	The following variables describe the memory managed by
**	the kernel.  This includes all memory above the kernel
**	itself.
*/

extern int	kpbase;		/* The address of the start of	*/
				/* the first physical page of	*/
				/* memory above the kernel.	*/
				/* Physical memory from here to	*/
				/* the end of physical memory	*/
				/* is represented in the pfdat.	*/
extern int	syssegs[];	/* Start of the system segment	*/
				/* from which kernel space is	*/
				/* allocated.  The actual value	*/
				/* is defined in the vuifile.	*/
extern int	win_ublk[];	/* A window into which a	*/
				/* u-block can be mapped.	*/
extern pde_t	*kptbl;		/* Kernel page table.  Used to	*/
				/* map sysseg.			*/
extern int	maxmem;		/* Maximum available free	*/
				/* memory.			*/
extern int	freemem;	/* Current free memory.		*/
extern int	availrmem;	/* Available resident (not	*/
				/* swapable) memory in pages.	*/
extern int	availsmem;	/* Available swapable memory in	*/
				/* pages.			*/

/*	Conversion macros
*/

/*	Get page number from system virtual address.  */

#define	svtop(X)	((uint)(X) >> PNUMSHFT) 

/*	Get system virtual address from page number.  */

#define	ptosv(X)	((uint)(X) << PNUMSHFT)


/*	These macros are used to map between kernel virtual
**	and physical address.  Note that all of physical
**	memory is mapped into kernel virtual address space
**	in segment zero at the actual physical address of
**	the start of memory which is MAINSTORE.
*/

#define kvtophys(vaddr) (svirtophys(vaddr))
#define phystokv(paddr) (paddr)

/*	Between kernel virtual address and physical page frame number.
*/

#define kvtopfn(vaddr) (svirtophys(vaddr) >> PNUMSHFT)
#define pfntokv(pfn)   ((pfn) << PNUMSHFT)

/*	Between kernel virtual addresses and the kernel
**	segment table entry.
*/

#define	kvtokstbl(vaddr) (&((sde_t *)(*(srama + 1)))[snum(vaddr)])

/*	Between kernel virtual addresses and the kernel page
**	table.
*/

#define	kvtokptbl(X)	(&kptbl[pgndx((uint)(X) - (uint)syssegs)])

/*	The following routines are involved with the pfdat
**	table described in pfdat.h
*/

#define	kvtopfdat(kv)	(&pfdat[kvtopfn(kv) - btoc(kpbase)])
#define	pfntopfdat(pfn)	(&pfdat[pfn - btoc(kpbase)])
#define	pfdattopfn(pfd)	(pfd - pfdat + btoc(kpbase))

/*	Test whether a virtual address is in the kernel dynamically
**	allocated area.
*/

#define	iskvir(va)	((secnum((uint)va) == SCN1)  &&  \
			 (uint)va >= (uint)syssegs)

/*
 * vatosde(v)
 * returns the segment descriptor entry location
 * of the virtual address v.
 */

sde_t *vatosde();

/*
 * pde_t *
 * svtopde(v)
 * returns the pde entry location of v.
 *
 * This macro works only with paged virtual address.
 */

#define svtopde(v) ((pde_t *)(phystokv(vatosde(v)->wd2.address)) + pnum(soff(v)))
/*
 * svtopfn(v)
 */

#define svtopfn(v) (pnum(svirtophys(v)))

/*	Page frame number to kernel pde.
*/

#define	pfntokptbl(P)	(kvtokptbl(pfntokv(P)))

/* flags used in ptmemall() call
*/

#define PHYSCONTIG 02
#define NOSLEEP    01

/* Section Id used in flushsect() and loadmmu() */

#define SCN0	0
#define SCN1	1
#define SCN2	2
#define SCN3	3


/*	Load the mmu registers for a section with the address
**	and length found in the proc table.  This is a macro
**	rather than a function since it speeds up context
**	switches by eliminating the subroutine linkage.
*/

#define loadmmu(p, section) \
{	\
	register int	index;	\
	register int	ipl;	\
				\
	ipl = spl7();		\
	index = (section) - SCN2;	\
				\
	if (((int *)(p)->p_sramb)[index] != -1) {		\
		srama[section] = (p)->p_srama[index];	\
		sramb[section] = (p)->p_sramb[index];	\
	}		\
	splx(ipl);	\
}
