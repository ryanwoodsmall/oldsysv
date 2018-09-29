/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:include/class.h	1.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

/**
 ** The internal flags seen by the Spooler/Scheduler and anyone who asks.
 **/

#define CS_REJECTED	0x001

/**
 ** The internal copy of a class as seen by the rest of the world:
 **/

/*
 * A (char **) list is an array of string pointers (char *) with
 * a null pointer after the last item.
 */
typedef struct CLASS {
	char   *name;		/* name of class (redundant) */
	char   **members;       /* members of class */
}			CLASS;

/**
 ** Various routines.
 **/

extern CLASS		*getclass();

extern int		putclass(),
			delclass();

extern void		freeclass();
