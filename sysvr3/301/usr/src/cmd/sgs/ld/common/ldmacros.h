/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:common/ldmacros.h	1.5"
#define MAXKEEPSIZE	1024

extern char	stat_name[];
#if FLEXNAMES
#define	PTRNAME(x)	((x)->n_zeroes == 0L) \
				? (x)->n_nptr \
				: strncpy( stat_name, (x)->n_name, 8 )
#define SYMNAME(x)	(x.n_zeroes == 0L) \
				? x.n_nptr \
				: strncpy( stat_name, x.n_name, 8 )
#else
#define PTRNAME(x)	strncpy( stat_name, (x)->n_name, 8 )
#define SYMNAME(x)	strncpy( stat_name, x.n_name, 8 )
#endif

#define OKSCNNAME(x)	((tvflag && (!rflag || aflag)) || strncmp( x, _TV, 8 ))
