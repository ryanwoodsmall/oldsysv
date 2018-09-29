/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:com/setup.c	1.27"

/*
 *	UNIX debugger
 *
 * 		Everything here is MACHINE DEPENDENT
 *			and operating system dependent
 */

#include "head.h"
#include "coff.h"
#if u3b2
#include <sys/sbd.h>
#include <sys/utsname.h>
#endif
#if u3b2 || u3b5 || u3b15
#include <sys/sys3b.h>	/* for S3BFPHW definition	*/
#endif

extern MSG		BADMAGN;

INT	wtflag = 0;	/* set to allow writing to a.out, core, ISP */

static L_INT		txtsiz;
static L_INT		datsiz;
static L_INT		datbas;
static L_INT		stksiz;
static L_INT		symbas;		/*  not using ?? */
static L_INT		entrypt;

extern INT		argcount;
extern INT		signo;
extern INT		magic;
extern int		errno;

extern STRING		symfil;
extern STRING		corfil;

#define AOUTHDRSIZ	(filhdr.f_opthdr)

#define	maxfile		(1L<<24)

FILHDR	filhdr;		/* a.out file header */
AOUTHDR	aouthdr;	/* a.out Unix (optional) header */
SCNHDR	*scnhdrp;	/* pointer to first section header (space by sbrk) */
int		hcnt,txt,dat;

setsym()
{
	int	i;
#if DEBUG
	if (debugflag ==1)
	{
		enter1("setsym");
	}
	else if (debugflag == 2)
	{
		enter2("setsym");
		closeparen();
	}
#endif
	fsym = getfile(symfil,1);
	txtmap.ufd = fsym;
	errno = 0;
	if (read(fsym,&filhdr,FILHSZ) == FILHSZ	&&
	   ISMAGIC(filhdr.f_magic)		&&
	   AOUTHDRSIZ == sizeof aouthdr		&&
	   read(fsym, &aouthdr, AOUTHDRSIZ)==AOUTHDRSIZ)
	{
		magic=aouthdr.magic;
		if (magic==OMAGIC   || 	/* Writable text: private */
		    magic==PMAGIC   ||	/* Paging text: private.  */
		    magic==NMAGIC)	/* Readonly text: private */
		{
			txtsiz=aouthdr.tsize;
			datsiz=aouthdr.dsize + aouthdr.bsize;
			symbas=txtsiz+datsiz;
			hcnt = rdschdrs();
			for (i = 0 ; i < hcnt ; i++ )
			{
				if (strcmp(scnhdrp[i].s_name,".text")==0)
				{
					txt = i;
				}
				else if (strcmp(scnhdrp[i].s_name,".data")==0)
				{
					dat = i;
				}
			}

			switch (magic) {
			/*  use to have many more "magic" cases here */
			/*	assuming text is first section */

			case OMAGIC:	/* 0407 */
				txtmap.b1=scnhdrp[txt].s_vaddr;
				txtmap.e1=txtmap.b1 + symbas;
				txtmap.f1=scnhdrp[txt].s_scnptr;
				txtmap.b2=datbas=scnhdrp[dat].s_vaddr;
				txtmap.e2=symbas;
				txtmap.f2=txtmap.f1;
				break;

			case PMAGIC:	/* 0413 */
			case NMAGIC:	/* 0410 */
				txtmap.b1=scnhdrp[txt].s_vaddr;
				txtmap.e1=txtmap.b1 + txtsiz;
				txtmap.f1=scnhdrp[txt].s_scnptr;
				txtmap.b2=datbas=scnhdrp[dat].s_vaddr;
				txtmap.e2=datbas+datsiz;
				txtmap.f2=scnhdrp[dat].s_scnptr;
				break;

			}
			entrypt = aouthdr.entry;
			ststart = filhdr.f_symptr;
		}
		else {
			magic = 0;
			fprintf(FPRT1, "Warn: No magic for %s;\n", symfil);
		}
	}
	else {		/*  may be a ".o" file */
		if (ISMAGIC(filhdr.f_magic))
		{
			magic = filhdr.f_magic;
			hcnt = rdschdrs();
			for (i = 0 ; i < hcnt ; i++ )
			{
				if (strcmp(scnhdrp[i].s_name,".text")==0)
				{
					txt = i;
				}
				else if (strcmp(scnhdrp[i].s_name,".data")==0)
				{
					dat = i;
				}
			}
			txtsiz = scnhdrp[txt].s_size;
			datsiz = scnhdrp[dat].s_size;
			symbas = txtsiz+datsiz;
			txtmap.b1 = 0;
			txtmap.e1 = txtsiz;
			txtmap.f1 = scnhdrp[txt].s_scnptr;
			txtmap.b2 = datbas = scnhdrp[txt].s_paddr;
			txtmap.e2 = txtsiz+datsiz;
			txtmap.f2 = scnhdrp[dat].s_scnptr;
			entrypt = 0;
			ststart = filhdr.f_symptr;
		}
	}
	if (magic == 0)
	{
		txtmap.e1 = maxfile;
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("setsym");
		endofline();
	}
#endif
}

