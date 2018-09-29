/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:swinc/sys/immu.h	1.1"
/*
 *	Memory Management
 */

/*  MMU-specific type definitions  */

#ifndef _MMU
#define IRONCLAD
#define _MMU

#define VBOFF	0x1ffffL


typedef union _PDE {			/*  page descriptor  */
/*                                                     */
/*  +---------------------+--+------+--+-+-+--+-+-+-+  */
/*  |         pfn         |ab|locked|  |R|W|DZ|L|M|P|  */
/*  +---------------------+--+------+--+-+-+--+-+-+-+  */
/*             21           2    1    2 1 1  1 1 1 1   */
/*                                                     */
    struct {
        unsigned int pg_pfn      : 21;		/* Page address		*/
	unsigned int pg_ab       :  2;		/* old name: ab		*/
	unsigned int pg_lock     :  1;		/* old name: locked	*/
	unsigned int             :  2;		/* Reserved		*/
	unsigned int pg_R        :  1;		/* Referenced flag	*/
	unsigned int pg_cw       :  1;		/* Write Trap flag	*/
	unsigned int pg_DZ       :  1;		/* Reserved		*/
	unsigned int pg_L        :  1;		/* Last Page flag	*/
	unsigned int pg_M        :  1;		/* Modified flag	*/
	unsigned int pg_P        :  1;		/* Present flag		*/
    } P1;

/*                                                     */
/*  +---------------------+----+--------+              */
/*  |         pfn         |iord| pflags |              */
/*  +---------------------+----+--------+              */
/*             21            3      8                  */
/*                                                     */
    struct {
        unsigned int pg_pfn      : 21;		/* old name: pfn	*/
	unsigned int pg_iord     :  3;		/* old name: iord	*/
        unsigned int pg_flags    :  8;		/* old name: pflags	*/
    } P_phys;

/*                                                     */
/*  +-------------------+-----+--------+               */
/*  |     devblock      |iord | flags  |               */
/*  +-------------------+-----+--------+               */
/*           19            5       8                   */
/*                                                     */
    struct {
	unsigned int pg_blk      : 19;		/* old name: devblock	*/
	unsigned int pg_iord     :  5;		/* old name: iord	*/
	unsigned int pg_flags    :  8;		/* old name: flags	*/
    } P0;

    unsigned int intpde;
} pde_t;

/*
 * access modes
 */
#define KNONE  (unsigned char) 0
#define KEO    (unsigned char) 0x40	/*  64   NOTE:  MEANS KRO on WE32001  */
#define KRE    (unsigned char) 0x80	/* 128 */
#define KRWE   (unsigned char) 0xc0	/* 192   NOTE:  MEANS KRW on WE32001  */

#define UNONE  (unsigned char)   0
#define UEO    (unsigned char)   1	/*  NOTE:  MEANS URO on WE32001  */
#define URE    (unsigned char)   2
#define URWE   (unsigned char)   3	/*  NOTE:  MEANS URW on WE32001  */

#define UACCESS (unsigned char)    3
#define KACCESS (unsigned char) 0xc0	/* 192 */

/*
 * descriptor bits
 */
#define SDE_I_bit	(unsigned char) 0x80	/* 128 */
#define SDE_V_bit	(unsigned char) 0x40	/*  64 */
#define SDE_T_bit	(unsigned char) 0x10	/*  16 */
#define	SDE_C_bit	(unsigned char)    4
#define SDE_P_bit	(unsigned char)    1	/* 'Present' bit */
#define SDE_flags	(unsigned char)(SDE_V_bit|SDE_C_bit|SDE_P_bit)	/*  69 sets V, C, and P bit */


#define SDE_SOFT        7

#define	PDE_locked	0x100		/* 256 */
#define	PDE_L_bit	    4

#define	DSC_P_bit       1

#define isvalid(sde)    ((sde)->flags & SDE_V_bit)
#define indirect(sde)   ((sde)->flags & SDE_I_bit)
#define	iscontig(sde)   ((sde)->flags & SDE_C_bit)
#define	istrap(sde)     ((sde)->flags & SDE_T_bit)
#define segbytes(sde)   (int)((((sde)->maxoff) + 1) << 3)
#define lastpg(sde)     (((sde)->maxoff) >> 8)
#define u_access(sde)   (((sde)->access) & UACCESS)
#define k_access(sde)   (((sde)->access) & KACCESS)
#define segindex(addr)	((int)addr & MSK_IDXSEG)

