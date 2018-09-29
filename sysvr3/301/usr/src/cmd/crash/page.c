/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:page.c	1.10"
/*
 * This file contains code for the crash functions: pfdat, region, sdt, and pdt.
 */

#include "sys/param.h"
#include "a.out.h"
#include "stdio.h"
#include "signal.h"
#include "sys/sysmacros.h"
#include "sys/types.h"
#include "sys/fs/s5dir.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/var.h"
#include "sys/pfdat.h"
#include "sys/region.h"
#include "sys/inode.h"
#include "sys/proc.h"
#include "crash.h"

extern struct syment *Inode;		/* namelist symbol pointers */
static struct syment *Pfdat;	/* namelist symbol pointers */
struct syment *Region;
extern char *sramapt[4];
extern SRAMB srambpt[4];
extern long vtop();
extern long lseek();

/* get arguments for pfdat function */
int
getpfdat()
{
	int slot = -1;
	int all = 0;
	int phys = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int c;
	struct syment *maxc, *kpb;
	unsigned  size, maxclick, kpbase;

	if(!Pfdat)
		if(!(Pfdat = symsrch("pfdat")))
			error("pfdat not found in symbol table\n");


	optind = 1;
	while((c = getopt(argcnt,args,"epw:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}
	/* collect system variables to calculate size */
	if(!(maxc = symsrch("maxclick"))) 
		error("maxclick not found in symbol table\n");
	readmem((long)maxc->n_value,1,-1,(char *)&maxclick,
		sizeof maxclick,"maxclick");
	
	if(!(kpb = symsrch("kpbase"))) 
		error("kpbase not found in symbol table\n");
	readmem((long)kpb->n_value,1,-1,(char *)&kpbase,
		sizeof kpbase,"kpbase");

	size = (maxclick*2048 - kpbase)/2048;

	fprintf(fp,"PFDATA TABLE SIZE: %d\n", size);
	fprintf(fp,"SLOT   BLKNO  USE MAJ/MNR  INUM SWPI RCNT WCNT NEXT PREV HASH FLAGS\n");
	if(args[optind]) {
		all = 1;
		do {
			getargs((int)size,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prpfdat(all,slot,phys,addr,size);
			else {
				if(arg1 < size)
					slot = arg1;
				else addr = arg1;
				prpfdat(all,slot,phys,addr,size);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < size; slot++)
		prpfdat(all,slot,phys,addr,size);
}


/* print pfdata table */
int
prpfdat(all,slot,phys,addr,size)
int all,slot,phys;
long addr;
unsigned size;
{

	long pfdat;
	pfd_t pfbuf;
	long next, prev, hash;

	readmem((long)Pfdat->n_value,1,-1,
		(char *)&pfdat,sizeof pfdat,"pfdat pointer");

	readbuf(addr,(long)(pfdat+slot*sizeof (struct pfdat)),phys,-1,
		(char *)&pfbuf,sizeof pfbuf,"pfdata table");
	if(addr > -1)
		slot = getslot(addr,(long)pfdat,sizeof pfbuf,phys);
	if(!pfbuf.pf_flags && !all)
		return;
	/* calculate pfdata entry number of pointers */
	next = ((long)pfbuf.pf_next - pfdat)/sizeof pfbuf;
	if ((next < 0) || (next > size)) next = -1;
	prev = ((long)pfbuf.pf_prev - pfdat)/sizeof pfbuf;
	if ((prev < 0) || (prev > size)) prev = -1;
	hash = ((long)pfbuf.pf_hchain - pfdat)/sizeof pfbuf;
	if ((hash < 0) || (hash > size)) hash = -1;
		sizeof (struct inode);
	if(addr > -1) 
		slot = getslot(addr,(long)pfdat,sizeof pfbuf,phys);
	fprintf(fp,"%4d %8d %3d %3u,%-3u %5d  %3d  %3d  %3d",
		slot,
		pfbuf.pf_blkno,
		pfbuf.pf_use,
		major(pfbuf.pf_dev),
		minor(pfbuf.pf_dev),
		pfbuf.pf_inumber,
		pfbuf.pf_swpi,
		pfbuf.pf_rawcnt,
		pfbuf.pf_waitcnt);
	if(next == -1)
		fprintf(fp,"     ");
	else fprintf(fp,"  %3d",next);
	if(prev == -1)
		fprintf(fp,"     ");
	else fprintf(fp,"  %3d",prev);
	if(hash == -1)
		fprintf(fp,"     ");
	else fprintf(fp,"  %3d",hash);
	fprintf(fp,"%s%s%s%s%s\n",
		pfbuf.pf_flags & P_QUEUE ? " que" : "",
		pfbuf.pf_flags & P_BAD ? " bad" : "",
		pfbuf.pf_flags & P_HASH ? " hsh" : "",
		pfbuf.pf_flags & P_DONE ? " don" : "",
		pfbuf.pf_flags & P_SWAP ? " swp" : "");
}

/* get arguments for sdt function */
int
getsdt()
{
	int proc = Procslot;
	int all = 0;
	int phys = 0;
	long addr = -1;
	int c;
	int section;
	int count = 1;
	struct proc prbuf;

	optind = 1;
	while((c = getopt(argcnt,args,"epw:s:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'p' :	phys = 1;
					break;
			case 's' :	proc = setproc();
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(!args[optind])
		longjmp(syn,0);
	if((section = strcon(args[optind++],'h')) == -1)
		error("\n");
	if(section > 3) {
		addr = (long)section;
		section = -1;
		if(args[optind]) 
			if((count = strcon(args[optind],'d')) == -1)
				error("\n");
	}
	procntry(proc,&prbuf);
	fprintf(fp,"SLOT SEG KPROT UPROT LENGTH  ADDRESS FLAGS\n");
	prsdt(all,phys,section,addr,count,proc,&prbuf);
}


/* print segment descriptor table */
int     
prsdt(all,phys,section,addr,count,proc,pp)
int all,phys,section,count,proc;
long addr;
struct proc *pp;
{
	long sdtaddr;
	sde_t segbuf;
	int len,i;

	if(section > -1) {
		/* determine address and length of table by section */
		switch(section) {
			case 0:	/* kernel section 0 */
				sdtaddr = (long)sramapt[0];
				len = srambpt[0].SDTlen;
				break;
			case 1:	/* kernel section 1 */
				sdtaddr = (long)sramapt[1];
				len = srambpt[1].SDTlen;
				break;
			case 2:	/* text, data and bss section */
				sdtaddr = pp->p_srama[0];
				len = pp->p_sramb[0].SDTlen;
				break;
			case 3:	/* shared memory, stack and ublock */
				sdtaddr = pp->p_srama[1];
				len = pp->p_sramb[1].SDTlen;
				break;
		}
	}
	else {
		if(phys || !Virtmode) {
			sdtaddr = addr;
			len = count;
		}
		else {
			sdtaddr = vtop(addr,proc);
			if(sdtaddr == -1)
				error("%x is an invalid address\n",addr);
			sdtaddr |= MAINSTORE;
			len = count;
		}
	}

	if(lseek(mem, sdtaddr - MAINSTORE, 0) == -1)
		error("seek error on segment descriptor table address\n");
	for(i = 0; i <= len; i++) {
		if(read(mem,&segbuf,sizeof segbuf) != sizeof segbuf) 
			error("read error on segment descriptor table\n");
		if(!(segbuf.seg_flags & SDE_V_bit) && !all)
			continue;
		fprintf(fp,"%4d ",i);
		if(iscontig(&segbuf))
			fprintf(fp,"con ");
		else fprintf(fp,"pdt ");
		switch(segbuf.seg_prot & KRWE) {
			case KNONE: fprintf(fp,"knone"); break;
			case KEO: fprintf(fp,"keo  "); break;
			case KRE: fprintf(fp,"kre  "); break;
			case KRWE: fprintf(fp,"krwe "); break;
			default: fprintf(fp,"  -  "); break;
		}
		switch(segbuf.seg_prot & URWE) {
			case UNONE: fprintf(fp," unone"); break;
			case UEO: fprintf(fp,"   ueo"); break;
			case URE: fprintf(fp,"   ure"); break;
			case URWE: fprintf(fp,"  urwe"); break;
			default: fprintf(fp,"   -  "); break;
		}
		fprintf(fp," %6u %8x %s%s%s%s%s\n",
			segbuf.seg_len * 8,
			segbuf.wd2.address,
			segbuf.seg_flags & SDE_I_bit ? "i" : "",
			segbuf.seg_flags & SDE_V_bit ? "v" : "",
			segbuf.seg_flags & SDE_T_bit ? "t" : "",
			segbuf.seg_flags & SDE_C_bit ? "c" : "",
			segbuf.seg_flags & SDE_P_bit ? "p" : "");
	}
}

/* get arguments for pdt function */
int
getpdt()
{
	int proc = Procslot;
	int all = 0;
	int phys = 0;
	long addr = -1;
	int c;
	struct proc prbuf;
	int section;
	int segment;
	int count = 1;

	optind = 1;
	while((c = getopt(argcnt,args,"epw:s:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'p' :	phys = 1;
					break;
			case 's' :	proc = setproc();
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(!args[optind])
		longjmp(syn,0);
	if((section = strcon(args[optind++],'h')) == -1)
		error("\n");
	if(section > 3) {
		addr = (long)section;
		segment = section = -1;
		if(args[optind]) 
			if((count = strcon(args[optind],'d')) == -1)
				error("\n");
	}
	else {
		if(!args[optind])
			longjmp(syn,0);
		if((segment = strcon(args[optind],'d')) == -1)
			error("\n");
	}
	procntry(proc,&prbuf);
	fprintf(fp,"SLOT   PFN    FLAGS\n");
	prpdt(all,phys,section,segment,addr,count,proc,&prbuf);
}

/* print page descriptor table */
int
prpdt(all,phys,section,segment,addr,count,proc,pp)
int all,phys,section,segment,count,proc;
long addr;
struct proc *pp;
{
	long plen,paddr;
	pde_t pgbuf;
	long sdtaddr,len;
	sde_t segbuf;
	int i;

	if(section > -1) {
		/* determine address and length by section */
		switch(section) {
			case 0:	/* kernel section 0 */
				sdtaddr = (long)sramapt[0];
				len = srambpt[0].SDTlen;
				break;
			case 1:	/* kernel section 1 */
				sdtaddr = (long)sramapt[1];
				len = srambpt[1].SDTlen;
				break;
			case 2:	/* text, data and bss section */
				sdtaddr = pp->p_srama[0];
				len = pp->p_sramb[0].SDTlen;
				break;
			case 3:	/* shared memory, stack and ublock */
				sdtaddr = pp->p_srama[1];
				len = pp->p_sramb[1].SDTlen;
				break;
		}
		if(segment < 0 || segment > len) 
			error("segment number out of range\n");

		readmem((long)((sdtaddr+segment*sizeof segbuf) - MAINSTORE),
			0,proc,(char *)&segbuf,sizeof segbuf,
			"segment descriptor table");

		if(iscontig(&segbuf)) 
			error("contiguous segment - not valid in page descriptor table\n");
		/* locate page descriptor table */
		paddr = segbuf.wd2.address & ~0x7;
		plen = btopt(segbuf.seg_len);
	}
	else {
		if(phys || !Virtmode) {
		paddr = addr;
		plen = count;
		}
		else {
			paddr = vtop(addr,proc);
			if(paddr == -1)
				error("%x is an invalid address\n",addr);
			paddr |= MAINSTORE;
			plen = count;
		}
	}
	if(lseek(mem,paddr - MAINSTORE, 0) == -1)
		error("seek error on page descriptor table address\n");
	for(i = 0; i < plen; i++) {
		if(read(mem,&pgbuf, sizeof pgbuf) != sizeof pgbuf) 
			error("read error on page descriptor table\n");
		if(!pgbuf.pgm.pg_pfn && !all)
			continue;
		fprintf(fp,"%4d %6x   %s%s%s%s%s%s%s\n",
			i,pgbuf.pgm.pg_pfn,
			pgbuf.pgm.pg_lock ? "lock " : "",	
			pgbuf.pgm.pg_ndref ? "ndref " : "",	
			pgbuf.pgm.pg_ref ? "ref " : "",	
			pgbuf.pgm.pg_cw ? "cw " : "",	
			pgbuf.pgm.pg_last ? "last " : "",	
			pgbuf.pgm.pg_mod ? "mod " : "",	
			pgbuf.pgm.pg_pres ? "pres " : "");	
	}
}

/* get arguments for region function */
int
getregion()
{
	int slot = -1;
	int full = 0;
	int all = 0;
	int phys = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int c;
	char * heading = "SLOT NSG PSZ #VL REF WAT   FSIZE  TYPE FOR BCK INO LISTADDR FLAGS\n";

	if(!Region)
		if(!(Region = symsrch("region")))
			error("region not found in symbol table\n");

	optind = 1;
	while((c = getopt(argcnt,args,"efpw:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'f' :	full = 1;
					break;
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}
	fprintf(fp,"REGION TABLE SIZE = %d\n",vbuf.v_region);
	if(!full)
		fprintf(fp,"%s",heading);
	if(args[optind]) {
		all = 1;
		do {
			getargs(vbuf.v_region,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prregion(all,full,slot,phys,addr,heading);
			else {
				if(arg1 < vbuf.v_region)
					slot = arg1;
				else addr = arg1;
				prregion(all,full,slot,phys,addr,heading);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < vbuf.v_region; slot++)
		prregion(all,full,slot,phys,addr,heading);
}


/* print region table */
int
prregion(all,full,slot,phys,addr,heading)
int all,full,slot,phys;
long addr;
char *heading;
{
	struct region rbuf;
	char *typ;
	long forw,back;

	readbuf(addr,(long)(Region->n_value+slot*sizeof rbuf),phys,-1,
		(char *)&rbuf,sizeof rbuf,"region table");
	if(!all && !rbuf.r_refcnt)
		return;
	if(addr > -1) 
		slot = getslot(addr,(long)Region->n_value,sizeof rbuf,phys);
	if(full)
		fprintf(fp,"%s",heading);
	fprintf(fp,"%4d %3d %3d %3d %3d %3d %8x ",
		slot,
		rbuf.r_listsz,
		rbuf.r_pgsz,
		rbuf.r_nvalid,
		rbuf.r_refcnt,
		rbuf.r_waitcnt,
		rbuf.r_filesz);
	switch(rbuf.r_type) {
		case RT_UNUSED:  typ = "nuse"; break;
		case RT_PRIVATE:  typ = "priv"; break;
		case RT_STEXT:  typ = "stxt"; break;
		case RT_SHMEM:  typ = "shmm"; break;
		default:  typ = "?"; break;
	}
	fprintf(fp,"%4s ",typ);
	if((forw = ((long)rbuf.r_forw-Region->n_value)/sizeof(struct region))
		>= vbuf.v_region)
		fprintf(fp,"    ");
	else fprintf(fp,"%3d ",forw);
	if((back = ((long)rbuf.r_back-Region->n_value)/sizeof(struct region))
		>= vbuf.v_region)
		fprintf(fp,"    ");
	else fprintf(fp,"%3d ",back);
	if(rbuf.r_iptr)
		fprintf(fp,"%3d ",rbuf.r_iptr-(struct inode *)Inode->n_value);
	else fprintf(fp,"    ");
	if(rbuf.r_pgsz)
		fprintf(fp,"%8x",rbuf.r_list);
	else fprintf(fp,"        ");
	fprintf(fp,"%s%s%s%s%s%s%s",
			rbuf.r_flags & XREMOTE ? " xremote" : "",
			rbuf.r_flags & RG_NOFREE ? " nofree" : "",
			rbuf.r_flags & RG_DONE ? " done" : "",
			rbuf.r_flags & RG_NOSHARE ? " noshare" : "",
			rbuf.r_flags & RG_LOCK ? " lock" : "",
			rbuf.r_flags & RG_WANTED ? " want" : "",
			rbuf.r_flags & RG_WAITING ? " wait" : "");
	if(full) { 
		if(rbuf.r_pgsz) 
			prlist(rbuf.r_list,(unsigned)(rbuf.r_pgsz + (NPGPT - 1))/NPGPT);
		fprintf(fp,"\n\tnoswapcnt: %d\n",rbuf.r_noswapcnt);
		fprintf(fp,"\n");
	}
	else fprintf(fp,"\n");
}


/* print list of pointers to page tables */
int
prlist(addr,nopg)
long addr;
unsigned nopg;
{

	int list, i;

	fprintf(fp,"\n\tPDT: ");
	seekmem(addr,1,-1);
	for(i = 0; i < nopg; i++) {
		if(read(mem,(char *)&list,sizeof list) != sizeof list) 
			error("read error on rlist\n");
		if(!list)
			break;
		fprintf(fp,"%8x", list);
		if(((i+1) % 4) == 0)
			fprintf(fp,"\n\t");
	}
}

