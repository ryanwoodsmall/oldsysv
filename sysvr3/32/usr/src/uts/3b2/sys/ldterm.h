/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:sys/ldterm.h	1.1"

/* tty line discipline header file for STREAMS TTY Subsystem */

struct ldterm
{
	short ld_iflags;	/* input mode: ioctl message can set */
	short ld_oflags;	/* output mode: ioctl can set */
	short ld_lflags;	/* line discipline mode: ioctl can set */
	short ld_state;		/* internal state of tty ld */
	char ld_col;		/* output column of tty */
	char ld_erase;		/* erase char */
	char ld_kill;		/* line kill char */
	char ld_intrc;		/* interrupt char */
	char ld_quitc;		/* quit char */
	char ld_startc;		/* start char */
	char ld_stopc;		/* stop char */
	char ld_eofc;		/* end-of-file char */
	char ld_eol;		/* input delim char (like NL) */
	char ld_eol2;		/* 2nd input delim char */
	char ld_swtchc;		/* switch char for shl */
	char canonb[CANBSIZ];   /* input buffer for line formation */
	char *op;		/* pointer for input buffer */
	mblk_t *echobuf;	/* pointer to temporary storage for echo */
	short lmode_flg;        /* save raw/cannon mode request  */
	int  rawlength;		/* Number of characters in the raw buffer */
	mblk_t *raw_message;	/* pointer to raw message being built */
};

#define CANBSIZ	256
#define RAWBSIZ 16
#define	CTRL(c)	('c'&037)
#define	EOT	CTRL(d)


/* define partab character types */
#define	ORDINARY	0
#define	CONTROL		1
#define	BACKSPACE	2
#define	NEWLINE		3
#define	TAB		4
#define	VTAB		5
#define	RETURN		6

/* internal states for ld module */
#define	TTSTOP		01		/* stopped by ^S */
#define	TTESC		02		/* have just seen '\' (ldinsrv)  */
#define TTCR		04		/* mapping NL to CR-NL */
#define	TTUSE		010		/* this structure in use */
#define	TTBLOCK		020		/* have sent STOP message */
#define TTIOC		040		/* TCGETA just went down */
#define TTESC2  	0100		/* 2nd '\' in a row */
#define TTESC1		0200		/* have just seen `\` (ldin) */
#define	TACT		0400		/* timer active for raw read */
#define	RTO		01000		/* Raw Timeout */

/* Type ahead bug defines */
#define B_CANON 	01
#define B_RAW  		02 

/* delay values	*/
#define TICK1  1
#define TICK2  2
#define TICK3  3
#define TICK4  4
#define TICK6  6
#define TICK9  9
#define MANYTICK  127

/* Debugging information */
int	lddebug;

#ifdef DBUG
#define DEBUG1(a)  if (lddebug >= 1) printf a
#define DEBUG2(a)  if (lddebug >= 2) printf a
#define DEBUG3(a)  if (lddebug >= 3) printf a 
#define DEBUG4(a)  if (lddebug >= 4) printf a 
#else
#define DEBUG1(a)
#define DEBUG2(a)
#define DEBUG3(a)
#define DEBUG4(a)
#endif 
/* some #defines used for clarity */

#define CHG_TO_RAW	(tp->lmode_flg & B_RAW)
#define CHG_TO_CANON	(tp->lmode_flg & B_CANON)
#define CANON_MODE 	(tp->ld_lflags & ICANON)
#define RAW_MODE	!(tp->ld_lflags & ICANON)
#define V_MIN		tp->ld_eofc
#define V_TIME		tp->ld_eol
#define ECHO_ON		(tp->ld_lflags & ECHO)
#define ECHO_OFF	!(tp->ld_lflags & ECHO)

