/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:des.c	1.2"
/* crash/des.c */
/*
 *		Copyright 1984 AT&T
 */
#include "sys/param.h"
#include "a.out.h"
#include "sys/signal.h"
#include "stdio.h"
#include "sys/sysmacros.h"
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/var.h"
#include "sys/region.h"

#include	<crash.h>
#include	<sys/proc.h>


extern	struct syment *Proc;
extern long lseek();
char * sramapt[4];			/* srama: initialized in init() */
SRAMB srambpt[4];			/* sramb: initialized in init() */


sde_t sdtmp;
struct proc prbuf; /* storage for proctable entry */

paddr_t
prdes(addr,slot)
long addr;
int slot;
{

/*
 * prdes(addr,slot) - given a virtual address will return a file offset that
 * is used to index into the crash dump. If the virtual address can not
 * be mapped to a file offset, a value of -1 will be returned to the 
 * calling function.
 */

	union {
		long intvar;
		struct _VAR vaddr;
	} virtaddr;
	sde_t *sdte;			/* segment descriptor table entry */
	long  sde_paddr;		/* physical addres of sde in crash file */
	pde_t pdte;			/* page descriptor table entry */
        long pde_paddr;                 /* physical address of pde in crash file */
	long cntgseg;			/* phys address of contiguous segment */
	long Padr;			/* physical address to return */
	int len;			/* length of segment table */


	virtaddr.intvar = addr;
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

	
	/*
	 * Check the flag bits for errors and get the address
	 * of the segment number specified in addr.
	 */
	if(virtaddr.vaddr.v_ssl > len)  {
		printf("error:segment number=%d; out of range of table length=%d\n",virtaddr.vaddr.v_ssl,len);
		return(-1);
	}

	/* read in the segment descriptor from the crash file */
	if (readsde( sde_paddr, &sdtmp ) ==-1 ) return(-1);  /* error on read */
	sdte = &sdtmp; /* need a pointer to the seg desc */

	if(!(isvalid(sdte))) {
		printf("segment descriptor table entry is invalid\n");
		return(-1);
	}

	/* indirect if shared memory */
	if(indirect(sdte))
	{
		sde_paddr = sdte -> wd2.address & ~7 ;
		if(readsde(sde_paddr, &sdtmp) == -1)
			return (-1) ;
			
		sdte = &sdtmp ;
		
		if(!(isvalid(sdte)))
		{
			printf("indirect segment descriptor table entry is invalid\n") ;
			return(-1) ;
		}
	}
	
	/*
	 * Check the "c" bit in the sdte to tell whether this
	 * is a "contiguous" or "paged" segment.
	 */
	if(iscontig(sdte)) {
		cntgseg=(long) sdte->wd2.address & 0xfffffff8;
		/* add in segment offset */
		Padr = virtaddr.intvar & 0x1ffffL;
		Padr += cntgseg;
		/* turn the address into an offset and return */
		return(Padr & ~MAINSTORE);
	}
	else {	/* get page descriptor table entry */
                pde_paddr = (sdte->wd2.address & ~0x1f) 
				+ virtaddr.vaddr.v_psl * sizeof (pde_t);
                if (readpde(pde_paddr,&pdte) == -1)
                     return (-1);

                if (!pdte.pgm.pg_v) {
                    printf("page not valid in memory\n");
                    return (-1);
                }
                /* calculate physical address and return */
                Padr = (pdte.pgi.pg_pde & PG_ADDR) +
                       (virtaddr.vaddr.v_byte & POFFMASK);
                return(Padr & ~MAINSTORE);

	}
}

procntry(slot,prbuf)
int slot;
struct  proc *prbuf;
{
	/* procntry() fills prbuf with the process table entry indicated
	 * by slot. If  slot == -1, the proc table entry for the current
	 * process is returned.
	 * (code stolen from getuarea() ) 
	 */

	int curproc;
	long	pslot_va;
	long	curpr_va;	/* address of Curproc */

