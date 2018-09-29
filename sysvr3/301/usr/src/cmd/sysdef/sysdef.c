/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sysdef-3b2:sysdef.c	1.16"
#include	<stdio.h>
#include	<a.out.h>
/*  Fix known problem with a.out.h internal inconsistency  */
#undef n_name
#include	<sys/types.h>
#include	<sys/sysmacros.h>
#include	<sys/sys3b.h>
#include	<sys/var.h>
#include	<sys/tuneable.h>
#include	<sys/ipc.h>
#include	<sys/msg.h>
#include	<sys/sem.h>
#include	<sys/shm.h>
#include	<sys/fcntl.h>
#include	<sys/flock.h>
#include	<sys/utsname.h>
#include	<sys/conf.h>
#include	<sys/stat.h>
#include	<sys/dir.h>
#include	<ldfcn.h>
#include	<ctype.h>

#ifdef u3b5
#include	<sys/mmu.h>
#define	ONLBE(d)	((d)&0x30)
#endif

#ifdef u3b2
#include	<sys/param.h>
#define ONLBE(d)	(0)
#define KVIOBASE	0x60000
#endif

extern char *ctime();
extern char *strcat();
extern char *strcpy();
extern char *strncpy();
extern char *strncat();
extern char *malloc();

#define	SYM_VALUE(ptr)	(ptr->n_value)

#define MEMSEEK(nl)	fseek(sysfile, (long) SYM_VALUE(nl) - offset, 0)
#define MEMREAD(var)	fread((char*)&var, sizeof(var), 1, sysfile)

#define	MAXI	300

struct	s3bboot	bootname;
struct	var	v;
struct  tune	tune;
struct	msginfo	minfo;
struct	seminfo	sinfo;
struct	shminfo	shinfo;
struct	flckinfo	flinfo;
				/*/
				/* if F_EXEC is set in /unix filehdr, then it is  */
int		absolute;	/* an absolute boot file; the device information
				/* can be extracted
				/*/
char	*os = "/unix";
char	*mr = "/etc/master.d";
char	line[256], flag[8], pre[8], pre_addr[20], rtn[20];
#ifdef  u3b5
struct	mmuseg	addr;
#endif
#ifdef  u3b2
long	addr;
#endif
int	nsp;
int	nmuxlink, nstrpush, nstrevent, maxsepgcnt, strmsgsz, strctlsz;
int	nadvertise, nrcvd, nrduser, nsndd, minserve, maxserve, maxgdp,
	rfsize, rfs_vhigh, rfs_vlow, nsrmount;
char	strlofrac, strmedfrac;
dev_t	root, swap, dump, pipe;
char	MAJ[128];
daddr_t	spl;
int	offset;
FILE	*sysfile, *mast, *mastf;
struct	scnhdr  scn;
LDFILE	*ldptr;

void	setln();

struct nlist	nl[MAXI], *nlptr, *setup(), *endnm,
	*rootdev, *swapdev, *dumpdev, *swplo, *nswap, *MAJOR, *sys3bboot,
	*pipedev, *vs, *tu, *msginfo, *seminfo,
	*shminfo, *FLckinfo, *utsnm, *bdev,
	*pnmuxlink, *pnstrpush, *pnstrevent, *pmaxsepgcnt, *pstrlofrac,
	*pstrmedfrac, *pstrmsgsz, *pstrctlsz, *pmaxgdp,
	*pnadvertise, *pnrcvd, *pnrduser, *pnsndd, *pminserve,
	*pmaxserve, *prfsize, *prfs_vhigh, *prfs_vlow, *pnsrmount;

#define ADDR	0	/* l_func[0] for _addr array */
#define OPEN	1	/* l_func[1] for open routine */
#define CLOSE	2	/* l_func[2] for close routine */
#define READ	3	/* l_func[3] for read routine */
#define WRITE	4	/* l_func[4] for write routine */
#define IOCTL	5	/* l_func[5] for ioctl routine */
#define STRAT	6	/* l_func[6] for strategy routine */

#define	MAXL	MAXI/7+10

struct	link {
	char	l_cfnm[DIRSIZ+1];	/* config name from master table */
	struct	nlist	*l_func;/* ptr to name list structure */
	unsigned int l_soft :1;	/* software driver flag from master table */
	unsigned int l_dtype:1;	/* set if block device */
	unsigned int l_used :1;	/* set when device entry is printed */
} ln[MAXL], *lnptr, *endln, *majsrch();



