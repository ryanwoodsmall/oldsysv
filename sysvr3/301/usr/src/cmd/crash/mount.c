/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:mount.c	1.14"
/*
 * This file contains code for the crash functions:  mount, srmount.
 */

#include "sys/param.h"
#include "sys/fs/s5param.h"
#include "a.out.h"
#include "stdio.h"
#include "sys/sysmacros.h"
#include "sys/types.h"
#include "sys/fs/s5dir.h"
#include "sys/var.h"
#include "sys/mount.h"
#include "sys/inode.h"
#include "sys/sema.h"
#include "sys/nserve.h"
#include "sys/comm.h"
#include "sys/fs/s5filsys.h"
#include "crash.h"

extern struct syment *Inode, *Mount, *Sndd, *Nsndd;	/* namelist symbol */
static struct syment *Srmount, *Nsrmount;		/* pointers */

/* get arguments for mount function */
int
getmount()
{
	int slot = -1;
	int all = 0;
	int phys = 0;
	long addr = -1;
	int c;
	long arg1 = -1;
	long arg2 = -1;

	if(!Mount)
		if(!(Mount = symsrch("mount")))
			error("mount table not found\n");

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
	fprintf(fp,"MOUNT TABLE SIZE = %d\n",vbuf.v_mount);
	fprintf(fp,"SLOT FS BSZ  MAJ/MIN  BUFPTR  BCOUNT IPTR MPTR RFLG FLAGS RD NAME\n");
	if(args[optind]) {
		all = 1;
		do {
			getargs(vbuf.v_mount,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prmount(all,slot,phys,addr);
			else {
				if(arg1 < vbuf.v_mount)
					slot = arg1;
				else addr = arg1;
				prmount(all,slot,phys,addr);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < vbuf.v_mount; slot++)
		prmount(all,slot,phys,addr);
}

/* print mount table */
int
prmount(all,slot,phys,addr)
int all,slot,phys;
long addr;
{
	struct mount mbuf;
	int nsndd;
	char namebuf[MAXDNAME+1];
	int send = 1;
	extern long lseek();

	if(!Sndd)
		if(!(Sndd = symsrch("sndd")))
			send = 0;
	if(send && !Nsndd)
		if(!(Nsndd = symsrch("nsndd"))) 
			send = 0;
	if(send) {
		if(lseek(mem,vtop((long)Nsndd->n_value,-1),0) == -1)
			send = 0;
		else if (read(mem,(char *)&nsndd,sizeof nsndd) != sizeof nsndd)
			send = 0;
	}
	readbuf(addr,(long)(Mount->n_value+slot*sizeof mbuf),phys,-1,
		(char *)&mbuf,sizeof mbuf,"mount table");
	if(!mbuf.m_flags && !all)
		return ;
	if(addr > -1) 
		slot = getslot(addr,(long)Mount->n_value,sizeof mbuf,phys);
	fprintf(fp,"%4u",slot);
	if(!mbuf.m_rflags)
		fprintf(fp," %2d %4u",
			mbuf.m_fstyp,
			mbuf.m_bsize);
	else fprintf(fp,"        ");
	if(!mbuf.m_rflags)
		fprintf(fp," %3u,%-3u",
			major(mbuf.m_dev),
			minor(mbuf.m_dev));
	else fprintf(fp,"        ");
	fprintf(fp," %8x %6u",
		mbuf.m_bufp,
		mbuf.m_bcount);
	slot = ((long)mbuf.m_inodp - (long)Inode->n_value)/
		sizeof (struct inode);
	if((slot >= 0) && (slot < vbuf.v_inode))
		fprintf(fp," I%3d",slot);
	else if(send) {
		slot = ((long)mbuf.m_up - (long)Sndd->n_value)/
			sizeof (struct sndd);
		if((slot >= 0) && (slot < nsndd))
			fprintf(fp," U%3d",slot);
		else fprintf(fp,"  -  ");
	}
	else fprintf(fp,"  -  ");
	slot = ((long)mbuf.m_mount - (long)Inode->n_value)/
		sizeof (struct inode);
	if((slot >= 0) && (slot < vbuf.v_inode))
		fprintf(fp," I%3d",slot);
	else if(send){
		slot = ((long)mbuf.m_down - (long)Sndd->n_value)/
			sizeof (struct sndd);
		if((slot >= 0) && (slot < nsndd))
			fprintf(fp," D%3d",slot);
		else fprintf(fp,"  -  ");
	}
	else fprintf(fp,"  -  ");
	switch(mbuf.m_rflags) {
		case MLBIN	: fprintf(fp," lbin"); break;
		case MDOTDOT	: fprintf(fp,"  .. "); break;
		default		: fprintf(fp,"   - "); break;
	}
	switch(mbuf.m_flags & 03) {
		case MINUSE	: fprintf(fp," inuse"); break;
		case MINTER	: fprintf(fp," inter"); break;
		default		: fprintf(fp,"   -  "); break;
	}
	fprintf(fp," %s",mbuf.m_flags & MRDONLY ? "ro" : "rw");
	if(mbuf.m_rflags) {
		readmem((long)mbuf.m_name,1,-1,namebuf,MAXDNAME,"name");
		namebuf[MAXDNAME+1] = '\0';
		fprintf(fp," %s",namebuf);
	}
	fprintf(fp,"\n");
}

/* get arguments for srmount function */
int
getsrmount()
{
	int slot = -1;
	int all = 0;
	int phys = 0;
	long addr = -1;
	int c;
	int nsrmount;
	long arg1 = -1;
	long arg2 = -1;

	if(!Nsrmount)
		if(!(Nsrmount = symsrch("nsrmount"))) 
			error("cannot determine size of server mount table\n");
	if(!Srmount)
		if(!(Srmount = symsrch("srmount")))
			error("server mount table not found\n");

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
	readmem((long)Nsrmount->n_value,1,-1,(char *)&nsrmount,
		sizeof nsrmount,"size of server mount table");
	fprintf(fp,"SERVER MOUNT TABLE SIZE = %d\n",nsrmount);
	fprintf(fp,"SLOT SYSID RINO MNDX RCNT DOTDOT BCOUNT SCNT RD FLAGS \n");
	if(args[optind]) {
		all = 1;
		do {
			getargs(nsrmount,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prsrmount(all,slot,phys,addr);
			else {
				if(arg1 < vbuf.v_mount)
					slot = arg1;
				else addr = arg1;
				prsrmount(all,slot,phys,addr);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < nsrmount; slot++)
		prsrmount(all,slot,phys,addr);
}

/* print server mount table */
int
prsrmount(all,slot,phys,addr)
int all,slot,phys;
long addr;
{
	struct srmnt srmntbuf;

	readbuf(addr,(long)(Srmount->n_value+slot*sizeof srmntbuf),phys,-1,
		(char *)&srmntbuf,sizeof srmntbuf,"server mount table");
	if((srmntbuf.sr_flags == MFREE) && !all)
		return;
	if(addr > -1) 
		slot = getslot(addr,(long)Srmount->n_value,sizeof srmntbuf,phys);
	fprintf(fp,"%4d  %4x %4u %4u %4u %6u %6u %4u",
		slot,
		srmntbuf.sr_sysid,
		((unsigned) srmntbuf.sr_rootinode - (unsigned) Inode->n_value) /
		 	sizeof(struct inode),
		srmntbuf.sr_mntindx,
		srmntbuf.sr_refcnt,
		srmntbuf.sr_dotdot,
		srmntbuf.sr_bcount,
		srmntbuf.sr_slpcnt);
	fprintf(fp," %s",(srmntbuf.sr_flags & MRDONLY) ? "ro" : "rw");
	fprintf(fp,"%s%s%s%s\n",
		(srmntbuf.sr_flags & MINUSE) ? " inuse" : "",
		(srmntbuf.sr_flags & MLINKDOWN) ? " ldown" : "",
		(srmntbuf.sr_flags & MFUMOUNT) ? " fumnt" : "",
		(srmntbuf.sr_flags & MINTER) ? " inter" : "");
}
