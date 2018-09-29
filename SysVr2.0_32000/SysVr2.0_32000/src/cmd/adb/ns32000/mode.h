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
/*	@(#)mode.h	1.2	*/
#include "machine.h"

typedef	char	 	    BOOL;
typedef	char		   *STRING;
typedef	char		   *MSG;
typedef	struct map	    MAP;
typedef	MAP		   *MAPPTR;
typedef	struct syment	    SYMTAB;
typedef	SYMTAB		   *SYMPTR;
typedef	struct symslave     SYMSLAVE;
typedef	struct bkpt	    BKPT;
typedef	BKPT		   *BKPTR;

/* file address maps */
struct map {
	unsigned b1;
	unsigned e1;
	unsigned f1;
	unsigned b2;
	unsigned e2;
	unsigned f2;
	int      ufd;
};

/* slave table for symbols */
struct symslave {
	unsigned valslave;
	int  	 typslave;
};

#define MAXCOM 64
struct bkpt {
	int  loc;
	int  ins;
	int  count;
	int  initcnt;
	int  flag;
	char comm[MAXCOM];
	BKPT *nxtbkpt;
};

typedef	struct reglist REGLIST;
typedef	REGLIST       *REGPTR;
struct reglist {
	STRING	rname;
	int  	roffs;
};

typedef union {
    int  siADDR;
    int (* siFUN)();
} SIGunion;

struct grres {				/* Returned by getreg */
	int sf;				/* Success/fail indicator */
	int roff;			/* Register offset on success */
} ;