#define ONEPAGE		0xff		/* 255 */
#define DZ_PAGE		(unsigned char) 8

#define ispresent(dscptr) ((*(int *)(dscptr)) & DSC_P_bit)

#define islocked(pde)	((pde)->P1.locked)
#define isphys(pde)	((pde)->P1.pfn < 0x1000)

typedef struct _VAR {			/*  virtual address     */
/*							*/
/*  +-------+-----------+-----------+------------+	*/
/*  | v_sid |   v_ssl   |   v_psl   |   v_byte   |	*/
/*  +-------+-----------+-----------+------------+	*/
/*      2        13           6           11		*/
/*							*/
    unsigned int v_sid  :  2;		/*  section number      */
    unsigned int v_ssl  : 13;		/*  segment number      */
    unsigned int v_psl  :  6;		/*  page number         */
    unsigned int v_byte : 11;		/*  offset within page  */
} VAR;

/*  masks to extract portions of a VAR  */

#define MSK_IDXSEG  0x1ffff		/*  lower 17 bits == PSL || BYTE  */

typedef struct  _FLTCR {		/*  fault code register  */
/*							*/
/*  +------------+----------+----------+---------+	*/
/*  |  Reserved  |  reqacc  |  xlevel  |  ftype  |	*/
/*  +------------+----------+----------+---------+	*/
/*        21          4          2          5		*/
/*							*/
    unsigned int        : 21;		/* Reserved		*/
    unsigned int reqacc :  4;		/* Access Requested	*/
    unsigned int xlevel :  2;		/* Access Xlevel	*/
    unsigned int ftype  :  5;		/* Fault Type		*/
} FLTCR;

/*
 * access types
 */
#define	AT_ILCKR_A	  1	/*  Interlocked Read - WE32001  */
#define	AT_ILCKR_B	  7	/*  Interlocked Read - WE32100  */
#define	AT_ADFTCH	  8
#define	AT_OPFTCH	  9
#define	AT_WRITE_A	0xb	/* 11   WRITE -- WE32001   */
#define	AT_WRITE_B	0xa	/* 11   WRITE -- WE32100   */
#define	AT_IFTCH_A	0xd	/* 13   Instr. Fetch - WE32001   */
#define	AT_IFTCH_B	0xe	/* 13   Instr. Fetch - WE32100   */
#define	AT_IFAD		0xc	/* 12 */
#define	AT_IPFTCH_A	0xe	/* 14   Instr. Prefetch - WE32001  */
#define	AT_IPFTCH_B	0xd	/* 14   Instr. Prefetch - WE32100  */
#define	AT_MT		  0

/*
 * access execution level
 */
#define	XLVL_K		0
#define	XLVL_U		3

/*
 * Fault types
 */
#define F_NONE          0
#define F_MPROC	        1
#define F_RMUPDATE      2
#define F_SDTLEN        3
#define F_PWRITE        4
#define F_PDTLEN        5
#define F_INVALID       6
#define F_SEG_N_P       7
#define F_OBJECT        8
#define F_PDT_N_P       9
#define F_P_N_P      0x0a	/* 10 */
#define F_INDIRECT   0x0b	/* 11 */
#define F_ACCESS     0x0d	/* 13 */
#define F_OFFSET     0x0e	/* 14 */
#define F_ACC_OFF    0x0f	/* 15 */
#define F_D_P_HIT    0x1f	/* 31 */

/*
 * Page Descriptor Cache (PDC)
 */

typedef union _mmupdc {
	struct
	{
		unsigned pdc_pa		: 21;	/* Page Address		*/
		unsigned		:  4;	/* Reserved		*/
		unsigned pdc_u		:  1;	/* Usage Bit		*/
		unsigned pdc_ref	:  1;	/* Referenced flag	*/
		unsigned pdc_wt		:  1;	/* Write Trap flag	*/
		unsigned pdc_c		:  1;	/* Cacheable Data flag	*/
		unsigned pdc_lpg	:  1;	/* Last Page flag	*/
		unsigned pdc_m		:  1;	/* Modified flag	*/
		unsigned pdc_g		:  1;	/* Good cache entry flag*/
	} pdcw1;				/* PDC Word 1		*/

	struct
	{
		unsigned pdc_accr	:  8;	/* Access rights	*/
		unsigned		:  8;	/* Reserved		*/
		unsigned pdc_tag	: 16;	/* Tag			*/
	} pdcw2;				/* PDC Word 2		*/

	int wrd;				/* PDC Word		*/
} _PDC;

