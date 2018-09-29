/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:stream.c	1.15"
/*
 * This file contains code for the crash functions:  stream, queue, mblock,
 * mbfree, dblock, dbfree, strstat, linkblk, dballoc, qrun.
 */

#include "sys/param.h"
#include "a.out.h"
#include "stdio.h"
#include "sys/sysmacros.h"
#include "sys/types.h"
#include "sys/fs/s5dir.h"
#include "sys/var.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/inode.h"
#include "sys/mount.h"
#include "sys/poll.h"
#include "sys/stream.h"
#include "sys/stropts.h"
#include "crash.h"

static struct syment *Dblock, *Qhead, *Mbfree, *Dbfree,
	*Linkblk, *Nmuxlink, *Dballoc;		/* namelist symbol pointers */
struct syment *Queue,*Mblock;
extern struct syment *Inode, *Streams, *Proc;
int ndblock = 0;	 /* number of data blocks */
int nmblock = 0;	 /* number of message blocks */


int
blockinit()
{
	/* Initialize streams data block and message block counts - these  */
	/* formulas should match those in space.h                          */

	if((ndblock == 0) || (nmblock == 0)) {
		ndblock = vbuf.v_nblk4096 + vbuf.v_nblk2048 + vbuf.v_nblk1024 +
			vbuf.v_nblk512 + vbuf.v_nblk256 + vbuf.v_nblk128 +
			vbuf.v_nblk64 + vbuf.v_nblk16 + vbuf.v_nblk4;   
		nmblock = ndblock + ndblock/4;
	}
}


