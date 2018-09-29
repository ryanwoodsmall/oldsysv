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
/*	@(#)reloc.h	2.1	*/
struct reloc {
	long	r_vaddr;	/* (virtual) address of reference */
	long	r_symndx;	/* index into symbol table */
	unsigned short	r_type;		/* relocation type */
	};


/*
 *   relocation types for all products and generics
 */

/*
 * All generics
 *	reloc. already performed to symbol in the same section
 */
# ifndef ns32000
#define  R_ABS		0
# endif

/*
 * X86 generic
 *	8-bit offset reference in 8-bits
 *	8-bit offset reference in 16-bits 
 *	12-bit segment reference
 *	auxiliary relocation entry
 */
#define	R_OFF8		07
#define R_OFF16		010
#define	R_SEG12		011
#define	R_AUX		013

/*
 * B16 and X86 generics
 *	16-bit direct reference
 *	16-bit "relative" reference
 *	16-bit "indirect" (TV) reference
 */
#define  R_DIR16	01
#define  R_REL16	02
#define  R_IND16	03

/*
 * 3B generic
 *	24-bit direct reference
 *	24-bit "relative" reference
 *	16-bit optimized "indirect" TV reference
 *	24-bit "indirect" TV reference
 *	32-bit "indirect" TV reference
 */
#define  R_DIR24	04
#define  R_REL24	05
#define  R_OPT16	014
#define  R_IND24	015
#define  R_IND32	016

/*
 * 3B and M32 generics
 *	32-bit direct reference
 */
#define  R_DIR32	06

/*
 * M32 generic
 *	32-bit direct reference with bytes swapped
 */
#define  R_DIR32S	012

/*
 * DEC Processors  VAX 11/780 and VAX 11/750
 *
 */

#define R_RELBYTE	017
#define R_RELWORD	020
#define R_RELLONG	021
#define R_PCRBYTE	022
#define R_PCRWORD	023
#define R_PCRLONG	024


#define	RELOC	struct reloc
#define	RELSZ	10	/* sizeof(RELOC) */

	/* Definition of a "TV" relocation type */

#if N3B
#define ISTVRELOC(x)	((x==R_OPT16)||(x==R_IND24)||(x==R_IND32))
#endif
#if B16 || X86
#define ISTVRELOC(x)	(x==R_IND16)
#endif
#if M32
#define ISTVRELOC(x)	(x!=x)	/* never the case */
#endif
#ifdef ns32000

#define	R_ADDRTYPE	0x000f		/* type of relocation item */
#define	  R_NOTHING	  0x0000	/* ignore */
#define	  R_ADDRESS	  0x0001	/* take address of symbol */
#define	  R_LINKENTRY	  0x0002	/* take symbol's link table entry
					   index */

#define R_RELTO		0x00f0		/* action to take when relocating */
#define	  R_ABS		  0x0000	/* keep symbol's address as such */
#define	  R_PCREL	  0x0010	/* subtract the pc address of hole */
#define	  R_SBREL	  0x0020	/* subtract the static base start of
					   this section's module */

#define R_FORMAT	0x0f00		/* relocation item data format */
#define	  R_NUMBER	  0x0000	/* retain as two's complement value */
#define	  R_DISPL	  0x0100	/* convert to NS32000 displacement */
#define	  R_PROCDES	  0x0200	/* convert to NS32000 procedure 
					   descriptor */
#define	  R_IMMED	  0x0300	/* convert to NS32000 immediat 
					   operand */

#define R_SIZESP	0xf000		/* relocation item size */
#define	  R_S_08	  0x0000	/* relocate 8 bits */
#define	  R_S_16	  0x1000	/* relocate 16 bits */
#define	  R_S_32	  0x2000	/* relocate 32 bits */

#endif
