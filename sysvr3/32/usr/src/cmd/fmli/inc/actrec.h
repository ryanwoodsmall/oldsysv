/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 * 
 */

#ident	"@(#)vm.inc:src/inc/actrec.h	1.2"

/* indices for the fcntbl[] array */

#define AR_CLOSE	(0)		/* prepare this record for destruction */
#define AR_NONCUR	(1)		/* make this record noncurrent */
#define AR_CURRENT	(2)		/* make this record current */
#define AR_REREAD	(3)		/* reread this record */
#define AR_CTL		(4)		/* change something about this record */
#define AR_ODSH		(5)		/* handle tokens */
#define AR_REINIT	(6)		/* init during checkworld */
#define AR_HELP		(7)		/* help function */
#define AR_NUMFCN	(8)

struct actrec {
	char	*path;
	int	serial;
	int	id;	/* menu, form, process */
	int	flags;
	int	lifetime;
	struct slk	*slks;
	struct actrec	*prevrec;
	struct actrec	*nextrec;
	struct actrec	*backup;

	/* object dependent fields */

	char	*odptr;			/* any structure you want */
	int	(*fcntbl[AR_NUMFCN])();	/* any functions you want */
};

/* lifetimes */

#define AR_SHORTERM	(1)
#define AR_HELPTERM	(2)
#define AR_LONGTERM	(3)
#define AR_PERMANENT	(4)
#define AR_IMMORTAL	(5)

/* flags */

#define AR_SKIP		(1)	/* don't stop here on prev/next wdw */
#define AR_ALTSLKS	(2)	/* use alternate slks by default */

/* macros for calling activation functions */

#define arf_close(X,Y)	(*(X->fcntbl[AR_CLOSE]))(Y)
#define arf_current(X,Y)	(*(X->fcntbl[AR_CURRENT]))(Y)
#define arf_noncur(X,Y)	(*(X->fcntbl[AR_NONCUR]))(Y)
#define arf_reread(X,Y)	(*(X->fcntbl[AR_REREAD]))(Y)
#define arf_reinit(X,Y)	(*(X->fcntbl[AR_REINIT]))(Y)
#define arf_odsh(X,Y)	(*(X->fcntbl[AR_ODSH]))(X, Y)
#define arf_help(X,Y)	(*(X->fcntbl[AR_HELP]))(Y)

/* Definitions of standard menu functions */

extern int AR_MEN_CLOSE(), AR_MEN_CURRENT(), AR_MEN_NONCUR(), AR_MEN_CTL(), 
			AR_NOHELP(), AR_NOP();

extern token AR_MEN_ODSH();
