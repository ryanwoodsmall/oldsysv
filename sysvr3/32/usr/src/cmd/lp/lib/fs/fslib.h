/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/fs/fslib.h	1.1"

#include "sys/types.h"

#include "fs.h"

#include "sys/fs/s5dir.h"
#include "signal.h"
#include "sys/stat.h"
#include "errno.h"
#include "string.h"

#include "lp.h"

/*
 * Define one of the following as the starting condition.
 *
 * NOTE: Currently we start with 0, because this code doesn't
 * seem to work on non-SVR3 machines! (spec. 2.0.5). The SIGSYS
 * trap occurs, but on leaving the trap, another SIGSYS occurs
 * and dumps core!
 */
#define SVR3_STARTUP_CHOICE	-1	/* check it out */
/* #define SVR3_STARTUP_CHOICE	 0	/* never try SVR3 features */
/* #define SVR3_STARTUP_CHOICE	 1	/* always try SVR3 features */

extern struct dirent	*s5readdir();

extern int		s5getdents(),
			s5rmdir(),
			s5mkdir();

extern void		see_if_SVR3();

/*
 * ``Tri-state logic'':		 0 if not running SVR3
 *				 1 if running SVR3
 *				-1 if we don't know
 */
extern int		have_SVR3;