setcor()
{
#if u3b2
	struct utsname sysinfo;
#endif

#if DEBUG
	if (debugflag ==1)
	{
		enter1("setcor");
	}
	else if (debugflag == 2)
	{
		enter2("setcor");
		closeparen();
	}
#endif
	datmap.ufd = fcor = getfile(corfil,2);
	if(fcor < 0)
	{
#if DEBUG
		if (debugflag == 1)
		{
			exit1();
		}
		else if (debugflag == 2)
		{
			exit2("setcor");
			endofline();
		}
#endif
		return;
	}
	   /*  sure a core file */
	if (read(fcor, uu, XTOB(USIZE))==XTOB(USIZE)
	   && magic
#if !(u3b5 || u3b15 || u3b2)
	   && magic == ((struct user *)uu)->u_exdata.ux_mag
#endif
/*
** ANDF (((struct user *)uu)->u_pcb.pcb_ksp & 0xF0000000L)==0x80000000L removed
** ANDF (((struct user *)uu)->u_pcb.pcb_usp & 0xF0000000L)==0x70000000L	removed
*/
	   )
	{
#if vax || u3b
	/* ((struct user *)uu)->u_ar0 is an absolute address, currently
	   0x7fff ffb8 in the VAX, and 0x00a0 07b8 in the 3B-20.
	   It must be converted to an address relative to the beginning
	   of UBLOCK, which is done by subtracting the absolute address
	   of UBLOCK (ADDR_U, #defined in machdep.h) to get the offset
	   from the beginning of the user area, and then adding the
	   sdb internal addess of the user area (uu).		*/

	((struct user *)uu)->u_ar0 =
		(int *) ((char *)(((struct user *)uu)->u_ar0) -
			ADDR_U + (int)uu);

		signo = ((struct user *)uu)->u_arg[0]&037;
#else
#if u3b5 || u3b15 || u3b2
		signo = 9;	/* signal number unavailable */
#endif
#endif
		txtsiz = XTOB(((struct user *)uu)->u_tsize);
		datsiz = XTOB(((struct user *)uu)->u_dsize);
		stksiz = XTOB(((struct user *)uu)->u_ssize);
#if vax || u3b	/* really should do this for the 3b2, but it works this way */
		datbas=scnhdrp[dat].s_vaddr;
		datbas=datmap.b1=(magic==PMAGIC ? datbas & (~POFFMASK) : datbas);
#else
#if u3b2 && SWAPPING
		datbas=datmap.b1=(magic==PMAGIC ? datbas & (~0x1fff) : datbas);
#else
		datbas=datmap.b1=scnhdrp[dat].s_vaddr;
#endif
#endif
		datmap.e1=datmap.b1+(magic==OMAGIC?datsiz+txtsiz:datsiz);
		datmap.f1 = XTOB(USIZE);
#if u3b || u3b5 || u3b15 || u3b2
		datmap.b2 = USRSTACK;	/* loc of usr stack, in <sys/seg.h> */
		datmap.e2 = USRSTACK + stksiz;
#else
		/* VAX stack grows down from USRSTACK, in <sys/param.h> */
		/*  stksiz includes sizeof user area - must add back */
		datmap.b2 = USRSTACK + XTOB(USIZE) - stksiz;
		datmap.e2 = USRSTACK;
#endif
		datmap.f2 = XTOB(USIZE) + (magic==OMAGIC?datsiz+txtsiz:datsiz);
#if u3b5 || u3b15 || u3b2
		{
		unsigned int	uar0,file_adr;
		int i,j;

		uname(&sysinfo);
		/* copy registers from core dump into regvals */
		uar0 = (unsigned int) (((struct user *) uu)->u_ar0);
		for (i = 0; i < NUMREGS; i++)
			if (reglist[i].roffs == 0x80000000)
			{
				continue;
			}
			else if (strncmp(sysinfo.release,"2.0",3)!=0)
			{
				regvals[i] = ((int *)uu)[
				  (uar0 - UVUBLK)/WORDSIZE +
				  reglist[i].roffs];
			}
			else
			{
				file_adr = uar0 + WORDSIZE * reglist[i].roffs -
						datmap.b2 + datmap.f2;
				lseek(datmap.ufd,file_adr,0);
				read(datmap.ufd,&(regvals[i]),sizeof(long));
			}
		for (i = 0; i < 4; i++)
		{
			for (j = 0; j < 3; j++)
			{
				fpregvals[i*3+j] =
					((struct user *)uu)->u_mau.fpregs[i][j];
			}
		}
		asrval = ((struct user *)uu)->u_mau.asr;
		for (j = 0; j < 3; j++)
		{
			drval[j] = ((struct user *)uu)->u_mau.dr[j];
		}
		}
#endif

/* ??	signo = *(ADDR *)(((ADDR)uu)+XTOB(USIZE)-4*sizeof(int)); */
	      /*  put test up front, so don't need here
	      ** if (magic && magic != ((struct user *)uu)->u_exdata.ux_mag) {
	      **	fprintf(FPRT1, "%s: 0%o, 0%o\n",
	      **		BADMAGN, magic, 
	      **		((struct user *)uu)->u_exdata.ux_mag);
	      ** }
	      */
	}
	else		/*  e.g. /dev/kmem or any ordinary file */
	{
		datmap.e1 = datmap.e2 = maxfile;
		datmap.b1 = datmap.f1 = 0;
		datmap.b2 = datmap.f2 = 0;
		fprintf(FPRT1, "Warning: `%s' not a core file.\n", corfil);
		fakecor = 1;
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("setcor");
		endofline();
	}
#endif
}