main(argc, argv)
	int	argc;
	char	**argv;
{

	struct stat mfbuf;
	char mfname[256];
	struct	utsname utsname;
	struct	direct fname;

	switch(argc) {
	case 3:	mr = argv[2];
	case 2: os = argv[1];
	case 1: break;
	default:
		fprintf(stderr, "usage: %s  [ /unix [ /etc/master ] ]\n", argv[0]);
		exit(1);
	}

	if((sysfile = fopen(os,"r")) == NULL) {
		fprintf(stderr,"cannot open %s\n",os);
		exit(1);
	}

	if((ldptr = ldopen(os,NULL)) == NULL) {
		fprintf(stderr,"cannot open %s\n",os);
		exit(1);
	}

	if(HEADER(ldptr).f_magic!=FBOMAGIC) {
		fprintf(stderr,"invalid a.out format\n");
		exit(1);
	}
	absolute = HEADER(ldptr).f_flags & F_EXEC;
	stat(mr,&mfbuf);
	if ((mfbuf.st_mode & S_IFMT) != S_IFDIR)
	{
		fprintf(stderr," %s not a directory\n",mr);
		exit(1);
	}
	if((mast = fopen(mr, "r")) == NULL) {
		fprintf(stderr, "cannot open %s\n", mr);
		exit(1);
	}
	ldnshread(ldptr,".data",&scn);
	offset = scn.s_vaddr - scn.s_scnptr;

	uname(&utsname);
	printf("*\n* %s Configuration\n*\n",utsname.machine);

	nlptr = nl;
	lnptr = ln;
	if ( absolute ) {
		bdev = setup("bdevsw");
		rootdev = setup("rootdev");
		pipedev = setup("pipedev");
		swapdev = setup("swapdev");
		swplo = setup("swplo");
		nswap = setup("nswap");
		dumpdev = setup("dumpdev");
		MAJOR = setup("MAJOR");
		sys3bboot = setup("sys3bboot");

		while(fread(&fname,sizeof(fname),1,mast) != NULL) {
			if (fname.d_ino == 0)
				continue;
			strcpy(mfname,mr);
			strcat(mfname,"/");
			strncat(mfname,fname.d_name,DIRSIZ);
			stat(mfname,&mfbuf);
			if ((mfbuf.st_mode & S_IFMT) != S_IFREG)
				continue;
			if ((mastf = fopen(mfname,"r")) == NULL) {
				fprintf(stderr,"cannot open %s\n",mfname);
				exit(1);
			}
			while(fgets(line, sizeof(line), mastf) != NULL) {
				if(line[0] == '$' && line[1] == '$')
					break;
				if(line[0] == '*' || line[0]=='\t' || line[0]==' ' || line[0]=='\n')
					continue;
				if( sscanf(line, " %s   %*s %s    ",
					     	flag,    pre) < 1)
					break;
				if (test(flag,'x'))
					break;
				strcat(strcpy(pre_addr,pre),"_addr");
				setln(fname.d_name, setup(pre_addr), test(flag,'b'), test(flag,'s') );
				setup(strcat(strcpy(rtn,pre),"open"));
				setup(strcat(strcpy(rtn,pre),"close"));
				setup(strcat(strcpy(rtn,pre),"read"));
				setup(strcat(strcpy(rtn,pre),"write"));
				setup(strcat(strcpy(rtn,pre),"ioctl"));
				setup(strcat(strcpy(rtn,pre),"strategy"));
				break;
			}
			if(fclose(mastf) != 0){
				fprintf(stderr,"cannot close %s\n",mfname);
				exit(1);
			}
		}
		endnm = setup("");
		endln = lnptr;
		nlist(os, nl);

		/* boot program name */

		if (sys3bboot->n_value == 0)
			printf("\n  Boot program: *unknown*\n" );
		else
			{
			MEMSEEK(sys3bboot);	MEMREAD(bootname);
			printf("\n  Boot program: /%s", bootname.path[0]=='/'? bootname.path+1 : bootname.path );
			printf("\n  Time stamp:   %s", ctime(&bootname.timestamp) );
			}

		/* rootdev, swapdev, pipedev, dumpdev */

		MEMSEEK(rootdev);	MEMREAD(root);
		MEMSEEK(pipedev);	MEMREAD(pipe);
		MEMSEEK(swapdev);	MEMREAD(swap);
		MEMSEEK(dumpdev);	MEMREAD(dump);

		printf("*\n* Devices\n*\n");
		devices();

		printf("*\n* Loadable Objects\n*\n");
		edtsect();

		printf("*\n* System Configuration\n*\n");

		MEMSEEK(MAJOR);
		fread((char *)MAJ, sizeof(MAJ), 1, sysfile);
		sysdev();
	}
	else
	{
		fprintf(stderr," %s: not an absolute boot file\n",os);
		exit(1);
	}

	/* easy stuff */
	printf("*\n* Tunable Parameters\n*\n");
	nlptr = nl;
	vs = setup("v");
	tu = setup("tune");
	utsnm = setup("utsname");
	FLckinfo = setup("flckinfo");
	endnm = msginfo = setup("msginfo");
	pnmuxlink = setup("nmuxlink");
	pnstrpush = setup("nstrpush");
	pnstrevent = setup("nstrevent");
	pmaxsepgcnt = setup("maxsepgcnt");
	pstrlofrac = setup("strlofrac");
	pstrmedfrac = setup("strmedfrac");
	pstrmsgsz = setup("strmsgsz");
	pstrctlsz = setup("strctlsz");
	pnadvertise = setup("nadvertise");
	pmaxgdp = setup("maxgdp");
	pnrcvd = setup("nrcvd");
	pnrduser = setup("nrduser");
	pnsndd = setup("nsndd");
	pminserve = setup("minserve");
	pmaxserve = setup("maxserve");
	prfsize = setup("rfsize");
	prfs_vhigh = setup("rfs_vhigh");
	prfs_vlow = setup("rfs_vlow");
	pnsrmount = setup("nsrmount");
	seminfo = setup("seminfo");
	shminfo = setup("shminfo");
	setup("");
	nlist(os, nl);
	for(nlptr = vs; nlptr != endnm; nlptr++) {
		if(nlptr->n_value == 0) {
printf("%s\n",nlptr->n_name);
			fprintf(stderr, "namelist error\n");
			exit(1);
		}
	}
	MEMSEEK(vs);	MEMREAD(v);
	printf("%6d	buffers in buffer cache (NBUF)\n",v.v_buf);
	printf("%6d	entries in callout table (NCALL)\n",v.v_call);
	printf("%6d	inodes  (NINODE)\n",v.v_inode);
	printf("%6d	s5inodes  (NS5INODE)\n",v.v_s5inode);
	printf("%6d	entries in file table (NFILE)\n",v.v_file);
	printf("%6d	entries in mount table (NMOUNT)\n",v.v_mount);
	printf("%6d	entries in proc table (NPROC)\n",v.v_proc);
	printf("%6d	entries in shared region table (NREGION)\n",
		v.v_region);
	printf("%6d	clist buffers (NCLIST)\n",v.v_clist);
	printf("%6d	processes per user id (MAXUP)\n",v.v_maxup);
	printf("%6d	hash slots for buffer cache (NHBUF)\n",v.v_hbuf);
	printf("%6d	buffers for physical I/O (NPBUF)\n",v.v_pbuf);
	printf("%6d	size of system virtual space map (SPTMAP)\n",
		v.v_sptmap);
	printf("%6d	fraction of memory for vhandlow (VHNDFRAC)\n",
		v.v_vhndfrac);
	printf("%6d	maximum physical memory to use (MAXPMEM)\n",
		v.v_maxpmem);
	printf("%6d	auto update time limit in seconds (NAUTOUP)\n",
		v.v_autoup);
	printf("%6d	maximum number of open files per process (NOFILES)\n",
		v.v_nofiles);
	printf("%6d	number of streams queues (NQUEUE)\n",v.v_nqueue);
	printf("%6d	number of streams head structures (NSTREAM)\n",
		v.v_nstream);
	printf("%6d	number of 4096 bytes stream buffers (NBLK4096)\n",
		v.v_nblk4096);
	printf("%6d	number of 2048 bytes stream buffers (NBLK2048)\n",
		v.v_nblk2048);
	printf("%6d	number of 1024 bytes stream buffers (NBLK1024)\n",
		v.v_nblk1024);
	printf("%6d	number of 512 bytes stream buffers (NBLK512)\n",
		v.v_nblk512);
	printf("%6d	number of 256 bytes stream buffers (NBLK256)\n",
		v.v_nblk256);
	printf("%6d	number of 128 bytes stream buffers (NBLK128)\n",
		v.v_nblk128);
	printf("%6d	number of 64 bytes stream buffers (NBLK64)\n",
		v.v_nblk64);
	printf("%6d	number of 16 bytes stream buffers (NBLK16)\n",
		v.v_nblk16);
	printf("%6d	number of 4 bytes stream buffers (NBLK4)\n",
		v.v_nblk4);

	MEMSEEK(tu);	MEMREAD(tune);
	printf("%6d  maximum size of user's virtual address space in pages (MAXUMEM)\n", tune.t_maxumem);
	printf("%6d  for package compatibility equal to MAXUMEM (MAXMEM)\n", 
   tune.t_maxumem);
	printf("%6d  page stealing low water mark (GPGSLO)\n", tune.t_gpgslo);
	printf("%6d  page stealing high water mark (GPGSHI)\n", tune.t_gpgshi);
	printf("%6d  vhand wake up rate (VHANDR)\n", tune.t_vhandr);
	printf("%6d  awaken vhand if free memory less than vhandl (VHANDL)\n",
		tune.t_vhandl);
	printf("%6d  bdflush run rate (BDFLUSHR)\n", tune.t_bdflushr);
	printf("%6d  minimum resident memory for avoiding deadlock (MINARMEM)\n",
		tune.t_minarmem);
	printf("%6d  minimum swapable memory for avoiding deadlock (MINASMEM)\n",
		tune.t_minasmem);
	printf("%6d  maximum number of pages swapped out (MAXSC)\n", tune.t_maxsc);
	printf("%6d  maximum number of pages saved (MAXFC)\n", tune.t_maxfc);

	printf("*\n* Utsname Tunables\n*\n");
	MEMSEEK(utsnm);	MEMREAD(utsname);
	printf("%8s  release (REL)\n",utsname.release);
	printf("%8s  node name (NODE)\n",utsname.nodename);
	printf("%8s  system name (SYS)\n",utsname.sysname);
	printf("%8s  version (VER)\n",utsname.version);

	printf("*\n* Streams Tunables\n*\n");
	if ( pnmuxlink->n_value ) {
		MEMSEEK(pnmuxlink);	MEMREAD(nmuxlink);
		printf("%6d	number of multiplexor links (NMUXLINK)\n",
			nmuxlink);
	}
	if ( pnstrpush->n_value ) {
		MEMSEEK(pnstrpush);	MEMREAD(nstrpush);
		printf("%6d	maximum number of pushes allowed (NSTRPUSH)\n",
			nstrpush);
	}
	if ( pnstrevent->n_value ) {
		MEMSEEK(pnstrevent);	MEMREAD(nstrevent);
		printf("%6d	initial number of stream event calls (NSTREVENT)\n",
			nstrevent);
	}
	if ( pmaxsepgcnt->n_value ) {
		MEMSEEK(pmaxsepgcnt);	MEMREAD(maxsepgcnt);
		printf("%6d	page limit for event cell allocation (MAXSEPGCNT)\n",
			maxsepgcnt);
	}
	if ( pstrmsgsz->n_value ) {
		MEMSEEK(pstrmsgsz);	MEMREAD(strmsgsz);
		printf("%6d	maximum stream message size (STRMSGSZ)\n",
			strmsgsz);
	}
	if ( pstrctlsz->n_value ) {
		MEMSEEK(pstrctlsz);	MEMREAD(strctlsz);
		printf("%6d	max size of ctl part of message (STRCTLSZ)\n",
			strctlsz);
	}
	if ( pstrlofrac->n_value ) {
		MEMSEEK(pstrlofrac);	MEMREAD(strlofrac);
		printf("%6d	max low priority % block usage (STRLOFRAC)\n",
			strlofrac);
	}
	if ( pstrmedfrac->n_value ) {
		MEMSEEK(pstrmedfrac);	MEMREAD(strmedfrac);
		printf("%6d	max medium priority % block usage (STRMEDFRAC)\n",
			strmedfrac);
	}

	printf("*\n* RFS Tunables\n*\n");
	if ( pnadvertise->n_value ) {
		MEMSEEK(pnadvertise);	MEMREAD(nadvertise);
		printf("%6d	entries in advertise table (NADVERTISE)\n",
			nadvertise);
	}
	if ( pnrcvd->n_value ) {
		MEMSEEK(pnrcvd);	MEMREAD(nrcvd);
		printf("%6d	receive descriptors (NRCVD)\n",
			nrcvd);
	}
	if ( pnrduser->n_value ) {
		MEMSEEK(pnrduser);	MEMREAD(nrduser);
		printf("%6d	maximum number of rd_user structures (NRDUSER)\n",
			nrduser);
	}
	if ( pnsndd->n_value ) {
		MEMSEEK(pnsndd);	MEMREAD(nsndd);
		printf("%6d	send descriptors (NSNDD)\n",
			nsndd);
	}
	if ( pminserve->n_value ) {
		MEMSEEK(pminserve);	MEMREAD(minserve);
		printf("%6d	minimum number of server processes (MINSERVE)\n",
			minserve);
	}
	if ( pmaxserve->n_value ) {
		MEMSEEK(pmaxserve);	MEMREAD(maxserve);
		printf("%6d	maximum number of server processes (MAXSERVE)\n",
			maxserve);
	}
	if ( pmaxgdp->n_value ) {
		MEMSEEK(pmaxgdp);	MEMREAD(maxgdp);
		printf("%6d	maximum number of remote systems with active mounts (MAXGDP)\n",
			maxgdp);
	}
	if ( prfsize->n_value ) {
		MEMSEEK(prfsize);	MEMREAD(rfsize);
		printf("%6d	size of static RFS administrative storage area (RFHEAP)\n",
			rfsize);
	}
	if ( prfs_vhigh->n_value ) {
		MEMSEEK(prfs_vhigh);	MEMREAD(rfs_vhigh);
		printf("%6d	latest compatible RFS version (RFS_VHIGH)\n",
			rfs_vhigh);
	}
	if ( prfs_vlow->n_value ) {
		MEMSEEK(prfs_vlow);	MEMREAD(rfs_vlow);
		printf("%6d	earliest compatible RFS version (RFS_VLOW)\n",
			rfs_vlow);
	}
	if ( pnsrmount->n_value ) {
		MEMSEEK(pnsrmount);	MEMREAD(nsrmount);
		printf("%6d	entries in server mount table (NSRMOUNT)\n",
			nsrmount);
	}

	if ( msginfo->n_value )
		{
		MEMSEEK(msginfo);	MEMREAD(minfo);
		printf("*\n* IPC Messages\n*\n");
		printf("%6d	entries in msg map (MSGMAP)\n",minfo.msgmap);
		printf("%6d	max message size (MSGMAX)\n",minfo.msgmax);
		printf("%6d	max bytes on queue (MSGMNB)\n",minfo.msgmnb);
		printf("%6d	message queue identifiers (MSGMNI)\n",minfo.msgmni);
		printf("%6d	message segment size (MSGSSZ)\n",minfo.msgssz);
		printf("%6d	system message headers (MSGTQL)\n",minfo.msgtql);
		printf("%6u	message segments (MSGSEG)\n",minfo.msgseg);
		}

	if ( seminfo->n_value )
		{
		MEMSEEK(seminfo);	MEMREAD(sinfo);
		printf("*\n* IPC Semaphores\n*\n");
		printf("%6d	entries in semaphore map (SEMMAP)\n",sinfo.semmap);
		printf("%6d	semaphore identifiers (SEMMNI)\n",sinfo.semmni);
		printf("%6d	semaphores in system (SEMMNS)\n",sinfo.semmns);
		printf("%6d	undo structures in system (SEMMNU)\n",sinfo.semmnu);
		printf("%6d	max semaphores per id (SEMMSL)\n",sinfo.semmsl);
		printf("%6d	max operations per semop call (SEMOPM)\n",sinfo.semopm);
		printf("%6d	max undo entries per process (SEMUME)\n",sinfo.semume);
		printf("%6d	semaphore maximum value (SEMVMX)\n",sinfo.semvmx);
		printf("%6d	adjust on exit max value (SEMAEM)\n",sinfo.semaem);
		}

	if ( shminfo->n_value )
		{
		MEMSEEK(shminfo);	MEMREAD(shinfo);
		printf("*\n* IPC Shared Memory\n*\n");
		printf("%6d	max shared memory segment size (SHMMAX)\n",shinfo.shmmax);
		printf("%6d	min shared memory segment size (SHMMIN)\n",shinfo.shmmin);
		printf("%6d	shared memory identifiers (SHMMNI)\n",shinfo.shmmni);
		printf("%6d	max attached shm segments per process (SHMSEG)\n",shinfo.shmseg);
		printf("%6d	max in use shared memory (SHMALL)\n",shinfo.shmall);
		}

	if ( FLckinfo->n_value )
		{
		MEMSEEK(FLckinfo);	MEMREAD(flinfo);
		printf("*\n* File and Record Locking\n*\n");
		printf("%6d	records configured on system (FLCKREC)\n",flinfo.recs);
		}
	else
		printf("flckinfo not available  \n");

	exit(0);
}

