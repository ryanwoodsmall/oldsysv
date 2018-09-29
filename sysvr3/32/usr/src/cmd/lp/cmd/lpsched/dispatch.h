/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/dispatch.h	1.4"
#include "time.h"
# include	"lpsched.h"

 void			alloc_files(),
			print_request(),
			start_change_request(),
			end_change_request(),
			cancel_request(),
			inquire_request(),
			load_printer(),
			unload_printer(),
			inquire_printer_status(),
			load_class(),
			unload_class(),
                        inquire_class(),
			_mount(),
			unmount(),
			move_request (),
			move_dest (),
			accept_dest(),
			reject_dest(),
			enable_dest(),
			disable_dest(),
			load_filter_table(),
			unload_filter_table(),
			load_printwheel(),
			unload_printwheel(),
			load_user_file(),
			unload_user_file(),
			load_form(),
			unload_form(),
			shutdown(),
			quiet(),
			send_fault();

/**
 ** dispatch_table[]
 **/

/*
 * The dispatch table is used to decide if we should handle
 * a message and which function should be used to handle it.
 *
 * D_ADMIN is set for messages that should be handled
 * only if it came from an administrator. These entries should
 * have a corresponding entry for the R_... message case, that
 * provides a routine for sending back a MNOPERM message to those
 * that aren't administrators. This is needed because the response
 * message varies in size with the message type.
 */

typedef struct DISPATCH {
	void			(*fncp)();
	ushort			flags;
}			DISPATCH;

#define	D_ADMIN		0x01	/* Only "lp" or "root" can use msg. */
#define D_BADMSG	0x02	/* We should never get this message */

