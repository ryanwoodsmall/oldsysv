/*	@(#)ar.h	2.1	*/
#if vax || u3b

/*		COMMON ARCHIVE FORMAT


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


#define	ARMAG	"<ar>"
#define	SARMAG	4


struct	ar_hdr {			/* archive header */
	char	ar_magic[SARMAG];	/* magic number */
	char	ar_name[16];		/* archive name */
	char	ar_date[4];		/* date of last archive modification */
	char	ar_syms[4];		/* number of ar_sym entries */
};

struct	ar_sym {			/* archive symbol table entry */
	char	sym_name[8];		/* symbol name, recognized by ld */
	char	sym_ptr[4];		/* archive position of symbol */
};

struct	arf_hdr {			/* archive file member header */
	char	arf_name[16];		/* file member name */
	char	arf_date[4];		/* file member date */
	char	arf_uid[4];		/* file member user identification */
	char	arf_gid[4];		/* file member group identification */
	char	arf_mode[4];		/* file member mode */
	char	arf_size[4];		/* file member size */
};

#else /* u370 || pdp11 */
		
#define	ARMAG	0177545
struct	ar_hdr {
	char	ar_name[14];
	long	ar_date;
	char	ar_uid;
	char	ar_gid;
	int	ar_mode;
	long	ar_size;
};

#endif
