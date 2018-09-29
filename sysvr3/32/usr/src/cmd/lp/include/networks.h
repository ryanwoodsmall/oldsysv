/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:include/networks.h	1.1"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

/**
 ** The internal copy of a network as seen by the rest of the world:
 **/

/*
 * A (char **) list is an array of string pointers (char *) with
 * a null pointer after the last item.
 */
typedef struct NETWORK {
	char   *name;		/* name of network (redundant) */
	char   *xfer;		/* file transfer command */
	char   *rex;		/* remote execution command */
	char   **machines;	/* names of machines on network */
}			NETWORK;

/**
 ** Various routines.
 **/

extern NETWORK		*getnetwork();

extern int		putnetwork(),
			delnetwork(),
			delmachine();

extern char		*badmachine;
