/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/disptab.c	1.5"
# include	"dispatch.h"

static void		r_H(),
			r_HS();

DISPATCH			dispatch_table[] = {
/* R_BAD_MESSAGE            */  0,			D_BADMSG,
/* S_NEW_QUEUE              */  0,			D_BADMSG,
/* R_NEW_QUEUE              */  0,			D_BADMSG,
/* S_ALLOC_FILES            */  alloc_files,		0,
/* R_ALLOC_FILES            */  0,			D_BADMSG,
/* S_PRINT_REQUEST          */  print_request,		0,
/* R_PRINT_REQUEST          */  0,			D_BADMSG,
/* S_START_CHANGE_REQUEST   */  start_change_request,	0,
/* R_START_CHANGE_REQUEST   */  0,			D_BADMSG,
/* S_END_CHANGE_REQUEST     */  end_change_request,	0,
/* R_END_CHANGE_REQUEST     */  0,			D_BADMSG,
/* S_CANCEL_REQUEST         */  cancel_request,		0,
/* R_CANCEL_REQUEST         */  0,			D_BADMSG,
/* S_INQUIRE_REQUEST        */  inquire_request,	0,
/* R_INQUIRE_REQUEST        */  0,                      D_BADMSG,
/* S_LOAD_PRINTER           */  load_printer,           D_ADMIN,
/* R_LOAD_PRINTER           */  r_H,                    D_BADMSG,
/* S_UNLOAD_PRINTER         */  unload_printer,         D_ADMIN,
/* R_UNLOAD_PRINTER         */  r_H,                    D_BADMSG,
/* S_INQUIRE_PRINTER_STATUS */  inquire_printer_status,	0,
/* R_INQUIRE_PRINTER_STATUS */  0,                      D_BADMSG,
/* S_LOAD_CLASS             */  load_class,             D_ADMIN,
/* R_LOAD_CLASS             */  r_H,                    D_BADMSG,
/* S_UNLOAD_CLASS           */  unload_class,           D_ADMIN,
/* R_UNLOAD_CLASS           */  r_H,                    D_BADMSG,
/* S_INQUIRE_CLASS          */  inquire_class,		0,
/* R_INQUIRE_CLASS          */  0,                      D_BADMSG,
/* S_MOUNT                  */  _mount,			D_ADMIN,
/* R_MOUNT                  */  r_H,                    D_BADMSG,
/* S_UNMOUNT                */  unmount,		D_ADMIN,
/* R_UNMOUNT                */  r_H,                    D_BADMSG,
/* S_MOVE_REQUEST           */  move_request,           D_ADMIN,
/* R_MOVE_REQUEST           */  r_H,                    D_BADMSG,
/* S_MOVE_DEST              */  move_dest,              D_ADMIN,
/* R_MOVE_DEST              */  r_HS,                   D_BADMSG,
/* S_ACCEPT_DEST            */  accept_dest,		D_ADMIN,
/* R_ACCEPT_DEST            */  r_H,                    D_BADMSG,
/* S_REJECT_DEST            */  reject_dest,		D_ADMIN,
/* R_REJECT_DEST            */  r_H,                    D_BADMSG,
/* S_ENABLE_DEST            */  enable_dest,		D_ADMIN,
/* R_ENABLE_DEST            */  r_H,                    D_BADMSG,
/* S_DISABLE_DEST           */  disable_dest,		D_ADMIN,
/* R_DISABLE_DEST           */  r_H,                    D_BADMSG,
/* S_LOAD_FILTER_TABLE      */  load_filter_table,	D_ADMIN,
/* R_LOAD_FILTER_TABLE      */  r_H,                    D_BADMSG,
/* S_UNLOAD_FILTER_TABLE    */  unload_filter_table,	D_ADMIN,
/* R_UNLOAD_FILTER_TABLE    */  r_H,                    D_BADMSG,
/* S_LOAD_PRINTWHEEL        */  load_printwheel,	D_ADMIN,
/* R_LOAD_PRINTWHEEL        */  r_H,                    D_BADMSG,
/* S_UNLOAD_PRINTWHEEL      */  unload_printwheel,	D_ADMIN,
/* R_UNLOAD_PRINTWHEEL      */  r_H,                    D_BADMSG,
/* S_LOAD_USER_FILE         */  load_user_file,		D_ADMIN,
/* R_LOAD_USER_FILE         */  r_H,                    D_BADMSG,
/* S_UNLOAD_USER_FILE       */  unload_user_file,	D_ADMIN,
/* R_UNLOAD_USER_FILE       */  r_H,                    D_BADMSG,
/* S_LOAD_FORM              */  load_form,		D_ADMIN,
/* R_LOAD_FORM              */  r_H,                    D_BADMSG,
/* S_UNLOAD_FORM            */  unload_form,		D_ADMIN,
/* R_UNLOAD_FORM            */  r_H,                    D_BADMSG,
/* S_RFS_MOUNT              */  0,			D_ADMIN,
/* R_RFS_MOUNT              */  0,                      D_BADMSG,
/* S_QUIET_ALERT            */  quiet,			D_ADMIN,
/* R_QUIET_ALERT            */  r_H,                    D_BADMSG,
/* S_SEND_FAULT             */  send_fault,		0,
/* R_SEND_FAULT             */  0,                      D_BADMSG,
/* S_SHUTDOWN               */  shutdown,               D_ADMIN,
/* R_SHUTDOWN               */  r_H,                    D_BADMSG,
};

