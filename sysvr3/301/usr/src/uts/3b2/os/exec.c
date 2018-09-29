/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:os/exec.c	10.19"

#include "sys/types.h"
#include "sys/param.h"
#include "sys/sysmacros.h"
#include "sys/sbd.h"
#include "sys/immu.h"
#include "sys/systm.h"
#include "sys/map.h"
#include "sys/fs/s5dir.h"
#include "sys/signal.h"
#include "sys/psw.h"
#include "sys/pcb.h"
#include "sys/user.h"
#include "sys/errno.h"
#include "sys/file.h"
#include "sys/buf.h"
#include "sys/inode.h"
#include "sys/fstyp.h"
#include "sys/acct.h"
#include "sys/sysinfo.h"
#include "sys/reg.h"
#include "sys/var.h"
#include "sys/region.h"
#include "sys/proc.h"
#include "sys/tuneable.h"
#include "sys/tty.h"
#include "sys/cmn_err.h"
#include "sys/debug.h"
#include "sys/message.h"
#include "sys/conf.h"

extern int	userstack[];
extern int	mau_present;
int		mappodflag = 0;
extern spath();

struct execa {
	char	*fname;
	char	**argp;
	char	**envp;
};

/*
 * exec system call, without and with environments.
 */

exec()
{
	((struct execa *)u.u_ap)->envp = NULL;
	exece();
}

exece()
{
	register reg_t   *rp;
	register preg_t  *prp;
	register inode_t *ip;
	int      vwinadr;
	int      newsp;
	unsigned size;

	unsigned shlb_scnsz;
	unsigned shlb_datsz;
	int      shlb_buf;
	struct   exdata *shlb_dat;
	char     exec_file[DIRSIZ];
	int      i;

	sysinfo.sysexec++;

	u.u_execsz = USIZE + SINCR + SSIZE + btoc(NCARGS-1);

	if (((ip = namei(upath,0)) == NULL) || (gethead(ip, &u.u_exdata)))
		return;

	/*
	 * Look at what we got, u.u_exdata.ux_mag = 410/411/413
	 *
	 *  410 is RO text (3B5 supports *only* this magic number)
	 *  411 is separated ID (3B5 treats this as 0410)
	 *  413 is RO text in an "aligned" a.out file
	 */
	switch (u.u_exdata.ux_mag) {
	    case 0410:
	    case 0411:
	    case 0413:
		break;
	    case 0443:
		u.u_error = ELIBEXEC;
		break;
	    default:
		u.u_error = ENOEXEC;
		break;
	}
	if (u.u_error) {
		iput(ip);
		return;
	}

	/* Clear defer-signal mask */
	u.u_procp->p_chold = 0;

	/*
	 *	Allocate memory to read the arguments, the shared library
	 *	section, and the amount of memory needed to store the inode
	 *	pointer and header data for each a.out.
	 */
	shlb_scnsz = (u.u_exdata.ux_lsize + NBPW) & (~(NBPW - 1));
	shlb_datsz = u.u_exdata.ux_nshlibs * sizeof(struct exdata);
	size = btoc( shlb_scnsz + shlb_datsz + 3*(NCARGS + NBPW) );

	if (availrmem - size < tune.t_minarmem
	  || availsmem - size < tune.t_minasmem) {
		nomemmsg("exece", size, 0, 0);
		u.u_error = EAGAIN;
		iput(ip);
		return;
	}
	availrmem -= size;
	availsmem -= size;

	while ((vwinadr = sptalloc(size, PG_P, 0, 0)) == NULL) {
		mapwant(sptmap)++;
		sleep(sptmap, PMEM);
	}

	/*	Locate and verify any needed shared libraries.
	 *
	 *	Note: ip is unlocked in getshlibs().
	 */

	bcopy((caddr_t)u.u_dent.d_name, (caddr_t)exec_file, DIRSIZ);

	if (u.u_exdata.ux_nshlibs) {
		shlb_buf = vwinadr + 3*(NCARGS + NBPW);
		shlb_dat = (struct exdata *)(shlb_buf + shlb_scnsz);

		if (getshlibs(shlb_buf, shlb_dat))
			goto done;
	} else {
		ip->i_flag |= ITEXT;
		prele(ip);
	}

	if (fuexarg(u.u_ap, vwinadr, &newsp)) {
		exec_err(shlb_dat, u.u_exdata.ux_nshlibs);
		goto done;
	}

	/*
	 * Remove the old process image
	 */

	shmexec();	/* insert calls for "execfunc"	*/
	punlock();	/* unlock locked regions before detaching */

	u.u_prof.pr_scale = 0;

	prp = u.u_procp->p_region;
	while (rp = prp->p_reg) {
		if (ip = rp->r_iptr)
			plock(ip);
		reglock(rp);
		detachreg(prp, &u);
	}
	u.u_dmm = 0;

	/*
	 * load any shared libraries that are needed
	 */

	if (u.u_exdata.ux_nshlibs) {
		for(i = 0; i < u.u_exdata.ux_nshlibs; i++, shlb_dat++) {
			if (getxfile(shlb_dat, PT_LIBTXT, PT_LIBDAT)) {
				exec_err(++shlb_dat, u.u_exdata.ux_nshlibs - i - 1);
				psignal(u.u_procp, SIGKILL);
				goto done;
			}
		}
	}

	/*
	 * load the a.out's text and data.
	 */

	if (getxfile(&u.u_exdata, PT_TEXT, PT_DATA)) {
		psignal(u.u_procp, SIGKILL);
		goto done;
	}

	/* SDT[2] table and size have been changed */
	loadmmu(u.u_procp, SCN2);

	/*
	 *	Set up the user's stack.
	 */

	if (stackbld(svtopde(vwinadr), newsp)) {
		psignal(u.u_procp, SIGKILL);
		goto done;
	}

	/*
	 * Remember file name for accounting.
	 */
	u.u_acflag &= ~AFORK;
	bcopy((caddr_t)exec_file, (caddr_t)u.u_comm, DIRSIZ);

	setregs();

	if (mau_present)
		mau_setup();

	if (mappodflag)
		mappod(u.u_procp);

done:

	sptfree(vwinadr, size, 1);
	availrmem += size;
	availsmem += size;

	return;
}