/*
 * setup - add an entry to a namelist structure array
 */
struct	nlist	*
setup(nam)
	char	*nam;
{
	if(nlptr >= &nl[MAXI]) { 
		fprintf(stderr, "internal name list overflow\n");
		exit(1);
	}

	nlptr->n_name = malloc((unsigned)(strlen(nam)+1));	/* initialize pointer to next string */
	strcpy(nlptr->n_name,nam);	/* move name into string table */
	nlptr->n_type = 0;
	nlptr->n_value = 0;
	return(nlptr++);
}

/*
 * setln - set up internal link structure for later
 * function look-up.  Records useful information from the
 * /etc/master table description.
 */
 void
setln(cf, np, block, software)
	char	*cf;
	struct	nlist	*np;
	int block, software;
{
	if(lnptr >= &ln[MAXL]) {
		fprintf(stderr, "internal link array overflow\n");
		exit(1);
	}
	strncat(strcpy(lnptr->l_cfnm," "), cf,DIRSIZ);
	lnptr->l_func = np;
	lnptr->l_soft = software;
	lnptr->l_dtype = block;
	lnptr->l_used = 0;
	lnptr++;
}

/*
 * Handle the configured devices
 */
devices()
{
	register struct link *lnkptr;

	/*
	 * for each dev_addr array found, read the
	 * addresses and calculate the board slot
	 */
	for( lnkptr=ln; lnkptr != endln; ++lnkptr )
		{
		if ( ! (lnkptr->l_func[ADDR]).n_value || lnkptr->l_soft )
			/* dev_addr undefined */
			continue;

		MEMSEEK( (&(lnkptr->l_func[ADDR])) );
		while( 1 )
			{
			MEMREAD( addr );

#ifdef u3b5
			if ( ! addr.valid )
				break;

			if ( addr.sys != 0 )
				printf( " %s\tboard=%d\t(on LBE)\n", lnkptr->l_cfnm, addr.base/256 );
			else
				printf( " %s\tboard=%d\n", lnkptr->l_cfnm, addr.base/256 );
#endif

#ifdef u3b2
			if ( addr == 0 )
				break;

			printf( " %s\tboard=%d\n", lnkptr->l_cfnm, ((addr-KVIOBASE)/2228224) + 1 );
#endif
			}
		lnkptr->l_used = 1;
		}

	/*
	 * for each remaining device, print the device if it has not
	 * already been used and it is found to be in the symbol table
	 */
	for( lnkptr=ln; lnkptr != endln; ++lnkptr )
		{
		if ( ! lnkptr->l_used )
			if ( lnkptr->l_func[OPEN].n_value
			  || lnkptr->l_func[CLOSE].n_value
			  || lnkptr->l_func[READ].n_value
			  || lnkptr->l_func[WRITE].n_value
			  || lnkptr->l_func[IOCTL].n_value
			  || lnkptr->l_func[STRAT].n_value )
					{
					printf( " %s\n", lnkptr->l_cfnm );
					lnkptr->l_used = 1;
					}
		}
}
/*
 * Handle loadable objects
 */
