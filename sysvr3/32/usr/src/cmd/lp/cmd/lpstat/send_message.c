/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpstat/send_message.c	1.4"

#include "stdio.h"

#include "lp.h"
#include "msgs.h"

#define	WHO_AM_I	I_AM_LPSTAT
#include "oam.h"

#include "lpstat.h"


/**
 ** send_message() - HANDLE MESSAGE SENDING TO SPOOLER
 **/

/*VARARGS1*/
void			send_message (type, p1, p2, p3, p4, p5)
	int			type;
	char			*p1,
				*p2,
				*p3,
				*p4,
				*p5;
{
	char			req[MSGMAX];


	if (!scheduler_active)
		return;

	switch (type) {
	case S_INQUIRE_CLASS:
	case S_INQUIRE_PRINTER_STATUS:
	case S_INQUIRE_REQUEST:
		(void)putmessage (req, type, p1, p2, p3, p4, p5);
		break;
	}

	if (msend(req) == -1) {
		LP_ERRMSG (ERROR, E_LP_MSEND);
		done(1);
	}
	return;
}
