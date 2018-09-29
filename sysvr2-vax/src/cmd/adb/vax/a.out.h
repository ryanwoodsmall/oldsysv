/*	@(#)a.out.h	1.2	*/
/*
 * Format of an a.out header
 */
 
#ifndef u3b

struct	exec {	/* a.out header */
#if u370
	int		a_magic;	/* magic number */
	int		a_stamp;	/* The version of a.out	*/
					/* format of this file.	*/
#else
	short		a_magic;	/* magic number */
#endif
#if vax
	short		a_stamp;
#endif
	unsigned	a_text;		/* size of text segment */
					/* in bytes		*/
					/* padded out to next	*/
					/* page boundary with	*/
					/* binary zeros.	*/
	unsigned	a_data;		/* size of initialized data */
					/* segment in bytes	*/
					/* padded out to next	*/
					/* page boundary with	*/
					/* binary zeros.	*/
	unsigned	a_bss;		/* Actual size of	*/
					/* uninitialized data	*/
					/* segment in bytes.	*/
	unsigned	a_entry;	/* entry point */
#if vax || u370
	unsigned	a_trsize;	/* size of text relocation */
	unsigned	a_drsize;	/* size of data relocation */
#endif
#if u370
	unsigned	a_origin;	/* The origin to which 	*/
					/* this file was	*/
					/* relocated.		*/
	unsigned	a_actext;	/* The actual size of	*/
					/* the text segment in	*/
					/* bytes.		*/
	unsigned	a_acdata;	/* The actual size of	*/
					/* the data segment in	*/
					/* bytes.		*/
#endif
#if pdp11
	char		a_unused;	/* not used */
	unsigned char	a_hitext;	/* high order text bits */
	char		a_flag;		/* reloc info stripped */
	char		a_stamp;	/* environment stamp */
#endif
};

#define	A_MAGIC1	0407		/* normal */
#define	A_MAGIC0	0401		/* lpd (UNIX/RT) */
#define	A_MAGIC2	0410		/* read-only text */
#define	A_MAGIC3	0411		/* separated I&D */
#define	A_MAGIC4	0405		/* overlay */
#define	A_MAGIC5	0437		/* system overlay, separated I&D */

#if vax || u370
struct relocation_info {
	  long  r_address;	/* relative to current segment */
	  unsigned int
		r_symbolnum:24,	/* if extern then symbol table */
				/* ordinal (0, 1, 2, ...) else */
				/* segment number (same as symbol types) */
	        r_pcrel:1, 	/* if so, segment offset has already */
				/* been subtracted */
	  	r_length:2,	/* 0=byte, 1=word, 2=long */
	  	r_extern:1,	/* does not include value */
				/* of symbol referenced */
	  	r_offset:1,	/* already includes origin */
				/* of this segment (?) */
		r_pad:3;	/* nothing, yet */
};
#endif
struct nlist
{
	char		*n_name;
	unsigned long	n_value;
	short		n_scnum;
	unsigned short	n_type;
	char		n_sclass;
	char		n_numaux;
};
/* in invocation of BADMAG macro, argument should not be a function. */

#define	BADMAG(X) (X.a_magic != A_MAGIC1 &&\
		X.a_magic != A_MAGIC2 &&\
		X.a_magic != A_MAGIC3 &&\
		X.a_magic != A_MAGIC4 &&\
		X.a_magic != A_MAGIC5 &&\
		X.a_magic != A_MAGIC0)

	/* values for type flag */

#define	N_UNDF	0	/* undefined */
#define	N_TYPE	037
#define	N_FN	037	/* file name symbol */