#if	defined(TRACE_MALLOC)
static char		*routine_names[] = {
/* R_BAD_MESSAGE            */	0,
/* S_NEW_QUEUE              */	0,
/* R_NEW_QUEUE              */	0,
/* S_ALLOC_FILES            */	"alloc_files",
/* R_ALLOC_FILES            */	0,
/* S_PRINT_REQUEST          */	"print_request",
/* R_PRINT_REQUEST          */	0,
/* S_START_CHANGE_REQUEST   */	"start_change_request",
/* R_START_CHANGE_REQUEST   */	0,
/* S_END_CHANGE_REQUEST     */	"end_change_request",
/* R_END_CHANGE_REQUEST     */	0,
/* S_CANCEL_REQUEST         */	"cancel_request",
/* R_CANCEL_REQUEST         */	0,
/* S_INQUIRE_REQUEST        */	"inquire_request",
/* R_INQUIRE_REQUEST        */	0,
/* S_LOAD_PRINTER           */	"load_printer",
/* R_LOAD_PRINTER           */	0,
/* S_UNLOAD_PRINTER         */	"unload_printer",
/* R_UNLOAD_PRINTER         */	0,
/* S_INQUIRE_PRINTER_STATUS */	"inquire_printer_status",
/* R_INQUIRE_PRINTER_STATUS */	0,
/* S_LOAD_CLASS             */	"load_class",
/* R_LOAD_CLASS             */	0,
/* S_UNLOAD_CLASS           */	"unload_class",
/* R_UNLOAD_CLASS           */	0,
/* S_INQUIRE_CLASS          */	"inquire_class",
/* R_INQUIRE_CLASS          */	0,
/* S_MOUNT                  */	"_mount",
/* R_MOUNT                  */	0,
/* S_UNMOUNT                */	"unmount",
/* R_UNMOUNT                */	0,
/* S_MOVE_REQUEST           */	"move_request",
/* R_MOVE_REQUEST           */	0,
/* S_MOVE_DEST              */	"move_dest",
/* R_MOVE_DEST              */	0,
/* S_ACCEPT_DEST            */	"accept_dest",
/* R_ACCEPT_DEST            */	0,
/* S_REJECT_DEST            */	"reject_dest",
/* R_REJECT_DEST            */	0,
/* S_ENABLE_DEST            */	"enable_dest",
/* R_ENABLE_DEST            */	0,
/* S_DISABLE_DEST           */	"disable_dest",
/* R_DISABLE_DEST           */	0,
/* S_LOAD_FILTER_TABLE      */	"load_filter_table",
/* R_LOAD_FILTER_TABLE      */	0,
/* S_UNLOAD_FILTER_TABLE    */	"unload_filter_table",
/* R_UNLOAD_FILTER_TABLE    */	0,
/* S_LOAD_PRINTWHEEL        */	"load_printwheel",
/* R_LOAD_PRINTWHEEL        */	0,
/* S_UNLOAD_PRINTWHEEL      */	"unload_printwheel",
/* R_UNLOAD_PRINTWHEEL      */	0,
/* S_LOAD_USER_FILE         */	"load_user_file",
/* R_LOAD_USER_FILE         */	0,
/* S_UNLOAD_USER_FILE       */	"unload_user_file",
/* R_UNLOAD_USER_FILE       */	0,
/* S_LOAD_FORM              */	"load_form",
/* R_LOAD_FORM              */	0,
/* S_UNLOAD_FORM            */	"unload_form",
/* R_UNLOAD_FORM            */	0,
/* S_RFS_MOUNT              */	0,
/* R_RFS_MOUNT              */	0,
/* S_QUIET_ALERT            */	"quiet",
/* R_QUIET_ALERT            */	0,
/* S_SEND_FAULT             */	"send_fault",
/* R_SEND_FAULT             */	0,
/* S_SHUTDOWN               */	"shutdown",
/* R_SHUTDOWN               */	0,
};
#endif

/**
 ** dispatch() - DISPATCH A ROUTINE TO HANDLE A MESSAGE
 **/

void
dispatch (type, m, n)
	int			type;
	char			*m;
	NODE			*n;
{
	register DISPATCH	*pd	= &dispatch_table[type];


	if (0 <= type && type <= LAST_UMESSAGE && pd->fncp)
		if (!pd->fncp || pd->flags & D_BADMSG)
			send (n, R_BAD_MESSAGE);
		else if (pd->flags & D_ADMIN && !n->admin)
			if ((++pd)->fncp)
				(*pd->fncp) (n, type+1);
			else
				send (n, R_BAD_MESSAGE);
		else {
			TRACE_ON (routine_names[type]);
			(*pd->fncp) (m, n);
			TRACE_OFF (routine_names[type]);
		}
	return;
}

/**
 ** r_H() - SEND MNOPERM RESPONSE MESSAGE
 ** r_HS() - SEND MNOPERM RESPONSE MESSAGE
 **/

static void
r_H (n, type)
	NODE			*n;
	int			type;
{
	send (n, type, MNOPERM);
	return;
}

static void
r_HS (n, type)
	NODE			*n;
	int			type;
{
	send (n, type, MNOPERM, "");
	return;
}