	/*
	 * Determine if the slot requested is a default to the current process. If so,
	 * seek on the file offset of the current process and if found read it.
	 * Otherwise, print an error message and discontinue processing. 
	 */

	if(slot == -1) {
		curpr_va = (long)(Curproc->n_value);
		if(lseek(mem, prdes(curpr_va, -1), 0) == -1) {
			error("seek error on current process");
			return(-1);
		}
		if(read(mem, (char *)&curproc, sizeof curproc) == -1) {
			error("read error on current process");
			return(-1);
		}
		slot = (curproc - Proc->n_value) / sizeof(struct proc);
	}
	if(slot > v.v_proc)
	{
		printf("%d out of range, use Process Table slot number\n",slot);
		return(-1);
	}
	/*
	 * Now locate the process table entry of the process specified from the file
	 * offset, which was calculated from the virtual address by calling prdes().
	 */

	pslot_va = (long)(Proc->n_value + slot * sizeof(struct proc));
	if(lseek(mem, prdes(pslot_va, slot), 0) == -1) {
		error(" error on seek to process slot");
		return(-1);
	}
	if(read(mem, (char *) prbuf, sizeof(struct proc)) != sizeof(struct proc)) {
		error("read error on process table");
		return(-1);
	}
	return(0);
}



readsde( sde_paddr, sdte )
long sde_paddr;		/* physical address of the segment descriptor */
sde_t	*sdte;		/* pointer to the temp sde */
{
	/* readsde() takes as input a physical address. It assumes
	   that the address is that of a segment descriptor. It reads the
	   information at that address in the crash file into temporary
	   storage in the form of a segment descriptor. 
	 
	   readsde() returns the value 0 if it was able to read the 
	   segment descriptor, -1 otherwise.

	   On exit, if readsde() was successful, sdte points to the 
	   segment descriptor that was read from the crash file
	*/



	long sde_off;	/* offset into crash file of the seg. des. */

	/* form the offset and read the descriptor */
	

	sde_off = sde_paddr & ~MAINSTORE;
	if ( lseek(mem, sde_off, 0) == -1)
	{	/* can't get to that offset */
		printf( "error on seek to segment descriptor\n");
		return (-1);
	}
	
	if ( read(mem, sdte, sizeof( sde_t )) != sizeof( sde_t ))
	{	/* error on read */
		printf( "error while reading segment descriptor\n" );
		return (-1);
	}

	return(0);
}


readpde( pde_paddr, pdte )
long pde_paddr;		/* physical address of the page descriptor */
pde_t	*pdte;		/* pointer to the pde buffer */
{
	/* readpde() takes as input a physical address. It assumes
	   that the address is that of a page descriptor. It reads the
	   information at that address in the crash file into 
           the buffer referred to by pdte.
	 
	   readpde() returns the value 0 if it was able to read the 
	   page descriptor, -1 otherwise.

	   On exit, if readpde() was successful, pdte points to the
	   page descriptor that was read from the crash file
	*/


	if ( lseek(mem, pde_paddr & ~MAINSTORE, 0) == -1)
	{	/* can't get to that offset */
		printf( "error on seek to page descriptor\n");
		return (-1);
	}
	
	if ( read(mem, pdte, sizeof( pde_t )) != sizeof( pde_t ))
	{	/* error on read */
		printf( "error while reading page descriptor\n" );
		return (-1);
	}

	return(0);
}


prvtop()
{
	/*
	 *  Converts virtual address to physical address by
	 *  calling prdes.
	 */
	
	register char *arg;
	long vaddr;		/* virtual address */
	paddr_t paddr;		/* physical address */
	char *token();

	if((arg = token()) == NULL) {
		printf("address expected\n");
		return;
	}
	printf("VIRTUAL    PHYSICAL\n");
	do {
		vaddr = hextoi(arg);
		if ((paddr = prdes(vaddr,-1)) == -1) 
			continue;
		printf("%8.8x   %8.8x\n", vaddr, paddr+MAINSTORE);
	} while (arg = token());
}

