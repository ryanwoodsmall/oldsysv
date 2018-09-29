/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:include/requests.h	1.9"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

/**
 ** The disk copy of the request files:
 **/

/*
 * There are 17 fields in the request file.
 */
#define RQ_MAX	17
# define RQ_COPIES	0
# define RQ_DEST	1
# define RQ_FILE	2
# define RQ_FORM	3
# define RQ_HANDL	4
# define RQ_NOTIFY	5
# define RQ_OPTS	6
# define RQ_PRIOR	7
# define RQ_PAGES	8
# define RQ_CHARS	9
# define RQ_TITLE	10
# define RQ_MODES	11
# define RQ_TYPE	12
# define RQ_USER	13
# define RQ_RAW		14
# define RQ_FAST	15
# define RQ_STAT	16

/**
 ** The internal copy of a request as seen by the rest of the world:
 **/

/*
 * A (char **) list is an array of string pointers (char *) with
 * a null pointer after the last item.
 */
typedef struct REQUEST {
	short  copies;        /* number of copies of request to print */
	char   *destination;  /* printer or class name */
	char   **file_list;   /* list of files to print: req. content */
	char   *form;         /* preprinted form to print on */
	ushort actions;       /* mail/write, immediate/hold/resume, raw */
	char   *alert;        /* program to run to alert user when done */
	char   *options;      /* print options; space separated list */
	short  priority;      /* priority level, 0-39, of the request */
	char   *pages;        /* list of pages to print (uniq. please!) */
	char   *charset;      /* character set to select or mount */
	char   *modes;        /* mode(s) of operation; space sep. list */
	char   *title;        /* optional title for banner page */
	char   *input_type;   /* type of content */
	char   *user;         /* user name of person submitting */
	ushort outcome;       /* success/fauilure */
}			REQUEST;

/*
 * Bit flags for the "actions" member:
 */
#define ACT_MAIL	0x0001	/* send mail when finished printing */
#define ACT_WRITE	0x0002	/* write to the terminal when finished */
#define ACT_IMMEDIATE	0x0010	/* print immediately */
#define ACT_HOLD	0x0020	/* don't print until resumed */
#define ACT_RESUME	0x0030	/* resume a held request */
#define ACT_SPECIAL	0x0030	/* bit mask of immediate/hold/resume */
#define ACT_RAW		0x0100	/* don't filter the input */

/*
 * Currently, the following is used only for alignment patterns:
 */
#define ACT_FAST	0x8000	/* force all filters to be fast */


/*
 * Bit flags for the "outcome" member:
 */
#define RS_HELD		0x0001	/* held pending resume */
#define RS_FILTERING	0x0002	/* slow filter is running */
#define RS_FILTERED	0x0004	/* slow filter has finished running */
#define RS_PRINTING	0x0008	/* on printer */
#define RS_PRINTED	0x0010	/* has successfully finished printing */
#define RS_CHANGING	0x0020	/* request is held pending user change */
#define RS_CANCELLED	0x0040	/* request was cancelled */
#define RS_IMMEDIATE	0x0080	/* should be next to print */
#define RS_FAILED	0x0100	/* slow filter or interface failed */
#define RS_RAW		0x0200	/* don't filter */
#define RS_NOTIFY	0x0400	/* user is to be notified (alert) */
#define RS_NOTIFYING	0x0800	/* notification (alert) is running */
#define RS_PWMAND	0x1000	/* matching pwheel must be mounted */
#define RS_ADMINHELD	0x2000	/* administrator placed RS_HELD */
#define RS_REFILTER	0x4000	/* had to change filters */
#define RS_STOPPED	0x8000	/* temporarily stopped the request */

#define RS_DONE		(RS_CANCELLED | RS_PRINTED | RS_FAILED)
				/* we're finished with request */
#define RS_ACTIVE	(RS_FILTERING | RS_PRINTING | RS_CHANGING | RS_NOTIFYING)
				/* request is busy, needs attention */

/**
 ** Various routines.
 **/

extern REQUEST		*getrequest();

extern int		putrequest();

extern void		freerequest();
