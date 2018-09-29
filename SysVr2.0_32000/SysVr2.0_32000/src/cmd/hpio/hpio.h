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
#define	SIZEFMT	"%11ld"	/* format string for size field */
#define	MODEFMT	"%d"	/* format string for mode field */
#define	CHKFMT	"%d"	/* format string for checksum field */
#define	SSIZ	12	/* # of characters in file size field */
#define	NSIZ	120	/* # of characters in file name field */
#define	MSIZ	7	/* # of characters in file mode field */

/*
**	hpio file header structure
*/

struct FH {
	char	FHsize[SSIZ];	/* ASCII representation of file size. */
	char	FHname[NSIZ];	/* File name. */
	char	FHmode[MSIZ];	/* File mode. */
	char	FHchkf;		/* Checksum flag. */
};

/* <@(#)hpio.h	1.1> */