/*
 * Read the a.out headers.  There must be at least three sections,
 * and they must be .text, .data and .bss (although not necessarily
 * in that order).
 *
 * Possible magic numbers are 0410, 0411 (treated as 0410), and
 * 0413.  If there is no optional UNIX header then magic number
 * 0410 is assumed.
 */

#define	F_BM32ID  	0160000
#define	F_BM32B 	0020000
#define	F_BM32MAU	0040000
#define F_BM32RST	0010000 /* bit indicates a.out contains restore fix */

/*
 *   Common object file header
 */

/* f_magic (magic number)
 *
 *   NOTE:   For 3b-5, the old values of magic numbers
 *           will be in the optional header in the
 *           structure "aouthdr" (identical to old
 *           unix aouthdr).
 */

#define  M32MAGIC	0560

/* f_flags
 *
 *	F_EXEC  	file is executable (i.e. no unresolved 
 *	        	  externel references).
 *	F_AR16WR	this file created on AR16WR machine
 *	        	  (e.g. 11/70).
 *	F_AR32WR	this file created on AR32WR machine
 *	        	  (e.g. vax).
 *	F_AR32W		this file created on AR32W machine
 *	        	  (e.g. 3b,maxi).
 */
#define  F_EXEC		0000002
#define  F_AR16WR	0000200
#define  F_AR32WR	0000400
#define  F_AR32W	0001000

