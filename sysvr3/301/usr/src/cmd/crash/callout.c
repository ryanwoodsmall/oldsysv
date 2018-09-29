/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:callout.c	1.8"
/*
 * This file contains code for the crash function callout.
 */

#include "stdio.h"
#include "a.out.h"
#include "sys/types.h"
#include "sys/callo.h"
#include "crash.h"


struct syment *Callout;			/* namelist symbol pointer */
extern char *strtbl;			/* pointer to string table */
extern short N_TEXT;			/* used in symbol table search */

/* get arguments for callout function */
int
getcallout()
{
	int c;

	if(!Callout)
		if(!(Callout = symsrch("callout")))
			error("callout not found in symbol table\n");

	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	fprintf(fp,"FUNCTION        ARGUMENT   TIME  ID\n");
	if(args[optind]) 
		longjmp(syn,0);
	else prcallout();
}

/* print callout table */
int
prcallout()
{
	struct syment *sp;
	extern struct syment *findsym();
	struct callo callbuf;
	static char tname[SYMNMLEN+1];
	char *name;

	seekmem((long)Callout->n_value,1,-1);
	for(;;) {
		if(read(mem,(char *)&callbuf,sizeof callbuf)!=sizeof callbuf) 
			error("read error on callout table\n");
		if(!callbuf.c_func)	
			return;
		if(!(sp = findsym((unsigned long)callbuf.c_func)))
			error("%8x does not match in symbol table\n",
				callbuf.c_func);
		if(sp->n_zeroes){
			strncpy(tname,sp->n_name,SYMNMLEN);
			name = tname;
		}
		else name = strtbl + sp->n_offset;
		fprintf(fp,"%-15s",name);
		fprintf(fp," %08lx  %5u  %5u\n", 
			callbuf.c_arg,
			callbuf.c_time,
			callbuf.c_id);
	}
}

