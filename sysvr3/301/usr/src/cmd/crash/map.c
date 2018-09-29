/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:map.c	1.4"
/*
 * This file contains code for the crash function map.
 */

#include "sys/param.h"
#include "a.out.h"
#include "stdio.h"
#include "sys/types.h"
#include "sys/map.h"
#include "crash.h"

/* get arguments for map function */
int
getmap()
{
	struct syment *sp;
	int c;

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind])
		do {
			if(sp = symsrch(args[optind])) {
				fprintf(fp,"\n%s:\n",args[optind]);
				prmap(sp);
			}
			else fprintf(fp,"%s not found in symbol table\n",
				args[optind]);
		}while(args[++optind]);
	else longjmp(syn,0);
}

/* print map */
int
prmap(name)
struct syment *name;
{
	struct map mbuf;
	unsigned units = 0, seg = 0;

	readmem((long)name->n_value,1,-1,(char *)&mbuf,
		sizeof mbuf,"map table");

	fprintf(fp,"MAPSIZE: %u\tSLEEP VALUE: %u\n",
		mbuf.m_size,
		mbuf.m_addr);
	fprintf(fp,"\nSIZE    ADDRESS\n");
	for(;;) {
	        if (read(mem,(char *)&mbuf,sizeof(mbuf)) != sizeof(mbuf)) 
			error("read error on %s map table\n",name);
		if (!mbuf.m_size) {
			fprintf(fp,"%u SEGMENTS, %u UNITS\n",
				seg,
				units);
			return;
		}
		fprintf(fp,"%4u   %8x\n",
			mbuf.m_size,
			mbuf.m_addr);
		units += mbuf.m_size;
		seg++;
	}
}
