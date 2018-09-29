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
/*	@(#)sysdef.c	1.12	*/
#include	"stdio.h"
#include	"sys/param.h"
#include	"sys/types.h"
#include	"sys/var.h"
#include	"a.out.h"
#ifndef	ns32000
#include	"sys/uba.h"
#endif
#include	"sys/flock.h"
#include	"sys/tuneable.h"

#ifdef	ns32000
#define	PAGESZ	1024
#else
#define	PAGESZ	512
#define	D15	0x78000000	/* From Unibus vector table in univec.c */
#define	USRMASK	0x3fffffff
#endif
#define	MAPSZ	4
#define	MAXI	200
#define	BUS	0340
#define	CC	0037
#ifdef	ns32000
#define	BLOCK	002
#else
#define	BLOCK	010
#endif
#define	INTR	0	/* l_nptr[0] for interrupt routines */
#define	ADDR	1	/* l_nptr[1] for device address     */
#define	CNT	2	/* l_nptr[2] for device count       */
#define	STRAT	3	/* l_nptr[3] for strategy if blkdev */

#define	INTSZ		sizeof (int)
#ifdef	ns32000
#define	SYM_VALUE(ptr)	(ptr->n_value)
#else
#define	SYM_VALUE(ptr)	(ptr->n_value & USRMASK)
#endif
#define	N_IFC		((start->n_value - IFC) / IFC_SZ)
#define	TXT		(HDR + aout.a_data)

struct	var	v;
#if vax || ns32000
struct sgs {
	struct filehdr filehdr;
	struct aouthdr aout;
	struct scnhdr scnhdr[3];
} sgs;
#define	HDR		sizeof (struct sgs)
#else
struct	exec	aout;
#define	HDR		sizeof (struct exec)
#endif
char	*os = "/unix";
char	*mr = "/etc/master";
char	line[256], nm[20], pre[20], intrnm[20],
	devaddr[20], devcnt[20], strat[20];
int	x, address, vsz, unit, bus, deft,
	count, typ, bcnt, maus, mcnt, ucnt, smem, mesg, sem,
	root, swap, dump, n_swap, power, pipe;
long	swap_low, dump_low;
int	IFC = 01000;	/* Start of Interface routines */
int	*vec, nexus;
int	bndry();
int	offset;
FILE	*system, *mast, *sysseek;

struct nlist	nl[MAXI], *nlptr, *setup(), *endnm, *start,
	*Messag, *Sema, *bdevcnt, *bdevsw,
	*rootdev, *swapdev, *dumpdev, *dmplo, *swplo, *nswap, *Shmem,
	*pipedev, *Power, *umemvad,
	*vs, *smap, *cmap, *callo, *restart,
	*tp, *FLCKinfo
#ifdef	ns32000
	;
#else
	, *Maus, *X25, *mbacnt, *UNIvec, *Mbacf, *Mbadev, *Ubacf, *ubacnt;
#endif

struct	link {
	char	l_cfnm[8];	/* config name from master table */
	struct	nlist	*l_nptr;/* ptr to name list structure */
	int	l_vsz;		/* vector size */
	int	l_def;		/* default unit count */
	int	l_dtype;		/* device type - BLOCK */
	int	l_bus;		/* bus request from master table */
	char	l_used;		/* set when device entry is printed */
} ln[MAXI/3], *lnptr, *setln(), *endln, *search(), *majsrch();

#ifndef	ns32000

struct	vector {
#ifdef	vax
	int	vaxvec;
#endif
#ifdef	pdp11
	int	v_pc;
	int	v_psw;
#endif
} vector, *addr;

struct mbacf {
	int	nexus;
	int	sbipri;
	int *	devptr;
};
struct mbacf mbacf;

int *mbadev;

#ifndef vax
#define MAXUBA	1
#endif
int ubacf[MAXUBA];

#endif

struct	bdev {
	int	b_open;
	int	b_close;
	int	b_strategy;
	int	b_tab;
} bent;

#ifdef	pdp11
struct	interface {	/* jsr r5, call; function */
	int	i_jsr;
	int	i_call;
	int	i_func;
} ifc[MAXI/3];

