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
/*
 * @(#)a.out.h	1.4	7/13/83
 * @(#)Copyright (C) 1983 by National Semiconductor Corp.
 */

/*
 * A.out file layout information:
 *
 * "+" indicates an addition to the Berkeley 4.1 format.
 *
 * Note:
 *	This file contains only an incomplete description of information
 *	present in an a.out file; it omits symbol table definitions.
 *
 *	Complete symbol table definitions reside in <stab.h> and simplified
 *	definitions in <nlist.h>.  These sets of definitions are not fully
 *	compatible with each other; symbol table information as defined in
 *	<nlist.h> does not correspond directly to a.out file contents, but
 *	instead is obtained by translating such information with nlist(3).
 */

/*
 * Header prepended to each a.out file.
 */
struct exec {
	 long	a_magic;	/* magic number				*/
unsigned long	a_text;		/* size of text segment			*/
unsigned long	a_data;		/* size of initialized data		*/
unsigned long	a_bss;		/* size of uninitialized data		*/
unsigned long	a_syms;		/* size of symbol table			*/
unsigned long	a_entry;	/* entry point address			*/
unsigned long	a_entry_mod;	/*+entry point - mod number		*/
unsigned long	a_trsize;	/* size of text relocation -
				 * specifies relocation of link
				 * tables at present			*/
unsigned long	a_drsize;	/* size of data relocation -
				 * current assembler sets to zero	*/
unsigned long	a_mod;		/*+size of mod table within text	*/
unsigned long	a_link;		/*+size of link table within text	*/
unsigned long	a_strings;	/*+size of string table			*/
unsigned long	a_text_addr;	/*+address of text section in address space */
unsigned long	a_mod_addr;	/*+address of mod table in address space */
unsigned long	a_dat_addr;	/*+address of data section in address space */
};

#define OMAGIC	0407		/* old impure format		*/
#define NMAGIC	0410		/* read-only text		*/
#define ZMAGIC	0413		/* demand load format		*/
#define XMAGIC	0414		/*+demand load format
				 *+locations 0-1023 unmapped	*/

/*
 * Macros that take exec structures as arguments and tell whether
 * the file has a reasonable magic number or give offsets to
 * text|symbols|strings.
 */
#define N_BADMAG(x) \
    (((x).a_magic)!=OMAGIC && ((x).a_magic)!=NMAGIC && \
     ((x).a_magic)!=XMAGIC && ((x).a_magic)!=ZMAGIC)

#define N_TXTOFF(x) \
	((x).a_magic>NMAGIC ? 1024 : sizeof (struct exec))
#define N_SYMOFF(x) \
	(N_TXTOFF(x) + (x).a_text + (x).a_data \
		     + (x).a_trsize + (x).a_drsize)
#define N_STROFF(x) \
	(N_SYMOFF(x) + (x).a_syms)