/* get arguments for stream function */
int
getstream()
{
	int slot = -1;
	int full = 0;
	int all = 0;
	int phys = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int c;
	char *heading = "SLOT  WRQ IOCB INODE  PGRP    IOCID   IOCWT WOFF ERR FLAGS\n";

	if(!Streams)
		if(!(Streams = symsrch("streams")))
			error("streams not found in symbol table\n");
	if(!Queue)
		if(!(Queue = symsrch("queue")))
			error("queue not found in symbol table\n");
	if(!Mblock)
		if(!(Mblock = symsrch("mblock")))
			error("mblock not found in symbol table\n");
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
	fprintf(fp,"STREAM TABLE SIZE = %d\n",vbuf.v_nstream);
	if(!full)
		fprintf(fp,"%s",heading);
	if(args[optind]) {
		all = 1;
		do {
			getargs(vbuf.v_nstream,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prstream(all,full,slot,phys,addr,heading);
			else {
				if(arg1 < vbuf.v_nstream)
					slot = arg1;
				else addr = arg1;
				prstream(all,full,slot,phys,addr,heading);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < vbuf.v_nstream; slot++)
		prstream(all,full,slot,phys,addr,heading);
}

/* print streams table */
int
prstream(all,full,slot,phys,addr,heading)
int all,full,slot,phys;
long addr;
char *heading;
{
	struct stdata strm;
	struct strevent evbuf;
	struct strevent *next;
	int ioc_slot; 

	readbuf(addr,(long)(Streams->n_value+slot*sizeof strm),phys,-1,
		(char *)&strm, sizeof strm,"streams table slot");
	if (!strm.sd_wrq && !all) 
		return;
	if(addr > -1) 
		slot = getslot(addr,(long)Streams->n_value,sizeof strm,phys);
	if(full)
		fprintf(fp,"%s",heading);
	fprintf(fp,"%4d ",slot);
	fprintf(fp,"%4d ",((long)strm.sd_wrq - Queue->n_value)/
		sizeof(struct queue));
	ioc_slot = ((long)strm.sd_iocblk - Mblock->n_value)/
		(sizeof(struct msgb));
	if ( (ioc_slot>=0)&&(ioc_slot<nmblock) )
		fprintf(fp,"%4d ",ioc_slot);
	else fprintf(fp,"   - ");
	fprintf(fp,"%5d ",((long)strm.sd_inode - Inode->n_value)/
		sizeof(struct inode));
	fprintf(fp,"%5d %10d %5d %4d %3o ", strm.sd_pgrp, strm.sd_iocid,
		strm.sd_iocwait, strm.sd_wroff, strm.sd_error);
	fprintf(fp,"%s%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		((strm.sd_flag & IOCWAIT) ? "iocw " : ""),
		((strm.sd_flag & RSLEEP) ? "rslp " : ""),
		((strm.sd_flag & WSLEEP) ? "wslp " : ""),
		((strm.sd_flag & STRPRI) ? "pri " : ""),
		((strm.sd_flag & STRHUP) ? "hup " : ""),
		((strm.sd_flag & STWOPEN) ? "stwo " : ""),
		((strm.sd_flag & STPLEX) ? "plex " : ""),
		((strm.sd_flag & CTTYFLG) ? "ctty " : ""),
		((strm.sd_flag & RMSGDIS) ? "mdis " : ""),
		((strm.sd_flag & RMSGNODIS) ? "mnds " : ""),
		((strm.sd_flag & STRERR) ? "err " : ""),
		((strm.sd_flag & STRTIME) ? "sttm " : ""),
		((strm.sd_flag & STR2TIME) ? "s2tm " : ""),
		((strm.sd_flag & STR3TIME) ? "s3tm " : ""));
	if(full) {
		fprintf(fp,"\t STRTAB  CNT\n");
		fprintf(fp,"\t%8x  %2d\n",
			strm.sd_strtab,
			strm.sd_pushcnt);
		fprintf(fp,"\tSIGFLAGS:  %s%s%s%s\n",
			((strm.sd_sigflags & S_INPUT) ? " input" : ""),
			((strm.sd_sigflags & S_HIPRI) ? " hipri" : ""),
			((strm.sd_sigflags & S_OUTPUT) ? " output" : ""),
			((strm.sd_sigflags & S_MSG) ? " msg" : ""));
		fprintf(fp,"\tSIGLIST:\n");
		next = strm.sd_siglist;
		while(next) {
			readmem((long)next,1,-1,(char *)&evbuf,
				sizeof evbuf,"stream event buffer");
			fprintf(fp,"\t\tPROC:  %3d   %s%s%s%s\n",
				((long)evbuf.se_procp-Proc->n_value)/
					sizeof(struct proc),
				((evbuf.se_events & S_INPUT) ? " input" : ""),
				((evbuf.se_events & S_HIPRI) ? " hipri" : ""),
				((evbuf.se_events & S_OUTPUT) ? " output" : ""),
				((evbuf.se_events & S_MSG) ? " msg" : ""));
			next = evbuf.se_next;	
		}
		fprintf(fp,"\tPOLLFLAGS:  %s%s%s\n",
			((strm.sd_pollflags & POLLIN) ? " in" : ""),
			((strm.sd_pollflags & POLLPRI) ? " pri" : ""),
			((strm.sd_pollflags & POLLOUT) ? " out" : ""));
		fprintf(fp,"\tPOLLIST:\n");
		next = strm.sd_pollist;
		while(next) {
			readmem((long)next,1,-1,(char *)&evbuf,
				sizeof evbuf,"stream event buffer");
			fprintf(fp,"\t\tPROC:  %3d   %s%s%s\n",
				((long)evbuf.se_procp-Proc->n_value)/
					sizeof(struct proc),
				((evbuf.se_events & POLLIN) ? " in" : ""),
				((evbuf.se_events & POLLPRI) ? " pri" : ""),
				((evbuf.se_events & POLLOUT) ? " out" : ""));
			next = evbuf.se_next;	
		}
		fprintf(fp,"\n");
	}
}

/* get arguments for queue function */
int
getqueue()
{
	int slot = -1;
	int all = 0;
	int phys = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int c;

	if(!Queue)
		if(!(Queue = symsrch("queue")))
			error("queue not found in symbol table\n");
	if(!Mblock)
		if(!(Mblock = symsrch("mblock")))
			error("mblock not found in symbol table\n");
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
	fprintf(fp,"QUEUE TABLE SIZE = %d\n",vbuf.v_nqueue);
	fprintf(fp,"SLOT   INFO   NEXT LINK   PTR     CNT HEAD TAIL MINP MAXP HIWT LOWT FLAGS\n");
	if(args[optind]) {
		all = 1;
		do {
			getargs(vbuf.v_nqueue,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prqueue(all,slot,phys,addr);
			else {
				if(arg1 < vbuf.v_nqueue)
					slot = arg1;
				else addr = arg1;
				prqueue(all,slot,phys,addr);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < vbuf.v_nqueue; slot++)
		prqueue(all,slot,phys,addr);
}

/* print queue table */
int
prqueue(all,slot,phys,addr)
int all,slot,phys;
long addr;
{
	queue_t que;
	mblk_t *m;
	int qn, ql;

	readbuf(addr,(long)(Queue->n_value+slot*sizeof que),phys,-1,
		(char *)&que, sizeof que,"queue slot");
	if (!(que.q_flag & QUSE) && !all)
		return;
	if(addr > -1) 
		slot = getslot(addr,(long)Queue->n_value,sizeof que,phys);
        fprintf(fp,"%4d ",slot);
	fprintf(fp,"%8x ",que.q_qinfo);
	qn = ((long)que.q_next - Queue->n_value)/(sizeof(struct queue));
	ql = ((long)que.q_link - Queue->n_value)/(sizeof(struct queue));
	if ((qn >= 0) && (qn < vbuf.v_nqueue))
		fprintf(fp,"%4d ",qn);
	else fprintf(fp,"   - ");
	if ((ql >= 0) && (ql < vbuf.v_nqueue))
		fprintf(fp,"%4d ",ql);
	else fprintf(fp,"   - ");
	fprintf(fp,"%8x ",que.q_ptr);
	fprintf(fp,"%4d ",que.q_count);
	m = que.q_first;
	if (m) {
		fprintf(fp,"%4d ",
			((long)m - Mblock->n_value)/(sizeof(struct msgb)));
	}
	else fprintf(fp,"   - ");
	m = que.q_last;
	if (m) {
		fprintf(fp,"%4d ",
			((long)m - Mblock->n_value)/(sizeof(struct msgb)));
	}
	else fprintf(fp,"   - ");
	fprintf(fp,"%4d %4d %4d %4d ",
		que.q_minpsz,
		que.q_maxpsz,
		que.q_hiwat,
		que.q_lowat);
	fprintf(fp,"%s%s%s%s%s%s%s\n",
		((que.q_flag & QENAB) ? "en " : ""),
		((que.q_flag & QWANTR) ? "wr " : ""),
		((que.q_flag & QWANTW) ? "ww " : ""),
		((que.q_flag & QFULL) ? "fl " : ""),
		((que.q_flag & QREADR) ? "rr " : ""),
		((que.q_flag & QUSE) ? "us " : ""),
		((que.q_flag & QNOENB) ? "ne " : ""));
	
}

/* get arguments for mblock function */
int
getmess()
{
	int slot = -1;
	int all = 0;
	int phys = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int c;

	if(!Mblock)
		if(!(Mblock = symsrch("mblock")))
			error("mblock not found in symbol table\n");
	if(!Dblock)
		if(!(Dblock = symsrch("dblock")))
			error("dblock not found in symbol table\n");
	blockinit();
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
	fprintf(fp,"MESSAGE BLOCK TABLE SIZE = %d\n",nmblock);
	fprintf(fp,"SLOT NEXT CONT PREV   RPTR     WPTR   DATAB\n");
	if(args[optind]) {
		all = 1;
		do {
			getargs(nmblock,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prmess(all,slot,phys,addr);
			else {
				if(arg1 < nmblock)
					slot = arg1;
				else addr = arg1;
				prmess(all,slot,phys,addr);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < nmblock; slot++)
		prmess(all,slot,phys,addr);
}

/* print mblock table */
int
prmess(all,slot,phys,addr)
int all,slot,phys;
long addr;
{
	mblk_t mblk;
	int mnext, mcont, datab, mprev;

	readbuf(addr,(long)(Mblock->n_value+slot*sizeof mblk),phys,-1,
		(char *)&mblk,sizeof mblk,"message block");
	if (!mblk.b_datap && !all) 
		return(-1);
	if(addr > -1) 
		slot = getslot(addr,(long)Mblock->n_value,sizeof mblk,phys);
        fprintf(fp,"%4d ",slot);
	mnext = ((long)mblk.b_next - Mblock->n_value)/sizeof(struct msgb);
	mcont = ((long)mblk.b_cont - Mblock->n_value)/sizeof(struct msgb);
	mprev = ((long)mblk.b_prev - Mblock->n_value)/sizeof(struct msgb);
	datab = ((long)mblk.b_datap - Dblock->n_value)/sizeof(struct datab);
	if ((mnext >= 0) && (mnext < nmblock))
		fprintf(fp,"%4d ",mnext);
	else fprintf(fp,"   - ");
	if ((mcont >= 0) && (mcont < nmblock))
		fprintf(fp,"%4d ",mcont);
	else fprintf(fp,"   - ");
	if ((mprev >= 0) && (mprev < nmblock))
		fprintf(fp,"%4d ",mprev);
	else fprintf(fp,"   - ");
	fprintf(fp,"%8x %8x ",
		mblk.b_rptr,
		mblk.b_wptr);
	if ((datab >= 0) && (datab < ndblock))
		fprintf(fp,"%4d\n",datab);
	else fprintf(fp,"   -\n");
	return(mnext);
	
}

/* get arguments for mbfree function */
int
getmbfree()
{
	int c;

	if(!Mbfree)
		if(!(Mbfree = symsrch("mbfreelist")))
			error("mbfreelist not found in symbol table\n");
	if(!Mblock)
		if(!(Mblock = symsrch("mblock")))
			error("mblock not found in symbol table\n");
	blockinit();
	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	fprintf(fp,"SLOT NEXT CONT PREV   RPTR     WPTR   DATAB\n");
	if(args[optind]) 
		longjmp(syn,0);
	prmbfree();
}

/* print mblock free list */
int
prmbfree()
{
	mblk_t *m;
	int  mnext;

	readmem((long)Mbfree->n_value,1,-1,(char *)&m,
			sizeof m,"mbfreelist");
	mnext = ((long)m - Mblock->n_value)/sizeof(struct msgb);
	while ((mnext >=0) && (mnext < nmblock))
		mnext = prmess(1,mnext,0,-1);
}


/* get arguments for dblock function */
int
getdblk()
{
	int slot = -1;
	int all = 0;
	int phys = 0;
	int class = 0;
	long addr = -1;
	long arg2 = -1;
	long arg1 = -1;
	int c;
	int klass;

	if(!Dblock)
		if(!(Dblock = symsrch("dblock")))
			error("dblock not found in symbol table\n");
	blockinit();
	optind = 1;
	while((c = getopt(argcnt,args,"epcw:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'p' :	phys = 1;
					break;
			case 'c' :	class = 1;
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	fprintf(fp,"DATA BLOCK TABLE SIZE = %d\n",ndblock);
	fprintf(fp,"SLOT CLASS SIZE  REF   TYPE     BASE     LIMIT  FREEP\n");
	if(args[optind]) {
		if(class) 
			do {
				if((klass = (int)strcon(args[optind++],'d'))
					== -1)
					continue;
				prclass(klass);
			}while(args[optind]);
		else {
			all = 1;
			do {
				getargs(ndblock,&arg1,&arg2);
				if(arg1 == -1) 
					continue;
				if(arg2 != -1)
					for(slot = arg1; slot <= arg2; slot++)
						prdblk(all,slot,phys,addr);
				else {
					if(arg1 < ndblock)
						slot = arg1;
					else addr = arg1;
					prdblk(all,slot,phys,addr);
				}
				slot = addr = arg1 = arg2 = -1;
			}while(args[++optind]);
		}
	}
	else for(slot = 0; slot < ndblock; slot++)
		prdblk(all,slot,phys,addr);
}


/* print dblock table */
int
prdblk(all,slot,phys,addr)
int all,slot,phys;
long addr;
{
	dblk_t dblk;
	int dfree;
	static int lastcls;

	readbuf(addr,(long)(Dblock->n_value+slot*sizeof dblk),phys,-1,
		(char *)&dblk, sizeof dblk,"data block");
	if (!dblk.db_ref && !all)
		return(-1);
	if(addr > -1) 
		slot = getslot(addr,(long)Dblock->n_value,sizeof dblk,phys);
	if (dblk.db_class != lastcls) {
		fprintf(fp,"\n");
		lastcls=dblk.db_class;
	}
	fprintf(fp,"%4d %5d ",
		slot,
		dblk.db_class);
	switch (dblk.db_class) {
		case 0: fprintf(fp,"   4 "); break;
		case 1: fprintf(fp,"  16 "); break;
		case 2: fprintf(fp,"  64 "); break;
		case 3: fprintf(fp," 128 "); break;
		case 4: fprintf(fp," 256 "); break;
		case 5: fprintf(fp," 512 "); break;
		case 6: fprintf(fp,"1024 "); break;
		case 7: fprintf(fp,"2048 "); break;
		case 8: fprintf(fp,"4096 "); break;
		default: fprintf(fp,"   - ");
	}
	fprintf(fp,"%4d ", dblk.db_ref); 
	switch (dblk.db_type) {
		case M_DATA: fprintf(fp,"data     "); break;
		case M_PROTO: fprintf(fp,"proto    "); break;
		case M_BREAK: fprintf(fp,"break    "); break;
		case M_PASSFP: fprintf(fp,"passfs   "); break;
		case M_SIG: fprintf(fp,"sig      "); break;
		case M_DELAY: fprintf(fp,"delay    "); break;
		case M_CTL: fprintf(fp,"ctl      "); break;
		case M_IOCTL: fprintf(fp,"ioctl    "); break;
		case M_SETOPTS: fprintf(fp,"setopts  "); break;
		case M_IOCACK: fprintf(fp,"iocack   "); break;
		case M_IOCNAK: fprintf(fp,"iocnak   "); break;
		case M_PCPROTO: fprintf(fp,"pcproto  "); break;
		case M_PCSIG: fprintf(fp,"pcsig    "); break;
		case M_FLUSH: fprintf(fp,"flush    "); break;
		case M_STOP: fprintf(fp,"stop     "); break;
		case M_START: fprintf(fp,"start    "); break;
		case M_HANGUP: fprintf(fp,"hangup   "); break;
		case M_ERROR: fprintf(fp,"error    "); break;
		default: fprintf(fp,"       - ");
	}
	fprintf(fp,"%8x %8x ",
		dblk.db_base,
		dblk.db_lim);
	dfree = ((long)dblk.db_freep - Dblock->n_value)/sizeof(struct datab);
	if ((dfree >= 0) && (dfree < ndblock))
		fprintf(fp," %4d\n",dfree);
	else fprintf(fp,"   -\n");
	return(dfree);
	
}

/* get class for dblock */
int
prclass(class)
int class;
{
	int i,n, *p;
	int clasize[NCLASS];

	if ((class < 0) || (class > NCLASS-1))
		error("%d is out of range\n",class);
	p = &(vbuf.v_nblk4096);
	for (i=NCLASS-1; i>=0; i--) clasize[i] = *p++;  /* load size of each block class */
	n=0;
	for (i=NCLASS-1; i>class; i--) n+=clasize[i];	/* compute slot of first block */
	for (i=0; i<clasize[class]; i++)
		prdblk(0,n++,0,-1);
}

/* get arguments for dbfree function */
int
getdbfree()
{
	int c;
	int class;
	int klass;

	if(!Dbfree)
		if(!(Dbfree = symsrch("dbfreelist")))
			error("dbfreelist not found in symbol table\n");
	if(!Dblock)
		if(!(Dblock = symsrch("dblock")))
			error("dblock not found in symbol table\n");
	blockinit();
	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	fprintf(fp,"SLOT CLASS SIZE  REF   TYPE     BASE     LIMIT  FREEP\n");
	if(args[optind]) 
		do {
			if((klass = (int)strcon(args[optind++],'d')) == -1)
				continue;
			prdbfree(klass);
		} while(args[optind]);
	else for(class = 0; class < NCLASS; class++)
		prdbfree(class);
}

/* print dblock free list */
int
prdbfree(class)
int class;
{
	dblk_t *d;
	int  dnext;

	if ((class < 0) || (class > NCLASS-1))
		error("%d is out of class range\n",class);
	readmem((long)(Dbfree->n_value+class*sizeof d),1,-1,(char *)&d,
		sizeof d,"dbfreelist");
	dnext = ((long)d - Dblock->n_value)/sizeof(struct datab);
	while ((dnext >=0) && (dnext < ndblock))
		dnext = prdblk(1,dnext,0,-1);
}

/* get arguments for qrun function */
int
getqrun()
{
	int c;

	if(!Queue)
		if(!(Queue = symsrch("queue")))
			error("queue not found in symbol table\n");
	if(!Qhead)
		if(!(Qhead = symsrch("qhead")))
			error("qhead not found in symbol table\n");
	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	prqrun();
}

/* print qrun information */
int
prqrun()
{
	queue_t que, *q;
	int  ql;

	readmem((long)Qhead->n_value,1,-1,(char *)&q,
		sizeof q,"qhead");
	fprintf(fp,"Queue slots scheduled for service: ");
	while (q) {
		ql = ((long)q - Queue->n_value)/(sizeof(struct queue));
		fprintf(fp,"%4d ",ql);
		readmem((long)q,1,-1,(char *)&que,
			sizeof que,"scanning queue list");
		q = que.q_link;
	}
	fprintf(fp,"\n");
}


/* initialization for namelist symbols */
int
streaminit()
{
	static int strinit = 0;

	if(strinit)
		return;
	if(!Queue)
		if(!(Queue = symsrch("queue")))
			error("queue not found in symbol table\n");
	if(!Streams)
		if(!(Streams = symsrch("streams")))
			error("streams not found in symbol table\n");
	if(!Mblock)
		if(!(Mblock = symsrch("mblock")))
			error("mblock not found in symbol table\n");
	if(!Dblock)
		if(!(Dblock = symsrch("dblock")))
			error("dblock not found in symbol table\n");
	if(!Qhead)
		if(!(Qhead = symsrch("qhead")))
			error("qhead not found in symbol table\n");
	if(!Dbfree)
		if(!(Dbfree = symsrch("dbfreelist")))
			error("dbfreelist not found in symbol table\n");
	if(!Mbfree)
		if(!(Mbfree = symsrch("mbfreelist")))
			error("mbfreelist not found in symbol table\n");
	blockinit();

	strinit = 1;
}


/* get arguments for strstat function */
int
getstrstat()
{
	int c;

	streaminit();
	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	prstrstat();
}

/* print stream statistics */
int
prstrstat()
{
	queue_t que, *q;
	struct stdata str;
	mblk_t mes, *m;
	dblk_t dbk, *d;
	int qusecnt, susecnt, mfreecnt, musecnt,
	    dfreecnt, dusecnt, dfc[NCLASS], duc[NCLASS], dcc[NCLASS], qruncnt;
	int i,j, *p;

	qusecnt = susecnt = mfreecnt = 0;
	musecnt = dfreecnt = dusecnt = qruncnt = 0;
	for (i=0; i<NCLASS; i++) dfc[i] = duc[i] =0;
	p = &(vbuf.v_nblk4096);
	for (i=NCLASS-1; i>=0; i--) dcc[i] = *p++;

	fprintf(fp,"ITEM             CONFIGURED   ALLOCATED     FREE\n");

	readmem((long)Qhead->n_value,1,-1,(char *)&q,
		sizeof q,"qhead");
	while (q) {
		qruncnt++;
		readmem((long)q,1,-1,(char *)&que,sizeof que,"queue run list");
		q = que.q_link;
	}
	
	seekmem((long)Queue->n_value,1,-1);
	for (i=0; i<vbuf.v_nqueue; i++) {
		if (read(mem, (char *)&que, sizeof que) != sizeof que) 
			error(fp,"read error in queue table\n");
		if (que.q_flag & QUSE) qusecnt++;
	}

	seekmem((long)Streams->n_value,1,-1);
	for (i=0; i<vbuf.v_nstream; i++) {
		if (read(mem, &str, sizeof str) != sizeof str) 
			error(fp,"read error in stream table\n");
		if (str.sd_wrq) susecnt++;
	}

	seekmem((long)Mbfree->n_value,1,-1);
	if (read(mem, &m, sizeof m) != sizeof m) 
		error(fp,"read error for mbfreelist\n");
	while (m) {
		mfreecnt++;
		readmem((long)m,1,-1,(char *)&mes,sizeof mes,"message list");
		m = mes.b_next;
	}

	seekmem((long)Mblock->n_value,1,-1);
	for (i=0; i<nmblock; i++) {
		if (read(mem, &mes, sizeof mes) != sizeof mes) 
			error(fp,"Read error for message table\n");
		if (mes.b_datap) musecnt++;
	}

	for (j=0; j<NCLASS; j++){
		readmem((long)(Dbfree->n_value + j*(sizeof d)),1,-1,
			(char *)&d,sizeof d,"data block table");
		while (d) {
			dfc[j]++;
			dfreecnt++;
			readmem((long)d,1,-1,(char *)&dbk,sizeof dbk,
				"data block table");
			d = dbk.db_freep;
		}
	}

	seekmem((long)Dblock->n_value,1,-1);
	for (i=0; i<ndblock; i++) {
		if (read(mem, &dbk, sizeof dbk) != sizeof dbk) 
			error(fp,"read error in data block table\n");
		if (dbk.db_ref) {
			dusecnt++;
			duc[dbk.db_class]++;
		}
	}

	fprintf(fp,"streams             %4d         %4d       %4d\n",
		vbuf.v_nstream, susecnt, vbuf.v_nstream - susecnt);
	fprintf(fp,"queues              %4d         %4d       %4d\n",
		vbuf.v_nqueue, qusecnt, vbuf.v_nqueue - qusecnt);
	fprintf(fp,"message blocks      %4d         %4d       %4d\n",
		nmblock, musecnt, mfreecnt);
	fprintf(fp,"data block totals   %4d         %4d       %4d\n",
		ndblock, dusecnt, dfreecnt);
	for (i=0; i<NCLASS; i++) 
		fprintf(fp,"data block class%2d  %4d         %4d       %4d\n",
			i, dcc[i], duc[i], dfc[i]);
	fprintf(fp,"\nCount of scheduled queues:%4d\n", qruncnt);


}


/* get arguments for linkblk function */
int
getlinkblk()
{
	int c;
	int slot = -1;
	int all = 0;
	int phys = 0;
	long addr = -1;
	long arg2 = -1;
	long arg1 = -1;
	int nmuxlink;

	if(!Linkblk)
		if(!(Linkblk = symsrch("linkblk")))
			error("linkblk not found in symbol table\n");
	if(!Nmuxlink)
		if(!(Nmuxlink = symsrch("nmuxlink")))
			error("nmuxlink not found in symbol table\n");
	if(!Queue)
		if(!(Queue = symsrch("queue")))
			error("queue not found in symbol table\n");
	optind = 1;
	while((c = getopt(argcnt,args,"epw:")) !=EOF) {
		switch(c) {
			case 'e' :	all = 1;
					break;
			case 'p' :	phys = 1;
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	readmem((long)Nmuxlink->n_value,1,-1,(char *)&nmuxlink,
		sizeof nmuxlink,"linkblk table size");
	fprintf(fp,"LINKBLK TABLE SIZE = %d\n",nmuxlink);
	fprintf(fp,"SLOT   QTOP     QBOT   INDEX\n");
	if(args[optind]) {
		all = 1;
		do {
			getargs(nmuxlink,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prlinkblk(all,slot,phys,addr);
			else {
				if(arg1 < nmuxlink)
					slot = arg1;
				else addr = arg1;
				prlinkblk(all,slot,phys,addr);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < nmuxlink; slot++)
		prlinkblk(all,slot,phys,addr);	
}

/* print linkblk table */
int
prlinkblk(all,slot,phys,addr)
int all,slot,phys;
long addr;
{
	struct linkblk linkbuf;

	readbuf(addr,(long)(Linkblk->n_value+slot*sizeof linkbuf),phys,-1,
		(char *)&linkbuf,sizeof linkbuf,"linkblk table");
	if(!linkbuf.l_qtop && !all)
		return;
	if(addr > -1)
		slot = getslot(addr,(long)Linkblk->n_value,sizeof linkbuf,phys);
	fprintf(fp,"%4d", slot);
		slot = ((long)linkbuf.l_qtop-Queue->n_value)/
			sizeof(struct queue);
		if((slot >= 0) && (slot < vbuf.v_nqueue))
			fprintf(fp,"    %5d",slot);
		else fprintf(fp," %8x",linkbuf.l_qtop);
		slot = ((long)linkbuf.l_qbot-Queue->n_value)/
			sizeof(struct queue);
		if((slot >= 0) && (slot < vbuf.v_nqueue))
			fprintf(fp,"    %5d",slot);
		else fprintf(fp," %8x",linkbuf.l_qbot);
		fprintf(fp,"   %3d\n",linkbuf.l_index);
}


/* get arguments for dballoc function */
int
getdballoc()
{
	int c,class;

	if(!Dballoc)
		if(!(Dballoc = symsrch("dballoc")))
			error("dballoc not found in symbol table\n");
	optind = 1;
	while((c = getopt(argcnt,args,"w:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		do {
			if((class = strcon(args[optind++],'d')) == -1)
				continue;
			if((class >= 0) && (class < NCLASS))
				prdballoc(class);
			else {
				fprintf(fp,"%d is out of class range\n",
					class);
				continue;
			}
		} while(args[optind]);
	}
	else for(class = 0; class < NCLASS; class++)
		prdballoc(class);
}

/* print dballoc table */
int
prdballoc(class)
int class;
{
	struct dbalcst dbalbuf;
	struct strevent *next;

	readmem((long)(Dballoc->n_value+class*sizeof dbalbuf),1,-1,
		(char *)&dbalbuf,sizeof dbalbuf,"dballoc table");
	fprintf(fp,"CLASS  CNT    LO    MED\n");
	fprintf(fp,"  %d   %5d %5d %5d\n",
		class,
		dbalbuf.dba_cnt,
		dbalbuf.dba_lo,
		dbalbuf.dba_med);
	fprintf(fp,"\tLOP:\n");
	next = dbalbuf.dba_lop;
	while(next)
		prfuncname(next);
	fprintf(fp,"\tMEDP:\n");
	next = dbalbuf.dba_medp;
	while(next)
		prfuncname(next);
	fprintf(fp,"\tHIP:\n");
	next = dbalbuf.dba_hip;
	while(next)
		prfuncname(next);
}

/* print function name for dballoc */
int
prfuncname(addr)
long(addr);
{
	struct syment *sp;
	extern struct syment *findsym();
	extern short N_TEXT;
	extern char *strtbl;
	struct strevent evbuf;
	static char tname[SYMNMLEN+1];
	char *name;
	
	readmem((long)addr,1,-1,(char *)&evbuf,
		sizeof evbuf,"stream event buffer");
	if(!(sp = findsym((unsigned long)evbuf.se_func))) {
		prerrmes("%8x does not match in symbol table\n",evbuf.se_func);
		return;
	}
	if(sp->n_zeroes){
		strncpy(tname,sp->n_name,SYMNMLEN);
		name = tname;
	}
	else name = strtbl + sp->n_offset;
	fprintf(fp,"\t\t%-15s",name);
	fprintf(fp,"(%8x)\n",evbuf.se_arg);
}