struct filehdr {
	ushort	f_magic;	
	ushort	f_nscns;	/* number of sections */
	long	f_timdat;	/* time & date stamp */
	long	f_symptr;	/* file pointer to symtab */
	long	f_nsyms;	/* number of symtab entries */
	ushort	f_opthdr;	/* sizeof(optional hdr) */
	ushort	f_flags;
};

/*
 *  Common object file section header
 */

/*
 *  s_name
 */
#define _TEXT ".text"
#define _DATA ".data"
#define _BSS  ".bss"
#define _LIB  ".lib"

/*
 * s_flags
 */
#define	STYP_TEXT	0x0020	/* section contains text only */
#define STYP_DATA	0x0040	/* section contains data only */
#define STYP_BSS	0x0080	/* section contains bss only  */
#define STYP_LIB	0x0800	/* section contains lib only  */

struct scnhdr {
	char	s_name[8];	/* section name */
	long	s_paddr;	/* physical address */
	long	s_vaddr;	/* virtual address */
	long	s_size;		/* section size */
	long	s_scnptr;	/* file ptr to raw	*/
				/* data for section	*/
	long	s_relptr;	/* file ptr to relocation */
	long	s_lnnoptr;	/* file ptr to line numbers */
	ushort	s_nreloc;	/* number of relocation	*/
				/* entries		*/
	ushort	s_nlnno;	/* number of line	*/
				/* number entries	*/
	long	s_flags;	/* flags */
};

/*
 * Common object file optional unix header
 */

struct aouthdr {
	short	o_magic;	/* magic number */
	short	o_stamp;	/* stamp */
	long	o_tsize;	/* text size */
	long	o_dsize;	/* data size */
	long	o_bsize;	/* bss size */
	long	o_entloc;	/* entry location */
	long	o_tstart;
	long	o_dstart;
};

