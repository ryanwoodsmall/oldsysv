/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm:inc/smdef.h	1.1"

struct atom {
	struct atom		*next;
	char			*address;
	char			*info;
};

struct supalias {
	int spec;
	int where;
	char *lname;
	char *fname;
	struct atom *atom_list;
	struct supalias *next;
	struct supalias *pre;
};

#define ATOM		struct atom
#define LENLEN 15
#define SEPARATOR	':'
#define SEMI		';'
#define ALIASSIZ	32
#ifndef PATHSIZ
#define PATHSIZ		128
#endif
#define MAXADDRS	1024
#define ENTRYSIZ	512

#define bool		int
#ifndef TRUE
#define TRUE		1
#define FALSE		0
#endif
#define SUBJECT 1
#define ADDRS 2
#define NOSTORE 4
#define EDITED 8
#define CALL 16
#define READIN 32
#define AUTOSEND 64

#define alloc(Q)	(Q *) calloc(1, sizeof(Q))

#define MAXLINE  80
#define MAXALIAS 500
#define SEPLINE ":::::::::::::::::::::::::::::::::::::::::::::::"
/*#define EPICSEND 1*/
/*#define POSTSEND 0*/
#define TO 1				/* TO field */
#define CC 2				/* CC field */
#define BEGIN	1
#define NEXT	2
#define PRV	3
#define	PNUM	4
/*#define EMPFAIL -1*/
#define CALLMEMO 0
#define SENDMAIL 1
#define CALENDAR 2
#define FIND 3
#define REPLY 1
#define RET_RECEIPT 2
#define	MAXSUB		300
#define MAXATTS	10
struct msg_head {	/* message header structure */
	char *filename;
	char *linkname;
	FILE *fp;
	struct supalias to[1];
	struct supalias cc[1];
	struct supalias bc[1];
	char subj[MAXSUB];
	char *msg_type;
	struct oeh atts[MAXATTS];
	int noatts;
	char *phone;
	char *mark;
	char *caller;
	int rec;
	char *mailto;
	char *paperto;
	char *replyid;
	long send_time;
	int flag;
	int attlen;
	int annot;
};
/* "No send" codes */
#define NS_ADDR		0
#define NS_ATTACH	1
#define NS_MSG		2
#define NS_GEN		3
struct addrlist {
	char *name;
	char *line2;
	char *address;
	bool pick_flg;
};
