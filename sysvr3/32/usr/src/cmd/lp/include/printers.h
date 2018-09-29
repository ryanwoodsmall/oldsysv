/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:include/printers.h	1.7"
/* EMACS_MODES: !fill, lnumb, !overwrite, !nodelete, !picture */

/**
 ** The disk copy of the printer files:
 **/

/*
 * There are 16 fields in the printer configuration file.
 */
#define PR_MAX	16
# define PR_BAN		0
# define PR_CPI		1
# define PR_CS		2
# define PR_ITYPES	3
# define PR_DEV		4
# define PR_DIAL	5
# define PR_RECOV	6
# define PR_INTFC	7
# define PR_LPI		8
# define PR_LEN		9
# define PR_LOGIN	10
# define PR_PTYPE	11
# define PR_REMOTE	12
# define PR_SPEED	13
# define PR_STTY	14
# define PR_WIDTH	15

/**
 ** The internal flags seen by the Spooler/Scheduler and anyone who asks.
 **/

#define PS_REJECTED	0x001
#define	PS_DISABLED	0x002
#define PS_FAULTED	0x004
#define PS_BUSY		0x008
#define PS_LATER	0x010	/* Printer is scheduled for attention */

/**
 ** The internal copy of a printer as seen by the rest of the world:
 **/

/*
 * A (char **) list is an array of string pointers (char *) with
 * a null pointer after the last item.
 */
typedef struct PRINTER {
	char   *name;		/* name of printer (redundant) */
	ushort banner;          /* banner page conditions */
	SCALED cpi;             /* default character pitch */
	char   **char_sets;     /* list of okay char-sets/print-wheels */
	char   **input_types;   /* list of types acceptable to printer */
	char   *device;         /* printer port full path name */
	char   *dial_info;      /* system name or phone # for dial-up */
	char   *fault_rec;      /* printer fault recovery procedure */
	char   *interface;      /* interface program full path name */
	SCALED lpi;             /* default line pitch */
	SCALED plen;            /* default page length */
	ushort login;		/* is/isn't a login terminal */
	char   *printer_type;   /* Terminfo look-up (TERM) value */
	char   *remote;         /* remote machine!printer-name */
	char   *speed;          /* baud rate for connection */
	char   *stty;           /* space separated list of stty options */
	SCALED pwid;            /* default page width */
	char   *description;	/* comment about printer */
	FALERT fault_alert;	/* how to alert on printer fault */
	short  daisy;           /* 1/0 - printwheels/character-sets */
}			PRINTER;

#define BAN_ALWAYS	0x01	/* user can't override banner */
#define BAN_OFF		0x02	/* don't print banner page */

#define LOG_IN		0x01	/* printer is login terminal */

#define PCK_TYPE	0x0001	/* printer type isn't in Terminfo */
#define PCK_CHARSET	0x0002	/* printer type can't handle ".char_sets" */
#define PCK_CPI		0x0004	/* printer type can't handle ".cpi" */
#define PCK_LPI		0x0008	/* printer type can't handle ".lpi" */
#define PCK_WIDTH	0x0010	/* printer type can't handle ".pwid" */
#define PCK_LENGTH	0x0020	/* printer type can't handle ".plen" */

/*
 * The following PCK_... bits are only set by the Spooler,
 * when refusing a request.
 */
#define PCK_BANNER	0x1000	/* printer needs banner */

/*
 * Flags set by "putprinter()" for things that go wrong.
 */
#define BAD_REMOTE	0x0001	/* has attributes of remote and local */
#define BAD_INTERFACE	0x0002	/* no interface or can't read it */
#define BAD_DEVDIAL	0x0004	/* no device or dial information */
#define BAD_FAULT	0x0008	/* not recognized fault recovery */
#define BAD_ALERT	0x0010	/* has reserved word for alert command */

/*
 * For print wheels:
 */

typedef struct PWHEEL {
	char   *name;		/* name of print wheel */
	FALERT alert;		/* how to alert when mount needed */
}			PWHEEL;

/**
 ** Various routines.
 **/

extern PRINTER		*getprinter();

extern PWHEEL		*getpwheel();

extern char		*getdefault();

extern int		putprinter(),
			delprinter(),
			putdefault(),
			deldefault(),
			putpwheel(),
			delpwheel(),
			okprinter();

extern unsigned long	chkprinter(),
			badprinter,
			ignprinter;

extern void		freeprinter(),
			freepwheel();

/**
 ** Aliases (copies) of some important Terminfo caps.
 **/

extern int		ti_daisy;