gethead(ip, exec_data)
struct   inode  *ip;
register struct exdata *exec_data;
{
	struct filehdr filhdr;
	struct aouthdr aouthdr;
	struct scnhdr  scnhdr;
	int    opt_hdr = 0;
	int    scns    = 0;

	if (FS_ACCESS(ip, IOBJEXEC)
	    || (PTRACED(u.u_procp) && FS_ACCESS(ip, IREAD)))
		goto bad;

	/*
	 * First, read the file header
	 */

	u.u_base = (caddr_t) &filhdr;
	u.u_count = sizeof( filhdr );
	u.u_offset = 0;
	u.u_segflg = 1;

	FS_READI(ip);

	if ((u.u_count != 0) || (filhdr.f_magic != M32MAGIC)) {
		u.u_error = ENOEXEC;
		goto bad;
	}
	if (((filhdr.f_flags & F_BM32B) && !fast32b()) ||
	    ((filhdr.f_flags & F_BM32MAU) && !mau_present)) {
		u.u_error = EFAULT;
		goto bad;
	}
	if (!(filhdr.f_flags & F_BM32RST) && !fast32b()) {
		u.u_error = EFAULT;
		goto bad;
	}

	/*
	 * Next, read the optional unix header if present; if not,
	 * then we will assume the file is a 410.
	 */

	if (filhdr.f_opthdr >= sizeof(aouthdr)) {
		u.u_base = (caddr_t) & aouthdr;
		u.u_count = sizeof(aouthdr);

		FS_READI(ip);

		if (u.u_count != 0) {
			u.u_error = ENOEXEC;
			goto bad;
		}

		opt_hdr = 1;
		exec_data->ux_mag = aouthdr.o_magic;
		exec_data->ux_entloc = aouthdr.o_entloc;
	};

	/*
	 * Next, read the section headers; there had better be at
	 * least three: .text, .data and .bss. The shared library
	 * section is optional, initialize the number needed to 0.
	 */

	exec_data->ux_nshlibs = 0;

	u.u_offset = sizeof(filhdr) + filhdr.f_opthdr;

	while (filhdr.f_nscns-- > 0) {

		u.u_base = (caddr_t) &scnhdr;
		u.u_count = sizeof( scnhdr );

		FS_READI(ip);

		if (u.u_count != 0) {
			u.u_error = ENOEXEC;
			goto bad;
		}

		switch ((int) scnhdr.s_flags) {

		case STYP_TEXT:
			scns |= STYP_TEXT;

			if (!opt_hdr) {
				exec_data->ux_mag = 0410;
				exec_data->ux_entloc = scnhdr.s_vaddr;
			}

			exec_data->ux_txtorg = scnhdr.s_vaddr;
			exec_data->ux_toffset = scnhdr.s_scnptr;
			u.u_execsz += btoc((exec_data->ux_tsize = scnhdr.s_size) + PREFETCH);
			break;

		case STYP_DATA:
			scns |= STYP_DATA;
			exec_data->ux_datorg = scnhdr.s_vaddr;
			exec_data->ux_doffset = scnhdr.s_scnptr;
			u.u_execsz += btoc(exec_data->ux_dsize = scnhdr.s_size);
			break;

		case STYP_BSS:
			scns |= STYP_BSS;
			u.u_execsz += btoc(exec_data->ux_bsize = scnhdr.s_size);
			break;

		case STYP_LIB:
			++shlbinfo.shlblnks;

			if ((exec_data->ux_nshlibs = scnhdr.s_paddr) > shlbinfo.shlbs) {
				++shlbinfo.shlbovf;
				u.u_error = ELIBMAX;
				goto bad;
			}

			exec_data->ux_lsize = scnhdr.s_size;
			exec_data->ux_loffset = scnhdr.s_scnptr;
			break;
		}
	}

	if (scns != (STYP_TEXT|STYP_DATA|STYP_BSS)) {
		u.u_error = ENOEXEC;
		goto bad;
	}

	/*
 	 * Check total memory requirements (in clicks) for a new process
	 * against the available memory or upper limit of memory allowed.
	 */

	if (u.u_execsz > tune.t_maxumem) {
		u.u_error = ENOMEM;
		goto bad;
	}

	if (!(ip->i_flag & ITEXT) && ip->i_count != 1) {
		register struct file *fp;

		for (fp = file; fp < (struct file *)v.ve_file; fp++)
			if (fp->f_count && fp->f_inode == ip &&
			    (fp->f_flag&FWRITE)) {
				u.u_error = ETXTBSY;
				goto bad;
			}
	}

	exec_data->ip = ip;
	return(0);
bad:
	iput(ip);
	return(-1);
}

 
fuexarg(uap, vaddr, newsp)
struct execa		*uap;
int			vaddr;
int			*newsp;
{

	register char *cp = (char *)vaddr;
	register char **ap;
	register char *sp;
	register int *statep = (int *)(vaddr + NCARGS + 1*NBPW);
	register int ucp = (int)userstack;
	register int np = 0;
	int nc = 0;
	int c;
	int na = 0;
	int nac = 0;
	int env = 0;
	int *newp;

	/* collect arglist */

	ap = uap->argp;
	for (;;) {
		if (ap) {
			while (sp = (char *)fuword((caddr_t)ap)) {
				ap++;
				if (++np > (NCARGS/2)) {
					u.u_error = E2BIG;
					return(-1);
				}
				*statep++ = ucp;
				if ((c = upath(sp, cp, NCARGS-nc)) < 0) {
					if (c == -2)
						u.u_error = E2BIG;
					else
						u.u_error = EFAULT;
					return(-1);
				}
				nc += ++c;
				ucp += c;
				cp += c;
			}
		}
		*statep++ = NULL;	/* add terminator */
		if (!env++) {
			na = np;	/* save argc */
			nac = nc;
			ap = uap->envp; /* switch to env */
			continue;
		}
		break;
	}

	/*
	 * copy back arglist -- i.e.
	 *	set up argument and environment pointers
	 */
	
	nc = (nc+NBPW-1)&~(NBPW-1);
	sp = (char *) ((int)userstack + nc);
	u.u_ar0[AP] = (int)sp;
	*newsp = (int)sp + (np + 1 + 2) * NBPW;

	statep = (int *)(vaddr + NCARGS + 1*NBPW);
	newp = (int *)(vaddr + nc);
	*newp++ = na;		/*  argc  	*/

	bcopy(statep, newp, (np+2)*NBPW);	/* copy pointers down */

	/* do psargs */
	sp = u.u_psargs;
	cp = (char *)vaddr;
	np = min( nac, PSARGSZ-1);
	while (np--) {
		if ( (*sp = *cp++) == '\0' )
			*sp = ' ';
		sp++;
	}
	*sp = '\0';

	return(0);
}	


