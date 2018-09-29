/* @(#)page.h	6.2 */
/*
 * VAX page table entry
 */

struct pt_entry {
	unsigned pg_pfnum:21,	/* Page frame number */
		pg_ndref : 1,	/* need reference		*/
		pg_ref : 1,	/* simulated reference bit	*/
		pg_lock : 1,	/* page locked			*/
		pg_cw : 1,	/* copy on write page		*/
		pg_disk : 1,	/* page on secondary storage	*/
		pg_m:1,		/* Modified bit */
		pg_prot:4,	/* Protection */
		pg_v:1;		/* Valid */
};

#define	PG_PFNUM	0x1fffff
#define	PG_M	0x04000000
#define	PG_PROT	0x78000000
#define	PG_V	0x80000000
#define	PG_TYPE	0x01800000
#define	PG_LOCK	0x800000

#define	PG_NOACC	0
#define	PG_KR	0x18000000
#define	PG_KW	0x10000000
#define	PG_UW	0x20000000
#define	PG_URKW	0x60000000
#define	PG_URKR	0x78000000

#define PG_REF		0x400000
#define PG_NDREF	0x200000

/*	Definition of a virtual address.	*/
struct vaddress {
	unsigned v_byte:9,	/* Byte within the page */
		v_vpn:21,	/* Virtual page number */
		v_region:2;	/* Region of memory(system,user,data,stack) */
};

/*	Definition of a physical address.	*/
struct paddress {
	unsigned p_byte:9,	/* Byte within the page */
		p_pfn:21,	/* Page frame number */
		p_mbz:2;	/* Must be zero */
};
#define	SYSVA	0x80000000
#define REGSZPGS	0x00200000
#define REGSZTBL	(4*REGSZPGS)
#define REGSZ		0x40000000

#define	svtoc(X)	(((int)(X) >> 9) & PG_PFNUM)
#define	ctosv(X)	(((int)(X) << 9) + SYSVA)
#define	svtop(X)	(((int)(X) >> 9) & PG_PFNUM)
#define	ptosv(X)	(((int)(X) << 9) + SYSVA)
#define	ctopv(X)	((int)(X) << 9)
#define	svtopte(X)	(sbrpte + svtoc(X))
extern int *sbrpte;

typedef union pte {

/*
 *	----------------------------------------------------------------
 *	|1| prot |m|r|cw|l| ab|rf|             pfn                     |
 *	----------------------------------------------------------------
 *	 1    4   1 1  1 1  1  1                21
 */
	struct {
	unsigned int	pg_pfn : 21,	/* real page number		*/
			pg_ndref : 1,	/* need reference 		*/
			pg_ref : 1,	/* simulated reference		*/
			pg_lock : 1,	/* page locked for IO		*/
			pg_cw : 1,	/* copy on write		*/
			pg_disk : 1,	/* on secondary storage		*/
			pg_m : 1,	/* page modified		*/
			pg_prot : 4,	/* protection field		*/
			pg_v : 1;	/* valid page			*/
	} pgm;


/*
 *	----------------------------------------------------------------
 *	|0| prot |m|1|cw|  iord  |             devblock                |
 *	----------------------------------------------------------------
 *	 1    4   1 1  1     4                    19
 */
	struct {
	unsigned int	pg_blk : 19,	/* secondary storage block	*/
				: 1,
			pg_iord : 4,	/* inode index			*/
			: (1+1+1+4+1);
	} pgd;

	/* 
	 * for mba, uba mapping registers which must
	 * read/write all 32 bits at once
	 * and cannot use insv, extzv, etc.
	 * Also for fast mask operations when needed
	 * (a single mask operation beats setting
	 * several individual fields to constant values).
	 */
	struct {
		int	pg_pte;
	} pgi;
} pte_t;

typedef union pte	spte_t;
typedef union pte	pt_t[128];

#define	PTE_KR		3
#define	PTE_KW		2
#define	PTE_UW		4
#define	PTE_UR		15
#define	PTE_URKR	15


#define PT_VAL		1		/* page valid			*/
#define PT_LOCK		1		/* page locked			*/
#define PT_COPYW	1		/* copy on write		*/
#define PT_MOD		1		/* modify bit			*/
#define PT_RES		1		/* resident bit			*/
#define PT_REF		1		/* reference bit		*/
#define PT_NDREF	1		/* need reference bit		*/

/*
 * Real memory page? (non I/O mapped)
 */
#define	isphys(pt)	(((pt)->pgi.pg_pte&0x80100000)==0x80100000)

#define	NPGPT		128		/* PTE per page	*/
typedef	pte_t ptpage[NPGPT];

extern pte_t *mapa();

/*
 * Need reference bit
 */
#define	pt_ndref(ptep)	(((ptep)->pgi.pg_pte & PG_NRMSK) == PG_NDREF)
/*
 * Valid page but not referenced?
 */
#define	pt_isval(ptep)	(((ptep)->pgi.pg_pte & (PG_RFMSK|PG_M)) == PG_V)
/*
 * Reference bit set ?
 */
#define	pt_isref(ptep)	(((ptep)->pgm.pg_v) && ((ptep)->pgi.pg_pte & PG_RFNCD))
/*
 * Page accessible (valid)?
 */
#define	pt_isacc(ptep)	((ptep)->pgm.pg_v)
#define	pt_inuse(ptep)	(pt_isacc(ptep) || pt_ndref(ptep))
/*
 * Set reference bit
 */
#define	ptsetref(ptep)	((ptep)->pgi.pg_pte |= PG_RFMSK)
/*
 * Clear reference bit
 */
#define	ptclrref(ptep)	((ptep)->pgi.pg_pte &= ~PG_RFNCD)

#define	ptclracc(ptep)	((ptep)->pgi.pg_pte = ((ptep)->pgi.pg_pte\
					& ~(PG_M | PG_V))\
					| PG_NDREF)
#define	ptclruse(ptep)	((ptep)->pgi.pg_pte &= ~(PG_V|PG_M|PG_NDREF))

#define pgfaddr(PTEVAL)	(((PTEVAL.pgi.pg_pte)&PG_PFNUM)<<BPPSHIFT)

extern pt_t *baseseg;