edtsect()
{
	int i,l;
	struct s3bconf *s3bconf;

	/*
	 * read <EDT> section header
	 */
	if (( l = ldnshread(ldptr,"<EDT>",&scn)) == SUCCESS);
	{
		s3bconf = (struct s3bconf *)malloc((unsigned)scn.s_size);
		ldnsseek(ldptr,"<EDT>");
		FREAD((char *)s3bconf,scn.s_size,1,ldptr);

		/*
		 * loop to extract loadable object names
		 */
		for (i=0; i < s3bconf->count; ++i)
		{
			if (s3bconf->driver[i].flag & S3BC_MOD)
			{
				lcase(s3bconf->driver[i].name);
				printf("  %.*s\n",DIRSIZ,s3bconf->driver[i].name);
			}
		}
	}
	ldclose(ldptr);
}

lcase(s)
	char *s;
{
	int c;

	while (c = *s)
	{
		if (isascii(c) && isupper(c))
			*s = tolower(c);
		++s;
	}
}

sysdev()
{
	register struct link *lnptr;
	int m;

	/* rootdev, swapdev, pipedev, dumpdev */

	if((lnptr = majsrch(root)) == NULL)
		fprintf(stderr, "unknown root device\n");
	else
		{
		m = major(root);
		if ( ONLBE(m) )
			printf("  rootdev\t%s(%d)\tminor=%d\ton LBE(%d)\n", lnptr->l_cfnm, m&0x0F, minor(root), 13+((m&0x30)>>4) );
		else
			if (lnptr->l_soft)
				printf("  rootdev\t%s\tminor=%d\n", lnptr->l_cfnm, minor(root) );
			else
				printf("  rootdev\t%s(%d)\tminor=%d\n", lnptr->l_cfnm, m, minor(root) );
		}

	if((lnptr = majsrch(swap)) == NULL)
		fprintf(stderr, "unknown swap device\n");
	else {
		m = major(swap);
		if ( ONLBE(m) )
			printf("  swapdev\t%s(%d)\tminor=%d\ton LBE(%d)", lnptr->l_cfnm, m&0x0F, minor(swap), 13+((m&0x30)>>4) );
		else
			if (lnptr->l_soft)
				printf("  swapdev\t%s\tminor=%d", lnptr->l_cfnm, minor(swap) );
			else
				printf("  swapdev\t%s(%d)\tminor=%d", lnptr->l_cfnm, m, minor(swap) );
		MEMSEEK(swplo);	MEMREAD(spl);
		MEMSEEK(nswap);	MEMREAD(nsp);
		printf("\tswplo=%ld\tnswap=%d\n", spl, nsp);
	}

	if((lnptr = majsrch(pipe)) == NULL)
		fprintf(stderr, "unknown pipe device\n");
	else
		{
		m = major(pipe);
		if ( ONLBE(m) )
			printf("  pipedev\t%s(%d)\tminor=%d\ton LBE(%d)\n", lnptr->l_cfnm, m&0x0F, minor(pipe), 13+((m&0x30)>>4) );
		else
			if (lnptr->l_soft)
				printf("  pipedev\t%s\tminor=%d\n", lnptr->l_cfnm, minor(pipe) );
			else
				printf("  pipedev\t%s(%d)\tminor=%d\n", lnptr->l_cfnm, m, minor(pipe) );
		}

#ifdef u3b5
	if((lnptr = majsrch(dump)) == NULL)
		fprintf(stderr, "unknown dump device\n");
	else
		{
		m = major(dump);
		if ( ONLBE(m) )
			printf("  dumpdev\t%s(%d)\tminor=%d\ton LBE(%d)\n", lnptr->l_cfnm, m&0x0F, minor(dump), 13+((m&0x30)>>4) );
		else
			if (lnptr->l_soft)
				printf("  dumpdev\t%s\tminor=%d\n", lnptr->l_cfnm, minor(dump) );
			else
				printf("  dumpdev\t%s(%d)\tminor=%d\n", lnptr->l_cfnm, m, minor(dump) );
		}
#endif
}


/*
 * return true if the flags from /etc/master contain the character "c"
 */
test( flags, c )
	register char *flags;
	char c;
	{
	char t;

	while( t = *flags++ )
		if ( t == c )
			return( 1 );
	return( 0 );
	}


/*
 * majsrch - search for a link structure given the device
 * number of the device in question.
 */
struct	link *
majsrch(device)
	dev_t	device;
{
	register struct link *lnkptr;
	register maj;
	struct bdevsw one_bdevsw;
	int int_maj;


	maj = major(device);
	int_maj = MAJ[maj];
	fseek(sysfile, (long) (SYM_VALUE(bdev) + int_maj*sizeof(struct bdevsw) - offset), 0);
	MEMREAD(one_bdevsw);

	for(lnkptr = ln; lnkptr != endln; lnkptr++)
		if(lnkptr->l_dtype) {
			if((lnkptr->l_func[STRAT]).n_value == (long)one_bdevsw.d_strategy)
				return(lnkptr);
		}
	return(NULL);
}