/*
 * Segment Descriptor Cache (SDC)
 */

typedef union _mmusdc {
	struct
	{
		unsigned sdc_add	: 27;	/* Segment or PDT Address*/
		unsigned sdc_t		:  1;	/* Trap on Access flag	*/
		unsigned sdc_c		:  1;	/* Cacheable Data flag	*/
		unsigned sdc_cg		:  1;	/* Contiguous flag	*/
		unsigned sdc_m		:  1;	/* Modified flag	*/
		unsigned sdc_g		:  1;	/* Good Cache Entry flag*/
	} sdcw1;				/* SDC Word 1 */

	struct
	{
		unsigned sdc_acc	:  8;	/* Access Rights	*/
		unsigned sdc_off	: 14;	/* Segment's Max Offset	*/
		unsigned sdc_tag	: 10;	/* Tag			*/
	} sdcw2;				/* SDC Word 2		*/

	int wrd;				/* SDC Word		*/
} _SDC;

typedef struct _CONFIG {		/*  configuration register  */
/*							*/
/*  +------------------------------+-------+-----+	*/
/*  |           Reserved           |  ref  | mod |	*/
/*  +------------------------------+-------+-----+	*/
/*                30                   1      1		*/
/*							*/
    unsigned int     : 30;		/* Reserved			 */
    unsigned int ref :  1;		/* Update R bit in segment desc. */
    unsigned int mod :  1;		/* Update M bit in segment desc. */
} CR;

#define SRAMBSHIFT 10

#define OFF 0
#define ON  1

#define PT_SWIORD 0

/*  MMU-specific addresses  */

#define srama  ((SRAMA *) (&mmusrama))   /*  0x40600  */
#define sramb  ((SRAMB *) (&mmusramb))   /*  0x40700  */
#define fltcr  ((FLTCR *) (&mmufltcr))   /*  0x40800  */
#define fltar  ((int *)   (&mmufltar))   /*  0x40900  */
#define crptr  ((CR *)    (&mmucr))      /*  0x40a00  */
#define varptr ((int *)   (&mmuvar))     /*  0x40b00  */

#define	PSRAMA	0x00040600L			/* SRAMA Area		*/
#define Psrama  ((SRAMA *) (PSRAMA))

#define	PSRAMB	0x00040700L			/* SRAMB Area		*/
#define Psramb  ((SRAMB *) (PSRAMB))

#define	PFLTCR	0x00040800L			/* Fault Code Register	*/
#define Pfltcr  ((FLTCR *) (PFLTCR))

#define	PFLTAR	0x00040900L			/* Fault Address Register*/
#define Pfltar  ((int *)   (PFLTAR))

#define	PCR	0x00040a00L			/* Configuration Register*/
#define Pcrptr  ((CR *)    (PCR))

#define	PVAR	0x00040b00L			/* Virtual Address Register*/
#define Pvarptr ((int *)   (PVAR))

/*
 * Peripheral mode functions
 */
#define seflush(vaddr) (*((int *)(varptr)) = (int)(vaddr))
#define sectfl(SDT,sectno) (*((SRAMA *)(srama + (sectno))) = ((SRAMA)(SDT)))

#endif

/*
 * Demand paging support required constants (initialized by pstart.c) and macros
 */

#ifdef MULTI

unsigned int kv_base;	/*  kernel virtual address of first page mapped in pfdat  */
int page1;	/*  physical address of mapped to kv_base                 */
int pfdincr;	/*  page1 >> PGSHIFT                                       */

unsigned int firstfree;	/*  page frame number of first allocatable page           */
unsigned int lastfree;	/*  page frame number of last allocatable page            */
unsigned int maxfree;	/*  							  */
unsigned int min_free;  /*  threshold for handling copy-on-write faults           */

unsigned int freemem;	/*  number of available page frames                       */

unsigned int sysspace;  /*  kernel virtual address of first system space segment  */
                /*      -- used for NI and Datakit                        */
#else

extern unsigned int kv_base;
extern int page1;
extern int pfdincr;
extern unsigned int firstfree;
extern unsigned int lastfree;
extern unsigned int maxfree;
extern unsigned int min_free;
extern unsigned int freemem;
extern unsigned int sysspace;

#endif

#define SYSSPSZ 16
