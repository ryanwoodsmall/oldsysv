/*	@(#)PDPmisc.c	1.3	*/
#include	"crash.h"
#include	"sys/inode.h"
#include	"sys/proc.h"
#include	"sys/file.h"
#include	"sys/mount.h"
#include	"sys/text.h"
#if vax
#define	SEGBYTES	0x40000000L	/* power of 2 to the 30th */
#define	CORECLKS	512		/* Core Clicks */
#define	BITMASK		0777		/* Remove low order bits */
#endif
#if pdp11
int	ovly;	/* if overlay-loaded system, ovly == 1 */
#define	SEGBYTES	8192
#define CORECLKS	64
#define	BITMASK		077
#endif

extern	struct	dstk	dmpstack;
extern	struct	glop	dumpglop;
extern	struct	uarea	x;
struct	exec	abuf;
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
	register  char  *s;
{
	register  base, numb;

	numb = 0;
	if(*s == '0')
		base = 8;
	else
		base = 10;
	while(*s) {
		if(*s < '0' || *s > '9') {
			error("number expected");
			return(-1);
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
	extern	struct	nlist	*stbl;
	extern  unsigned  symcnt;
	extern	struct	var	v;

	long	symloc;
	char	*sbrk();
	int	sigint();
	struct	nlist	*symsrch();

	if((mem = open(dumpfile, 0)) < 0)
		fatal("cannot open dump file");

#ifdef	pdp11
	if(read(mem, &dumpglop, sizeof dumpglop) != sizeof dumpglop)
		error("cannot read dump registers");
	Kfp = dumpglop.g_r5;
#endif

	nmfd = open(namelist, 0);
	if(read(nmfd, &abuf, sizeof abuf) != sizeof abuf)
		fatal("cannot read namelist file");

	if(BADMAG(abuf))
		fatal("namelist not in a.out format");

	symloc = (long)abuf.a_text + (long)abuf.a_data;
#ifdef	pdp11
	symloc += ((long)abuf.a_hitext<<16);
	ovly = (abuf.a_magic == A_MAGIC5);
	if (ovly) printf("** PDP11 Overlay System **\n");
#endif
#ifdef	vax
	symloc += (long) abuf.a_trsize + abuf.a_drsize;
#endif
#ifdef	pdp11
	if ((abuf.a_flag & 01) == 0)
		symloc *= 2;
#endif
	symcnt = abuf.a_syms / sizeof (struct nlist);
#ifdef DEBUG
	printf("symcnt: %d\n", symcnt);
#endif
	if((stbl = (struct nlist *)sbrk(abuf.a_syms)) == (struct nlist *)-1)
		fatal("cannot allocate space for namelist");
	if(lseek(nmfd, (long)symloc, 1) == -1) perror("lseek");
	if(read(nmfd, stbl, abuf.a_syms) != abuf.a_syms)
		fatal("cannot read namelist");
	Dmpstk = symsrch("dumpstack");
	Curproc = symsrch("_curproc");
	U = symsrch("_u");
	File = symsrch("_file");
	Inode = symsrch("_inode");
	Mount = symsrch("_mount");
	Proc = symsrch ("_proc");
	Text = symsrch("_text");
	Swap = symsrch("_swapmap");
	Buf = symsrch("_buf");
	Core = symsrch("_coremap");
	V = symsrch("_v");
	Sbrpte = symsrch("_sbrpte");
	Sys = symsrch("_utsname");
	Time = symsrch("_time");
	Lbolt = symsrch("_lbolt");
	Panic = symsrch("_panicstr");
	Etext = symsrch("_etext");
	End = symsrch("_end");
	Callout = symsrch("_callout");

#ifdef DEBUG
	printf("calling readmem: V = %lo\n", SYM_VALUE(V));
	printf("V: name=%s, value=%o, type=%d\n",
		V->n_name, V->n_value, V->n_type);
#endif
	if(readmem(&v, (long)SYM_VALUE(V), sizeof v) != sizeof v)
		error("read error on v structure");
	v.ve_inode = (char *)(((unsigned)v.ve_inode - Inode->n_value) /
		sizeof (struct inode));
	v.ve_file = (char *)(((unsigned)v.ve_file - File->n_value) /
		sizeof (struct file));
	v.ve_mount = (char *)(((unsigned)v.ve_mount - Mount->n_value) /
		sizeof (struct mount));
	v.ve_proc = (char *)(((unsigned)v.ve_proc - Proc->n_value) /
		sizeof (struct proc));
	v.ve_text = (char *)(((unsigned)v.ve_text - Text->n_value) /
		sizeof (struct text));

#ifdef	vax
	if(symsrch("_con_tty")) {
		if (!settty("dz")) 
			if (!settty("dzb"))
				printf("invalid tty structure\n");
	}
#endif
#ifdef	pdp11
	if(symsrch("_kl_tty"))
		settty("dh");
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
	int	sbrpte, ww, spte;
	int	curproc;
	int	upte[128];
	long	addr;

	if(slot == -1) {
		if(readmem((char *)&curproc, (long)SYM_VALUE(Curproc),
			sizeof curproc) == -1) {
				error("read error on curproc");
				return(-1);
		}
		slot = (curproc - Proc->n_value) / sizeof(struct proc);
	}
	if(readmem((char *)&pbuf, (long)SYM_VALUE(Proc)+slot*
		sizeof(struct proc), sizeof(pbuf)) == -1) {
			error("read error on proc table");
			return(-1);
	}

#ifdef	vax
	if(readmem((char *)&sbrpte, (long)(SYM_VALUE(Sbrpte)),
		sizeof sbrpte) == -1) {
			error("read error on sbrpte");
			return(-1);
	}
	sbrpte &= VIRT_MEM;

	if ((pbuf.p_flag& (SLOAD | SSPART)) == 0) {
		return(SWAPPED);
	} else {
		ww = (int)(pbuf.p_spt + (pbuf.p_nspt-1)*128);
		ww = sbrpte + ((ww&VIRT_MEM)>>9)*4;
		lseek(mem,(long)ww&VIRT_MEM,0);
		read(mem,&spte, sizeof(spte));
		lseek(mem,(spte<<9),0);
		read(mem, upte, sizeof upte);
		for(i=0; i<USIZE; i++) {
			lseek(mem, upte[128-USIZE+i]<<9, 0);
			/* get u page */
			if(read(mem,(char *)(((int *)&x.u)+128*i),512) != 512)
				return(-1);
		}
	}
#endif
#ifdef pdp11
	if (pbuf.p_flag&SLOAD) {
		addr = ctob((long)pbuf.p_addr);
	} else {
		return(SWAPPED);
	}
	lseek(mem, (long)addr, 0);
	if (read(mem, (char *)&x, USIZ) != USIZ)
		return(-1);
#endif

	return(0);
}