#if pdp11
#define	N_TEXT	02	/* text symbol */
#define	N_DATA	03	/* data symbol */
#define	N_BSS	04	/* bss symbol */
#define	N_REG	024	/* register name */
#define	N_EXT	040	/* external bit, or'ed in */
#define	FORMAT	"%.6o"	/* to print a value */
#else
/* by cfb 21 Aug 82 */
#define SECT1 1
#define SECT2 2
#define SECT3 3
/* end of cfb stuff */
#define	N_TEXT	04	/* text */
#define	N_DATA	06	/* data */
#define	N_BSS	010
#define	N_GSYM	0040	/* global sym: name,,type,0 */
#define	N_FNAME 0042	/* procedure name (f77 kludge): name,,,0 */
#define	N_FUN	0044	/* procedure: name,,linenumber,address */
#define	N_STSYM 0046	/* static symbol: name,,type,address */
#define	N_LCSYM 0050	/* .lcomm symbol: name,,type,address */
#define	N_BSTR	0060	/* begin structure: name,,, */
#define	N_RSYM	0100	/* register sym: name,,register,offset */
#define	N_SLINE	0104	/* src line: ,,linenumber,address */
#define	N_ESTR	0120	/* end structure: name,,, */
#define	N_SSYM	0140	/* structure elt: name,,type,struct_offset */
#define	N_SO	0144	/* source file name: name,,,address */
#define	N_BENUM	0160	/* begin enum: name,,, */
#define	N_LSYM	0200	/* local sym: name,,type,offset */
#define	N_SOL	0204	/* #line source filename: name,,,address */
#define	N_ENUM	0220	/* enum element: name,,,value */
#define	N_PSYM	0240	/* parameter: name,,type,offset */
#define	N_ENTRY	0244	/* alternate entry: name,,linenumber,address */
#define	N_EENUM	0260	/* end enum: name,,, */
#define	N_LBRAC	0300	/* left bracket: ,,nesting level,address */
#define	N_RBRAC	0340	/* right bracket: ,,nesting level,address */
#define	N_BCOMM	0342	/* begin common: name,,, */
#define	N_ECOMM	0344	/* end common: name,,, */
#define	N_ECOML	0350	/* end common (local name): ,,,address */
#define	N_STRU	0374	/* 2nd entry for structure: str tag,,,length */
#define	N_LENG	0376	/* second stab entry with length information */
#define	N_EXT	01	/* external bit, or'ed in */
#define	FORMAT	"%.8x"
#define	STABTYPES 0340
#endif

#else

/* static char ID_filhdrh[] = "@(#) filehdr.h: 4.1.1.12 8/27/80"; */
/*
 *	Format of a 3B a.out header
 */

struct filehdr {
	unsigned short	f_magic;	/* magic number */
	unsigned short	f_nscns;	/* number of sections */
	long		f_timdat;	/* time & date stamp */
	long		f_symptr;	/* file pointer to symtab */
	long		f_nsyms;	/* number of symtab entries */
	unsigned short	f_opthdr;	/* sizeof(optional hdr) */
	unsigned short	f_flags;	/* flags */
	};


/*
 *   Bits for f_flags:
 *
 *	F_RELFLG	relocation info stripped from file
 *	F_EXEC		file is executable  (i.e. no unresolved
 *				externel references)
 *	F_LNNO		line nunbers stripped from file
 *	F_LSYMS		local symbols stripped from file
 *	F_MINMAL	this is a minimal object file (".m") output of fextract
 *	F_UPDATE	this is a fully bound update file, output of ogen
 *	F_SWABD		this file has had its bytes swabbed (in names)
 *	F_AR16WR	this file created on AR16WR machine(e.g. 11/70)
 *	F_AR32WR	this file created on AR32WR machine(e.g. vax)
 *	F_AR32W		this file created on AR32W machine (e.g. 3b,maxi)
 *	F_PATCH		file contains "patch" list in optional header
 */

#define  F_RELFLG	0000001
#define  F_EXEC		0000002
#define  F_LNNO		0000004
#define  F_LSYMS	0000010
#define  F_MINMAL	0000020
#define  F_UPDATE	0000040
#define  F_SWABD	0000100
#define  F_AR16WR	0000200
#define  F_AR32WR	0000400
#define  F_AR32W	0001000
#define  F_PATCH	0002000

/*
 *   Magic Numbers
 */

#define  N3BMAGIC	0550

#define	FILHDR	struct filehdr
#define	FILHSZ	sizeof(FILHDR)

