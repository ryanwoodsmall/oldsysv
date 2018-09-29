/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpadmin/send_message.c	1.6"

#include "stdio.h"
#include "varargs.h"

#include "msgs.h"

#define	WHO_AM_I	I_AM_LPADMIN
#include "oam.h"

#include "lpadmin.h"


/**
 ** send_message() - HANDLE MESSAGE SENDING TO SPOOLER
 **/

/*VARARGS0*/

void			send_message (va_alist)
	va_dcl
{
	va_list			ap;

	int			type,
				n;

	char			*p1,
				*p2,
				*p3,
				req[MSGMAX];


	if (!scheduler_active)
		return;

	va_start (ap);
	switch ((type = va_arg(ap, int))) {

	case S_LOAD_PRINTER:
	case S_UNLOAD_PRINTER:
	case S_LOAD_CLASS:
	case S_UNLOAD_CLASS:	
	case S_LOAD_PRINTWHEEL:
	case S_UNLOAD_PRINTWHEEL:
	case S_INQUIRE_PRINTER_STATUS:
	case S_PRINT_REQUEST:
	case S_ENABLE_DEST:
	case S_ACCEPT_DEST:
	case S_CANCEL_REQUEST:
		p1 = va_arg(ap, char *);
		(void)putmessage (req, type, p1);
		break;

	case S_MOUNT:
	case S_UNMOUNT:
		p1 = va_arg(ap, char *);
		p2 = va_arg(ap, char *);
		p3 = va_arg(ap, char *);
		(void)putmessage (req, type, p1, p2, p3);
		break;

	case S_ALLOC_FILES:
		n = va_arg(ap, int);
		(void)putmessage (req, type, n);
		break;

	case S_QUIET_ALERT:
		p1 = va_arg(ap, char *);
		n = va_arg(ap, int);
		(void)putmessage (req, type, p1, n);
		break;

	case S_REJECT_DEST:
		p1 = va_arg(ap, char *);
		p2 = va_arg(ap, char *);
		(void)putmessage (req, type, p1, p2);
		break;

	case S_DISABLE_DEST:
		p1 = va_arg(ap, char *);
		p2 = va_arg(ap, char *);
		n = va_arg(ap, int);
		(void)putmessage (req, type, p1, p2, n);
		break;

	}
	va_end (ap);

	if (msend(req) == -1) {
		LP_ERRMSG (ERROR, E_LP_MSEND);
		done(1);
	}
	return;
}
