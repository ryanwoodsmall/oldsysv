/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:sizenet.c	1.4"
/*
 * This file contains code for the crash function size.  The RFS and
 * Streams tables and sizes are listed here to allow growth and not
 * overrun the compiler symbol table.
 */

#include "sys/param.h"
#include "a.out.h"
#include "stdio.h"
#include "sys/types.h"
#include "sys/fs/s5dir.h"
#include "sys/mount.h"
#include "sys/adv.h"
#include "sys/sema.h"
#include "sys/comm.h"
#include "sys/gdpstr.h"
#include "sys/nserve.h"
#include "sys/cirmgr.h" 
#include "sys/fs/s5inode.h"
#include "sys/inode.h"
#include "sys/stream.h"
#include "crash.h"


struct sizenetable {
	char *name;
	char *symbol;
	unsigned size;
};
struct sizenetable sizntab[] = {
	"adv","advertise",sizeof (struct advertise),
	"advertise","advertise",sizeof (struct advertise),
	"datab","dblock",sizeof (struct datab),
	"dblk","dblock",sizeof (struct datab),
	"dblock","dblock",sizeof (struct datab),
	"dbalcst","dballoc",sizeof (struct dbalcst),
	"dballoc","dballoc",sizeof (struct dbalcst),
	"gdp","gdp",sizeof (struct gdp), 
	"linkblk","linkblk",sizeof (struct linkblk),
	"mblk","mblock",sizeof (struct msgb),
	"mblock","mblock",sizeof (struct msgb),
	"msgb","mblock",sizeof (struct msgb),
	"queue","queue",sizeof (struct queue),
	"rcvd","rcvd",sizeof (struct rcvd),
	"sndd","sndd",sizeof (struct sndd),
	"srmnt","srmount",sizeof (struct srmnt),
	"srmount","srmount",sizeof (struct srmnt),
	"stdata","streams",sizeof (struct stdata),
	"streams","streams",sizeof (struct stdata),
	NULL,NULL,NULL
};	


/* get size from size table */
unsigned
getsizenetab(name)
char *name;
{
	unsigned size = 0;
	struct sizenetable *st;

	for(st = sizntab; st->name; st++) {
		if(!(strcmp(st->name,name))) {
			size = st->size;
			break;
		}
	}
	return(size);
}

/* print size */
int
prsizenet(name)
char *name;
{
	struct sizenetable *st;
	int i;

	if(!strcmp("",name)) {
		for(st = sizntab,i = 0; st->name; st++,i++) {
			if(!(i & 3))
				fprintf(fp,"\n");
			fprintf(fp,"%-15s",st->name);
		}
		fprintf(fp,"\n");
	}
}

/* get symbol name and size */
int
getnetsym(name,symbol,size)
char *name;
char *symbol;
unsigned *size;
{
	struct sizenetable *st;

	for(st = sizntab; st->name; st++) 
		if(!(strcmp(st->name,name))) {
			strcpy(symbol,st->symbol);
			*size = st->size;
			break;
		}
}
