/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:inode.c	1.14"
/*
 * This file contains code for the crash functions:  inode, file.
 */

#include "sys/param.h"
#include "a.out.h"
#include "stdio.h"
#include "sys/sysmacros.h"
#include "sys/types.h"
#include "sys/fs/s5dir.h"
#include "sys/var.h"
#include "sys/mount.h"
#include "sys/fstyp.h"
#include "sys/fsid.h"
#include "sys/conf.h"
#include "sys/file.h"
#include "sys/sema.h"
#include "sys/comm.h"
#include "sys/inode.h"
#include "sys/fs/s5inode.h"
#include "sys/stream.h"
#include "crash.h"

extern struct syment *Inode, *Streams, *Mount,	/* namelist symbol pointers */
	*File, *Fsinfo, *Rcvd, *Nrcvd;
struct syment *S5inode;

struct prinode {		/* declaration for /proc prinode is */
	short pri_mode;		/* in /fs/proc/proca.c	 */
	struct prinode *pri_next;
}pribuf;
struct s5inode s5ibuf;		/* buffer for s5inode */
struct fsinfo fsbuf;		/* buffer for fsinfo */


/* get arguments for inode function */
int
getinode()
{
	int slot = -1;
	int full = 0;
	int all = 0;
	int phys = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int receive = 1;
	int c;
	char *heading = "SLOT MAJ/MIN FS INUMB REF LINK  UID   GID    SIZE     MODE  MNT M/ST RCVD FLAGS\n";

	if(!Streams)
		if(!(Streams = symsrch("streams")))
			error("streams not found in symbol table\n");
	if(!Fsinfo)
		if(!(Fsinfo = symsrch("fsinfo")))
			error("fsinfo not found in symbol table\n");
	if(!Rcvd)
		if(!(Rcvd = symsrch("rcvd")))
			receive = 0;
	if(receive && !Nrcvd)
		if(!(Nrcvd = symsrch("nrcvd")))
			receive = 0;
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
	fprintf(fp,"INODE TABLE SIZE = %d\n",vbuf.v_inode);
	if(!full)
		fprintf(fp,"%s",heading);
	if(args[optind]) {
		all = 1;
		do {
			getargs(vbuf.v_inode,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prinode(all,full,slot,phys,addr,heading,
						receive);
			else {
				if(arg1 < vbuf.v_inode)
					slot = arg1;
				else addr = arg1;
				prinode(all,full,slot,phys,addr,heading,
					receive);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < vbuf.v_inode; slot++)
		prinode(all,full,slot,phys,addr,heading,receive);
}

/* get inode mode according to file system switch type */
short
getimode(type,ibuf)
short type;
struct inode ibuf;
{
	char name[FSTYPSZ+1];

	readmem((long)fsbuf.fs_name,1,-1,name,sizeof name,"fs_name");
	if(!(strcmp(name,"S51K"))) {	
		readmem((long)ibuf.i_fsptr,1,-1, (char *)&s5ibuf,sizeof s5ibuf,
			"s5inode table");
		return(s5ibuf.s5i_mode);
	}
	if(!(strcmp(name,"DUFST")))
		return(0);
	if(!(strcmp(name,"PROC"))) {
		readmem((long)ibuf.i_fsptr,1,-1,(char *)&pribuf,
			sizeof pribuf,"prinode table");
		return(pribuf.pri_mode);
	}
	return(0);
}

/* print inode table */
int
prinode(all,full,slot,phys,addr,heading,receive)
int all,full,slot,phys,receive;
long addr;
char *heading;
{
	struct inode ibuf;
	short mode;
	char ch;
	int str_slot;
	int mnt_slot;
	int fstypp = 0;
	int nrcvd;
	char name[FSTYPSZ+1];
	extern long lseek();

	readbuf(addr,(long)(Inode->n_value+slot*sizeof ibuf),phys,-1,
		(char *)&ibuf,sizeof ibuf,"inode table");
	readmem((long)(Fsinfo->n_value+ibuf.i_fstyp*sizeof fsbuf),1,-1,
		(char *)&fsbuf,sizeof fsbuf,"file system information table");
	if(!ibuf.i_count && !all)
			return ;
	if(addr > -1) 
		slot = getslot(addr,(long)Inode->n_value,sizeof ibuf,phys);
	if(full)
		fprintf(fp,"%s",heading);
	fprintf(fp,"%4d %3u,%-3u %2d %5u %3d %4d %5d %5d %8ld",
		slot,
		major(ibuf.i_dev),
		minor(ibuf.i_dev),
		ibuf.i_fstyp,
		ibuf.i_number,
		ibuf.i_count,
		ibuf.i_nlink,
		ibuf.i_uid,
		ibuf.i_gid,
		ibuf.i_size);
	switch(ibuf.i_ftype) {
		case IFDIR: ch = 'd'; break;
		case IFCHR: ch = 'c'; break;
		case IFBLK: ch = 'b'; break;
		case IFREG: ch = 'f'; break;
		case IFIFO: ch = 'p'; break;
		default:    ch = '-'; break;
	}
	fprintf(fp," %c",ch);
	if(ibuf.i_fstyp) {
		if(ibuf.i_fsptr) {
			mode = getimode(ibuf.i_fstyp,ibuf);
			fprintf(fp,"%s%s%s%03o",
				mode & ISUID ? "u" : "-",
				mode & ISGID ? "g" : "-",
				mode & ISVTX ? "v" : "-",
				mode & 0777);
		}
		else fprintf(fp,"------");
	}
	else fprintf(fp,"------");
	mnt_slot = ((long)ibuf.i_mntdev - Mount->n_value)/(sizeof(struct mount));
	fprintf(fp," %3d",mnt_slot);
	str_slot = ((long)ibuf.i_sptr - Streams->n_value)/(sizeof(struct stdata));
	if ((str_slot >= 0) && (str_slot <= vbuf.v_nstream))
		fprintf(fp," S%3d", str_slot);
	else {
		mnt_slot = ((long)ibuf.i_mnton - Mount->n_value)/
			(sizeof(struct mount));
		if ((mnt_slot >= 0) && (mnt_slot <= vbuf.v_mount))
			fprintf(fp," M%3d", mnt_slot);
		else fprintf(fp,"   - ");
	}
	if(receive){
		if(lseek(mem,vtop((long)Nrcvd->n_value,-1),0) == -1) 
			receive = 0;
		else if(read(mem,(char *)&nrcvd, sizeof nrcvd) != sizeof nrcvd)
			receive = 0;
	}
	if(receive) {
		slot = ((long)ibuf.i_rcvd - (long)Rcvd->n_value)/
			sizeof (struct rcvd);
		if((slot >= 0) && (slot < nrcvd))
			fprintf(fp," %4d",slot);
		else fprintf(fp,"   - ");
	}
	else fprintf(fp,"   - ");
	fprintf(fp,"%s%s%s%s%s%s%s%s%s%s%s%s%s\n",
		ibuf.i_flag & ILOCK ? " lk" : "",
		ibuf.i_flag & IUPD ? " up" : "",
		ibuf.i_flag & IACC ? " ac" : "",
		ibuf.i_flag & IMOUNT ? " mt" : "",
		ibuf.i_flag & IWANT ? " wt" : "",
		ibuf.i_flag & ITEXT ? " tx" : "",
		ibuf.i_flag & ICHG ? " ch" : "",
		ibuf.i_flag & ISYN ? " sy" : "",
		ibuf.i_flag & IADV ? " ad" : "",
		ibuf.i_flag & IDOTDOT ? " dt" : "",
		ibuf.i_flag & ILBIN ? " lb" : "",
		ibuf.i_flag & IRMOUNT ? " rm" : "",
		ibuf.i_flag & IISROOT ? " rt" : "");
	if(!full)
		return;
	fprintf(fp,"\tFORW BACK AFOR ABCK\n");
	slot = ((long)ibuf.i_forw - (long)Inode->n_value) / sizeof ibuf;
	if((slot >= 0) && (slot < vbuf.v_inode))
		fprintf(fp,"\t%4d",slot);
	else fprintf(fp,"\t  - ");
	slot = ((long)ibuf.i_back - (long)Inode->n_value) / sizeof ibuf;
	if((slot >= 0) && (slot < vbuf.v_inode))
		fprintf(fp," %4d",slot);
	else fprintf(fp,"   - ");
	slot = ((long)ibuf.av_forw - (long)Inode->n_value) / sizeof ibuf;
	if((slot >= 0) && (slot < vbuf.v_inode))
		fprintf(fp," %4d",slot);
	else fprintf(fp,"   - ");
	slot = ((long)ibuf.av_back - (long)Inode->n_value) / sizeof ibuf;
	if((slot >= 0) && (slot < vbuf.v_inode))
		fprintf(fp," %4d\n",slot);
	else fprintf(fp,"   - \n");
	fprintf(fp,"\tRMAJ/MIN  FSTYPP   FSPTR   FILOCKS  FS\n");
	fprintf(fp,"\t %3u,%-3u",
		major(ibuf.i_rdev),
		minor(ibuf.i_rdev));
#ifdef FSPTR
	fstypp = (int)ibuf.i_fstypp,
#endif
	fprintf(fp," %8x %8x %8x",
		fstypp,
		ibuf.i_fsptr,
		ibuf.i_filocks);
	readmem((long)fsbuf.fs_name,1,-1,name,sizeof name,"fs_name");
	fprintf(fp," %-20s\n",name);
	if(!(strcmp(name,"S51K")))
		prs5fs(ibuf.i_ftype);
	else if(!(strcmp(name,"PROC")))
		prprocfs();
	fprintf(fp,"\n");
}


/* print s5 inode */
int
prs5fs(ftype)
ushort ftype;
{
	int i,slot;

	fprintf(fp,"\t  LASTR   MAP/DOFF  FLAGS\n");
	fprintf(fp,"\t%8x",
		s5ibuf.s5i_lastr);
	if(ftype == IFREG)
		fprintf(fp,"  %8x",s5ibuf.s5i_map);
	else if(ftype == IFDIR)
		fprintf(fp,"  %8x",s5ibuf.s5i_diroff);
	else fprintf(fp,"          ");
	if(s5ibuf.s5i_flags == S5IFREE) 
		fprintf(fp,"  free");
	if((ftype == IFDIR) || (ftype == IFREG)) {
		for(i = 0; i < NADDR; i++) {
			if(!(i & 3))
				fprintf(fp,"\n\t");
			fprintf(fp,"[%2d]: %-10x",i,s5ibuf.s5i_addr[i]);
		}
		fprintf(fp,"\n");
	}
	else if(ftype == IFIFO) {
		for(i = 0; i < NFADDR; i++) {
			if(!(i & 3))
				fprintf(fp,"\n\t");
			fprintf(fp,"[%2d]: %-10x",i,s5ibuf.s5i_faddr);
		}
		fprintf(fp,"\n");
	}
	else {
		fprintf(fp,"\n\tNEXT:  ");
		if(!S5inode)
			if(!(S5inode = symsrch("s5inode"))) {
				fprintf(fp,"-\n");
				return;
			}
		slot = ((long)s5ibuf.s5i_next - (long)S5inode->n_value)/
			sizeof (struct s5inode);
		if((slot >= 0) && (slot < vbuf.v_s5inode))
			fprintf(fp,"%4d\n",slot);
		else fprintf(fp,"-\n");
	}
}


/* print /proc inode information */
int
prprocfs()
{
	fprintf(fp,"\n\tNEXT = %8x\n",
		pribuf.pri_next);
}


/* get arguments for file function */
int
getfile()
{
	int slot = -1;
	int all = 0;
	int phys = 0;
	long addr = -1;
	long arg1 = -1;
	long arg2 = -1;
	int c;

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
	fprintf(fp,"FILE TABLE SIZE = %d\n",vbuf.v_file);
	fprintf(fp,"SLOT  REF  I/FL   OFFSET  FLAGS\n");
	if(args[optind]) {
		all = 1;
		do {
			getargs(vbuf.v_file,&arg1,&arg2);
			if(arg1 == -1) 
				continue;
			if(arg2 != -1)
				for(slot = arg1; slot <= arg2; slot++)
					prfile(all,slot,phys,addr);
			else {
				if(arg1 < vbuf.v_file)
					slot = arg1;
				else addr = arg1;
				prfile(all,slot,phys,addr);
			}
			slot = addr = arg1 = arg2 = -1;
		}while(args[++optind]);
	}
	else for(slot = 0; slot < vbuf.v_file; slot++)
		prfile(all,slot,phys,addr);
}


/* print file table */
int
prfile(all,slot,phys,addr)
int all,slot,phys;
long addr;
{
	struct file fbuf;
	int inoslot,fileslot;

	readbuf(addr,(long)(File->n_value+slot*sizeof fbuf),phys,-1,
		(char *)&fbuf,sizeof fbuf,"file table");
	if(!fbuf.f_count && !all)
		return;
	if(addr > -1) 
		slot = getslot(addr,(long)File->n_value,sizeof fbuf,phys);
	fprintf(fp,"%4d  %3d",
		slot,
		fbuf.f_count);
	inoslot = ((long)fbuf.f_inode - Inode->n_value)/(sizeof (struct inode));
	if((inoslot >= 0) && (inoslot < vbuf.v_inode))
		fprintf(fp,"  I%3d",inoslot);
	else {
		fileslot = ((long)fbuf.f_next - File->n_value)/
			(sizeof (struct file));
		if((fileslot >= 0) && (fileslot < vbuf.v_file))
			fprintf(fp,"  F%3d",fileslot);
		else fprintf(fp,"    - ",fbuf.f_inode);
	}
	fprintf(fp,"  %8x",fbuf.f_offset);
	fprintf(fp,"%s%s%s%s%s%s%s%s%s\n",
		fbuf.f_flag & FREAD ? " read" : "",
		fbuf.f_flag & FWRITE ? " write" : "",  /* print the file flag */
		fbuf.f_flag & FAPPEND ? " appen" : "",
		fbuf.f_flag & FSYNC ? " sync" : "",
		fbuf.f_flag & FNET ? " net" : "",
		fbuf.f_flag & FCREAT ? " creat" : "",
		fbuf.f_flag & FTRUNC ? " trunc" : "",
		fbuf.f_flag & FEXCL ? " excl" : "",
		fbuf.f_flag & FNDELAY ? " ndelay" : "");
}
