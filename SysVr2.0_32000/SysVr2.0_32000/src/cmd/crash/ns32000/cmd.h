/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/* @(#)cmd.h	1.4 */
struct	tsw	t[] = {
	"buffer",	BUFFER,	"buffer data",
	"b",		BUFFER,	NULL,
	"buf",		BUFHDR,	"buffer headers",
	"bufhdr",	BUFHDR,	NULL,
	"hdr",		BUFHDR,	NULL,
	"ds",		DS,	"data address namelist search",
	"file",		FILES,	"file table",
	"files",	FILES,	NULL,
	"f",		FILES,	NULL,
	"inode",	INODE,	"inode table",
	"inode -",	INODE,	"inode table with block addresses",
	"ino",		INODE,	NULL,
	"i",		INODE,	NULL,
	"kfp",		KFP,	"frame pointer for kernel stack trace start",
	"fp",		KFP,	NULL,
	"r5",		KFP,	NULL,
	"lck",		LCK,	"record lock tables",
	"l",		LCK,	NULL,
	"map",		MAP,	"map structures",
	"mount",	MOUNT,	"mount table",
	"mnt",		MOUNT,	NULL,
	"m",		MOUNT,	NULL,
	"nm",		NM,	"name search",
	"od",		OD,	"dump symbol values",
	"dump",		OD,	NULL,
	"rd",		OD,	NULL,
	"pcb",		PCBLK,	NULL,	/* was "process control block", */
	"proc",		PROC,	"process table",
	"proc -",	PROC,	"process table long listing",
	"proc -r",	PROC,	"runnable processes only",
	"ps",		PROC,	NULL,
	"p",		PROC,	NULL,
	"!",		Q,	"escape to shell",
	"?",		Q,	"print this list of available commands",
	"quit",		QUIT,	"exit",
	"q",		QUIT,	NULL,
	"pregion",	PREGION, "pregion dump",
	"preg",		PREGION, NULL,
	"region",	REGION, "region table",
	"r",		REGION, NULL,
	"kernel",	STACK,	NULL,
	"k",		STACK,	NULL,
	"stack",	STACK,	"kernel stack",
	"stk",		STACK,	NULL,
	"s",		STACK,	NULL,
	"stat",		STAT,	"dump statistics",
	"text",		TEXT,	"text table",
	"txt",		TEXT,	NULL,
	"x",		TEXT,	NULL,
	"callout",	TOUT,	"callout table",
	"calls",	TOUT,	NULL,
	"call",		TOUT,	NULL,
	"timeout",	TOUT,	NULL,
	"time",		TOUT,	NULL,
	"tout",		TOUT,	NULL,
	"c",		TOUT,	NULL,
	"trace",	TRACE,	"kernel stack trace",
	"trace -r",	TRACE,	"trace using saved kernel frame pointer",
	"t",		TRACE,	NULL,
	"ts",		TS,	"text address namelist search",
	"tty",		TTY,	"tty structures",
	"tty -",	TTY,	"tty structures with full stty parameters",
#ifdef	ns32000
	"tty type",	TTY,	"use type tty structure (i.e., sio or con)",
#else
	"tty type",	TTY,	"use type tty structure (i.e.,dz,dzb,dh,kl)",
#endif
	"term",		TTY,	NULL,
	"dz",		TTY,	NULL,
	"dh",		TTY,	NULL,
	"user",		UAREA,	"user area",
	"u_area",	UAREA,	NULL,
	"uarea",	UAREA,	NULL,
	"u",		UAREA,	NULL,
	"var",		VAR,	"system variables",
	"tunables",	VAR,	NULL,
	"tunable",	VAR,	NULL,
	"tune",		VAR,	NULL,
	"v",		VAR,	NULL,
	NULL,		NULL,	NULL
};

struct	prmode	prm[] = {
	"decimal",	DECIMAL,
	"dec",		DECIMAL,
	"e",		DECIMAL,
	"octal",	OCTAL,
	"oct",		OCTAL,
	"o",		OCTAL,
	"directory",	DIRECT,
	"direct",	DIRECT,
	"dir",		DIRECT,
	"d",		DIRECT,
	"write",	WRITE,
	"w",		WRITE,
	"character",	CHAR,
	"char",		CHAR,
	"c",		CHAR,
	"inode",	INODE,
	"ino",		INODE,
	"i",		INODE,
	"byte",		BYTE,
	"b",		BYTE,
	"longdec",	LDEC,
	"ld",		LDEC,
	"D",		LDEC,
	"longoct",	LOCT,
	"lo",		LOCT,
	"O",		LOCT,
	"hex",		HEX,
	"h",		HEX,
	"x",		HEX,
	"hexadec",	HEX,
	"hexadecimal",	HEX,
	NULL,		NULL
} ;
