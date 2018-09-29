/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dump:common/flagdefs.h	1.5"
struct flaglist {
	int	setting;
	int	(*ldfcn)( );
};

#define	FLAG	struct flaglist
#define	FLAGSZ	sizeof(FLAG)

#define	A	0	/* for testing ldahread( ) */
#define	F	1	/* for testing ldfhread( ) */
#define	O	2	/* for testing ldohseek( ) */
#define	H	3	/* for testing ldshread( ) and ldnshread( ) */
#define	S	4	/* for testing ldsseek( ) and ldnsseek( ) */
#define	R	5	/* for testing ldrseek( ) and ldnrseek( ) */
#define	L	6	/* for testing ldlseek( ) and ldnlseek( ) */
#define Z	7	/* for testing ldlread( ) ldlinit( ) and ldlsearch( ) */
#define	T	8	/* for testing ldtbseek( ) ldtbread( ) ldtbindex( ) */
#define l	9	/* for dumping target shared library pathnames in the .lib
			 * section */
#if FLEXNAMES
#define C	10	/* for dumping string table */
#endif

#ifdef FLEXNAMES
#define NUMFLAGS	11
#else
#define NUMFLAGS	10
#endif

#define	ON	1
#define OFF	0


/* a list of FLAG structures is used to determine which functions to call */

/*
*/
