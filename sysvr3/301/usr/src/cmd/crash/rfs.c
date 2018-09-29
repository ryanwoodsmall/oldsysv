/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:rfs.c	1.16"
/*
* This file contains code for the crash functions:  adv, gdp, rcvd, sndd.
*/

#include "stdio.h"
#include "sys/param.h"
#include "a.out.h"
#include "sys/types.h"
#include "sys/sbd.h"
#include "sys/file.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/fs/s5param.h"
#include "sys/fs/s5filsys.h"
#include "sys/sema.h"
#include "sys/comm.h"
#include "sys/gdpstr.h"
#include "sys/nserve.h"
#include "sys/cirmgr.h"
#include "sys/fs/s5dir.h"
#include "sys/mount.h"
#include "sys/nserve.h"
#include "sys/adv.h"
#include "sys/var.h"
#include "sys/stream.h"
#include "sys/inode.h"
#include "crash.h"

extern struct syment *Queue,*File,*Inode,*Mblock,*Proc;	/* namelist symbol */
static struct syment *Nadv,*Advbuf,*Gdp;		/* pointers	*/
struct syment *Rcvd, *Nrcvd,*Nsndd,*Sndd,*Maxgdp;


/* check for rfs activity */
int
checkboot()
{
	int bootstate;
	struct syment *boot;

	if(!(boot = symsrch("bootstate"))) 
		error("bootstate not found in symbol table\n");

	readmem((long)boot->n_value,1,-1,(char *)&bootstate,
		sizeof bootstate,"bootstate");

	if(bootstate != DU_UP) {
		if(bootstate == DU_DOWN)
			prerrmes("rfs not started\n\n");
		else prerrmes("rfs in process of starting\n\n");
	}
}


