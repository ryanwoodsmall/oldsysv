/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:include/access.h	1.3"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

/*
 * To speed up reading in each allow/deny file, ACC_MAX_GUESS slots
 * will be preallocated for the internal copy. If these files
 * are expected to be substantially larger than this, bump it up.
 */
#define ACC_MAX_GUESS	100

extern int		is_user_admin(),
			deny_user_form(),
			allow_user_form(),
			deny_user_printer(),
			allow_user_printer(),
			deny_form_printer(),
			allow_form_printer(),
			is_user_allowed(),
			is_user_allowed_form(),
			is_user_allowed_printer(),
			is_form_allowed_printer(),
			allowed(),
			load_userform_access(),
			load_userprinter_access(),
			load_formprinter_access(),
			loadaccess(),
			dumpaccess();

extern void		close_access();

extern char		*getaccessfile();

#if	defined(BUFSIZ)
extern FILE		*open_access();
#endif
