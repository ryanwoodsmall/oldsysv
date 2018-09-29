/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/*	@(#)NSCmisc.c	1.1	*/
#include	"crash.h"
#include	"sys/inode.h"
#include	"sys/proc.h"
#include	"sys/file.h"
#include	"sys/mount.h"
#include	"sys/text.h"
#define	SEGBYTES	0x40000000L	/* power of 2 to the 30th */
#define	CORECLKS	512		/* Core Clicks */
#define	BITMASK		0777		/* Remove low order bits */

extern	struct	dstk	dmpstack;
extern	struct	glop	dumpglop;
extern	struct	uarea	x;
int	nmfd;

fatal(str)
	char	*str;
{
	printf("error: %s\n", str);
	exit(1);
}

error(str)
	char	*str;
{
	printf("error: %s\n", str);
}

atoi(s)
char  *s;
{
	register  base, numb;
	char *ssav;

	ssav = s;
	numb = 0;
	if(*s == '0')
		base = 8;
	else
		base = 10;
	while(*s) {
		if(*s < '0' || *s > '9') {
			numb = strtol(ssav, &s, 16);
			if (s == ssav) {
				error("number expected");
				return(-1);
			} else
				return(numb);
		}
		numb = numb * base + *s++ - '0';
	}
	return(numb);
}

init()
{
	extern	char	*dumpfile;
	extern	struct	dstk	dmpstack;
	extern	int	Kfp;
	extern	char	*namelist;
	extern	struct	syment	*stbl;
	extern  unsigned  symcnt;
	extern	struct	var	v;

	long	symloc;
#ifdef	ns32000
	struct syment *fploc;
#else
	long	dumploc;
#endif
	char	*sbrk();
	int	sigint();
	struct	syment	*symsrch();
	int	bufaddr;

	if((mem = open(dumpfile, 0)) < 0)
		fatal("cannot open dump file");

	nmfd = open(namelist, 0);

	rdsymtab();

	Dmpstk = symsrch("dumpstack");
	Curproc = symsrch("_curproc");
	U = symsrch("_u");
	File = symsrch("_file");
	Inode = symsrch("_inode");
	Mount = symsrch("_mount");
	Proc = symsrch ("_proc");
	Text = symsrch("_text");
	Swap = symsrch("_swapmap");
	Sbuf = symsrch("_sbuf");
	Core = symsrch("_coremap");
	V = symsrch("_v");
#ifndef	ns32000
	Sbrpte = symsrch("_sbrpte");
#else
	Kpte2 = symsrch("_kpte2");
#endif
	Sys = symsrch("_utsname");
	Time = symsrch("_time");
	Lbolt = symsrch("_lbolt");
	Panic = symsrch("_panicstr");
	Etext = symsrch("_etext");
	End = symsrch("_end");
	Callout = symsrch("_callout");
	Region = symsrch("_region");
#ifdef	DEBUG
	printf("SYMVALUE of Proc = 0x%x, SYMVALUE of Kpte2 = 0x%x\n",
		SYM_VALUE(Proc), SYM_VALUE(Kpte2));
	printf("SYMVALUE of V = 0x%x\n", SYM_VALUE(V));
#endif
	vaddrinit();

	if(readmem(&bufaddr, SYM_VALUE(Sbuf),
		sizeof bufaddr) != sizeof bufaddr) {
		Buf = symsrch("_buf");
		error("read error on _buf");
	} else {
		Sbuf->n_value = bufaddr;
#ifdef DEBUG
		printf("bufaddr = %x\n", bufaddr);
#endif
		Buf = Sbuf;
	}

	if(readmem(&v,(long)SYM_VALUE(V), sizeof v) != sizeof v)
		error("read error on v structure");
	v.ve_inode = (char *)(((unsigned)v.ve_inode - Inode->n_value) /
		sizeof (struct inode));
	v.ve_file = (char *)(((unsigned)v.ve_file - File->n_value) /
		sizeof (struct file));
	v.ve_mount = (char *)(((unsigned)v.ve_mount - Mount->n_value) /
		sizeof (struct mount));
	v.ve_proc = (char *)(((unsigned)v.ve_proc - Proc->n_value) /
		sizeof (struct proc));
#ifdef	DEBUG
	printf("v.ve_text = 0x%x, Text = 0x%x, diff. = 0x%x, num = %d\n",
		v.ve_text, Text->n_value, (unsigned)v.ve_text - Text->n_value,
		((unsigned)v.ve_text - Text->n_value) / sizeof (struct text));
#endif
	v.ve_text = (char *)(((unsigned)v.ve_text - Text->n_value) /
		sizeof (struct text));

	if(!symsrch("_con_tty")) {
#ifdef	ns32000
		if (!settty("sio"))
#else
		if (!settty("dz")) 
			if (!settty("dzb"))
#endif
				printf("invalid tty structure\n");
	}

#ifdef	vax
	dumploc=(SYM_VALUE(Dmpstk)-sizeof(struct dstk) + 4);
	if(readmem(&dmpstack,(long)dumploc,
		sizeof(dmpstack)) != sizeof(dmpstack)) {
			error("read error on dumpstack");
			return(-1);
	}
	Kfp = dmpstack.stkptr;
#endif
#ifdef	ns32000
	fploc = symsrch("intstack");	/* fp value is at intstack-36 */
	if(readmem(&Kfp, fploc->n_value-36, sizeof Kfp) != sizeof Kfp)
		Kfp = 0;
#endif

	signal(SIGINT, sigint);
}