/* static char ID_scnhdrh[] = "@(#) scnhdr.h: 4.1.1.9 8/22/80"; */

/*
 *	Format of a 3B a.out section header
 */

struct scnhdr {
	char		s_name[8];	/* section name */
	long		s_paddr;	/* physical address */
	long		s_vaddr;	/* virtual address */
	long		s_size;		/* section size */
	long		s_scnptr;	/* file ptr to raw data for section */
	long		s_relptr;	/* file ptr to relocation */
	long		s_lnnoptr;	/* file ptr to line numbers */
	unsigned short	s_nreloc;	/* number of relocation entries */
	unsigned short	s_nlnno;	/* number of line number entries */
	long		s_flags;	/* flags */
	};

#define	SCNHDR	struct scnhdr
#define	SCNHSZ	sizeof(SCNHDR)

/*
 * Define constants for names of "special" sections
 */

#define _TEXT ".text"
#define _DATA ".data"
#define _BSS  ".bss"

/*
 * The low 4 bits of s_flags is used as a section "type"
 */

#define STYP_REG	0x00		/* "regular" section:
						allocated, relocated, loaded */
#define STYP_DSECT	0x01		/* "dummy" section:
						not allocated, relocated,
						not loaded */
#define STYP_NOLOAD	0x02		/* "noload" section:
						allocated, relocated,
						 not loaded */
#define STYP_GROUP	0x04		/* "grouped" section:
						formed of input sections */
#define STYP_PAD	0x08		/* "padding" section:
						not allocated, not relocated,
						 loaded */

/*
 * static char ID_aouth[] = "@(#) aouthdr.h: 4.1.1.2 8/22/80";
 * static char ID_sgsh[] = "@(#) sgs.h: 2.1.1.9 8/22/80";
 */

/*
 *	Format of the UNIX (optional) header
 */

/*	The symbol N3BMAGIC is defined in filehdr.h	*/

#define MAGIC	N3BMAGIC
#define TVMAGIC (MAGIC+1)

#define ISMAGIC(x)	((((unsigned short)x)==(unsigned short)MAGIC)||(((unsigned short)x)==(unsigned short)TVMAGIC))

/* Must include ar.h so that ARMAG is defined for ISARCHIVE and BADMAGIC */
#include	<ar.h>

#define	ISARCHIVE(x)	((unsigned short) x == (unsigned short) ARMAG)
#define BADMAGIC(x)	((((x) >> 8) < 7) && !ISMAGIC(x) && !ISARCHIVE(x))

/*
 *	When a UNIX aout header is to be built in the optional header,
 *	the following magic numbers can appear in that header:
 *
 *		AOUT1MAGIC : default : one output segment is generated
 *		AOUT2MAGIC :	      : two segments are generated
 *						(separate I and D space )
 */

#define AOUT1MAGIC 0407
#define AOUT2MAGIC 0410

typedef	struct aouthdr {
	short	magic;		/* see magic.h				*/
	short	flags;		/* see below				*/
	long	tsize;		/* text size in bytes, padded to FW
				   bdry					*/
	long	dsize;		/* initialized data "  "		*/
	long	bsize;		/* uninitialized data "   "		*/
	long	nsize;		/* nsyms table size in bytes		*/
	long	ssize;		/* ssyms table size in bytes		*/
	long	entry;		/* entry pt.				*/
	long	text_start;	/* base of text used for this file	*/
	long	data_start;	/* base of data used for this file	*/
} AOUTHDR;


/*
 *	Bits for a.out header flags word
 */

/* set if reloc info stripped from file					*/
#define	RELFLG		01
/* set if user specified base of text (default 0)			*/
#define	SETTEXT		02
/* set if user specified base of data (def next seg after text)		*/
#define	SETDATA		04
/* set if nsyms not given						*/
#define	NSYMFLG		010

/* static char ID_reloch[] = "@(#) reloc.h: 4.1.1.14 8/22/80"; */

/*
 *	Format of a 3B relocation entry
 */

struct reloc {
	long	r_vaddr;	/* (virtual) address of reference */
	long	r_symndx;	/* index into symbol table */
	unsigned short	r_type;		/* relocation type */
	};


