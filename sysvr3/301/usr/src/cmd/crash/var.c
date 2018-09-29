/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:var.c	1.5"
/*
 * This file contains code for the crash function var.
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
#include "sys/inode.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/file.h"
#include "sys/mount.h"
#include "sys/utsname.h"
#include "crash.h"

extern struct syment *Inode,*File,*Mount,*Proc;   /* namelist symbol pointers */

/* get arguments for var function */
int
getvar()
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
		longjmp(syn,0);
	else prvar();
}

/* print var structure */
int
prvar()
{
	extern struct syment *V;
	unsigned index,fndex,mndex,pndex;

	readmem((long)V->n_value,1,-1,(char *)&vbuf,
		sizeof vbuf,"var structure");
	index = (unsigned)(vbuf.ve_inode - Inode->n_value) /
		sizeof (struct inode);
	fndex = (unsigned)(vbuf.ve_file - File->n_value) /sizeof (struct file);
	mndex = (unsigned)(vbuf.ve_mount - Mount->n_value) /sizeof (struct mount);
	pndex = (unsigned)(vbuf.ve_proc - Proc->n_value) /sizeof (struct proc);

	fprintf(fp,"v_buf: %3d\nv_call: %3d\nv_clist: %3d\nv_maxup: %3d\n",
		vbuf.v_buf,
		vbuf.v_call,
		vbuf.v_clist,
		vbuf.v_maxup);
	fprintf(fp,"v_hbuf: %3d\nv_hmask: %3d\nv_pbuf: %3d\nv_nofiles: %3d\n",
		vbuf.v_hbuf,
		vbuf.v_hmask,
		vbuf.v_pbuf, 
		vbuf.v_nofiles);
	fprintf(fp,"v_inode: %3d\t\tve_inode: %3d\nv_s5inode: %3d\n",
		vbuf.v_inode,
		index,
		vbuf.v_s5inode);
	fprintf(fp,"v_file: %3d\t\tve_file: %3d\n",
		vbuf.v_file,
		fndex);
	fprintf(fp,"v_mount: %3d\t\tve_mount: %3d\nv_proc: %3d\t\tve_proc: %3d\n",
		vbuf.v_mount,
		mndex,
		vbuf.v_proc,
		pndex);
	fprintf(fp,"v_region: %3d\nv_sptmap: %3d\nv_nstreams: %3d\nv_nqueues: %3d\n",
		vbuf.v_region,
		vbuf.v_sptmap,
		vbuf.v_nstream,
		vbuf.v_nqueue);
	fprintf(fp,"v_vhndfrac: %d\nv_maxpmem: %d\nv_autoup: %d\n",
		vbuf.v_vhndfrac,
		vbuf.v_maxpmem,
		vbuf.v_autoup);
	fprintf(fp,"v_nblk4096: %d\tv_nblk2048: %d\tv_nblk1024: %d\tv_nblk512: %d\n",
		vbuf.v_nblk4096,
		vbuf.v_nblk2048,
		vbuf.v_nblk1024,
		vbuf.v_nblk512); 
	fprintf(fp,"v_nblk256: %d\tv_nblk128: %d\tv_nblk64: %d\tv_nblk16: %d\n",
		vbuf.v_nblk256,
		vbuf.v_nblk128,
		vbuf.v_nblk64,
		vbuf.v_nblk16);
	fprintf(fp,"v_nblk4: %d\n",
		vbuf.v_nblk4);
}