getxfile(exec_data, tpregtyp, dpregtyp)
struct exdata *exec_data;
short  tpregtyp;
short  dpregtyp;
{
	register reg_t	*rp;
	register preg_t	*prp;
	register struct inode  *ip = exec_data->ip;
	int doffset = exec_data->ux_doffset;
	int dbase   = exec_data->ux_datorg;
	int dsize   = exec_data->ux_dsize;
	int bsize   = exec_data->ux_bsize;
	int npgs;


	plock(ip);

	/*	Load text region.  Note that if xalloc returns
	 *	an error, then it has already done an iput.
	 */

	if ((struct inode *)xalloc(exec_data, tpregtyp) == NULL) 
		return(-1);

	/*
	 *	Allocate the data region.
	 */

	if ((rp = allocreg(ip, RT_PRIVATE, 0)) == NULL) {
		prele(ip);
		goto out;
	}

	/*
	 *	Attach the data region to this process.
	 */

	prp = attachreg(rp, &u, dbase & ~SOFFMASK, dpregtyp, SEG_RW);
	if (prp == NULL) {
		freereg(rp);
		goto out;
	}

	/*
	 * Load data region
	 */

	if (dsize) {
		switch (exec_data->ux_mag) {
		case 0413:
		case 0443:
			if (mapreg(prp, dbase, ip, doffset, dsize)) {
				detachreg(prp, &u);
				goto out;
			}
			break;
			/* fall thru case if REMOTE */
		default: 
			if (loadreg(prp, dbase,  ip, doffset, dsize)) {
				detachreg(prp, &u);
				goto out;
			}
		}
	}

	/*
	 * Allocate bss as demand zero
	 */
	if (npgs = btoc(dbase + dsize + bsize) - btoc(dbase + dsize)) {
		if (growreg(prp, npgs, DBD_DZERO) < 0) {
			detachreg(prp, &u);
			goto out;
		}
	}

	regrele(rp);

	/* set SUID/SGID protections, if no tracing */

	if (tpregtyp == PT_TEXT) {
		if (!PTRACED(u.u_procp)) {
			if (!FS_ACCESS(ip, ISUID))
				u.u_uid = ip->i_uid;
			if (!FS_ACCESS(ip, ISGID))
				u.u_gid = ip->i_gid;

			u.u_procp->p_suid = u.u_uid;
			u.u_procp->p_sgid = u.u_gid;
		}
		if (u.u_procp->p_flag & SSEXEC)
			u.u_procp->p_flag |= SPRSTOP;
		else if (u.u_procp->p_flag & STRC)
			psignal(u.u_procp, SIGTRAP);
	}

	iput(ip);
	return(0);
out:
	if (u.u_error == 0)
		u.u_error = ENOEXEC;

	plock(ip);
	iput(ip);

	return(-1);
}


getshlibs(bp, dat_start)
unsigned *bp;
struct   exdata *dat_start;
{
	struct   inode  *ip  = u.u_exdata.ip;
	struct   exdata *dat = dat_start;
	unsigned nlibs = u.u_exdata.ux_nshlibs;
	unsigned rem_wrds;
	unsigned n = 0;

