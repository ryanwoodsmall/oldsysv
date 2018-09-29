/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)crash-3b2:u.c	1.20"
/*
 * This file contains code for the crash functions:  user, pcb, stack,
 * trace, and kfp.
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
#include "sys/acct.h"
#include "sys/file.h"
#include "sys/nvram.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/inode.h"
#include "sys/lock.h"
#include "crash.h"


#define FP	1
#define AP	0
#define DECR	4
#define USTKADDR 0xc0020000
#define UBADDR 0xc0000000
#define UPCBADDR 0xc000000c
#define UKPCBADDR 0xc000007c
#define min(a,b) ((a) > (b) ? (b) : (a))

extern struct user *ubp;		/* ublock pointer */
extern int active;			/* active system flag */
struct proc procbuf;			/* proc entry buffer */
static long Kfp = 0;			/* kernal frame pointer */
extern	char	*strtbl ;		/* pointer to string table */
int	*stk_bptr;			/* stack pointer */
extern struct xtra_nvr xtranvram;	/* xtra nvram buffer */
extern	struct	syment	*Proc, *File,
	*Inode, *Curproc, *Panic;	/* namelist symbol pointers */
extern char *ctime();
extern struct	syment	*findsym();
extern long vtop();
extern long lseek();
extern char *malloc();
void free();


/* read ublock into buffer */
int
getublock(slot)
int slot;
{
	int 	i,cnt;
	long	pslot_va;

	if(slot == -1) 
		slot = getcurproc();
	if(slot >=  vbuf.v_proc || slot < 0)
		error("%d out of range\n",slot);

	pslot_va = (long)(Proc->n_value+slot*sizeof(struct proc));
	readmem(pslot_va,1,slot,(char *)&procbuf,sizeof procbuf,
		"process table");
	if (!procbuf.p_stat)
		error("%d is not a valid process\n",slot);
	if (procbuf.p_stat == SZOMB)
		error("%d is a zombie process\n",slot);
	i=((char*)ubptbl((proc_t*)pslot_va) - (char*)pslot_va -
		((char*)procbuf.p_ubptbl - (char*)&procbuf)) >> 2;
	for(cnt=0; cnt < sizeof(*ubp) + sizeof(int) * vbuf.v_nofiles;
		cnt += NBPP, i++) {
		/* seek from beginning of memory to ith page of uarea */
		if(lseek(mem,(long)(procbuf.p_ubptbl[i].pgm.pg_pfn<<11)-
			MAINSTORE,0) == -1)
			error("seek error on ublock address\n");
		if(read(mem,(char *)ubp+cnt,min(NBPP,(sizeof(*ubp)+
			sizeof(int)*vbuf.v_nofiles)-cnt)) !=
			min(NBPP, (sizeof(*ubp)+sizeof(int)*
				vbuf.v_nofiles)-cnt))
			error("read error on ublock\n");
	}
}

/* allocate buffer for stack */
unsigned
setbf(top, bottom, slot)
long top;
long bottom;
int slot;
{
	unsigned range;
	char *bptr;
	long remainder;
	long nbyte;
	long paddr;


	if (bottom > top) 
		error("Top of stack value less than bottom of stack\n");
	range = (unsigned)(top - bottom);
	if((stk_bptr = (int *)malloc(range)) == NULL)
	{
		prerrmes("Insufficient memory available for stack buffering.\n"); 
		prerrmes("Only the most recently pushed 4K bytes will be dumped from the stack.\n");
		prerrmes("New stack lower bound: %8.8x\n",top - range);
		range = 4096;
		if((stk_bptr = (int *)malloc(range)) == 0)
			error("Second attempt to allocate memory for stack buffering failed, try again later\n");

	}
	
	bottom = top - range;
	bptr = (char *)stk_bptr;
	do {
		remainder = ((bottom + NBPP) & ~((long)NBPP -1)) - bottom;
		nbyte = min(remainder, top-bottom);
		if((paddr = vtop(bottom,slot)) == -1) {
			free((char *)stk_bptr);
			stk_bptr = NULL;
			error("The stack lower bound, %x, is an invalid address\nThe saved stack frame pointer is %x\n",bottom,top);
		}
		if(lseek(mem,paddr,0) == -1) {
			free((char *)stk_bptr);
			error("seek error on stack\n");
		}
		if(read(mem,bptr,(unsigned)nbyte) != (unsigned)nbyte) {
			free((char *)stk_bptr);
			stk_bptr = NULL;
			error("read error on stack\n");
		}
		bptr += nbyte;
		bottom += nbyte;
	} while (bottom < top);
	return(range);
}