/*
 *	Gather the uarea together and put it into the structure x.u
 *
 *	The particular uarea is selectable by the process table slot.
*/
getuarea(slot)
{
	struct	proc pbuf;
	int	i;
	int	curproc;
#ifndef	ns32000
	int	sbrpte, ww, spte;
	int	upte[128];
#else
	int	ww;
	struct {
		int pte, pte2;
	} udotpte[USIZE];
#endif
	long	addr;

	if(slot == -1) {
		if(readmem((char *)&curproc, (long)SYM_VALUE(Curproc),
			sizeof curproc) == -1) {
				error("read error on curproc");
				return(-1);
		}
		slot = (curproc - Proc->n_value) / sizeof(struct proc);
	}
	if(readmem((char *)&pbuf, SYM_VALUE(Proc)+slot*sizeof(struct proc),
		sizeof(pbuf)) != sizeof(pbuf)) {
			error("read error on proc table");
			return(-1);
	}

#ifndef	ns32000
	if(readmem((char *)&sbrpte, (long)(SYM_VALUE(Sbrpte)),
		sizeof sbrpte) != sizeof sbrpte) {
			error("read error on sbrpte");
			return(-1);
	}
	sbrpte &= VIRT_MEM;

	ww = (int)(pbuf.p_spt + (pbuf.p_nspt-1)*128);
	ww = sbrpte + ((ww&VIRT_MEM)>>9)*4;
	lseek(mem,(long)ww&VIRT_MEM,0);
	read(mem,&spte, sizeof(spte));
	lseek(mem,((spte&0x1fffff)<<9),0);
	read(mem, upte, sizeof upte);
	for(i=0; i<USIZE; i++) {
		lseek(mem, (upte[128-USIZE+i]&0x1fffff)<<9, 0);
		/* get u page */
		if(read(mem,(char *)(((int *)&x.u)+128*i),512) != 512)
			return(-1);
	}
#else
	ww = (pbuf.p_addr << 10) + 512;
	lseek(mem,(long)ww,0);
	read(mem,udotpte, sizeof(udotpte));
	for(i=0; i<USIZE; i++) {
		lseek(mem, udotpte[i].pte & 0xfffc00, 0);
		/* get u page */
		if(read(mem,(char *)(((int *)&x.u)+256*i),1024) != 1024)
			return(-1);
	}
#endif

	return(0);
}