struct	xinterface {	/* jsr r5, call; jmp function */
	int	i_jsr;
	int	i_call;
	int	i_jmp;
	int	i_func;
} xifc[MAXI/3];

int	ovly = 0;	/* set to 1 if overlay system */
int	IFC_SZ = sizeof(ifc[0]);
#endif

#ifndef	ns32000
struct x25info{
	int	x25_0_0;
	int	x25_0_1;
	int	x25links;
	int	x25bufs;
	int	x25bytes;
} x25info;
#endif

tune_t tune;

struct flckinfo flckinfo;

main(argc, argv)
	int	argc;
	char	**argv;
{

	register int i;

	switch(argc) {
	case 3:	mr = argv[2];
	case 2: os = argv[1];
	case 1: break;
	default:
		fprintf(stderr, "usage: sysdef  [ /unix [ /etc/master ] ]");
		exit(1);
	}

#ifdef	pdp11
	offset = (-HDR);
#endif

	if((system = fopen(os,"r")) == NULL) {
		fprintf(stderr,"cannot read %s\n",os);
		exit(1);
	}

/* read in a.out header */
#if vax || ns32000
	if(fread(&sgs, sizeof sgs, 1, system) != 1) {
#else
	if(fread(&aout, sizeof aout, 1, system) != 1) {
#endif
		printf("cannot read a.out header\n");
		exit(1);
	}
#if vax || ns32000
	if((sgs.aout.magic != 0410) && (sgs.aout.magic != 0413)) {
#else
	if(BADMAG(aout)) {
#endif
		printf("invalid a.out format\n");
		exit(1);
	}
#if vax || ns32000
	offset = PAGESZ - (sgs.aout.tsize % PAGESZ) - HDR;
#endif
#ifdef pdp11
	ovly = (aout.a_magic == A_MAGIC5);
	if (ovly) printf ("* PDP11 overlay system\n");
#endif
	if((sysseek = fopen(os,"r")) == NULL) {
		fprintf(stderr,"cannot read %s\n",os);
		exit(1);
	}
	if((mast = fopen(mr, "r")) == NULL) {
		fprintf(stderr, "cannot read %s\n", mr);
		exit(1);
	}

	nlptr = nl;
	lnptr = ln;
	start = setup("start");
#ifndef	ns32000
	restart = setup("restart");
	UNIvec = setup("_UNIvec");
	Mbacf = setup("_mbacf");
	Mbadev = setup("_mbadev");
	Ubacf = setup("_ubacf");
#endif
	umemvad = setup("_umemvad");
	bdevcnt = setup("_bdevcnt");
#ifndef	ns32000
	mbacnt = setup("_mba_cnt");
	ubacnt = setup("_uba_cnt");
#endif
	bdevsw = setup("_bdevsw");
	rootdev = setup("_rootdev");
	pipedev = setup("_pipedev");
	swapdev = setup("_swapdev");
	swplo = setup("_swplo");
	nswap = setup("_nswap");
	dumpdev = setup("_dumpdev");
#ifdef	ns32000
	dmplo = setup("_dmplo");
#endif
	Power = setup("_pwr_clr");
	Sema = setup("_seminfo");
	Messag = setup("_msginfo");
#if vax || ns32000
	Shmem = setup("_shminfo");
#endif
#ifdef pdp11
	Maus = setup("_mausmap");
#endif
#ifndef	ns32000
	X25 = setup("_x25info");
#endif
	tp = setup("_tune");
	FLCKinfo = setup("_flckinfo");
	pre[0] = '_';

	while(fgets(line, 256, mast) != NULL) {
		if(line[0] == '*')
			continue;
		if(line[0] == '$')
			break;
		if (sscanf(line, "%s %d %d %o %s %d %o %d %d %d ",
			nm,&vsz,&x,&typ,&pre[1],&x,&x,&x,&deft,&bus,&x) != 10)
			continue;
		if(!strcmp(nm, "memory") || !strcmp(nm, "tty"))
			continue;
		strcpy(intrnm, pre);
		strcpy(devaddr, pre);
		strcpy(devcnt, pre);
		strcat(devaddr, "_addr");	/* _pre_addr */
		strcat(devcnt, "_cnt");	/* _pre_cnt */
		switch(vsz) {
#ifdef	ns32000
			case 2: strcat(intrnm, "rint"); break;
			case 1: strcat(intrnm, "intr"); break;
#else
			case 8: strcat(intrnm, "rint"); break;
			case 1: /* massbus devices */
			case 4: strcat(intrnm, "intr"); break;
#endif
			case 0: break; /* pseudo devices */
			default: continue;
		}
		setln(nm, setup(intrnm), vsz, deft, typ & BLOCK, bus);
		setup(devaddr);
		setup(devcnt);
		if(typ & BLOCK) {
			strcpy(strat, pre);
			strcat(strat, "strategy");
			setup(strat);
		}
	}
	endnm = setup("");
	endln = lnptr;
	if (nlist(os, nl) != 0) {
		printf("nlist error on %s\n", os);
		exit(1);
	}

#ifndef pdp11
#undef	n_name
#endif

#ifdef	DEBUG
/*
*	Print out the namelist
*/
	for(lnptr=ln; lnptr != endln; ++lnptr) {
		printf(NBPW==4 ? "%s\t%o\t%x\n" : "%s\t%o\t%o\n" ,
			(lnptr->l_nptr[INTR]).n_name,
			(lnptr->l_nptr[CNT]).n_type,
			(lnptr->l_nptr[CNT]).n_value);
		printf(NBPW==4 ? "%s\t%o\t%x\n" : "%s\t%o\t%o\n" ,
			(lnptr->l_nptr[ADDR]).n_name,
			(lnptr->l_nptr[ADDR]).n_type,
			(lnptr->l_nptr[ADDR]).n_value);
	}
#endif

	fseek(system, (long) SYM_VALUE(bdevcnt) - offset, 0);
	fread(&bcnt, INTSZ, 1, system);
#ifndef	ns32000
	fseek(system, (long) SYM_VALUE(mbacnt) - offset, 0);
	fread(&mcnt, INTSZ, 1, system);
	fseek(system, (long) SYM_VALUE(ubacnt) - offset, 0);
	fread(&ucnt, INTSZ, 1, system);
#endif
	fseek(system, (long) SYM_VALUE(rootdev) - offset, 0);
	fread(&root, INTSZ, 1, system);
	fseek(system, (long) SYM_VALUE(pipedev) - offset, 0);
	fread(&pipe, INTSZ, 1, system);
	fseek(system, (long) SYM_VALUE(swapdev) - offset, 0);
	fread(&swap, INTSZ, 1, system);
	fseek(system, (long) SYM_VALUE(dumpdev) - offset, 0);
	fread(&dump, INTSZ, 1, system);
	fseek(system, (long) SYM_VALUE(Power) - offset, 0);
	fread(&power, INTSZ, 1, system);
#ifndef	ns32000
	fseek(system, (long) SYM_VALUE(Maus) - offset, 0);
	fread(&maus, INTSZ, 1, system);
#endif

#ifdef	vax
	vaxdevs();
#endif
#ifdef	pdp11
	pdpdevs();
#endif
#ifdef	ns32000
	nscdevs();
#endif

	printf("*\n* Pseudo Devices\n*\n");
	pseudo_devs();

	/* rootdev, swapdev, pipedev, dumpdev, power */

	printf("*\n* System Devices\n*\n");

	if((lnptr = majsrch((root >> 8) & 0377)) == NULL)
		fprintf(stderr, "unknown root device\n");
	else
		printf("root\t%s\t%o\n", lnptr->l_cfnm, root & 0377);

	if((lnptr = majsrch((swap >> 8) & 0377)) == NULL)
		fprintf(stderr, "unknown swap device\n");
	else {
		printf("swap\t%s\t%o\t", lnptr->l_cfnm, swap & 0377);
		fseek(system, (long) SYM_VALUE(swplo) - offset, 0);
		fread(&swap_low, 4, 1, system);
		fseek(system, (long) SYM_VALUE(nswap) - offset, 0);
		fread(&n_swap, 2, 1, system);
		printf("%ld\t%d\n", swap_low, n_swap);
	}

	if((lnptr = majsrch((pipe >> 8) & 0377)) == NULL)
		fprintf(stderr, "unknown pipe device\n");
	else
		printf("pipe\t%s\t%o\n", lnptr->l_cfnm, pipe & 0377);

	if((lnptr = majsrch((dump >> 8) & 0377)) == NULL)
		fprintf(stderr, "unknown dump device\n");
	else {
#ifdef	ns32000
		printf("dump\t%s\t%o\t", lnptr->l_cfnm, dump & 0377);
		fseek(system, (long)SYM_VALUE(dmplo) - offset, 0);
		fread(&dump_low, INTSZ, 1, system);
		printf("%ld\n", dump_low);
#else
		printf("dump\t%s\t%o\n", lnptr->l_cfnm, dump & 0377);
#endif
	}

	/* easy stuff */

	nlptr = nl;
	vs = setup("_v");
	callo = setup("_callout");
	endnm = setup("");
	if (nlist(os, nl) != 0) {
		printf("nlist error on %s\n", os);
		exit(1);
	}
	for(nlptr = vs; nlptr != endnm; nlptr++) {
#if vax || ns32000
		if(nlptr->n_value == 0) {
#else
		if(nlptr->n_type == 0) {
#endif
			fprintf(stderr, "namelist error: %s\n", nlptr->n_name);
			exit(1);
		}
	}
	fseek(system, (long) SYM_VALUE(vs) - offset, 0);
	fread(&v, sizeof v, 1, system);
	printf("*\n* Tunable Parameters\n*\n");
	printf("buffers\t%11d\n", v.v_buf);
	printf("calls\t%11d\n", v.v_call);
	printf("inodes\t%11d\n", v.v_inode);
	printf("files\t%11d\n", v.v_file);
	printf("mounts\t%11d\n", v.v_mount);
	printf("procs\t%11d\n", v.v_proc);
	printf("clists\t%11d\n", v.v_clist);
	printf("sabufs\t%11d\n", v.v_sabuf);
	printf("maxproc\t%11d\n", v.v_maxup);
#ifdef	pdp11
	printf("coremap\t%11d\n", v.v_cmap);
#endif
	printf("hashbuf\t%11d\n", v.v_hbuf);
	printf("physbuf\t%11d\n", v.v_pbuf);
#ifdef	pdp11
	printf("iblocks\t%11d\n", v.v_iblk);
#endif
#ifndef	ns32000
	if(X25->n_value != 0) {
		fseek(system, (long) SYM_VALUE(X25) - offset, 0);
		fread(&x25info, sizeof(x25info), 1, system);
		printf("x25links%11d\n", x25info.x25links);
		printf("x25bufs\t%11d\n", x25info.x25bufs);
		printf("x25bytes  (%d*1024)\n",(x25info.x25bytes/1024));
	}
#endif
	printf("power\t%11d\n", power ? 1 : 0);
	printf("mesg\t%11d\n", Messag->n_value ? 1 : 0);
	printf("sema\t%11d\n", Sema->n_value ? 1 : 0);
#if vax || ns32000
	printf("shmem\t%11d\n", Shmem->n_value ? 1 : 0);
#endif
#ifdef	pdp11
	printf("maus\t%11d\n", (maus != -1) ? 1 : 0);
#endif

	/* File/Record Locking Parameters */
	if(FLCKinfo->n_value != 0) {
		fseek(system, (long) SYM_VALUE(FLCKinfo) - offset, 0);
		fread(&flckinfo, sizeof(flckinfo), 1, system);
		printf("flckfil\t%11d\n", flckinfo.fils);
		printf("flckrec\t%11d\n", flckinfo.recs);
	}

	/* Paging Parameters */
	if(tp->n_value != 0) {
		fseek(system, (long) SYM_VALUE(tp) - offset, 0);
		fread(&tune, sizeof(tune_t), 1, system);
		printf("nregion\t%11d\n", v.v_region);
		printf("nsptmap\t%11d\n", v.v_nsptmap);
		printf("vhndfrac%11d\n",  v.v_vhndfrac);
		printf("maxpmem\t%11d\n", v.v_maxpmem);
		printf("getpgslo%11d\n",  tune.t_gpgslo);
		printf("getpgshi%11d\n",  tune.t_gpgshi);
		printf("agerate\t%11d\n", tune.t_age);
		printf("vhandr\t%11d\n",  tune.t_vhandr);
		printf("vhandl\t%11d\n",  tune.t_vhandl);
		printf("maxsc\t%11d\n",   tune.t_maxsc);
		printf("prepage\t%11d\n", tune.t_prep);
	}
	exit(0);
}

/*
 * setup - add an entry to a namelist structure array
 */
struct	nlist	*
setup(nam)
	char	*nam;
{
	extern char *malloc();

	if (nlptr >= &nl[MAXI]) {
		fprintf(stderr, "internal name list overflow\n");
		exit(1);
	}
#if vax || ns32000
	if ((nlptr->n_name = malloc(strlen(nam)+1)) == NULL) {
		fprintf(stderr, "cannot allocate memory for name list\n");
		exit(1);
	}
#endif
			
	strcpy(nlptr->n_name, nam);
	nlptr->n_type = 0;
	nlptr->n_value = 0;
	return(nlptr++);	/* increment pointer */
}

/*
 * setln - set up internal link structure for later
 * function look-up.  Records useful information from the
 * /etc/master table description.
 */
struct	link	*
setln(cf, np, vz, dft, dt, br)
	char	*cf;
	struct	nlist	*np;
	int	vz;
	int	dft;
	int	dt;
	int	br;
{
	if(lnptr >= &ln[MAXI/3]) {
		fprintf(stderr, "internal link array overflow\n");
		exit(1);
	}
	strcpy(lnptr->l_cfnm, cf);
	lnptr->l_nptr = np;
	lnptr->l_vsz = vz;
	lnptr->l_def = dft;
	lnptr->l_dtype = dt;
	lnptr->l_bus = br;
	return(lnptr++);
}

/*
 * search - search for a function indirectly through
 * the link structure.
 */
struct	link	*
search(vctr,sub)
#if vax || ns32000
	int	vctr;
#else
	struct	interface	*vctr;
#endif
	int	sub;
{
	register struct	link	*lnkptr;
#ifdef pdp11
	struct xinterface xi;
	long	off;
#endif

	for(lnkptr = ln; lnkptr != endln; lnkptr++) {
#ifdef	vax
#ifdef	DEBUG
printf("link val=%x vctr=%x\n",(lnkptr->l_nptr[sub]).n_value, (vctr & ~D15));
#endif
		if((lnkptr->l_nptr[sub]).n_value == (vctr & ~D15)) {
#endif
#ifdef	ns32000
#ifdef	DEBUG
printf("link val=%x vctr=%x\n",(lnkptr->l_nptr[sub]).n_value, vctr);
#endif
		if((lnkptr->l_nptr[sub]).n_value == vctr) {
#endif
#ifdef	pdp11
#ifdef	DEBUG
printf("link val=%o vctr=%o\n",(lnkptr->l_nptr[sub]).n_value, vctr->i_func);
#endif
		if((lnkptr->l_nptr[sub]).n_value == (unsigned)vctr->i_func) {
#endif
			lnkptr->l_used = (char)1;
			return(lnkptr);
		}
	}
#ifdef pdp11
       /*
	* if overlay-loaded system, use interface table address to follow
	* chain into overlay segments and give search a second chance.
	*/
	if (ovly) {
#ifdef DEBUG
		printf("in overlay search\n");
#endif
		off = (unsigned) vctr->i_func;
		fseek(system, off + TXT, 0);
		fread(&xi, sizeof(xi), 1, system);
		for(lnkptr = ln; lnkptr != endln; lnkptr++) {
#ifdef	DEBUG
printf("link val=%o vctr=%o\n",(lnkptr->l_nptr[sub]).n_value, xi.i_func);
#endif
			if((lnkptr->l_nptr[sub]).n_value == (unsigned)xi.i_func-4) {
				lnkptr->l_used = (char)1;
				return(lnkptr);
			}
		}
	}
#endif
	return(NULL);
}

/*
 * majsrch - search for a link structure given the major
 * device number of the device in question.
 */
struct	link *
majsrch(maj)
	int	maj;
{
	register struct	link	*lnkptr;
#ifdef pdp11
	struct xinterface	xi;
	long	off;
#endif

	if(maj > bcnt - 1)
		return(NULL);

	fseek(system,(long)SYM_VALUE(bdevsw)+(maj*sizeof(bent))-offset,0);
	fread(&bent, sizeof bent, 1, system);

	for(lnkptr = ln; lnkptr != endln; lnkptr++)
		if(lnkptr->l_dtype) {
#ifdef	DEBUG
#ifdef	pdp11
printf("link val=%o bent=%o\n",(lnkptr->l_nptr[STRAT]).n_value, bent.b_strategy);
#endif
#if vax || ns32000
printf("link val=%x vctr=%x\n",(lnkptr->l_nptr[STRAT]).n_value, bent.b_strategy);
#endif
#endif
			if((lnkptr->l_nptr[STRAT]).n_value == bent.b_strategy)
				return(lnkptr);
		}
#ifdef pdp11
       /*
	* if overlay-loaded system, use interface table address to follow
	* chain into overlay segments and give search a second chance.
	*/
	if (ovly) {
#ifdef DEBUG
		printf("in overlay search\n");
#endif
		off = (unsigned) bent.b_strategy;
		fseek(system, off + TXT, 0);
		fread(&xi, sizeof(xi), 1, system);
		for(lnkptr = ln; lnkptr != endln; lnkptr++) {
#ifdef	DEBUG
printf("link val=%o vctr=%o\n",(lnkptr->l_nptr[STRAT]).n_value, xi.i_func);
#endif
			if((lnkptr->l_nptr[STRAT]).n_value == (unsigned)xi.i_func-4) {
				lnkptr->l_used = (char)1;
				return(lnkptr);
			}
		}
	}
#endif
	return(NULL);
}

#ifdef vax
vaxdevs()
{
	register int i;
	if(UNIvec->n_value == 0) {
		fprintf(stderr, "%s %s\n", "symbol \"UNIvec\" undefined; ",
			"invalid /unix file");
		exit(1);
	}
	if(Mbacf->n_scnum == 0) {
		fprintf(stderr, "%s %s\n", "symbol \"_mbacf\" undefined; ",
			"invalid /unix file");
		exit(1);
	}
	if(Mbadev->n_scnum == 0) {
		fprintf(stderr, "%s %s\n", "symbol \"_mbadev\" undefined; ",
			"invalid /unix file");
		exit(1);
	}
	if(Ubacf->n_scnum == 0) {
		fprintf(stderr, "%s %s\n", "symbol \"_ubacf\" undefined; ",
			"invalid /unix file");
		exit(1);
	}

	/* get mbadev, the offsets for the device pointers */
	if ((mbadev=(int *)calloc(mcnt, sizeof(int))) == NULL) {
		fprintf(stderr, "sysdef: cannot allocate memory for mbadev\n");
		exit(1);
	}
	if(fseek(system, (long)SYM_VALUE(Mbadev) - offset, 0) != NULL) {
		fprintf(stderr, "sysdef: seek failed on %s\n",system);
		exit(1);
	}
	fread(mbadev, sizeof(int), mcnt, system);

	printf("*\n* MASSBUS\n* dev\tnexus\taddr\tbus\n*\n");

/* MASSBUS devices from _mbacf */
	if(fseek(system, (long)SYM_VALUE(Mbacf) - offset, 0) != NULL) {
		fprintf(stderr, "sysdef: seek failed on %s\n",system);
		exit(1);
	}

	for(i=0; i < mcnt; ++i) {
		fread(&mbacf, sizeof mbacf, 1, system);
		if(mbacf.nexus == NULL) continue;
		if((lnptr = search(&mbacf.devptr[-mbadev[i]],ADDR)) == NULL) {
			fprintf(stderr,
				"unknown device at NEXUS %3d\n", mbacf.nexus);
			continue;
		}

		printf("%s\t%3d\t%d\t%d",
			lnptr->l_cfnm,mbacf.nexus,0,mbacf.sbipri);
		printf("\n");
	}

/* on a VAX the following works only for UNIBUS devices */
/* UNIBUS nexus numbers from _ubacf */
	if(fseek(system, (long)SYM_VALUE(Ubacf) - offset, 0) != NULL) {
		fprintf(stderr, "sysdef: seek failed on %s\n",system);
		exit(1);
	}

	for(i=0; i < ucnt; ++i)
		fread(&ubacf[i], sizeof(ubacf[0]), 1, system);

	if(fseek(system, (long)SYM_VALUE(UNIvec) - offset, 0) != NULL) {
		fprintf(stderr, "sysdef: seek failed on %s\n",system);
		exit(1);
	}

	printf("*\n* UNIBUS\n* dev\tvector\taddr\tbus\tcount\tnexus\n*\n");

	for(vec=0; vec < (int *)(INTSZ * 128); ++vec) {
		fread(&vector, sizeof vector, 1, system);
		if(vector.vaxvec == NULL) continue;
		if((lnptr = search(vector.vaxvec,INTR)) == NULL) {
			fprintf(stderr,"unknown device at vector %3o\n", vec);
			continue;
		}
		printf("%s\t%3o", lnptr->l_cfnm, vec);
		unit = (vector.vaxvec & D15) >> 27;
		if(fseek(sysseek,
			(long)(((lnptr->l_nptr[ADDR]).n_value & USRMASK)
			+ (INTSZ * unit) - offset), 0) != NULL) {
			fprintf(stderr,"bad seek for device address\n");
		}
		if(fread(&address, sizeof(address), 1, sysseek) != 1) {
			fprintf(stderr,"cannot read dev_addr\n");
		}

		printf("\t%lo\t%1o",(UBADEV(address)), lnptr->l_bus);

		fseek(sysseek, (long)(lnptr->l_nptr[CNT].n_value & USRMASK)
			- ftell(sysseek) - offset, 1);
		fread(&count, INTSZ, 1, sysseek);
		if((unit + 1) * lnptr->l_def > count)
			printf("\t%d", count % lnptr->l_def);
		else
			printf("\t%d", lnptr->l_def);
		printf("\t%d", ubacf[UBANUM(address)]);
		printf("\n");

		if(lnptr->l_vsz == 8) {
			vec++;
			fread(&vector, sizeof vector, 1, system);
		}
	}
}
#endif

#ifdef	pdp11
pdpdevs()
{
	register int i;
	if(start->n_type == 0) {
		fprintf(stderr, "%s %s\n",
			"symbol \"start\" undefined; ",
			"invalid /unix file");
		exit(1);
	}
	if(restart->n_value) {
		IFC = 01020;
		IFC_SZ = sizeof(xifc[0]);
		if((start->n_value - IFC) > (MAXI / 3 * (sizeof ifc[0]))) {
			printf("internal interface array overflow\n");
			exit(1);
		}
		fseek(system, (long) IFC + TXT, 0);
		fread(xifc, sizeof xifc[0], N_IFC, system);
		for(i=0; i < N_IFC; ++i) {
			ifc[i].i_jsr = xifc[i].i_jsr;
			ifc[i].i_call = xifc[i].i_call;
			ifc[i].i_func = (xifc[i].i_func + IFC + 
				( i + 1) * 8);
		}
	}
	else {
		if((start->n_value - IFC) > (MAXI / 3 * (sizeof ifc[0]))) {
			fprintf(stderr, "internal interface array overflow\n");
			exit(1);
		}
		fseek(system, (long) IFC - offset, 0);
		fread(ifc, sizeof ifc[0], (start->n_value - IFC) /
			sizeof ifc[0], system);
	}

	printf("*\n* dev\tvector\taddr\tbus\tcount\n*\n");
	for(addr = (struct vector *)0; addr != (struct vector *)01000; addr++) {
		if(addr == (struct vector *)060)
			addr = (struct vector *)070;	/* skips console */
		fseek(system, (long)addr - offset, 0);
		fread(&vector, sizeof vector, 1, system);
		if((vector.v_pc <= IFC) || (vector.v_pc >= start->n_value))
			continue;	/* skips clio, traps, jmp, etc */
		if((lnptr = search(&ifc[(vector.v_pc - IFC) / IFC_SZ],INTR))
			== NULL) {
			fprintf(stderr,
				"unknown device interrupts at vector %3o\n",
				addr);
			continue;
		}
		printf("%s\t%3o", lnptr->l_cfnm, addr);
		unit = vector.v_psw & CC;
		bus = (vector.v_psw & BUS) >> 5;
		fseek(system,(long)(lnptr->l_nptr[ADDR].n_value+2*unit-offset),0);
		fread(&address, 2, 1, system);
		printf("\t%6o\t%1o", address, bus);
		fseek(system, (long)(lnptr->l_nptr[CNT].n_value - offset), 0);
		fread(&count, NBPW, 1, system);
		if((unit + 1) * lnptr->l_def > count)
			printf("\t%d", count % lnptr->l_def);
		else
			printf("\t%d", lnptr->l_def);
		printf("\n");
		if(lnptr->l_vsz == 8)
			addr++;
	}
}
#endif

#ifdef	ns32000
nscdevs()
{
	register struct link *linkptr;
	long dvaddr;

	printf("*\n* dev\tvector\taddr\tbus\tcount\n*\n");
	for (linkptr = ln; linkptr < endln; linkptr++) {
		if(strcmp(linkptr->l_cfnm,"dc") == 0) {
			fseek(system,(linkptr->l_nptr[ADDR]).n_value-offset,0);
			fread(&dvaddr, sizeof dvaddr, 1, system);
			printf("%s\t%d\t%x\t%d\n",linkptr->l_cfnm,
				3, dvaddr, linkptr->l_bus);
		linkptr->l_used = (char) 1;
		}
	}
	for (linkptr = ln; linkptr < endln; linkptr++) {
		if(strcmp(linkptr->l_cfnm,"tc") == 0) {
			fseek(system,(linkptr->l_nptr[ADDR]).n_value-offset,0);
			fread(&dvaddr, sizeof dvaddr, 1, system);
			printf("%s\t%d\t%x\t%d\n",linkptr->l_cfnm,
				4, dvaddr, linkptr->l_bus);
		linkptr->l_used = (char) 1;
		}
	}
	for (linkptr = ln; linkptr < endln; linkptr++) {
		if(strcmp(linkptr->l_cfnm,"lp") == 0) {
			fseek(system,(linkptr->l_nptr[ADDR]).n_value-offset,0);
			fread(&dvaddr, sizeof dvaddr, 1, system);
			printf("%s\t%d\t%x\t%d\n",linkptr->l_cfnm,
				12, dvaddr, linkptr->l_bus);
		linkptr->l_used = (char) 1;
		}
	}
	for (linkptr = ln; linkptr < endln; linkptr++) {
		if(strcmp(linkptr->l_cfnm,"sio") == 0) {
			fseek(system,(linkptr->l_nptr[ADDR]).n_value-offset,0);
			fread(&dvaddr, sizeof dvaddr, 1, system);
			printf("%s\t%d\t%x\t%d\n",linkptr->l_cfnm,
				14, dvaddr, linkptr->l_bus);
		linkptr->l_used = (char) 1;
		}
	}
}
#endif

pseudo_devs()
{
	register struct link	*lnkptr;
	int			count;

	for(lnkptr = ln; lnkptr != endln; lnkptr++) {
		if( ! (lnkptr->l_nptr[CNT]).n_value) {  /* dev_cnt undefined*/
			continue;
		}
		if(lnkptr->l_used) {	/*has already been printed*/
			continue;
		}
#ifdef	ns32000
		if(strcmp(lnkptr->l_cfnm,"con") == 0) { /* /dev/console */
#else
		if(strcmp(lnkptr->l_cfnm,"dl11") == 0) { /* /dev/console */
#endif
			continue;
		}

		fseek(system, (long)
#ifdef	vax
			((lnkptr->l_nptr[CNT]).n_value & USRMASK) - offset, 0);
#else
			(lnkptr->l_nptr[CNT]).n_value - offset, 0);
#endif
		fread(&count, INTSZ, 1, system);
		printf("%s\t%d\t%d\t%d\t%d\n",lnkptr->l_cfnm,0,0,0,count);
	}
}
