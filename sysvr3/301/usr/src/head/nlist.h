/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:nlist.h	1.8"
/*
*/

#if vax || u3b || M32 || u3b15 || u3b5 || u3b2

/* symbol table entry structure */

struct nlist
{
	char		*n_name;	/* symbol name */
	long		n_value;	/* value of symbol */
	short		n_scnum;	/* section number */
	unsigned short	n_type;		/* type and derived type */
	char		n_sclass;	/* storage class */
	char		n_numaux;	/* number of aux. entries */
};

#else /* pdp11 || u370 */

struct nlist
{
	char		n_name[8];	/* symbol name */
#if pdp11
	int		n_type;		/* type of symbol in .o */
#else
	char		n_type;
	char		n_other;
	short		n_desc;
#endif
	unsigned	n_value;	/* value of symbol */
};

#endif
