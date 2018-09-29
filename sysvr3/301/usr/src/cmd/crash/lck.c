/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:lck.c	1.9"
/*
 * This file contains code for the crash function lck.
 */

#include "stdio.h"
#include "sys/types.h"
#include "sys/param.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/var.h"
#include "a.out.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/inode.h"
#include "sys/fcntl.h"
#include "sys/flock.h"
#include "crash.h"

static struct syment *Flckinfo,*Sleeplcks,*Frlock,*Flox; /* namelist symbol */
extern struct syment *Inode,*Proc;			 /* pointers */
struct procid {			/* effective and sys ids */
	short epid;
	short sysid;
};
struct procid *procptr;		/* pointer to procid structure */
extern char *malloc();

/* get effective and sys ids into table */
int
getprocid()
{
	int i;
	struct proc prbuf;
	static int lckinit = 0;

	if(lckinit == 0) {
		procptr = (struct procid *)malloc((unsigned)
			(sizeof (struct procid) * vbuf.v_proc));
		lckinit = 1;
	}
	for(i = 0; i < vbuf.v_proc; i++) {
		readmem((long)(Proc->n_value+i*sizeof prbuf),1,
			-1,(char *)&prbuf,sizeof prbuf,"proc table");
		procptr[i].epid = prbuf.p_epid;
		procptr[i].sysid = prbuf.p_sysid;
	}
}

/* find process with same id and sys id */
int
findproc(pid,sysid)
short pid,sysid;
{
	int slot;

	for (slot = 0; slot < vbuf.v_proc; slot++) 
		if ((procptr[slot].epid == pid)
		    && (procptr[slot].sysid == sysid))
			break;
	if (slot == vbuf.v_proc)
		slot = -1;
	return(slot);
}

