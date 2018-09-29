/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:vtop.c	1.5"
/*
 * This file contains code for the crash functions:  vtop and mode, as well as
 * the virtual to physical offset conversion routine vtop.
 */

#include "sys/param.h"
#include "a.out.h"
#include "stdio.h"
#include "sys/types.h"
#include "sys/sbd.h"
#include "sys/fs/s5dir.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "crash.h"

extern	struct syment *Proc;		/* namelist symbol pointers */
extern struct syment *Curproc;
char * sramapt[4];			/* srama: initialized in init() */
SRAMB srambpt[4];			/* sramb: initialized in init() */

int abortflag = 0;			/* flag to abort or continue */
					/* used in vtop function */
struct proc prbuf;			/* process entry buffer */
int prsid;				/* temporary variables to hold */
long prssl,prsram,prpsl;		/* values to be printed by vtop */
					/* function */

/* virtual to physical office address translation */
paddr_t
vtop(vaddr,slot)
long vaddr;
int slot;
{

	union {
		long intvar;
		struct _VAR vaddr;
	} virtaddr;
	sde_t *sdte;			/* segment descriptor table entry */
	sde_t sdtbuf;
	long  sde_paddr;		/* physical address of sde in crash file */
	pde_t pdte;			/* page descriptor table entry */
        long pde_paddr;                 /* physical address of pde in crash file */
	long cntgseg;			/* phys address of contiguous segment */
	long paddr;			/* physical address to return */
	int len;			/* length of segment table */

	virtaddr.intvar = vaddr;
	/* 
	 * Use the section number in the addr to get the 
	 * address and the length of the segment table.
	 */
	switch (virtaddr.vaddr.v_sid) {
		case 0:			/* kernel section 0 */
			sde_paddr = (long) (sramapt[0] 
				+ (virtaddr.vaddr.v_ssl * sizeof(sde_t)));
			len = srambpt[0].SDTlen;
			break;
		case 1:			/* kernel section 1 */
			sde_paddr = (long) (sramapt[1] 
				+ (virtaddr.vaddr.v_ssl * sizeof(sde_t)));
			len = srambpt[1].SDTlen;
			break;
		case 2:			/* text, data and bss section */
			/* get a proc entry for given process */
			procntry(slot, &prbuf);
			sde_paddr = (long) (prbuf.p_srama[0]
				+ (virtaddr.vaddr.v_ssl * sizeof(sde_t)));
			/* maximum offset in SDT */
			len = prbuf.p_sramb[0].SDTlen;            
			break;
		case 3:			/* shared memory, stack and ublock */
			/* get a proc entry for given process */
			procntry(slot, &prbuf);
			sde_paddr = (long) (prbuf.p_srama[1]
				+ (virtaddr.vaddr.v_ssl * sizeof(sde_t)));
			/* maximum offset in SDT */
			len = prbuf.p_sramb[1].SDTlen;            
			break;
		}
	/* sde_paddr now contains the physical address of 
	   the proper segment descriptor */
		prsid = virtaddr.vaddr.v_sid;
		prssl = virtaddr.vaddr.v_ssl;
		if(virtaddr.vaddr.v_sid > 1) 
			prsram = prbuf.p_srama[virtaddr.vaddr.v_sid-2];
		else prsram = -1;
	
	/*
	 * Check the flag bits for errors and get the address
	 * of the segment number specified in addr.
	 */
	if(virtaddr.vaddr.v_ssl > len)  {
		if(abortflag) {
			abortflag = 0;
			error("%d out of range for segment table\n",len);
		}
		return(-1);
	}

	/* read in the segment descriptor from the crash file */
	readmem((sde_paddr&~MAINSTORE),0,slot,(char *)&sdtbuf,
		sizeof (sde_t),"segment descriptor table");
	sdte = &sdtbuf;

	if(!(isvalid(sdte))) {
		if(abortflag) {
			abortflag = 0;
			error("segment descriptor table entry is invalid\n");
		}
		return(-1);
	}

	/* indirect if shared memory */
	if(indirect(sdte))
	{
		sde_paddr = sdte->wd2.address & ~7;
		
		readmem((sde_paddr&~MAINSTORE),0,slot,(char *)&sdtbuf,
			sizeof (sde_t),"segment descriptor table");

		if(!(isvalid(sdte))) {
			if(abortflag) {
				abortflag = 0;
				error("indirect segment descriptor table entry is invalid\n");
			}
			return(-1);
		}
	}
	
	/*
	 * Check the "c" bit in the sdte to tell whether this
	 * is a "contiguous" or "paged" segment.
	 */
	if(iscontig(sdte)) {
		cntgseg=(long) sdte->wd2.address & 0xfffffff8;
		/* add in segment offset */
		paddr = virtaddr.intvar & 0x1ffffL;
		paddr += cntgseg;
		/* turn the address into an offset and return */
		return(paddr & ~MAINSTORE);
	}
	else {	/* get page descriptor table entry */
                pde_paddr = (sdte->wd2.address & ~0x1f) 
				+ virtaddr.vaddr.v_psl * sizeof (pde_t);

		readmem((pde_paddr&~MAINSTORE),0,slot,(char *)&pdte,
			sizeof (pde_t),"page descriptor table");

                if (!pdte.pgm.pg_v) {
			if(abortflag) {
				abortflag = 0;
                    		error("page not valid in memory\n");
			}
			return(-1);
		}
		prpsl = virtaddr.vaddr.v_psl;
                /* calculate physical address and return */
                paddr = (pdte.pgi.pg_pde & PG_ADDR) +
                       (virtaddr.vaddr.v_byte & POFFMASK);
                return(paddr & ~MAINSTORE);

	}
}

/* get arguments for vtop function */
int
getvtop()
{
	int proc = Procslot;
	struct syment *sp;
	long addr;
	int c;


	optind = 1;
	while((c = getopt(argcnt,args,"w:s:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 's' :	proc = setproc();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		fprintf(fp,"VIRTUAL  PHYSICAL SECT SDT   SRAM   PDT\n");
		do {
			if(*args[optind] == '(') {
				if((addr = eval(++args[optind])) == -1)
					continue;
				prvtop(addr,proc);
			}
			else if(sp = symsrch(args[optind])) 
				prvtop((long)sp->n_value,proc);
			else if(isasymbol(args[optind]))
				error("%s not found in symbol table\n",
					args[optind]);
			else {
				if((addr = strcon(args[optind],'h')) == -1)
					continue;
				prvtop(addr,proc);
			}
		}while(args[++optind]);
	}
	else longjmp(syn,0);
}

/* print vtop information */
int
prvtop(addr,proc)
long addr;
int proc;
{
	int paddr;

	abortflag = 1;
	paddr = vtop(addr,proc) + MAINSTORE;
	fprintf(fp,"%8x %8x %4d %3d",
		addr,
		paddr,
		prsid,
		prssl);
	if(prsram == -1)
		fprintf(fp,"         ");
	else fprintf(fp," %8x",prsram);
	fprintf(fp," %3d\n",
		prpsl);
	abortflag = 0;
}


/* get arguments for mode function */
int
getmode()
{
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
		prmode(args[optind]);
	else prmode("s");
}

/* print mode information */
int
prmode(mode)
char *mode;
{

	switch(*mode) {
		case 'p' :  Virtmode = 0;
			    break;
		case 'v' :  Virtmode = 1;
			    break;
		case 's' :  break;
		default  :  longjmp(syn,0);
	}
	if(Virtmode)
		fprintf(fp,"Mode = virtual\n");
	else fprintf(fp,"Mode = physical\n");
}
	