	u.u_base = (caddr_t)bp;
	u.u_count = u.u_exdata.ux_lsize ;
	u.u_offset = u.u_exdata.ux_loffset ;
	u.u_segflg = 1;

	FS_READI(ip);

	if (u.u_count != 0) {
		iput(ip);
		return(-1);
	}

	ip->i_flag |= ITEXT;
	prele(ip);

	while ((bp < (unsigned int *)dat) && (n < nlibs)) {

		/* Check the validity of the shared lib entry. */

		if ((bp[0] > (rem_wrds = (unsigned int *)dat_start - bp)) ||
		    (bp[1] > rem_wrds) || (bp[0] < 3)) {
			u.u_error = ELIBSCN;
			goto bad;
		}

		/* Locate the shared lib and get its header info.  */

		u.u_syscall = DUEXEC;

		u.u_dirp = (caddr_t)(bp + bp[1]);
		bp += bp[0];

		if ((ip = namei(spath, 0)) == NULL) {
			u.u_error = ELIBACC;
			goto bad;
		}

		if (gethead(ip, dat)) {
			if (u.u_error == EACCES)
				u.u_error = ELIBACC;
			else if (u.u_error != ENOMEM)
				u.u_error = ELIBBAD;
			goto bad;
		}

		if (dat->ux_mag != 0443) {
			u.u_error = ELIBBAD;
			iput(ip);
			goto bad;
		}

		ip->i_flag |= ITEXT;
		prele(ip);

		++dat;
		++n;
	}

	if (n != nlibs) {
		u.u_error = ELIBSCN;
		goto bad;
	}

	return(0);
bad:
	exec_err(dat_start, n);
	return(-1);
}


/*
 * Build the user's stack.
 */

stackbld(stkptp, newsp)
pde_t	*stkptp;	/* Pointer to page table entries to be used to	*/
			/* initialize the stack with the exec arguments	*/
int	newsp;
{
	reg_t   *rp;
	preg_t  *prp;
	register int	nargc, t;
	register dbd_t	*dbd;
	register pde_t	*pt;

	/*	Allocate a region for the stack and attach it to
	 *	the process.
	 */

	if ((rp = allocreg(NULL, RT_PRIVATE, 0)) == NULL)
		return(-1);

	if ((prp = attachreg(rp, &u, userstack, PT_STACK, SEG_RW)) == NULL) {
		freereg(rp);
		return(-1);
	}
	
	/*	Grow the stack but don't actually allocate
	 *	any pages.
	 */
	nargc = btoc( newsp - (int)userstack );

	if (growreg(prp, SSIZE+nargc, DBD_DZERO) < 0) {
		detachreg(prp,&u);
		return(-1);
	}
	
	/*	Initialize the stack with the pages containing the
	 *	exec arguments.  We got these from exece().
	 */

	pt = rp->r_list[0];
	dbd = (dbd_t *)pt + NPGPT;

	for (t = nargc ; --t >= 0 ; pt++, stkptp++, dbd++) {
		pt->pgm.pg_pfn = stkptp->pgm.pg_pfn;
		pg_setvalid(pt);
		dbd->dbd_type = DBD_NONE;
		stkptp->pgi.pg_pde = 0;
	}

	rp->r_nvalid = nargc;
	loadmmu(u.u_procp, SCN3);
	u.u_pcb.sub = (int *)((uint)userstack + ctob(rp->r_pgsz));
	regrele(rp);

	u.u_ssize = rp->r_pgsz;

	u.u_ar0[SP] = newsp;
	u.u_ar0[FP] = newsp;

	bzero(newsp, ctob(btoc(newsp)) - newsp);

	return(0);
}


exec_err(shlb_data, n)
register struct exdata *shlb_data;
register int    n;
{
	for (; n > 0; --n, ++shlb_data) {
		plock(shlb_data->ip);
		iput(shlb_data->ip);
	}

	plock(u.u_exdata.ip);
	iput(u.u_exdata.ip);
}