/* get arguments for adv function */
int
getadv()
{
	int slot = -1;
	int all = 0;
	int phys = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int c;
	int nadv;

	if(!Advbuf)
		if(!(Advbuf = symsrch("advertise")))
			error("advertise table not found\n");
	if(!Nadv)
		if(!(Nadv = symsrch("nadvertise"))) 
			error("cannot determine size of advertise table\n");
	readmem((long)Nadv->n_value,1,-1,(char *)&nadv,
		sizeof nadv,"size of advertise table");
	if(!Rcvd)
		if(!(Rcvd = symsrch("rcvd"))) 
			error("receive descriptor table not found\n");
	if(!Nrcvd)
		if(!(Nrcvd = symsrch("nrcvd"))) 
			error("cannot determine size of receive descriptor table\n");
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
	checkboot();
	fprintf(fp,"ADVERTISE TABLE SIZE = %d\n",nadv);
	fprintf(fp,"SLOT  CNT      NAME      RCVD   CLIST   FLAGS\n");
	if(args[optind]) {
		all = 1;
		do {
			getargs(nadv,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					pradv(all,slot,phys,addr);
			else {
				if(arg1 < nadv)
					slot = arg1;
				else addr = arg1;
				pradv(all,slot,phys,addr);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < nadv; slot++)
		pradv(all,slot,phys,addr);
}


/* print advertise table */
int
pradv(all,slot,phys,addr)
int all,slot,phys;
long addr;
{
	struct advertise advbuf;
	int nrcvd;

	readbuf(addr,(long)(Advbuf->n_value+slot*sizeof advbuf),phys,-1,
		(char *)&advbuf,sizeof advbuf,"advertise table");
	readmem((long)Nrcvd->n_value,1,-1,(char *)&nrcvd,
		sizeof nrcvd,"size of receive descriptor table");
	if ((advbuf.a_flags == A_FREE) && !all)
		return;
	if(addr > -1) 
		slot = getslot(addr,(long)Advbuf->n_value,sizeof advbuf,phys);
	fprintf(fp,"%4d %4u %-14s",
		slot,
		advbuf.a_count,
		advbuf.a_name);
	slot = ((long)advbuf.a_queue-(long)Rcvd->n_value)/sizeof (struct rcvd);
	if((slot >= 0) && (slot < nrcvd))
		fprintf(fp," %4d",slot);
	else fprintf(fp,"   - ");
	fprintf(fp," %8x", advbuf.a_clist);
	fprintf(fp," %s%s%s",
		advbuf.a_flags & A_RDONLY ? " ro" : " rw",
		advbuf.a_flags & A_CLIST ? " cl" : "",
		advbuf.a_flags & A_MODIFY ? " md" : "");
	if(all)
		fprintf(fp,"%s", advbuf.a_flags & A_INUSE ? " use" : "");
	fprintf(fp,"\n");
}


/* get arguments for gdp function */
int
getgdp()
{
	int slot = -1;
	int full = 0;
	int all = 0;
	int phys = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int c;
	char *heading = "SLOT QUEUE FILE MNT SYSID FLAG\n";
	int maxgdp;

	if(!Gdp)
		if(!(Gdp = symsrch("gdp"))) 
			error("gdp table not found\n");
	if(!Maxgdp)
		if(!(Maxgdp = symsrch("maxgdp"))) 
			error("cannot determine size of gift descriptor table\n");
	readmem((long)Maxgdp->n_value,1,-1,(char *)&maxgdp,
		sizeof maxgdp,"size of gift descriptor table");
	if(!Queue)
		if(!(Queue = symsrch("queue")))
			error("queue not found in symbol table\n");
	optind = 1;
	while((c = getopt(argcnt,args,"efpw:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'f' :	full = 1;
					break;
			case 'p' :	phys = 1;
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	checkboot();
	fprintf(fp,"GDP MAX SIZE = %d\n",maxgdp);
	if(!full)
		fprintf(fp,"%s",heading);
	if(args[optind]) {
		all = 1;
		do {
			getargs(maxgdp,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prgdp(full,all,slot,phys,addr,heading);
			else {
				if(arg1 < maxgdp)
					slot = arg1;
				else addr = arg1;
				prgdp(full,all,slot,phys,addr,heading);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < maxgdp; slot++)
		prgdp(full,all,slot,phys,addr,heading);
}

/* print gdp table */
int
prgdp(full,all,slot,phys,addr,heading)
int full,all,slot,phys;
long addr;
char *heading;
{
	struct gdp gdpbuf;
	char temp[MAXDNAME+1];

	readbuf(addr,(long)(Gdp->n_value+slot*sizeof gdpbuf),phys,-1,
		(char *)&gdpbuf,sizeof gdpbuf,"gdp structures");
	if (!gdpbuf.queue && !all)
		return;
	if(addr > -1) 
		slot = getslot(addr,(long)Gdp->n_value,sizeof gdpbuf,phys);
	if(full)
		fprintf(fp,"%s",heading);
	fprintf(fp,"%4d",slot);
	slot = ((long)gdpbuf.queue - (long)Queue->n_value)/sizeof (struct queue);
	if((slot >= 0) && (slot < vbuf.v_nqueue))
		fprintf(fp,"  %4d",slot);
	else fprintf(fp,"    - ");	
	slot = ((long)gdpbuf.file - (long)File->n_value)/sizeof (struct file);
	if((slot >= 0) && (slot < vbuf.v_file))
		fprintf(fp," %4d",slot);
	else fprintf(fp,"   - ");	
	fprintf(fp,"  %2d  %4x %s%s%s\n",
		gdpbuf.mntcnt,
		gdpbuf.sysid,
		(gdpbuf.flag & GDPDISCONN) ? " dis" : "",
		(gdpbuf.flag & GDPRECOVER) ? " rec" : "",
		(gdpbuf.flag & GDPCONNECT) ? " con" : "");
	if(full) {
		fprintf(fp,"\tHET VER  UID   GID    TIME\n");
		fprintf(fp,"\t%3x  %2d %5x %5x %10d\n",
			gdpbuf.hetero,
			gdpbuf.version,
			gdpbuf.idmap[0],
			gdpbuf.idmap[1],
			gdpbuf.time);
		strncpy(temp,gdpbuf.token.t_uname,MAXDNAME);
		fprintf(fp,"\tTOKEN ID:  %4x    TOKEN NAME:  %s\n",
			gdpbuf.token.t_id,
			temp);
		fprintf(fp,"\n");
	}
}


/* get arguments for rcvd function */
int
getrcvd()
{
	int slot = -1;
	int full = 0;
	int all = 0;
	int phys = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int c;
	int nrcvd;
	char *heading = "SLOT ACNT  QTYPE  I/SD QCNT RCNT STATE\n";

	if(!Rcvd)
		if(!(Rcvd = symsrch("rcvd"))) 
			error("receive descriptor table not found\n");
	if(!Nrcvd)
		if(!(Nrcvd = symsrch("nrcvd"))) 
			error("cannot determine size of receive descriptor table\n");
	if(!Sndd)
		if(!(Sndd = symsrch("sndd"))) 
			error("send descriptor table not found\n");
	if(!Nsndd)
		if(!(Nsndd = symsrch("nsndd"))) 
			error("cannot determine size of send descriptor table\n");
	if(!Mblock)
		if(!(Mblock = symsrch("mblock"))) 
			error("mblock not found in symbol table\n");
	if(!Queue)
		if(!(Queue = symsrch("queue"))) 
			error("queue not found in symbol table\n");
	blockinit();
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
	checkboot();
	readmem((long)Nrcvd->n_value,1,-1,(char *)&nrcvd,
		sizeof nrcvd,"size of receive descriptor table");
	fprintf(fp,"RECEIVE DESCRIPTOR TABLE SIZE = %d\n",nrcvd);
	if(!full)
		fprintf(fp,"%s",heading);
	if(args[optind]) {
		all = 1;
		do {
			getargs(nrcvd,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prrcvd(all,full,slot,phys,addr,
						heading,nrcvd);
			else {
				if(arg1 < vbuf.v_mount)
					slot = arg1;
				else addr = arg1;
				prrcvd(all,full,slot,phys,addr,heading,nrcvd);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < nrcvd; slot++)
		prrcvd(all,full,slot,phys,addr,heading,nrcvd);
}

/* print rcvd table */
int
prrcvd(all,full,slot,phys,addr,heading,size)
int all,full,slot,phys,size;
long addr;
char *heading;
{
	struct rcvd rcvdbuf;
	struct rd_user userbuf;
	struct rd_user *next;
	int nsndd;

	readbuf(addr,(long)(Rcvd->n_value+slot*sizeof rcvdbuf),phys,-1,
		(char *)&rcvdbuf,sizeof rcvdbuf,"received descriptor table");
	readmem((long)Nsndd->n_value,1,-1,(char *)&nsndd,
		sizeof nsndd,"size of send descriptor table");
	if(((rcvdbuf.rd_stat == 0) || (rcvdbuf.rd_stat & RDUNUSED)) && !all)
		return;
	if(addr > -1) 
		slot = getslot(addr,(long)Rcvd->n_value,sizeof rcvdbuf,phys);
	if(full) {
		streaminit();
        	fprintf(fp,"%s",heading);
	}
	fprintf(fp,"%4d %4u %c",
		slot,
		rcvdbuf.rd_act_cnt,
		(rcvdbuf.rd_qtype & GENERAL) ? 'G' : 'S');
		fprintf(fp," %s",
			(rcvdbuf.rd_qtype & RDLBIN) ? "lbin" : 
			((rcvdbuf.rd_qtype & RDTEXT) ? "text" : "    "));
	if(rcvdbuf.rd_qtype & GENERAL) {
		slot = ((long)rcvdbuf.rd_inode-(long)Inode->n_value)/
			sizeof (struct inode);
		if((slot >= 0) && (slot < vbuf.v_inode))
			fprintf(fp," I%4d",slot);
		else fprintf(fp,"   -  ");
	}
	else {
		slot = ((long)rcvdbuf.rd_inode-(long)Sndd->n_value)/
			sizeof (struct sndd);
		if((slot >= 0) && (slot < nsndd))
			fprintf(fp," S%4d",slot);
		else fprintf(fp,"   -  ");
	}
	fprintf(fp," %4u %4u",
		rcvdbuf.rd_qcnt,
		rcvdbuf.rd_refcnt);
	fprintf(fp,"%s%s",
		(rcvdbuf.rd_stat & RDUSED) ? " used" : "",
		(rcvdbuf.rd_stat & RDLINKDOWN) ? " ldown" : "");
	fprintf(fp,"\n");
	if(full) {
		fprintf(fp,"\tMSERVE QSIZE CONID SNDD NEXT   QSLP   RHEAD RTAIL\n");
		fprintf(fp,"\t %5d %5d %5u",
			rcvdbuf.rd_max_serv,
			rcvdbuf.rd_qsize,
			rcvdbuf.rd_connid);
		slot = ((long)rcvdbuf.rd_sdnack-(long)Sndd->n_value)/
			sizeof (struct sndd);
		if((slot >= 0) && (slot < nsndd))
			fprintf(fp," %4d",slot);
		else fprintf(fp,"  -  ");
		slot = ((long)rcvdbuf.rd_next-(long)Rcvd->n_value)/
			sizeof (struct rcvd);
		if((slot >= 0) && (slot < size))
			fprintf(fp," %4d",slot);
		else fprintf(fp,"  -  ");
		fprintf(fp," %8x", rcvdbuf.rd_qslp);
		slot = ((long)rcvdbuf.rd_rcvdq.qc_head-(long)Mblock->n_value)/
			sizeof (struct msgb);
		if((slot >= 0) && (slot < nmblock))
			fprintf(fp,"   %3d",slot);
		else fprintf(fp,"   -  ");
		slot = ((long)rcvdbuf.rd_rcvdq.qc_tail-(long)Mblock->n_value)/
			sizeof (struct msgb);
		if((slot >= 0) && (slot < nmblock))
			fprintf(fp,"   %3d",slot);
		else fprintf(fp,"   -  ");
		if(rcvdbuf.rd_user_list) {
			fprintf(fp,"   USER_LIST: %8x",rcvdbuf.rd_user_list);
			if(rcvdbuf.rd_qtype & GENERAL) {
				next = rcvdbuf.rd_user_list;
				if(next)
					fprintf(fp,"\n\t\tQUEUE SRMNT  ICNT  FCNT  RCNT  WCNT   NEXT\n");
				while(next) {
					readmem((long)next,1,-1,(char *)&userbuf,
						sizeof userbuf,"user list");
					slot = ((long)userbuf.ru_queue-
					(long)Queue->n_value)/
						sizeof (struct queue);
					if((slot >= 0) && (slot < vbuf.v_nqueue))
						fprintf(fp,"\t\t%5d",slot);
					else fprintf(fp,"\t\t   - ");
					fprintf(fp," %5d", userbuf.ru_srmntindx);
					fprintf(fp," %5d %5d %5d %5d",
						userbuf.ru_icount,
						userbuf.ru_fcount,
						userbuf.ru_frcnt,
						userbuf.ru_fwcnt);
					next = userbuf.ru_next;
					if(next)
						fprintf(fp," %8x\n",next);
				else fprintf(fp,"     -   \n");
				}
			}
			else fprintf(fp,"\n");
		}
		else fprintf(fp,"\n");
		fprintf(fp,"\n");
	}
}

/* get arguments for sndd function */
int
getsndd()
{
	int slot = -1;
	int all = 0;
	int phys = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int c;
	int nsndd;

	if(!Sndd)
		if(!(Sndd = symsrch("sndd"))) 
			error("send descriptor table not found\n");
	if(!Nsndd)
		if(!(Nsndd = symsrch("nsndd"))) 
			error("cannot determine size of send descriptor table\n");
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
	checkboot();
	readmem((long)Nsndd->n_value,1,-1,(char *)&nsndd,
		sizeof nsndd,"size of send descriptor table");
	fprintf(fp,"SEND DESCRIPTOR TABLE SIZE = %d\n",nsndd);
        fprintf(fp,"SLOT RCNT SNDX MNDX CONID COPY PROC SQUE NEXT  MODE  STATE\n");
	if(args[optind]) {
		all = 1;
		do {
			getargs(nsndd,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prsndd(all,slot,phys,addr,nsndd);
			else {
				if(arg1 < nsndd)
					slot = arg1;
				else addr = arg1;
				prsndd(all,slot,phys,addr,nsndd);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < nsndd; slot++)
		prsndd(all,slot,phys,addr,nsndd);
}

/* print sndd function */
int
prsndd(all,slot,phys,addr,size)
int all,slot,phys,size;
long addr;
{
	struct sndd snddbuf;

	readbuf(addr,(long)(Sndd->n_value+slot*sizeof snddbuf),phys,-1,
		(char *)&snddbuf,sizeof snddbuf,"send descriptor table");
	if(((snddbuf.sd_stat == 0) || (snddbuf.sd_stat & SDUNUSED)) && !all)
		return;
	if(addr > -1) 
		slot = getslot(addr,(long)Sndd->n_value,sizeof snddbuf,phys);
	fprintf(fp,"%4d %4u %4u %4u %5u %4u",
		slot,
		snddbuf.sd_refcnt,
		snddbuf.sd_sindex,
		snddbuf.sd_mntindx,
		snddbuf.sd_connid,
		snddbuf.sd_copycnt);
	slot = ((long)snddbuf.sd_srvproc - (long)Proc->n_value)/
		sizeof (struct proc);
	if((slot >= 0) && (slot < vbuf.v_proc))
		fprintf(fp," %4d",slot);
	else fprintf(fp,"  -  ");
	slot = ((long)snddbuf.sd_queue - (long)Queue->n_value)/
		sizeof (struct queue);
	if((slot >= 0) && (slot < vbuf.v_nqueue))
		fprintf(fp," %4d",slot);
	else fprintf(fp,"  -  ");
	slot = ((long)snddbuf.sd_next - (long)Sndd->n_value)/
		sizeof (struct sndd);
	if((slot >= 0) && (slot < size))
		fprintf(fp," %4d",slot);
	else fprintf(fp,"  -  ");
	fprintf(fp," %s%s%s%03o",
		snddbuf.sd_mode & ISUID ? "u" : "-",
		snddbuf.sd_mode & ISGID ? "g" : "-",
		snddbuf.sd_mode & ISVTX ? "v" : "-",
		snddbuf.sd_mode & 0777);
	fprintf(fp,"%s%s%s",
		(snddbuf.sd_stat & SDUSED) ? " used" : "",
		(snddbuf.sd_stat & SDSERVE) ? " serve" : "",
		(snddbuf.sd_stat & SDLINKDOWN) ? " ldown" : "");
	fprintf(fp,"\n");
}