/* get arguments for user function */
int
getuser()
{
	int slot = Procslot;
	int full = 0;
	int c;

	optind = 1;
	while((c = getopt(argcnt,args,"fw:")) !=EOF) {
		switch(c) {
			case 'f' :	full = 1;
					break;
			case 'w' :	redirect();
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		if((slot = strcon(args[optind],'d')) == -1)
			error("\n");
		if((slot < 0) || (slot >= vbuf.v_proc)) 
			error("%d is out of range\n",slot);
		pruser(full,slot);
	}
	else pruser(full,slot);
}

/* print ublock */
int
pruser(full,slot)
int full,slot;
{
	register  int  i,j;
	unsigned offset;

	getublock(slot);
	if(slot == -1)
		slot = getcurproc();
	fprintf(fp,"PER PROCESS USER AREA FOR PROCESS %d\n",slot);
	fprintf(fp,"USER ID's:\t");
	fprintf(fp,"uid: %u, gid: %u, real uid: %u, real gid: %u\n",
		ubp->u_uid,
		ubp->u_gid,
		ubp->u_ruid,
		ubp->u_rgid);
	fprintf(fp,"PROCESS TIMES:\t");
	fprintf(fp,"user: %ld, sys: %ld, child user: %ld, child sys: %ld\n",
		ubp->u_utime,
		ubp->u_stime,
		ubp->u_cutime,
		ubp->u_cstime);
	fprintf(fp,"PROCESS MISC:\n");
	fprintf(fp,"\tcommand: %s,", ubp->u_comm);
	fprintf(fp," psargs: %s\n", ubp->u_psargs);
	fprintf(fp,"\tproc slot: %d", ((unsigned)ubp->u_procp - Proc->n_value)
		/sizeof (struct proc));
	if(ubp->u_ttyp != 0)
		fprintf(fp,", cntrl tty: %3u,%-3u\n",
			major(ubp->u_ttyd),
			minor(ubp->u_ttyd));
	else fprintf(fp,", cntrl tty: maj(??) min(??)\n");
	fprintf(fp,"\tstart: %s", ctime(&ubp->u_start));
	fprintf(fp,"\tmem: %x, type: %s%s\n",
		ubp->u_mem,
		ubp->u_acflag & AFORK ? "fork" : "exec",
		ubp->u_acflag & ASU ? " su-user" : "");
	fprintf(fp,"\t%s", ubp->u_dmm ? "double mapped, " : "");
	fprintf(fp,"shared segments: %d, ", ubp->u_shmcnt);
	fprintf(fp,"proc/text lock:%s%s%s%s\n",
		ubp->u_lock & TXTLOCK ? " txtlock" : "",
		ubp->u_lock & DATLOCK ? " datlock" : "",
		ubp->u_lock & PROCLOCK ? " proclock" : "",
		ubp->u_lock & (PROCLOCK|TXTLOCK|DATLOCK) ? "" : " none");
	fprintf(fp,"\tstack: %8x", ubp->u_stack);
	fprintf(fp,"\tinode of current directory: ");
	slot = ((unsigned)ubp->u_cdir - Inode->n_value) /
			sizeof (struct inode);
	if((slot >= 0) && (slot < vbuf.v_inode))
		fprintf(fp,"%d",slot);
	else fprintf(fp," - ");
	if(ubp->u_rdir) {
		fprintf(fp,", inode of root directory: ");
		slot = ((unsigned)ubp->u_rdir - Inode->n_value) /
			sizeof (struct inode);
		if((slot >= 0) && (slot < vbuf.v_inode))
			fprintf(fp,"%d,",slot);
		else fprintf(fp," - ,");
	}
	fprintf(fp,"\nOPEN FILES AND POFILE FLAGS:\n");
	for(i = 0, j = 1; i < vbuf.v_nofiles; i++){
		if(ubp->u_ofile[i] != 0) {
			if(!(j++ & 3))
				fprintf(fp,"\n");
			fprintf(fp,"\t[%d]: F#%d,",
				i,
				((unsigned)ubp->u_ofile[i] -
				File->n_value)/sizeof (struct file));
			offset = ((char *)(ubp->u_pofile+i) - (char *)UBADDR);
			fprintf(fp," %x\t",((char *)ubp)[offset]);
		}
	}
	fprintf(fp,"\n");
	fprintf(fp,"FILE I/O:\n\tu_base: %8x,",ubp->u_base);
	fprintf(fp," file offset: %d, bytes: %d,\n",
		ubp->u_offset,
		ubp->u_count);
	fprintf(fp,"\tsegment: %s,", ubp->u_segflg == 0 ? "data" :
		(ubp->u_segflg == 1 ? "sys" : "text"));
	fprintf(fp," cmask: %4.4o, ulimit: %d\n",
		ubp->u_cmask,
		ubp->u_limit);
	fprintf(fp,"\tfile mode(s):");	
	fprintf(fp,"%s%s%s%s%s%s%s%s%s\n",
		ubp->u_fmode & FREAD ? " read" : "",
		ubp->u_fmode & FWRITE ? " write" : "",
		ubp->u_fmode & FAPPEND ? " append" : "",
		ubp->u_fmode & FSYNC ? " sync" : "",
		ubp->u_fmode & FNET ? " net" : "",
		ubp->u_fmode & FCREAT ? " creat" : "",
		ubp->u_fmode & FTRUNC ? " trunc" : "",
		ubp->u_fmode & FEXCL ? " excl" : "",
		ubp->u_fmode & FNDELAY ? " ndelay" : "");
	fprintf(fp,"SIGNAL DISPOSITION:");
	for (i = 0; i < NSIG; i++) {
		if(!(i & 3))
			fprintf(fp,"\n\t");
		fprintf(fp,"%4d: ", i+1);
		if((int)ubp->u_signal[i] == 0 || (int)ubp->u_signal[i] == 1)
			fprintf(fp,"%8s",(int)ubp->u_signal[i] ? "ignore" : "default");
		else fprintf(fp,"%-8x",(int)ubp->u_signal[i]);
	}
	if(full) {
		fprintf(fp,"\n\tpcbp: %x, r0tmp: %x, spop: %x\n",
			ubp->u_pcbp,
			ubp->u_r0tmp,
			ubp->u_spop);
		fprintf(fp,"\tmau asr: %x, mau dr[0]: %x, mau dr[1]: %x, mau dr[2]",
			ubp->u_mau.asr,
			ubp->u_mau.dr[0],
			ubp->u_mau.dr[1],
			ubp->u_mau.dr[2]);
		for(i = 0; i < 4; i++) {
			fprintf(fp,"\n\t");
			for(j = 0; j < 3; j++)
				fprintf(fp,"fpregs[%d][%d]: %x  ",
					i,j,ubp->u_mau.fpregs[i][j]);
		}
		fprintf(fp,"\n");
		fprintf(fp,"\tcaddrflt: %x, priptrsv: %x, prisv: %u, nshmseg: %d\n",
			ubp->u_caddrflt,
			ubp->u_priptrsv,
			ubp->u_prisv,
			ubp->u_nshmseg);
		fprintf(fp,"\tux_uid: %u, ux_gid: %u,",
			ubp->u_exuid,
			ubp->u_exgid);
		fprintf(fp," ux_mode: %s%s%s%s%s%s%s%s%s\n",
			ubp->u_exmode & FREAD ? " read" : "",
			ubp->u_exmode & FWRITE ? " write" : "",
			ubp->u_exmode & FAPPEND ? " append" : "",
			ubp->u_exmode & FSYNC ? " sync" : "",
			ubp->u_exmode & FNET ? " net" : "",
			ubp->u_exmode & FCREAT ? " creat" : "",
			ubp->u_exmode & FTRUNC ? " trunc" : "",
			ubp->u_exmode & FEXCL ? " excl" : "",
			ubp->u_exmode & FNDELAY ? " ndelay" : "");
		fprintf(fp,"\tcomp: %x, nextcp: %x\n",
			ubp->u_comp,
			ubp->u_nextcp);
		fprintf(fp,"\tbsize: %d, pgproc: %d, qsav: %x, error: %d\n",
			ubp->u_bsize,
			ubp->u_pgproc,
			ubp->u_qsav,
			ubp->u_error);
		fprintf(fp,"\tap: %x, u_r: %x, pbsize: %d\n",
			ubp->u_ap,
			ubp->u_rval1,
			ubp->u_pbsize);
		fprintf(fp,"\tpboff: %d,",ubp->u_pboff);
		fprintf(fp," pbdev: %3u,%-3u,",
			major(ubp->u_pbdev),
			minor(ubp->u_pbdev));
		fprintf(fp," rablock: %x, errcnt: %d\n",
			ubp->u_rablock,
			ubp->u_errcnt);
		fprintf(fp,"\tdirp: %x,",ubp->u_dirp);
		fprintf(fp," dent.d_ino: %d",ubp->u_dent.d_ino);
		fprintf(fp," dent.d_name: %.14s,",ubp->u_dent.d_name); 
		fprintf(fp," pdir: ");
		slot = ((unsigned)ubp->u_pdir - Inode->n_value) /
			sizeof (struct inode);
		if((slot >= 0) && (slot < vbuf.v_inode))
			fprintf(fp,"%d\n",slot);
		else fprintf(fp," - \n");
		fprintf(fp,"\tttyip: ");
		slot = ((unsigned)ubp->u_ttyip - Inode->n_value)/
			sizeof (struct inode);
		if((slot >= 0) && (slot < vbuf.v_inode))
			fprintf(fp,"%d,",slot);
		else fprintf(fp," - ,");
		fprintf(fp," tsize: %x, dsize: %x, ssize: %x\n",
			ubp->u_tsize,
			ubp->u_dsize,
			ubp->u_ssize);
		fprintf(fp,"\targ[0]: %x, arg[1]: %x, arg[2]: %x\n",
			ubp->u_arg[0],
			ubp->u_arg[1],
			ubp->u_arg[2]);
		fprintf(fp,"\targ[3]: %x, arg[4]: %x, arg[5]: %x\n",
			ubp->u_arg[3],
			ubp->u_arg[4],
			ubp->u_arg[5]);	
		fprintf(fp,"\tiop: %x, ar0: %x, ttyp: %x, ticks: %x\n",
			ubp->u_iop,
			ubp->u_ar0,
			ubp->u_ttyp,
			ubp->u_ticks);
		fprintf(fp,"\tpr_base: %x, pr_size: %d, pr_off: %x, pr_scale: %d\n",
			ubp->u_prof.pr_base,
			ubp->u_prof.pr_size,
			ubp->u_prof.pr_off,
			ubp->u_prof.pr_scale);
		fprintf(fp,"\tior: %x, iow: %x, iosw: %x, ioch: %x\n",
			ubp->u_ior,
			ubp->u_iow,
			ubp->u_iosw,
			ubp->u_ioch);
		fprintf(fp,"\tEXDATA:\n");
		fprintf(fp,"\tip: ");
		slot = ((unsigned)ubp->u_exdata.ip - Inode->n_value) /
				sizeof (struct inode);
		if((slot >= 0) && (slot < vbuf.v_inode))
			fprintf(fp," %d,",slot);
		else fprintf(fp," - , ");
		fprintf(fp,"tsize: %x, dsize: %x, bsize: %x, lsize: %x\n",
			ubp->u_exdata.ux_tsize,
			ubp->u_exdata.ux_dsize,
			ubp->u_exdata.ux_bsize,
			ubp->u_exdata.ux_lsize);
		fprintf(fp,"\tmagic#: %o, toffset: %x, doffset: %x, loffset: %x\n",
			ubp->u_exdata.ux_mag,
			ubp->u_exdata.ux_toffset,
			ubp->u_exdata.ux_doffset,
			ubp->u_exdata.ux_loffset);
		fprintf(fp,"\ttxtorg: %x, datorg: %x, entloc: %x, nshlibs: %d\n",
			ubp->u_exdata.ux_txtorg,
			ubp->u_exdata.ux_datorg,
			ubp->u_exdata.ux_entloc,
			ubp->u_exdata.ux_nshlibs);
		fprintf(fp,"\texecsz: %x\n",ubp->u_execsz);
		fprintf(fp,"\ttracepc: %x\n",ubp->u_tracepc);
		fprintf(fp,"\tRFS:\n");
		fprintf(fp,"\trflags:%s%s%s%s%s%s\n",
			ubp->u_rflags & U_RCDIR ? " rcdir" : "",
			ubp->u_rflags & U_RRDIR ? " rrdir" : "",
			ubp->u_rflags & U_RSYS ? " rsys" : "",
			ubp->u_rflags & U_LBIN ? " lbin" : "",
			ubp->u_rflags & U_DOTDOT ? " dotdot" : "",
			ubp->u_rflags & U_RCOPY ? " rcopy" : "");
		fprintf(fp,"\trrcookie.sysid: %x, rrcookie.rcvd: %x\n",
			ubp->u_rrcookie.c_sysid,
			ubp->u_rrcookie.c_rcvd);
		fprintf(fp,"\tsyscall: %d, mntindx: %d, gift: %x\n",
			ubp->u_syscall,
			ubp->u_mntindx,
			ubp->u_gift);
		fprintf(fp,"\tnewgift.sysid: %x, newgift.rcvd: %x\n",
			ubp->u_newgift.c_sysid,
			ubp->u_newgift.c_rcvd);
		fprintf(fp,"\tcopymsg: %x, copybp: %x, msgend: %x\n",
			ubp->u_copymsg,
			ubp->u_copybp,
			ubp->u_msgend);
	}
	fprintf(fp,"\n");
}

/* get arguments for pcb function */
int
getpcb()
{
	int proc = Procslot;
	int phys = 0;
	char type = 'n';
	long addr = -1;
	int c;
	struct syment *sp;

	optind = 1;
	while((c = getopt(argcnt,args,"iukpw:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			case 'i' :	type = 'i';
					break;
			case 'u' :	type = 'u';
					break;
			case 'k' :	type = 'k';
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(type == 'i') {
		if(!args[optind])
			longjmp(syn,0);
		if(*args[optind] == '(')  {
			if((addr = eval(++args[optind])) == -1)
				error("\n");
		}
		else if(sp = symsrch(args[optind])) 
			addr = (long)sp->n_value;
		else if(isasymbol(args[optind]))
			error("%s not found in symbol table\n",args[optind]);
		else if((addr = strcon(args[optind],'h')) == -1)
				error("\n");
		pripcb(phys,addr);
		}
	else {
		if(args[optind]) {
			if((proc = strcon(args[optind],'d')) == -1)
				error("\n");
			if((proc > vbuf.v_proc) || (proc < 0))
				error("%d out of range\n",proc);
			prpcb(proc,type);
		}
		else prpcb(proc,type);
	}
}


/* print user, kernel, or active pcb */
int
prpcb(proc,type)
int proc;
char type;
{
	int	i, j;
	struct kpcb *kpcbp;
	struct pcb *pcbp;

	getublock(proc);
	switch(type) {
		case 'n' : kpcbp = (struct kpcb *)(((long)ubp->u_pcbp - 
				sizeof (struct ipcb) - UBADDR) + (long)ubp);
			   break;
		case 'u' : kpcbp = (struct kpcb*)&ubp->u_pcb;
			   break;
		case 'k' : kpcbp = (struct kpcb *)((long)&ubp->u_kpcb);
			   break;
		default  : longjmp(syn,0);
			   break;
	}
	pcbp = (struct pcb *)&kpcbp->psw;
	if (!pcbp->psw.I) {
		fprintf(fp,"ipsw: %08x   ipc: %08x   isp: %08x\n",
			kpcbp->ipcb.psw,
			kpcbp->ipcb.pc,
			kpcbp->ipcb.sp);
		fprintf(fp,"psw: %08x   pc: %08x   sp: %08x   slb: %08x   sub: %08x\n",
			pcbp->psw,
			pcbp->pc,
			pcbp->sp,
			pcbp->slb,
			pcbp->sub);
		fprintf(fp,"AP: %08x    FP: %08x   r0: %08x   r1: %08x    r2: %08x\n",
			pcbp->regsave[0],
			pcbp->regsave[1],
			pcbp->regsave[2],
			pcbp->regsave[3],
			pcbp->regsave[4]);
		fprintf(fp,"r3: %08x    r4: %08x   r5: %08x   r6: %08x    r7: %08x\n",
			pcbp->regsave[5],
			pcbp->regsave[6],
			pcbp->regsave[7],
			pcbp->regsave[8],
			pcbp->regsave[9]);
		fprintf(fp,"r8: %08x\n", pcbp->regsave[10]);
		if (pcbp->mapinfo[1].movesize != NULL) 
			for (i = 1; i > MAPINFO; i++) {
				fprintf(fp,"pcb map block # %08x\n", i);
				for (j = 1; j > MOVEDATA; j++) {
					fprintf(fp,"Data to move: %08x\n",pcbp->mapinfo[i].movedata[j]);
				}
			}
	}
	else error("initial pcb\n");
}

/* print interrupt pcb */
int
pripcb(phys,addr)
int phys;
long addr;
{
	struct 	kpcb 	pcbuf;

	readbuf(addr,addr,phys,-1,(char *)&pcbuf,sizeof pcbuf,"interrupt pcb");
	fprintf(fp,"ipcb.psw: %08x   ipcb.pc: %08x   ipcb.sp: %08x\n",
		pcbuf.ipcb.psw,
		pcbuf.ipcb.pc,
		pcbuf.ipcb.sp);
	fprintf(fp,"psw: %08x   pc: %08x   sp: %08x   slb: %08x   sub: %08x\n",
		pcbuf.psw,
		pcbuf.pc,
		pcbuf.sp,
		pcbuf.slb,
		pcbuf.sub);
	fprintf(fp,"AP: %08x   FP: %08x   r0: %08x   r1: %08x   r2: %08x\n",
		pcbuf.regsave[0],
		pcbuf.regsave[1],
		pcbuf.regsave[2],
		pcbuf.regsave[3],
		pcbuf.regsave[4]);
	fprintf(fp,"r3: %08x   r4: %08x   r5: %08x   r6: %08x   r7: %08x\n",
		pcbuf.regsave[5],
		pcbuf.regsave[6],
		pcbuf.regsave[7],
		pcbuf.regsave[8],
		pcbuf.regsave[9]);
	fprintf(fp,"r8: %08x\n", pcbuf.regsave[10]);
}

/* get arguments for stack function */
int
getstack()
{
	int proc = Procslot;
	int phys = 0;
	char type = 'k';
	long addr = -1;
	int c;
	struct syment *sp;

	optind = 1;
	while((c = getopt(argcnt,args,"iukpw:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			case 'i' :	type = 'i';
					break;
			case 'u' :	type = 'u';
					break;
			case 'k' :	type = 'k';
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(type == 'i') {
		if(!args[optind])
			longjmp(syn,0);
		if(*args[optind] == '(') {
			if((addr = eval(++args[optind])) == -1)
				error("\n");
		}
		else if(sp = symsrch(args[optind])) 
			addr = (long)sp->n_value;
		else if(isasymbol(args[optind]))
			error("%s not found in symbol table\n",args[optind]);
			else if((addr = strcon(args[optind],'h')) == -1)
				error("\n");
		pristk(phys,addr);
	}
	else {
		if(args[optind]) {
			if((proc = strcon(args[optind],'d')) == -1)
				error("\n");
			if((proc > vbuf.v_proc) || (proc < 0))
				error("%d out of range\n",proc);
			if(type == 'u')
				prustk(proc);
			else prkstk(proc);
		}
		else if(type == 'u')
			prustk(proc);
		else prkstk(proc);
	}
}

/* print kernel stack */
int
prkstk(proc)
int proc;
{
	int panicstr;
	long stkfp ;
	long stklo ;

	getublock(proc);
	if((proc == -1) || (proc == getcurproc())){
		seekmem((long)Panic->n_value,1,-1);
		if((read(mem,(char *)&panicstr,sizeof panicstr)
			!= sizeof panicstr))
				error("read error on panic string\n");
		if(panicstr == 0) 
			error("information to process stack for current process not available\n");
		if(((long)ubp->u_stack <= UBADDR)  ||
			((long)ubp->u_stack >= USTKADDR))
			error("kernel stack not valid for current process\n");
		stklo = (long)ubp->u_stack;
		stkfp = xtranvram.systate.lfp;
	}
	else {
		if(ubp->u_kpcb.psw.CM == PS_USER)
			error("user mode\n");
		if(procbuf.p_flag == SSYS) 
			stklo = (long)ubp->u_stack;
		else stklo = (long) ubp->u_kpcb.slb;
		stkfp = ubp->u_kpcb.regsave[FP] ;
	}
	prstack(stkfp,stklo,proc);
}


/* print user stack */
int
prustk(proc)
int proc;
{
	int			panicstr;
	long			stkfp ;
	long			stklo ;

	getublock(proc);
	if((proc == -1) || (proc == getcurproc())){
		seekmem((long)Panic->n_value,1,-1);
		if((read(mem,(char *)&panicstr,sizeof panicstr)
			!= sizeof panicstr))
				error("read error on panic string\n");
		if(panicstr == 0)
			error("information to process stack for current process not available\n");
		if((long)ubp->u_stack < USTKADDR)
			error("user stack not valid for current process\n");
		stkfp = xtranvram.systate.lfp;
	}
	else stkfp = ubp->u_pcb.regsave[FP] ;
	stklo = USTKADDR;
	prstack(stkfp,stklo,proc);
}

/* print interrupt stack */
int
pristk(phys,addr)
int phys;
long addr;
{
	struct kpcb kpcbuf;
	long stkfp,stklo;

	if(active)
		error("invalid interrupt stack on running system\n");
	readbuf(addr,addr,phys,-1,(char *)&kpcbuf,sizeof kpcbuf,
		"interrupt process pcb");
	stkfp = kpcbuf.regsave[FP];
	if(procbuf.p_flag == SSYS) 
		stklo = (long)ubp->u_stack;
	else stklo = (long)kpcbuf.slb;
	prstack(stkfp,stklo,-1);
}

/* dump stack */
int
prstack(stkfp,stklo,slot)
long stkfp,stklo;
int slot;
{
	unsigned dmpcnt;
	int *stkptr;
	int prcnt;

	fprintf(fp,"FP: %x\n",stkfp);
	fprintf(fp,"LOWER BOUND: %x\n",stklo) ;
	
	if ( stkfp < stklo)
		error("upper bound < lower bound, unable to process stack\n") ;
	
	dmpcnt = setbf(stkfp, stklo, slot);
	stklo = stkfp - dmpcnt ;
	stkptr = (int *)(stk_bptr);

	prcnt = 0;
	for(; dmpcnt != 0; stkptr++, dmpcnt -= DECR)
	{
		if((prcnt++ % 4) == 0){
			fprintf(fp,"\n%8.8x: ",
				(int)(((long)stkptr - (long)stk_bptr) + stklo));
		}
		fprintf(fp,"  %8.8x", *stkptr);
	}
	free((char *)stk_bptr);
	stk_bptr = NULL;

	fprintf(fp,"\n\nSTACK FRAME:\n");
	fprintf(fp,"	ARG1 ... ARGN  RA'  AP'  FP'  (REGS  6 WORDS)  LOCAL1 ...\n");
	fprintf(fp,"  AP-----^					FP------^\n");
}

/* get arguments for trace function */
int
gettrace()
{
	int proc = Procslot;
	int phys = 0;
	int kfpset = 0;
	char type = 'k';
	long addr = -1;
	int c;
	struct syment *sp;

	optind = 1;
	while((c = getopt(argcnt,args,"irpw:")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 'p' :	phys = 1;
					break;
			case 'r' :	kfpset = 1;
					break;
			case 'i' :	type = 'i';
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(type == 'i') {
		if(!args[optind])
			longjmp(syn,0);
		if(*args[optind] == '(') {
			if((addr = eval(++args[optind])) == -1)
				error("\n");
		}
		else if(sp = symsrch(args[optind])) 
			addr = (long)sp->n_value;
		else if(isasymbol(args[optind]))
			error("%s not found in symbol table\n",args[optind]);
			else if((addr = strcon(args[optind],'h')) == -1)
				error("\n");
		pritrace(phys,addr,kfpset,proc);
	}
	else {
		if(args[optind]) {
			if((proc = strcon(args[optind],'d')) == -1)
				error("\n");
			if((proc > vbuf.v_proc) || (proc < 0))
				error("%d out of range\n",proc);
			prktrace(proc,kfpset);
		}
		else prktrace(proc,kfpset);
	}
}

/* print kernel trace */
int
prktrace(proc,kfpset)
int proc,kfpset;
{

	int panicstr;
	struct 	pcb	pcbuf;
	long	stklo, stkhi, pcbaddr;
	long	savefp,savesp,saveap,savepc;
	struct syment *symsrch();
	unsigned range;

	
	getublock(proc);
	if((proc == -1) || (proc == getcurproc())){
		seekmem((long)Panic->n_value,1,-1);
		if((read(mem,(char *)&panicstr,sizeof panicstr)
			!= sizeof panicstr))
				error("read error on panic string\n");
		if(panicstr == 0)
			error("information to process stack trace for current process not available\n");
		if(((long)ubp->u_stack <= UBADDR)  ||
			((long)ubp->u_stack >= USTKADDR))
			error("kernel stack not valid for current process\n");
		stklo = (long)ubp->u_stack;
		savesp = (long)xtranvram.systate.ofp;
		saveap = (long)xtranvram.systate.oap;
		savefp = (long)xtranvram.systate.ofp;
		savepc = (long)xtranvram.systate.opc;
		stkhi = savefp;
		if ( stkhi < stklo)
			error("upper bound < lower bound, unable to process stack\n") ;
		range = setbf(stkhi,stklo,proc);
		stklo = stkhi - range;	
	}
	else {
		pcbaddr = (long)ubp->u_pcbp;
		readmem((long)pcbaddr,1,proc,(char *)&pcbuf,sizeof pcbuf,"pcb");
		if(pcbuf.psw.CM == PS_USER) 
			error("user mode\n");
		if(procbuf.p_flag == SSYS) {
			stklo = (long)ubp->u_stack;
			stkhi = (long)pcbuf.regsave[K_SP];
		}
		else {
			stklo = (long)pcbuf.slb;
			stkhi = (long)pcbuf.sub;	
		}
		if ( stkhi < stklo)
			error("upper bound < lower bound, unable to process stack\n") ;
		range = setbf(stkhi,stklo,proc);
		stklo = stkhi - range;	
		if((procbuf.p_wchan == 0) && (procbuf.p_stat != SXBRK)) {
			/* proc did not go through sleep() */
			savesp = (long)pcbuf.regsave[K_SP];
			saveap = (long)pcbuf.regsave[K_AP];
			savepc = (long)pcbuf.regsave[K_PC];
			savefp = pcbuf.regsave[K_FP];
		}	
		else {			/* proc went through sleep() */
			savesp = (long)pcbuf.regsave[K_AP];
			saveap = stk_bptr[(((long)pcbuf.sp-sizeof(long))-stklo)/
				sizeof(long)];
			savepc = stk_bptr[(((long)pcbuf.sp-2*sizeof(long))-
				stklo)/ sizeof(long)];
			savefp = pcbuf.regsave[K_FP];
		}
	}
	if(kfpset) {
		if(Kfp)
			savefp = Kfp;
		else error("stack frame pointer not saved\n");
	}
	puttrace(stklo,savefp,savesp,saveap,savepc,kfpset);
}

/* print interrupt trace */
int
pritrace(phys,addr,kfpset,proc)
int phys,kfpset,proc;
long addr;
{
	struct kpcb kpcbuf;
	long stklo,stkhi,savefp,savesp,saveap,savepc;
	unsigned range;

	if(active)
		error("invalid trace of interrupt stack on running system\n");
	readbuf(addr,addr,phys,-1,(char *)&kpcbuf,sizeof kpcbuf,
		"interrupt process pcb");
	if(kfpset) {
		if(Kfp)
			savefp = Kfp;
		else error("stack frame pointer not saved\n");
	}
	else savefp = kpcbuf.regsave[FP];
	if(procbuf.p_flag == SSYS) {
		stklo = (long)ubp->u_stack;
		stkhi = (long)kpcbuf.regsave[K_SP];
	}
	else {
		stklo = (long)kpcbuf.slb;
		stkhi = (long)kpcbuf.sub;
	}
	if ( stkhi < stklo)
		error("upper bound < lower bound, unable to process stack\n") ;
	range = setbf(stkhi,stklo,proc);
	stklo = stkhi - range;	
	savepc = (long)kpcbuf.pc;
	savesp = (long)kpcbuf.sp;
	saveap = (long)kpcbuf.regsave[AP];
	puttrace(stklo,savefp,savesp,saveap,savepc,kfpset);
}

/* dump trace */
int
puttrace(stklo,savefp,savesp,saveap,savepc,kfpset)
long stklo,savefp,savesp,saveap,savepc;
int kfpset;
{
#define RET	(unsigned long)stk_bptr[(sfp - stklo) / sizeof (long) - 9]
#define OAP	(unsigned long)stk_bptr[(sfp - stklo) / sizeof (long) - 8]
#define OFP	(unsigned long)stk_bptr[(sfp - stklo) / sizeof (long) - 7]

	struct syment *func_nm;
	unsigned long sfp,sap,*argp;
	extern short N_TEXT;
	int noaptr = 0;
	static char tname[SYMNMLEN+1];
	char *name;
	
	if(kfpset) {
		sfp = savefp;
		fprintf(fp,"SET FRAMEPTR = %x\n\n",sfp);
		fprintf(fp,"STKADDR   FRAMEPTR   ARGPTR   FUNCTION\n");
	}
	else {
		sfp = savefp;
		sap = saveap;
		fprintf(fp,"STKADDR   FRAMEPTR   ARGPTR   FUNCTION\n");
	
		fprintf(fp,"%8.8x  %8.8x  %8.8x",savesp,savefp,sap);
		if((savepc == 0) ||
		   ((func_nm = findsym((unsigned long)savepc)) == 0))
				fprintf(fp,"  %8.8x",(long)savepc);
		else {
			if(func_nm -> n_zeroes) {
				strncpy(tname,func_nm->n_name,SYMNMLEN);
				name = tname;
			}
			else name = strtbl+func_nm -> n_offset;
			fprintf(fp,"  %-8.8s", name);
		}
		fprintf(fp,"(");
		argp=(unsigned long *)&stk_bptr[(sap-stklo)/sizeof(long)];
		if(argp < (unsigned long *)&stk_bptr[(sfp-stklo)/
			sizeof(long)-9])
			fprintf(fp,"%x",*argp++);
		while(argp < (unsigned long *)&stk_bptr[(sfp-stklo)/
			sizeof(long)-9])
			fprintf(fp,",%x",*argp++);
		fprintf(fp,")\n");
	}
	
	while((sfp > (stklo+36)) && (sfp <= savefp)){
		if(!RET)
			break;
		if(noaptr)
			error("next argument pointer, %x, not valid\n",sap);
		if((OFP < stklo) || (OFP > savefp))
			error("next stack frame pointer, %x, is out of range\n",
				OFP);
		fprintf(fp,"%8.8x  %8.8x  %8.8x",sap,OFP,OAP);
		
		if((func_nm = findsym((unsigned long)RET)) == 0)
			fprintf(fp,"  %8.8x", RET);
		else {
			if(func_nm -> n_zeroes) {
				strncpy(tname,func_nm->n_name,SYMNMLEN);
				name = tname;
			}
			else name = strtbl+func_nm -> n_offset;
			fprintf(fp,"  %-8.8s", name);
		}
		sap = OAP;
		sfp = OFP;
		fprintf(fp,"(");
		if((sap < stklo) || (sap > savefp))
			noaptr = 1;
		else {
			argp=(unsigned long *)&stk_bptr[(sap-stklo)/
				sizeof(long)];
			if(argp < (unsigned long *)&stk_bptr[(sfp-stklo)/
				sizeof(long)-9]) {
				fprintf(fp,"%x",*argp++);
				while(argp < (unsigned long *)&stk_bptr[(sfp-stklo)/
					sizeof(long)-9])
					fprintf(fp,",%x",*argp++);
			}
		}
		fprintf(fp,")\n");
	}
	free((char *)stk_bptr);
	stk_bptr = NULL;
}

/* get arguments for kfp function */
int
getkfp()
{
	int c;
	struct syment *sp;
	int reset = 0;
	int proc = Procslot;
	long value;

	optind = 1;
	while((c = getopt(argcnt,args,"w:s:r")) !=EOF) {
		switch(c) {
			case 'w' :	redirect();
					break;
			case 's' :	proc = setproc();
					break;
			case 'r' :	reset = 1;
					break;
			default  :	longjmp(syn,0);
		}
	}
	if(args[optind]) {
		if(*args[optind] == '(') {
			if((value = eval(++args[optind])) == -1)
				error("\n");
			prkfp(value,proc,reset);
		}
		else if(sp = symsrch(args[optind])) 
			prkfp((long)sp->n_value,proc,reset);
		else if(isasymbol(args[optind]))
			error("%s not found in symbol table\n",args[optind]);
		else {
			if((value = strcon(args[optind],'h')) == -1)
				error("\n");
			prkfp(value,proc,reset);
		}
	}
	else prkfp(-1,proc,reset);
}

/* print kfp */
int
prkfp(value,proc,reset)
long value;
int proc,reset;
{
	int panicstr;
	
	if(value != -1)
		Kfp = value;
	else if(reset) {
		getublock(proc);
		if((proc == -1) || (proc == getcurproc())){
			seekmem((long)Panic->n_value,1,proc);
			if((read(mem,(char *)&panicstr,sizeof panicstr) ==
				sizeof panicstr) && (panicstr != 0)
				&& ( xtranvram.systate.pcbp == UPCBADDR)) 
					Kfp = xtranvram.systate.lfp;
			else error("Kfp not available for current process on running system\n");
		}
		else {
			if(ubp->u_kpcb.psw.CM == PS_USER) 
				error("process in user mode, no valid Kfp for kernel stack\n");
			else Kfp = ubp->u_kpcb.regsave[FP] ;
		}
	}
	fprintf(fp,"kfp: %8.8x\n", Kfp);
}
