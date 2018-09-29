/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpstat/lpstat.h	1.3"

#define	DESTMAX	14	/* max length of destination name */
#define	SEQLEN	8	/* max length of sequence number */
#define	IDSIZE	DESTMAX+SEQLEN+1	/* maximum length of request id */
#define	LOGMAX	15	/* maximum length of logname */
#define	OSIZE	7

#define INQ_UNKNOWN	-1
#define INQ_ACCEPT	0
#define INQ_PRINTER	1
#define INQ_STORE	2
#define INQ_USER	3

typedef struct mounted {
	char			*name,
				**printers;
	struct mounted		*forward;
}			MOUNTED;

extern char		**get_charsets();

extern void		do_accept(),
			do_class(),
			do_device(),
			do_charset(),
			do_printer(),
			do_form(),
			do_request(),
			do_user(),
			done(),
			def(),
			running(),
			send_message(),
			startup(),
			add_mounted(),
			putoline(),
			putpline(),
			putqline(),
			parse();

extern int		exit_rc,
			inquire_type,
			verbose,
			output(),
			D,
			scheduler_active;

extern char		*alllist[];

extern MOUNTED		*mounted_forms,
			*mounted_pwheels;
