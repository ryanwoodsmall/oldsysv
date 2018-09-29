/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:major.c	1.3"
/*
 * This file contains code for the crash function major.
 */

#include "sys/param.h"
#include "a.out.h"
#include "stdio.h"
#include "crash.h"

#define MAJSIZE 128		/* size of MAJOR table */

char majbuf[MAJSIZE];		/* buffer for MAJOR table */
static struct syment *Major;	/* namelist symbol pointer */

/* get arguments for major function */
int
getmajor()
{
	int slot = -1;
	int c;

	if(!Major)
		if((Major = symsrch("MAJOR")) == NULL)
			error("MAJOR not found in symbol table\n");
	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	fprintf(fp,"MAJOR TABLE SIZE = %d\n",MAJSIZE);
	readmem((long)Major->n_value,1,-1,(char *)majbuf,
		MAJSIZE,"MAJOR Table");

	if(args[optind]) {
		do {
			if((slot = strcon(args[optind++],'d')) == -1)
				continue;
			if((slot < 0) || (slot >= MAJSIZE))
				error("%d is out of range\n",slot);
			prmajor(slot);
		}while(args[optind]);
	}
	else prmajor(slot);
}

/* print MAJOR table */
int
prmajor(slot)
int slot;
{
	int i;

	if(slot == -1) {
		for(i = 0; i < MAJSIZE; i++) {
			if(!(i & 3))
				fprintf(fp,"\n");
			fprintf(fp,"[%3d]: %3d\t",i,majbuf[i]);
		}
		fprintf(fp,"\n");
	}
	else fprintf(fp,"[%3d]: %3d\n",slot,majbuf[slot]);
}
