/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:proc.c	1.15"
/*
 * This file contains code for the crash functions:  proc, defproc.
 */

#include "sys/param.h"
#include "a.out.h"
#include "stdio.h"
#include "signal.h"
#include "time.h"
#include "sys/sysmacros.h"
#include "sys/types.h"
#include "sys/fs/s5dir.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/var.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "crash.h"

#define min(a,b) ((a) > (b) ? (b) : (a))

extern struct user *ubp;		/* pointer to the ublock */
extern struct syment *Proc,*Curproc,*Region;	/* namelist symbol pointers */
static struct syment *Pregpp;


/* get arguments for proc function */
int
getproc()
{
	int slot = Procslot;
	int all = 0;
	int full = 0;
	int phys = 0;
	int run = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int id = -1;
	int c;
	char *heading = "SLOT ST PID   PPID  PGRP   UID PRI CPU   EVENT  NAME     FLAGS\n";

	if(!Pregpp)
		if(!(Pregpp = symsrch("pregpp")))
			error("pregpp not found in symbol table\n");
	if(!Region)
		if(!(Region = symsrch("region")))
			error("region not found in symbol table\n");

	optind = 1;
	while((c = getopt(argcnt,args,"fprw:")) !=EOF) {
		switch(c) {
			case 'f' :	full = 1;
					break;
			case 'w' :	redirect();
					break;
			case 'r' :	run = 1;
					break;
			case 'p' :	phys = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}
	fprintf(fp,"PROC TABLE SIZE = %d\n",vbuf.v_proc);
	if(!full)
		fprintf(fp,"%s",heading);
	if(args[optind]) {
		all = 1;
		do {
			if(*args[optind] == '#') {
				if((id = strcon(++args[optind],'d')) == -1)
					error("\n");
				prproc(all,full,slot,id,phys,run,addr,heading);
			}
			else {
				getargs(vbuf.v_proc,&arg1,&arg2);
				if(arg1 == -1) 
					continue;
				if(arg2 != -1)
					for(slot = arg1; slot <= arg2; slot++)
						prproc(all,full,slot,id,phys,
							run,addr,heading);
				else {
					if(arg1 < vbuf.v_proc)
						slot = arg1;
					else addr = arg1;
					prproc(all,full,slot,id,phys,run,addr,
						heading);
				}
			}
			id = slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < vbuf.v_proc; slot++)
		prproc(all,full,slot,id,phys,run,addr,heading);
}


/* print proc table */
int
prproc(all,full,slot,id,phys,run,addr,heading)
int all,full,slot,id,phys,run;
long addr;
char *heading;
{
	char ch,*typ;
	char *cp = "";
	struct proc procbuf;
	struct pregion pregbuf;
	int i,j,cnt,pregpp,regslot;
	long pslot_va;
	extern long lseek();

	if(id != -1) {
		for(slot = 0; slot < vbuf.v_proc; slot++) {
			readmem((long)(Proc->n_value+slot*sizeof procbuf),1,
				slot,(char *)&procbuf,sizeof procbuf,
					"proc table");
				if(procbuf.p_pid == id) 
					break;
		}
		if(slot == vbuf.v_proc) {
			fprintf(fp,"%d not valid process id\n",id);
			return;
		}
	}
	else readbuf(addr,(long)(Proc->n_value+slot*sizeof procbuf),phys,-1,
		(char *)&procbuf,sizeof procbuf,"proc table");
	if(!procbuf.p_stat && !all)
		return;
	if(!procbuf.p_stat) {
		fprintf(fp,"%d is not a valid process\n",slot);
		return;	
	}
	if(run)
		if(!(procbuf.p_stat == SRUN || procbuf.p_stat == SONPROC))
			return;
	if(addr > -1) 
		slot = getslot(addr,(long)Proc->n_value,sizeof procbuf,phys);
	if(full)
		fprintf(fp,"%s",heading);
	switch(procbuf.p_stat) {
	case NULL:   ch = ' '; break;
	case SSLEEP: ch = 's'; break;
	case SRUN:   ch = 'r'; break;
	case SIDL:   ch = 'i'; break;
	case SZOMB:  ch = 'z'; break;
	case SSTOP:  ch = 't'; break;
	case SONPROC:  ch = 'p'; break;
	case SXBRK:  ch = 'x'; break;
	default:     ch = '?'; break;
	}
	fprintf(fp,"%4d %c %5u %5u %5u %5u  %2u %3u",
		slot,
		ch,
		procbuf.p_pid,
		procbuf.p_ppid,
		procbuf.p_pgrp,
		procbuf.p_uid,
		procbuf.p_pri,
		procbuf.p_cpu);
	if(procbuf.p_stat == SONPROC)
		fprintf(fp,"          ");
	else fprintf(fp," %08lx ",procbuf.p_wchan);
	if(procbuf.p_stat == SZOMB)
		cp = "zombie";
	else {
		/* get ublock */
		pslot_va = (long)(Proc->n_value+slot*sizeof(struct proc));
		i=((char*)ubptbl((proc_t*)pslot_va) - (char*)pslot_va -
			((char*)procbuf.p_ubptbl - (char*)&procbuf)) >> 2;
		for(cnt=0; cnt < sizeof(*ubp) + sizeof(int) * vbuf.v_nofiles;
			cnt += NBPP, i++) {
			/* seek from begining of memory to ith page of uarea */
			if(lseek(mem,(long)(procbuf.p_ubptbl[i].pgm.pg_pfn<<11)-
				MAINSTORE,0) == -1) {
				prerrmes("seek error on ublock address\n");
				return;
			}
			if(read(mem,(char *)ubp+cnt,min(NBPP,(sizeof(*ubp)+
				sizeof(int)*vbuf.v_nofiles)-cnt)) !=
				min(NBPP,(sizeof(*ubp)+sizeof(int)*
					vbuf.v_nofiles)-cnt)){
				prerrmes("read error on ublock\n");
				return;
			}
		}
		cp = ubp->u_comm;
	}
	for(i = 0; i < 8 && cp[i]; i++) {
		if(cp[i] < 040 || cp[i] > 0176) {
			cp = "unprint";
			break;
		}
	}
	fprintf(fp,"%-8s", cp);
	fprintf(fp,"%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		procbuf.p_flag & SLOAD ? " load" : "",
		procbuf.p_flag & SSYS ? " sys" : "",
		procbuf.p_flag & SLOCK ? " lock" : "",
		procbuf.p_flag & STRC ? " trc" : "",
 		procbuf.p_flag & SWTED ? " wted" : "",
 		procbuf.p_flag & SNWAKE ? " nwak" : "",
 		procbuf.p_flag & SRSIG ? " rsig" : "",
 		procbuf.p_flag & SPOLL ? " poll" : "",
 		procbuf.p_flag & SPRSTOP ? " prst" : "",
 		procbuf.p_flag & SPROCTR ? " prtr" : "",
 		procbuf.p_flag & SPROCIO ? " prio" : "",
 		procbuf.p_flag & SSEXEC ? " sx" : "",
 		procbuf.p_flag & SPROPEN ? " prop" : "");
	if(!full)
		return;
 	fprintf(fp,"\tsrama[0]: %-8x, sramb[0]: %-3d, srama[1]: %-8x, sramb[1]: %-3d\n",
	        procbuf.p_srama[0],
		procbuf.p_sramb[0].SDTlen,
		procbuf.p_srama[1],
		procbuf.p_sramb[1].SDTlen); 
 	fprintf(fp,"\ttime: %d, nice: %d, exit %d\n",
		procbuf.p_time,
		procbuf.p_nice,  
		procbuf.p_xstat);  
	fprintf(fp,"\tsig: %x, clktim: %d, suid: %d, sgid: %d, psize: %d\n",
		procbuf.p_sig,
		procbuf.p_clktim,
		procbuf.p_suid,
		procbuf.p_sgid,
		procbuf.p_size);
	fprintf(fp,"\tparent: %x\tchild: %x\tsibling: %x\n",
		procbuf.p_parent,
		procbuf.p_child,
		procbuf.p_sibling);
	if(procbuf.p_flink)
		fprintf(fp,"\tflink: %d",
		((unsigned)procbuf.p_flink - Proc->n_value) /sizeof (procbuf));
	if(procbuf.p_blink)
		fprintf(fp,"\tblink: %d",
		((unsigned)procbuf.p_blink - Proc->n_value) /sizeof (procbuf));
	if(procbuf.p_mlink)
		fprintf(fp,"\tmlink: %d\n",
		((unsigned)procbuf.p_mlink - Proc->n_value) /sizeof (procbuf));
	fprintf(fp,"\tutime: %ld\tstime: %ld\n",
		procbuf.p_utime,procbuf.p_stime);
	fprintf(fp, "\tubptbl:  ");
	for(i = 0,j = 1;  i < USIZE + 7  ;  i++) 
		if(*(int *)&procbuf.p_ubptbl[i] != 0) {
			if(!(j++ & 3))
				fprintf(fp,"\n\t");
			fprintf(fp,"%d: %8x   ",i,procbuf.p_ubptbl[i]);
		}	
	fprintf(fp,"\n\tepid: %d, sysid: %x, minwd: %x, rlink: %x\n",
		procbuf.p_epid,
		procbuf.p_sysid,
		procbuf.p_minwd,
		procbuf.p_rlink);
	fprintf(fp,"\ttrlock: %d, trace: %x, sigmask: %x,",
		procbuf.p_trlock,
		procbuf.p_trace,
		procbuf.p_sigmask);
	fprintf(fp," mpgneed: %d\n",procbuf.p_mpgneed);
	fprintf(fp,"\thold: %x, chold: %x\n",
		procbuf.p_hold,
		procbuf.p_chold);

	/* locate and print per process region table */

	readmem((long)Pregpp->n_value,1,slot,(char *)&pregpp,
		sizeof pregpp,"pregpp");

	fprintf(fp,"\tpreg reg#   regva  type  flags\n");
	for( i = 0; i < pregpp; i++) {
	readmem((long)(procbuf.p_region+i),1,slot,
		(char *)&pregbuf,sizeof pregbuf,"pregion table");

		if(!pregbuf.p_reg) 
			break;
		fprintf(fp,"\t%4d ",i);
		regslot = ((long)pregbuf.p_reg-Region->n_value)/
			sizeof(struct region);
		if((regslot >= 0) && (regslot < vbuf.v_region))
			fprintf(fp,"%4d ",regslot);
		else fprintf(fp,"  -  ");
		fprintf(fp,"%8x ",pregbuf.p_regva);
		switch(pregbuf.p_type) {
			case PT_UNUSED: typ = "unusd"; break;
			case PT_TEXT: typ = "text"; break;
			case PT_DATA: typ = "data"; break;
			case PT_STACK: typ = "stack"; break;
			case PT_SHMEM: typ = "shmem"; break;
			case PT_DMM: typ = "dmm"; break;
			case PT_LIBTXT: typ = "lbtxt"; break;
			case PT_LIBDAT: typ = "lbdat"; break;
			default: typ = ""; break;
		}
		fprintf(fp,"%5s ",typ);
		fprintf(fp,"%s\n",
			pregbuf.p_flags & PF_RDONLY ? "rdonly" : "");
	}
	fprintf(fp,"\n");
}


/* get arguments for defproc function */
int
getdefproc()
{
	int c;
	int proc = -1;
	int reset = 0;

	optind = 1;
	while((c = getopt(argcnt,args,"cw:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 'c' :	reset = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) 
		if((proc = strcon(args[optind],'d')) == -1)
			error("\n");
	prdefproc(proc,reset);
}

/* print results of defproc function */
int
prdefproc(proc,reset)
int proc,reset;
{

	if(reset)
		Procslot = getcurproc();
	else if(proc > -1) {
		if((proc > vbuf.v_proc) || (proc < 0))
			error("%d out of range\n",proc);
		Procslot = proc;
	}
	fprintf(fp,"Procslot = %d\n",Procslot);
}
