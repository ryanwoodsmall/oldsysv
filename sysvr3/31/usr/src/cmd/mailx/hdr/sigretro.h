/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mailx:hdr/sigretro.h	1.5"
#

/*
 * mailx -- a modified version of a University of California at Berkeley
 *	mail program
 *
 * Define extra stuff not found in signal.h
 */

#ifndef SIG_HOLD

#define	SIG_HOLD	(int (*)()) 3		/* Phony action to hold sig */
#endif /* SIG_HOLD */

#ifndef BADSIG
#define	BADSIG		(int (*)()) -1		/* Return value on error */
#endif /* BADSIG */
