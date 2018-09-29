/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/trg.h	1.5"

/* target dependent definitions */
#define BTSIZ	503	/* size of branch hash table, btlst */
#define MAXBT	503	/* initial size of the btorder array */
#define NONE	-3	/* illegal section index number */
#define EXPSIZ	503	/* Size of export symbol hash table, explst */
#define HIDSIZ	503	/* Size of hide symbol hash table, hidelst */
#define USSIZ	503	/* Size of undefined symbol hash table, uslst */
#define	OBJCHUNK 500	/* Initial space declaration - chunk this size obtained */
			/* whenever space is needed */
#define SLCHUNK	8	/* Initial space for number of dirs given with -L option */

/* structure used to hold information about the branch table */
typedef struct btrec {
        char    *name;  /* symbol name */
        long    pos;    /* position in branch table */
        int     found;  /* indicates if symbol was found in pltname (initially 0) */
        struct btrec *next;
} btrec;

/* Structure used to hold information about a symbol referenced in another */
/* shared library 							   */
typedef struct usdef {		/* Undefined symbol definition structure */
	char	*name;		/* Symbol name */
	unsigned long	value;	/* Absolute value of symbol */
	SYMENT	**objdefs;	/* Defined symbols found in object file */
	int	nobjdefs;	/* Number of defined symbols found in obj file */
	int	found;		/* Indicates if symbol found */
	char	*offset;	/* Offset into object file in archive	*/
	struct	usdef	*next;	/* Linked list of undefined symbols found in */
				/* other shared libraries */
} usdef;

extern char	*pltname,
		*tpltnam,
		*btname,
		*ifil1name,
		*ifil2name,
		*ifil3name,
		*ifil4name;
extern char	*trgpath;
extern char	*assemnam;
extern btrec	*btlst[BTSIZ];
extern usdef	*uslst[USSIZ];
extern long	tabsiz;
extern btrec	**btorder;

/* User supplied functions */
extern void	genbtassem();
extern void	genlibassem();
extern void	labelassem();
extern char	*makelab();
extern void	mkassem();
extern void	mkplt();
extern void	mktarget();
extern btrec	*newbtrec();
extern void	patchsym();
extern void	prlbtassem();