create(f)
STRING	f;
{	int fd;
#if DEBUG
	if (debugflag ==1)
	{
		enter1("create");
	}
	else if (debugflag == 2)
	{
		enter2("create");
		arg("f");
		printf("\"%s\"",f);
		closeparen();
	}
#endif
	if ((fd=creat(f,0644)) >= 0) {
		close(fd);
		fd = open(f,wtflag);
	}
	else fd = -1;
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("create");
		printf("0x%x",fd);
		endofline();
	}
#endif
	return fd;
}

getfile(filnam,cnt)
STRING	filnam;
{
	REG INT		fd;

#if DEBUG
	if (debugflag ==1)
	{
		enter1("getfile");
	}
	else if (debugflag == 2)
	{
		enter2("getfile");
		arg("filnam");
		printf("\"%s\"",filnam);
		comma();
		arg("cnt");
		printf("0x%x",cnt);
		closeparen();
	}
#endif
	if (!eqstr("-",filnam))
	{
		fd=open(filnam,wtflag);
		if (fd<0 && argcount>cnt)
		{
			if (fd < 0)
			{
				fprintf(FPRT1, "cannot open `%s'\n", filnam);
			}
		}
	}
	else
	{
		fd = -1;
	}
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("getfile");
		printf("0x%x",fd);
		endofline();
	}
#endif
	return(fd);
}

/* rdschdrs(): read section headers */
rdschdrs()
{
	register unsigned nb;
	extern FILHDR filhdr;		/* a.out file header */

#if DEBUG
	if (debugflag ==1)
	{
		enter1("rdschdrs");
	}
	else if (debugflag == 2)
	{
		enter2("rdschdrs");
		closeparen();
	}
#endif
	nb = filhdr.f_nscns * SCNHSZ;
	scnhdrp = (SCNHDR *) sbrk(nb);
	if (read(fsym, scnhdrp, nb) != nb)
		fprintf(FPRT1, "Warn: section header read error\n");
	/* chkerr();	 does longjmp and haven't done setjmp yet */
#if DEBUG
	if (debugflag == 1)
	{
		exit1();
	}
	else if (debugflag == 2)
	{
		exit2("rdschdrs");
		printf("%d",filhdr.f_nscns);
		endofline();
	}
#endif
	return(filhdr.f_nscns);
}

#if u3b2 || u3b5 || u3b15

#ifndef SIG_ERR
#define SIG_ERR (int(*)()) -1
#endif

extern int	mauflag;
extern int	_fp_hw;		/* default: 0 == no hardware	*/
extern int	fpe();
void		sys3b();
static double	a,
		b,
		c;
setmauflg()
{
	int  old_sys();
	int illegal();

	if (signal(SIGSYS,old_sys) == SIG_ERR ) 
	{ 
		mauflag = -1;
		return;
	}
	_fp_hw = 0;

	sys3b( S3BFPHW, &_fp_hw ); /* query OS for HW status*/

	switch ( _fp_hw ) 
	{
	case	0:
		if (signal(SIGILL,illegal) == SIG_ERR ) 
		{ 
			mauflag = -1;
			break;
		}
		a = 1.0;
		b = 1.0;
		asm("	mfaddd3	a,b,c");
		if (c == 2.0) 
			mauflag = 1;
		break;

	case	1:
		if (signal(SIGILL,illegal) == SIG_ERR ) 
		{ 
			mauflag = 1;
			break;
		}
		a = 1.0;
		b = 1.0;
		asm("	mfaddd3	a,b,c");
		if (c == 2.0) 
			mauflag = 1;
		break;

	default	:
		mauflag = -1;
	}
}
old_sys()
{
	mauflag = -1;
	longjmp(env);
}

illegal()
{
	signal(SIGILL,fpe);	/* cancel pending signal , restore fpe */
	mauflag = -1;
	longjmp(env);
}
#endif
