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
/*	@(#)5.0.ar.h	1.2	*/
#if vax || ns32000 || u3b

/*		5.0 COMMON ARCHIVE FORMAT


	ARCHIVE File Organization:

	_______________________________________________
	|__________ARCHIVE_HEADER_DATA________________|
	|					      |
	|	Archive Header	"ar_hdr"	      |
	|.............................................|
	|					      |
	|	Symbol Directory "ar_sym"	      |
	|					      |
	|_____________________________________________|
	|________ARCHIVE_FILE_MEMBER_1________________|
	|					      |
	|	Archive File Header "arf_hdr"	      |
	|.............................................|
	|					      |
	|	Member Contents (either a.out.h       |
	|			 format or text file) |
	|_____________________________________________|
	|					      |
	|	.		.		.     |
	|	.		.		.     |
	|	.		.		.     |
	|_____________________________________________|
	|________ARCHIVE_FILE_MEMBER_n________________|
	|					      |
	|	Archive File Header "arf_hdr"	      |
	|.............................................|
	|					      |
	|	Member Contents (either a.out.h       |
	|			 format or text file) |
	|_____________________________________________|              */


#define	ARMAG5	"<ar>"
#define	SARMAG5	4


struct	ar_hdr5 {			/* archive header */
	char	ar_magic[SARMAG5];	/* magic number */
	char	ar_name[16];		/* archive name */
	char	ar_date[4];		/* date of last archive modification */
	char	ar_syms[4];		/* number of ar_sym entries */
};

struct	ar_sym5 {			/* archive symbol table entry */
	char	sym_name[8];		/* symbol name, recognized by ld */
	char	sym_ptr[4];		/* archive position of symbol */
};

struct	arf_hdr5 {			/* archive file member header */
	char	arf_name[16];		/* file member name */
	char	arf_date[4];		/* file member date */
	char	arf_uid[4];		/* file member user identification */
	char	arf_gid[4];		/* file member group identification */
	char	arf_mode[4];		/* file member mode */
	char	arf_size[4];		/* file member size */
};

#else /* u370 || pdp11 */

/*				At least let convert compile on other machines
*	#define	ARMAG	0177545
*	struct	ar_hdr {
*		char	ar_name[14];
*		long	ar_date;
*		char	ar_uid;
*		char	ar_gid;
*		int	ar_mode;
*		long	ar_size;
*	};
*/

#endif