/*
 *	relocation types
 *
 *	reloc. already performed to symbol in the same section
 *	24-bit direct reference
 *	24-bit "relative" reference
 *	16-bit optimized "indirect" TV reference
 *	24-bit "indirect" TV reference
 *	32-bit "indirect" TV reference
 *	32-bit direct reference
 */

#define  R_ABS		0
#define  R_DIR24	04
#define  R_REL24	05
#define  R_OPT16	014
#define  R_IND24	015
#define  R_IND32	016
#define  R_DIR32	06

#define	RELOC	struct reloc
#define	RELSZ	10	/* sizeof(RELOC) */

/* static char ID_lnumh[] = "@(#) linenum.h: 4.1.1.5 8/22/80"; */

/*
 *	Format of a 3B line number entry
 *
 *  There is one line number entry for every 
 *  "breakpointable" source line in a section.
 *  Line numbers are grouped on a per function
 *  basis; the first entry in a function grouping
 *  will have l_lnno = 0 and in place of physical
 *  address will be the symbol table index of
 *  the function name.
 */

struct lineno
{
	union
	{
		long	l_symndx ;	/* sym. table index of function name
						iff l_lnno == 0      */
		long	l_paddr ;	/* (physical) address of line number */
	}		l_addr ;
	unsigned short	l_lnno ;	/* line number */
} ;

#define	LINENO	struct lineno
#define	LINESZ	6	/* sizeof(LINENO) */

/*
 *   STORAGE CLASSES
 */

#define  C_EFCN          -1    /* physical end of function */
#define  C_NULL          0
#define  C_AUTO          1     /* automatic variable */
#define  C_EXT           2     /* external symbol */
#define  C_STAT          3     /* static */
#define  C_REG           4     /* register variable */
#define  C_EXTDEF        5     /* external definition */
#define  C_LABEL         6     /* label */
#define  C_ULABEL        7     /* undefined label */
#define  C_MOS           8     /* member of structure */
#define  C_ARG           9     /* function argument */
#define  C_STRTAG        10    /* structure tag */
#define  C_MOU           11    /* member of union */
#define  C_UNTAG         12    /* union tag */
#define  C_TPDEF         13    /* type definition */
#define C_USTATIC	 14    /* undefined static */
#define  C_ENTAG         15    /* enumeration tag */
#define  C_MOE           16    /* member of enumeration */
#define  C_REGPARM	 17    /* register parameter */
#define  C_FIELD         18    /* bit field */
#define  C_BLOCK         100   /* ".bb" or ".eb" */
#define  C_FCN           101   /* ".bf" or ".ef" */
#define  C_EOS           102   /* end of structure */
#define  C_FILE          103   /* file name */

#define  C_ALIAS	 105   /* duplicate tag */

/*		Number of characters in a symbol name */
#define  SYMNMLEN	8
/*		Number of characters in a file name */
#define  FILNMLEN	14
/*		Number of array dimensions in auxiliary entry */
#define  DIMNUM		4

struct syment
{
	union
	{
		char	*n_name;
		struct
		{
			long	_n_zeroes;
			long	_n_offset;
		} _n_n;
		char	*_n_nptr[ 2 ];
	} _n;
	unsigned long	n_value;	/* value of symbol */
	short	n_scnum;	/* section number */
	unsigned short	n_type;	/* type and derived type */
	char	n_sclass;	/* storage class */
	char	n_numaux;	/* number of auxiliary entries */
};

#define	n_name		_n._n_name
#define	n_nptr		_n._n_nptr[ 1 ]
#define	n_zeroes	_n._n_n._n_zeroes
#define	n_offset	_n._n_n._n_offset

/*	UNIX name for a symbol table entry */

#define	nlist	syment

/*
   Relocatable symbols have a section number of the
   section in which they are defined.  Otherwise, section
   numbers have the following meanings:
*/
        /* undefined symbol */
#define  N_UNDEF	0
        /* special debugging symbol -- value of symbol is meaningless */
#define  N_DEBUG	-2