/* get arguments for lck function */
int
getlcks()
{
	int slot = -1;
	int phys = 0;
	int all = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int c;
	struct flckinfo infobuf;

	if(!Flckinfo)
		if(!(Flckinfo = symsrch("flckinfo")))
			error("flckinfo not found in symbol table\n");
	if(!Sleeplcks)
		if(!(Sleeplcks = symsrch("sleeplcks")))
			error("sleeplcks not found in symbol table\n");
	if(!Frlock)
		if(!(Frlock = symsrch("frlock")))
			error("frlock not found in symbol table\n");
	if(!Flox)
		if(!(Flox = symsrch("flox")))
			error("filock not found in symbol table\n");
	optind = 1;
	while((c = getopt(argcnt,args,"epw:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			case 'e' :	all = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}
	getprocid();
	readmem((long)Flckinfo->n_value,1,-1,(char *)&infobuf,
		sizeof infobuf,"flckinfo table");
	if(all || args[optind]) {
		fprintf(fp,"LOCK TABLE SIZE = %d\n",infobuf.recs);
		fprintf(fp,"SLOT TYP W    START       LEN     EPID SYSID   STAT   PREV NEXT\n");
		if(args[optind]) {
			do {
				getargs((int)infobuf.recs,&arg1,&arg2);
				if(arg1 == -1) 
					continue;
				if(arg2 != -1)
					for(slot = arg1; slot <= arg2; slot++)
						prlcks(slot,phys,addr,(int)infobuf.recs);
				else {
					if(arg1 < (int)infobuf.recs)
						slot = arg1;
					else addr = arg1;
					prlcks(slot,phys,addr,(int)infobuf.recs);
				}
				slot = addr = arg1 = arg2 = -1;
			}while(args[++optind]);
		}
		else for(slot = 0; slot < (int)infobuf.recs; slot++)
			prlcks(slot,phys,addr,(int)infobuf.recs);
	}
	else prilcks((int)infobuf.recs);
}

/* print lock information relative to inodes (default) */
int
prilcks(size)
int size;
{
	struct	filock	*actptr,*slptr,*freeptr,fibuf;
	struct flckinfo info;
	struct	inode	ibuf;
	int active = 0;
	int free = 0;
	int sleep = 0;
	int slot,next,prev;


	fprintf(fp,"ACTIVE LOCKS:\n");
	fprintf(fp,"INO TYP W    START       LEN    PROC  EPID SYSID WAIT PREV NEXT\n");
	for(slot = 0; slot < vbuf.v_inode; slot++) {
		readmem((long)(Inode->n_value+slot*sizeof ibuf),1,-1,
			(char *)&ibuf,sizeof ibuf,"inode table");
		actptr = (struct filock *)ibuf.i_filocks;
		while(actptr) {
			readmem((long)actptr,1,-1,(char *)&fibuf,
				sizeof fibuf,"filock information");
			++active;
			fprintf(fp,"%3d ",slot);
			if(fibuf.set.l_type == F_RDLCK) 
				fprintf(fp," R  ");
			else if(fibuf.set.l_type == F_WRLCK) 
				fprintf(fp," W  ");
			else fprintf(fp," ?  ");
			fprintf(fp,"%d %10ld %10ld %4d %5hd  %4x %4d",
				fibuf.set.l_whence,
				fibuf.set.l_start,
				fibuf.set.l_len,
				findproc(fibuf.set.l_pid,fibuf.set.l_sysid),
				fibuf.set.l_pid,
				fibuf.set.l_sysid,
				fibuf.stat.wakeflg);
			prev = ((long)fibuf.prev - Flox->n_value)/
				(sizeof (struct filock));
			if((prev < 0 ) || (prev >= size))
				fprintf(fp,"   - ");
			else fprintf(fp," %4d",prev);
			next = ((long)fibuf.next - Flox->n_value)/
				(sizeof (struct filock));
			if((next < 0 ) || (next >= size))
				fprintf(fp,"   - \n");
			else fprintf(fp," %4d\n",next);
			actptr = fibuf.next;

		}
	}

	fprintf(fp,"\nSLEEP  LOCKS:\n");
	fprintf(fp,"TYP W    START       LEN    LPRC  EPID SYSID BPRC  EPID SYSID PREV NEXT\n");
	readmem((long)Sleeplcks->n_value,1,-1,(char *)&slptr,
		sizeof slptr,"sleep lock information table");
	while (slptr) {
		readmem((long)slptr,1,-1,(char *)&fibuf,sizeof fibuf,
			"sleep lock information table slot");
		++sleep;
		if(fibuf.set.l_type == F_RDLCK) 
			fprintf(fp," R  ");
		else if(fibuf.set.l_type == F_WRLCK) 
			fprintf(fp," W  ");
		else fprintf(fp," ?  ");
		fprintf(fp,"%1d %10ld %10ld %4d %5hd  %4x %4d %5hd  %4x",
			fibuf.set.l_whence,
			fibuf.set.l_start,
			fibuf.set.l_len,
			findproc(fibuf.set.l_pid,fibuf.set.l_sysid),
			fibuf.set.l_pid,
			fibuf.set.l_sysid,
			findproc(fibuf.stat.blk.pid,fibuf.stat.blk.sysid),
			fibuf.stat.blk.pid,
			fibuf.stat.blk.sysid);
		prev = ((long)fibuf.prev - Flox->n_value)/
			(sizeof (struct filock));
		if((prev < 0 ) || (prev >= size))
			fprintf(fp,"   - ");
		else fprintf(fp," %4d",prev);
		next = ((long)fibuf.next - Flox->n_value)/
			(sizeof (struct filock));
		if((next < 0 ) || (next >= size))
			fprintf(fp,"   - \n");
		else fprintf(fp," %4d\n",next);
		slptr = fibuf.next;
	}

	readmem((long)Frlock->n_value,1,-1,(char *)&freeptr,
		sizeof (long),"free lock information table");
	while (freeptr) {
		readmem((long)freeptr,1,-1,(char *)&fibuf,
			sizeof fibuf,"filock information");
		++free;
		freeptr = fibuf.next;
	}

	readmem((long)Flckinfo->n_value,1,-1,(char *)&info,
		sizeof info,"file lock information table");
	fprintf(fp,"\nCONFIGURED RECORD LOCKS:\n");
	fprintf(fp,"RECS  RECCNT  RECOVF  RECTOT\n");
	fprintf(fp,"%4d   %4d    %4d    %4d\n",
		info.recs,
		info.reccnt,
		info.recovf,
		info.rectot);
	fprintf(fp,"\nACTUAL RECORD LOCKS:\n");
	fprintf(fp,"TOTAL  ACTIVE  FREE  SLEEP\n");
	fprintf(fp," %4d   %4d   %4d   %4d\n",
		active+sleep+free,
		active,
		free,
		sleep);
}    

/* print linked list of locks */
int
prlcks(slot,phys,addr,size)
int slot,phys,size;
long addr;
{
	struct filock fibuf;
	int prev,next;

	readbuf(addr,(long)(Flox->n_value+slot*sizeof fibuf),phys,-1,
		(char *)&fibuf,sizeof fibuf,"lock table");
	if(addr > -1)
		slot = getslot(addr,(long)Flox->n_value,sizeof fibuf,phys);
	fprintf(fp,"%4d %c%c%c",
		slot,
	(fibuf.set.l_type == F_RDLCK) ? 'r' : ' ',
	(fibuf.set.l_type == F_WRLCK) ? 'w' : ' ',
	(fibuf.set.l_type == F_UNLCK) ? 'u' : ' ');
	fprintf(fp," %1d %10ld %10ld %5hd  %4x %8x",
		fibuf.set.l_whence,
		fibuf.set.l_start,
		fibuf.set.l_len,
		fibuf.set.l_pid,
		fibuf.set.l_sysid,
		fibuf.stat.wakeflg);
	prev = ((long)fibuf.prev - Flox->n_value)/(sizeof (struct filock));
	if((prev < 0 ) || (prev >= size))
		fprintf(fp,"   - ");
	else fprintf(fp," %4d",prev);
	next = ((long)fibuf.next - Flox->n_value)/(sizeof (struct filock));
	if((next < 0 ) || (next >= size))
		fprintf(fp,"   - \n");
	else fprintf(fp," %4d\n",next);
}
