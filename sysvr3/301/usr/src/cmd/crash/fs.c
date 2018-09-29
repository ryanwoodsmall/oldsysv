/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:fs.c	1.6"
/*
 * This file contains code for the crash function fs.
 */

#include "sys/param.h"
#include "a.out.h"
#include "stdio.h"
#include "sys/types.h"
#include "sys/fs/s5dir.h"
#include "sys/var.h"
#include "sys/nami.h"
#include "sys/fstyp.h"
#include "sys/conf.h"
#include "sys/mount.h"
#include "crash.h"

struct syment  *Nfstyp;		/*namelist symbol pointers */
struct syment *Fsinfo;
extern struct syment *Mount;

/* get arguments for fs function */
int
getfs()
{
	int slot = -1;
	int all = 0;
	int phys = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int c;
	short nfstypes;

	if(!Fsinfo)
		if((Fsinfo = symsrch("fsinfo")) == NULL)
			error("fsinfo not found in symbol table\n");
	if(!Nfstyp)
		if((Nfstyp = symsrch("nfstyp")) == NULL)
			error("nfstyp not found in symbol table\n");

	optind = 1;
	while((c = getopt(argcnt,args,"pw:")) !=EOF) {
		switch(c) {
			case 'p' :	phys = 1;
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	readmem((long)Nfstyp->n_value,1,-1,(char *)&nfstypes,
		sizeof (int), "number of file systems types");
	fprintf(fp,"FILE SYSTEM INFORMATION TABLE SIZE = %d\n",nfstypes-1);
	fprintf(fp,"SLOT        NAME          PIPE   NOTIFY  FLAGS\n");
	if(args[optind]) {
		all = 1;
		do {
			getargs(nfstypes,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg1 == 0) {
				fprintf(fp,"0 is out of range\n");
				continue;
			}
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prfs(all,slot,phys,addr);
			else {
				if(arg1 < nfstypes)
					slot = arg1;
				else addr = arg1;
				prfs(all,slot,phys,addr);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 1; slot < nfstypes; slot++)
		prfs(all,slot,phys,addr);
}

/* print fsinfo table */
int
prfs(all,slot,phys,addr)
int all,slot,phys;
long addr;
{
	struct fsinfo fsbuf;
	char name[FSTYPSZ+1];

	readbuf(addr,(long)(Fsinfo->n_value+slot*sizeof fsbuf),phys,-1,
		(char *)&fsbuf,sizeof fsbuf,"file system information table");
	if(addr > -1)
		slot = getslot(addr,(long)Fsinfo->n_value,sizeof fsbuf,phys);
	if(!fsbuf.fs_name && !all)
		return; 
	readmem((long)fsbuf.fs_name,1,-1,name,sizeof name,"fs_name");
	fprintf(fp,"%4d %-20s",
		slot,
		name); 
	slot = ((long)fsbuf.fs_pipe - (long)Mount->n_value)/
		sizeof (struct mount);
	if((slot >= 0) && (slot < vbuf.v_mount))
		fprintf(fp," %4d",slot);
	else fprintf (fp,"   - ");
	fprintf(fp," %s%s%s %s\n",
		fsbuf.fs_notify & NO_CHDIR ? " dr" : "   ",
		fsbuf.fs_notify & NO_CHROOT ? " rt" : "   ",
		fsbuf.fs_notify & NO_SEEK ? " sk" : "   ",
		fsbuf.fs_flags & FS_NOICACHE ? " noic" : "");
}