/*
   The fundamental type of a symbol packed into the low 
   4 bits of the word.
*/

#define  T_NULL     0
#define  T_ARG      1          /* function argument (only used by compiler) */
#define  T_CHAR     2          /* character */
#define  T_SHORT    3          /* short integer */
#define  T_INT      4          /* integer */
#define  T_LONG     5          /* long integer */
#define  T_FLOAT    6          /* floating point */
#define  T_DOUBLE   7          /* double word */
#define  T_STRUCT   8          /* structure  */
#define  T_UNION    9          /* union  */
#define  T_ENUM     10         /* enumeration  */
#define  T_MOE      11         /* member of enumeration */
#define  T_UCHAR    12         /* unsigned character */
#define  T_USHORT   13         /* unsigned short */
#define  T_UINT     14         /* unsigned integer */
#define  T_ULONG    15         /* unsigned long */

/*
 * derived types are:
 */

#define  DT_NON      0          /* no derived type */
#define  DT_PTR      1          /* pointer */
#define  DT_FCN      2          /* function */
#define  DT_ARY      3          /* array */

/*
 *   type packing constants
 */

#define  N_BTMASK     017
#define  N_TMASK      060
#define  N_TMASK1     0300
#define  N_TMASK2     0360
#define  N_BTSHFT     4
#define  N_TSHIFT     2

/*
 *   MACROS
 */

	/*   Basic Type of  x   */

#define  BTYPE(x)  ((x) & N_BTMASK)

	/*   Is  x  a  pointer ?   */

#define  ISPTR(x)  (((x) & N_TMASK) == (DT_PTR << N_BTSHFT))

	/*   Is  x  a  function ?  */

#define  ISFCN(x)  (((x) & N_TMASK) == (DT_FCN << N_BTSHFT))

	/*   Is  x  an  array ?   */

#define  ISARY(x)  (((x) & N_TMASK) == (DT_ARY << N_BTSHFT))

	/* Is x a structure, union, or enumeration TAG? */

#define ISTAG(x)  ((x)==C_STRTAG || (x)==C_UNTAG || (x)==C_ENTAG)

#define  INCREF(x) ((((x)&~N_BTMASK)<<N_TSHIFT)|(DT_PTR<<N_BTSHFT)|(x&N_BTMASK))

#define  DECREF(x) ((((x)>>N_TSHIFT)&~N_BTMASK)|((x)&N_BTMASK))

/*
 *	AUXILIARY ENTRY FORMAT
 */

union auxent
{
	struct
	{
		long		x_tagndx;	/* str, un, or enum tag indx */
		union
		{
			struct
			{
				unsigned short	x_lnno;	/* declaration line number */
				unsigned short	x_size;	/* str, union, array size */
			} x_lnsz;
			long	x_fsize;	/* size of function */
		} x_misc;
		union
		{
			struct			/* if ISFCN, tag, or .bb */
			{
				long	x_lnnoptr;	/* ptr to fcn line # */
				long	x_endndx;	/* entry ndx past block end */
			} 	x_fcn;
			struct			/* if ISARY, up to 4 dimen. */
			{
				unsigned short	x_dimen[DIMNUM];
			} 	x_ary;
		}		x_fcnary;
		unsigned short  x_tvndx;		/* tv index */
	} 	x_sym;
	struct
	{
		char	x_fname[FILNMLEN];
	} 	x_file;
        struct
        {
                long    x_scnlen;          /* section length */
                unsigned short  x_nreloc;  /* number of relocation entries */
                unsigned short  x_nlinno;  /* number of line numbers */
        }       x_scn;

	struct
	{
		unsigned short	x_tvlen;	/* length of .tv */
		unsigned short	x_tvran[2];	/* tv range */
		long		x_tvfill;	/* tv fill value */
	}	x_tv;	/* info about .tv section (in auxent of symbol .tv)) */
};

#define	SYMENT	struct syment
#define	SYMESZ	18	/* sizeof(SYMENT) */

#define	AUXENT	union auxent
#define	AUXESZ	18	/* sizeof(AUXENT) */

#endif
