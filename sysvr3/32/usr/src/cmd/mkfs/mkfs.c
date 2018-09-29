/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkfs:mkfs.c	1.29"
/*	mkfs	COMPILE:	cc -O mkfs.c -s -i -o mkfs
 * mkfs - with support for 512, 1KB and 2KB logical block sizes
 * Make a file system prototype.
 * usage: mkfs filsys size[:inodes] [gap blocks/cyl] [-b blocksize]
 *        mkfs filsys proto [gap blocks/cyl] [-b blocksize]
 */

#include <stdio.h>
#include <a.out.h>

#ifndef RT			/*  ts - includes  */


#if FsTYPE == 3
#undef FsTYPE		/* FsTYPE == 3 is certain death when using prototypes */
#endif
#ifndef FsTYPE
#define FsTYPE	2
#endif

#include <sys/param.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/ino.h>
#include <sys/fs/s5inode.h>
#include <sys/inode.h>
#include <sys/fs/s5filsys.h>
#include <sys/fs/s5fblk.h>
#include <sys/fs/s5dir.h>
#include <sys/stat.h>

#else

#include <rt/types.h>
#include <rt/fmgr/ino.h>
#include <rt/fmgr/inode.h>
#include <rt/fmgr/filsys.h>
#include <rt/fmgr/fblk.h>
#include <rt/dir.h>
#include <rt/fmgr/param.h>
#include <rt/stat.h>

#endif

/* file system block size */

#if u3b15  && (FsTYPE == 2) || u3b2 && (FsTYPE == 4)  /* BSIZE=2048 (see param.h) */
#undef itod
#define	itod(x)	(daddr_t)(((x+63)>>5)&017777)
#undef itoo
#define	itoo(x)	(int)((x+63)&037)
#endif

#if u3b15  && (FsTYPE == 4) || u3b2 && (FsTYPE == 2)  /* BSIZE=1024 (see param.h) */
#undef itod
#define	itod(x)	(daddr_t)(((x+31)>>4)&037777)
#undef itoo
#define	itoo(x)	(int)((x+31)&017)
#endif

#define	FSBSIZE BSIZE

/* boot-block size */
#define BBSIZE	512
/* super-block size */
#define SBSIZE	512
/* number of sectors in a block */
#define SECTPB (FSBSIZE/512)

#define	NIDIR	(FSBSIZE/sizeof(daddr_t))
#define	NFB	(NIDIR+1300)	/* NFB must be greater than NIDIR+LADDR */
#define	NDIRECT	(FSBSIZE/sizeof(struct direct))
#define	NBINODE	(FSBSIZE/sizeof(struct dinode))
#define	LADDR	10
#ifdef u3b15
#define STEPSIZE	24
#define CYLSIZE		32
#else
#define	STEPSIZE	7
#define	CYLSIZE		400
#endif
#define	MAXFN	1500

time_t	utime;
FILE 	*fin;
int	fsi;
int	fso;
char	*charp;
char	buf[FSBSIZE];

int work0[FSBSIZE/sizeof(int)];
struct fblk *fbuf = (struct fblk *)work0;

#if u3b || u3b15 || u3b2 || vax
struct aouthdr head;
#else
struct exec head;
#endif
char	string[512];

int work1[FSBSIZE/sizeof(int)];
struct filsys *filsys = (struct filsys *)work1;
char	*fsys;
char	*proto;
int	f_n = CYLSIZE;
int	f_m = STEPSIZE;
int	error;
ino_t	ino;
long	getnum();
daddr_t	alloc();


#ifdef RT			/*  RT - only declarations  */

daddr_t	bitmap();
daddr_t	cf_alloc();

/*
 * additional parameters for MERT file system #2
 */

int	nbmapblk;	/* Number of blocks for bit map */
daddr_t	firstblk;	/* First available free block */
int	*bmap;		/* Start of bit map */

int	nline	1;
char	*sp;
FILE	*srcin;
struct fblk *fptr;

#endif

char *arg0 ;	 /* program name */
main(argc, argv)
int argc;
char *argv[];
{
	int f, c, i;
	int errflag = 0;
	int bflag = 0;
	int bpos = 0;
	long n, nb;
	int blocksize;		/* logical block size */
	struct stat statarea;
	struct {
		daddr_t tfree;
		ino_t tinode;
		char fname[6];
		char fpack[6];
	} ustatarea;
	arg0 = argv[0] ;	 /* program name, i.e. mkfs */


	time(&utime);
	if( argc < 3 || argc > 7 )
		errusage();
	/* find block size argument if it exists */
	for  (i = 1; i  < argc; i++) {
		if (strncmp(argv[i], "-b", 2) == 0) {
			bpos = i;
			bflag++;
		}
	}
	if (bflag > 1)
		errusage();
	if (bflag) {
		if (bpos != 3 && bpos != 5)
			errusage();
		if (bpos == 3 && argc > 5)
			errusage();
		if (strlen(argv[bpos]) == 2)
			blocksize = atoi(argv[bpos + 1]);
		else
			blocksize = atoi(&argv[bpos][2]);
	}
	else  {
		if (argc > 5)
			errusage();
	}
			
	if (bflag) {
/* If this program was invoked as mkfs (FsTYPE == 2), and if the -b
 *   option was used to specify  a block size which is not the default for
 *   this type of machine, then we must exec the appropriate executable
 *   to build file system with desired logical block size.
 * If user invoked program as mkfs512, mkfs1K, or mkfs2K and specified a
 *   block size that is inconsistent with the executable he used (e.g.
 *   he used mkfs512 to try to build a 2K) we fail and tell him to use
 *   mkfs with -b option.
 */
#if FsTYPE == 2	/* we are in mkfs */
	if (blocksize == 512) {
		if (execvp("/etc/mkfs512", argv) == -1) {
			fprintf(stderr, "%s: can't exec /etc/mkfs512\n", arg0);
			exit(1);
		}
	}
#if u3b2
	if (blocksize == 2048) {
		if (execvp("/etc/mkfs2K", argv) == -1) {
			fprintf(stderr, "%s: can't exec /etc/mkfs2K\n", arg0);
			exit(1);
		}
	}
	if (blocksize != 1024) {
		fprintf(stderr, "%s: %d is invalid logical block size\n", arg0, blocksize);
		exit(1);
	}

#endif
#if u3b15
	if (blocksize == 1024) {
		if (execvp("/etc/mkfs1K", argv) == -1) {
			fprintf(stderr, "%s: can't exec /etc/mkfs1K\n", arg0);
			exit(1);
		}
	}
	if (blocksize != 2048) {
		fprintf(stderr, "%s: %d is invalid logical block size\n", arg0, blocksize);
		exit(1);
	}
#endif
#endif	/* end FsTYPE == 2 */

#if FsTYPE == 1	/* we are in mkfs512 */
	if (blocksize != 512) {
	    if (blocksize == 1024 || blocksize == 2048)
		fprintf(stderr, "%d is invalid block size for %s, use mkfs with -b option\n", blocksize, arg0);
	    else
		fprintf(stderr, "%s: %d is invalid logical block size\n", arg0, blocksize);
	    exit(1);
	}
#endif	/* end FsTYPE == 1 */

#if FsTYPE == 4	/* we are in mkfs2K on 3b2 or mkfs1K on 3b15 */
#if u3b2
	if (blocksize != 2048) {
	    if (blocksize == 512 || blocksize == 1024)
		fprintf(stderr, "%d is invalid block size for %s, use mkfs with -b option\n", blocksize, arg0);
	    else
		fprintf(stderr, "%s: %d is invalid logical block size\n", arg0, blocksize);
	    exit(1);
	}
#endif
#if u3b15
	if (blocksize != 1024) {
	    if (blocksize == 512 || blocksize == 2048)
		fprintf(stderr, "%d is invalid block size for %s, use mkfs with -b option\n", blocksize, arg0);
	    else
		fprintf(stderr, "%s: %d is invalid logical block size\n", arg0, blocksize);
	    exit(1);
	}
#endif
#endif	/* end FsTYPE == 4 */
	}	/* end of if(bflag) */

	/* get the other arguments */
	fsys = argv[1];
	proto = argv[2];
	if ((bflag && argc >= 6) || (!bflag && argc >= 5)) {
		f_m = atoi(argv[3]);
		f_n = atoi(argv[4]);
		if (f_n <= 0 || f_n >= MAXFN)
			f_n = CYLSIZE;
		if (f_m <= 0 || f_m > f_n)
			f_m = STEPSIZE;
	}
	
	if(stat(fsys, &statarea) < 0) {
		fprintf(stderr, "%s: %s: cannot stat\n", arg0, fsys);
		exit(1);
	}
#ifdef RT
	setio (-1, 1);
#endif
	fsi = open(fsys, 0);
	if(fsi < 0) {
		fprintf(stderr, "%s: %s: cannot open\n", arg0, fsys);
		exit(1);
	}
	if((statarea.st_mode & S_IFMT) == S_IFBLK)
		if(ustat(statarea.st_rdev,&ustatarea) >= 0) {
			fprintf(stderr, "%s: %s: *** MOUNTED FILE SYSTEM\n", arg0, fsys);
			exit(1);
		}

	
	printf("Mkfs: %s? \n(DEL if wrong)\n", fsys);
	sleep(10);	/* 10 seconds to DEL */
	fso = creat(fsys, 0666);
	if(fso < 0) {
		fprintf(stderr, "%s: %s: cannot create\n", arg0, fsys);
		exit(1);
	}
	fin = fopen(proto, "r");
	if(fin == NULL) {	/* setup from command line (or bad proto file) */
		nb = n = 0;
		for(f=0; c=proto[f]; f++) {
			if(c<'0' || c>'9') {
				if(c == ':') {
					nb = n;
					n = 0;
					continue;
				}
				fprintf(stderr, "%s: cannot open proto file '%s'\n", arg0, proto);
				exit(1);
			}
			n = n*10 + (c-'0');
		}
		if(!nb) {
			nb = n / SECTPB;
			n = nb/4;
		} else {
			nb /= SECTPB;
		}

				/* nb is number of logical blocks in fs,
				   n is number of inodes */

		filsys->s_fsize = nb;
		charp = "d--777 0 0 $ ";
	}
	else {				/* proto file setup */

		/*
		 * get name of boot load program
		 * and read onto block 0
		 */

		getstr();
		f = open(string, 0);
		if(f < 0) 
			fprintf(stderr, "%s: cannot  open boot program '%s'\n", arg0, string);
		else {
			read(f, (char *)&head, sizeof head);

#if u3b || u3b15 || u3b2 || vax
			c = head.tsize + head.dsize;
#else
			c = head.a_text + head.a_data;
#endif
			if(c > BBSIZE) 
				fprintf(stderr, "%s: '%s' too big\n", arg0, string);
			else {
				read(f, buf, c);

				/* write boot-block to file system */
				lseek(fso, 0L, 0);
				if(write(fso, buf, BBSIZE) != BBSIZE) {
					fprintf(stderr, "%s: error writing boot-block\n", arg0);
					exit(1);
				}
			}

			close(f);

		}
		/*
		 * get total disk size
		 * and inode block size
		*/

		nb = getnum();
		filsys->s_fsize = nb / SECTPB;
		n = getnum();

	}
	n /= NBINODE;		/* number of logical blocs for inodes */
	if(n <= 0)
		n = 1;
	if(n > 65500/NBINODE)
		n = 65500/NBINODE;
	filsys->s_isize = n + 2;

	/* set magic number for file system type */
	filsys->s_magic = FsMAGIC;
	if (FSBSIZE == 512)
		filsys->s_type = Fs1b;
	else if (FSBSIZE == 1024)
		filsys->s_type = Fs2b;
	else if (FSBSIZE == 2048)
		filsys->s_type = Fs4b;
	else {
		fprintf(stderr, "%s: unknown block size\n", arg0);
		exit(1);
	}
	filsys->s_dinfo[0] = f_m;
	filsys->s_dinfo[1] = f_n;
	f_n /= SECTPB;
	f_m = (f_m +(SECTPB -1))/SECTPB;  /* gap rounded up to the next block */

	printf("bytes per logical block = %d\n", FSBSIZE);
	printf("total logical blocks = %ld\n", filsys->s_fsize);
	printf("total inodes = %ld\n", n*NBINODE);
	printf("gap (physical blocks) = %d\n", filsys->s_dinfo[0]);
	printf("cylinder size (physical blocks) = %d \n", filsys->s_dinfo[1]);

	if(filsys->s_isize >= filsys->s_fsize) {
		fprintf(stderr, "%s: %ld/%ld: bad ratio\n", arg0, filsys->s_fsize, filsys->s_isize-2);
		exit(1);
	}
	filsys->s_tinode = 0;
#ifdef RT
	filsys->s_m = f_m;
	filsys->s_n = f_n;
	filsys->s_cfree = 0;
	nbmapblk = ((filsys->s_fsize - filsys->s_isize) + 4095)/4096;
	filsys->s_nxtblk = firstblk = filsys->s_isize + nbmapblk;
	bmap = sbrk ( nbmapblk * 512 );
	filsys->s_tfree = filsys->s_fsize - firstblk;
#endif
	for(c=0; c<FSBSIZE; c++)
		buf[c] = 0;
	for(n=2; n!=filsys->s_isize; n++) {
		wtfs(n, buf);
		filsys->s_tinode += NBINODE;
	}
	ino = 0;

	bflist();

	cfile((struct inode *)0);

	filsys->s_time = utime;
#if u3b2 || u3b15
	filsys->s_state = FsOKAY - (long)filsys->s_time;
#endif
#ifdef RT
	filsys->s_nxtcon = filsys->s_nxtblk;
	filsys->s_label = 'RT';
#endif

/* write super-block onto file system */
	lseek(fso, (long)SUPERBOFF, 0);
	if(write(fso, (char *)filsys, SBSIZE) != SBSIZE) {
		fprintf(stderr, "%s: write error: super-block\n", arg0);
		exit(1);
	}

#ifdef RT
	n = filsys->s_isize;
	for ( c = 0; c < nbmapblk; c++)  {
		wtfs ( (long)n, bmap+c*256 );
		n++;
	}
#endif
	printf("mkfs: Available blocks = %ld\n",filsys->s_tfree);
	exit(error);
}

cfile(par)
struct inode *par;
{
	struct inode in;
	struct s5inode s5in;
	daddr_t bn, nblk;
	int dbc, ibc;
	char db[FSBSIZE];
	daddr_t ib[NFB];
	int i, f, c;

	/*
	 * get mode, uid and gid
	 */

	getstr();
	in.i_fsptr = (int *) &s5in;
#ifndef RT
	in.i_ftype  = gmode(string[0], "-bcd", IFREG, IFBLK, IFCHR, IFDIR, 0, 0, 0);
#else
	in.i_ftype  = gmode(string[0], "-bcdreL", IFREG, IFBLK, IFCHR, IFDIR, IFREC, IFEXT, IF1EXT );
#endif
	s5in.s5i_mode = gmode(string[1], "-u", 0, ISUID, 0, 0, 0, 0, 0);
	s5in.s5i_mode |= gmode(string[2], "-g", 0, ISGID, 0, 0, 0, 0, 0);
	for(i=3; i<6; i++) {
		c = string[i];
		if(c<'0' || c>'7') {
			fprintf(stderr, "%s: %c/%s: bad octal mode digit\n", arg0, c, string);
			error = 1;
			c = 0;
		}
		s5in.s5i_mode |= (c-'0')<<(15-3*i);
	}
	in.i_uid = getnum();
	in.i_gid = getnum();

	/*
	 * general initialization prior to
	 * switching on format
	 */

	ino++;
	in.i_number = ino;
	for(i=0; i<FSBSIZE; i++)
		db[i] = 0;
	for(i=0; i<NFB; i++)
		ib[i] = (daddr_t)0;
	in.i_nlink = 1;
	in.i_size = 0;
	for(i=0; i<NADDR; i++)
		s5in.s5i_faddr[i] = (daddr_t)0;
	if(par == (struct inode *)0) {
		par = &in;
		in.i_nlink--;
	}
	dbc = 0;
	ibc = 0;
	switch(in.i_ftype) {

	case IFREG:
		/*
		 * regular file
		 * contents is a file name
		 */

		getstr();
		f = open(string, 0);
#ifdef RT
		sp = string;
#endif
		if(f < 0) {
			fprintf(stderr, "%s: %s: cannot open\n", arg0, string);
			error = 1;
			break;
		}
		while((i=read(f, db, FSBSIZE)) > 0) {
#ifdef RT
			bn = bitmap ( &in, in.i_size>>BSHIFT );
#endif
			in.i_size += i;
#ifndef RT
			newblk(&dbc, db, &ibc, ib);
#else
			wtfs (bn, db);
			if (error) {
				printf ("%s\n", string );
				error = 0;
				break;
		}
#endif
		}
#ifdef RT
	for ( i = 0; i < 3; i++ )
		flushbuf ( i );
#endif
		close(f);
		break;

	case IFBLK:
	case IFCHR:
#ifdef RT
	case IFREC:
#endif
		/*
		 * special file
		 * content is maj/min types
		 */

		i = getnum() & 0377;
		f = getnum() & 0377;
		s5in.s5i_faddr[0] = (i<<8) | f;
		break;

	case IFDIR:
		/*
		 * directory
		 * put in extra links
		 * call recursively until
		 * name of "$" found
		 */

		par->i_nlink++;
		in.i_nlink++;
		entry(in.i_number, ".", &dbc, db, &ibc, ib);
		entry(par->i_number, "..", &dbc, db, &ibc, ib);
		in.i_size = 2*sizeof(struct direct);
		for(;;) {
			getstr();
			if(string[0]=='$' && string[1]=='\0')
				break;
			entry(ino+1, string, &dbc, db, &ibc, ib);
			in.i_size += sizeof(struct direct);
			cfile(&in);
		}
		break;

#ifdef RT
	case IFEXT:
	case IF1EXT:
		/*
		 * contiguous file
		 * contents is a file name
		 */

		getstr();
		f = open(string, 0);
		sp = string;
		if(f < 0) {
			fprintf(stderr,"%s: %s: cannot open\n", arg0, string);
			error = 1;
			break;
		}
		while((i=read(f, db, FSBSIZE)) > 0) {
			in.i_size =+ i;
			nblk = 1;
			bn = cf_alloc(&nblk);
			if(par->i_un.i_addr[0] == 0)
				par->i_un.i_addr[0] = bn;
			par->i_un.i_addr[1]++;
			wtfs(bn, db);
		}
		close(f);
		break;
#endif
	}
	if(dbc != 0)
		newblk(&dbc, db, &ibc, ib);
	iput(&in, &ibc, ib);
}

gmode(c, s, m0, m1, m2, m3, m4, m5, m6)
char c, *s;
{
	int i;

	for(i=0; s[i]; i++)
		if(c == s[i])
			return((&m0)[i]);
	fprintf(stderr, "%s: %c/%s: bad mode\n", arg0, c, string);
	error = 1;
	return(0);
}

long 
getnum()
{
	int i, c;
	long n;

	getstr();
	n = 0;
	for(i=0; c=string[i]; i++) {
		if(c<'0' || c>'9') {
			fprintf(stderr, "%s: %s: bad number\n", arg0, string);
			error = 1;
			return((long)0);
		}
		n = n*10 + (c-'0');
	}
	return(n);
}

getstr()
{
	int i, c;

loop:
	switch(c=getch()) {

	case ' ':
	case '\t':
	case '\n':
		goto loop;

	case '\0':
		fprintf(stderr, "%s: EOF\n", arg0);
		exit(1);

	case ':':
		while(getch() != '\n');
		goto loop;

	}
	i = 0;

	do {
		string[i++] = c;
		c = getch();
	} 
	while(c!=' '&&c!='\t'&&c!='\n'&&c!='\0');
	string[i] = '\0';
}

rdfs(bno, bf)
daddr_t bno;
char *bf;
{
	int n;

	lseek(fsi, (long)(bno*FSBSIZE), 0);
	n = read(fsi, bf, FSBSIZE);
	if(n != FSBSIZE) {
		fprintf(stderr, "%s: read error: %ld\n", arg0, bno);
		exit(1);
	}
}

wtfs(bno, bf)
daddr_t bno;
char *bf;
{
	int n;

	lseek(fso, (long)(bno*FSBSIZE), 0);
	n = write(fso, bf, FSBSIZE);
	if(n != FSBSIZE) {
		fprintf(stderr, "%s: write error: %ld\n", arg0, bno);
		exit(1);
	}
}

#ifndef RT
daddr_t 
alloc()
{
	int i;
	daddr_t bno;

	filsys->s_tfree--;
	bno = filsys->s_free[--filsys->s_nfree];
	if(bno == 0) {
		fprintf(stderr, "%s: out of free space\n", arg0);
		exit(1);
	}
	if(filsys->s_nfree <= 0) {
		rdfs(bno, (char *)fbuf);
		filsys->s_nfree = fbuf->df_nfree;
		for(i=0; i<NICFREE; i++)
			filsys->s_free[i] = fbuf->df_free[i];
	}
	return(bno);
}

#else
daddr_t 
alloc()
{
	int i;
	daddr_t bno;

	fptr = fbuf;
	filsys->s_tfree--;
	if(filsys->s_cfree == 0) {
addfr:
		addfree();
		if(filsys->s_cfree <= 1) {
			fprintf(stderr,"%s: out of free space %s\n", arg0, sp);
			exit(1);
			exit(1);
		}
	}
	bno = filsys->s_free[--filsys->s_nfree];
	filsys->s_cfree--;
	if(bno == 0)
		goto addfr;
	if(filsys->s_nfree <= 0) {
		rdfs(bno, fbuf);
		filsys->s_nfree = fptr->df_nfree;
		for(i=0; i<NICFREE; i++)
			filsys->s_free[i] = fptr->df_free[i];
	}
	if(bno == 0)
		abort();
	return(bno);
}
#endif

bfree(bno)
daddr_t bno;
{
	int i;

#ifndef RT
	filsys->s_tfree++;
#endif
	if(filsys->s_nfree >= NICFREE) {
		fbuf->df_nfree = filsys->s_nfree;
		for(i=0; i<NICFREE; i++)
			fbuf->df_free[i] = filsys->s_free[i];
		wtfs(bno, (char *)fbuf);
		filsys->s_nfree = 0;
	}
	filsys->s_free[filsys->s_nfree++] = bno;
#ifdef RT
	filsys->s_cfree++;
#endif
}

entry(in, str, adbc, db, aibc, ib)
ino_t in;
char *str;
int *adbc, *aibc;
char *db;
daddr_t *ib;
{
	struct direct *dp;
	int i;

	dp = (struct direct *)db;
	dp += *adbc;
	(*adbc)++;
	dp->d_ino = in;
	for(i=0; i<DIRSIZ; i++)
		dp->d_name[i] = 0;
	for(i=0; i<DIRSIZ; i++)
		if((dp->d_name[i] = str[i]) == 0)
			break;
	if(*adbc >= NDIRECT)
		newblk(adbc, db, aibc, ib);
}

newblk(adbc, db, aibc, ib)
int *adbc, *aibc;
char *db;
daddr_t *ib;
{
	int i;
	daddr_t bno;

	bno = alloc();
	wtfs(bno, db);
	for(i=0; i<FSBSIZE; i++)
		db[i] = 0;
	*adbc = 0;
	ib[*aibc] = bno;
	(*aibc)++;
	if(*aibc >= NFB) {
		fprintf(stderr, "%s: file too large\n", arg0);
		error = 1;
		*aibc = 0;
	}
}

getch()
{

	if(charp)
		return(*charp++);
	return(getc(fin));
}

bflist()
{
	struct inode in;
	struct s5inode s5in;
	daddr_t ib[NFB];
	int ibc;
	char flg[MAXFN];
	int adr[MAXFN];
	int i, j;
	daddr_t f, d;

	for(i=0; i<f_n; i++)
		flg[i] = 0;
	i = 0;
	for(j=0; j<f_n; j++) {
		while(flg[i])
			i = (i+1)%f_n;
		adr[j] = i+1;
		flg[i]++;
		i = (i+f_m)%f_n;
	}

	ino++;
	in.i_fsptr = (int *) &s5in;
	in.i_number = ino;
	in.i_ftype = IFREG;
	in.i_uid = 0;
	in.i_gid = 0;
	in.i_nlink = 0;
	in.i_size = 0;
	s5in.s5i_mode = 0;
	for(i=0; i<NADDR; i++)
		s5in.s5i_faddr[i] = (daddr_t)0;

#ifndef RT
	for(i=0; i<NFB; i++)
		ib[i] = (daddr_t)0;
	ibc = 0;
	bfree((daddr_t)0);
	filsys->s_tfree = 0;
	d = filsys->s_fsize;
	while(d%f_n)
		d++;
	for(; d > 0; d -= f_n)
		for(i=0; i<f_n; i++) {
			f = d - adr[i];
			if(f < filsys->s_fsize && f >= filsys->s_isize)
				if(badblk(f)) {
					if(ibc >= NIDIR) {
						fprintf(stderr, "%s: too many bad blocks\n", arg0);
						error = 1;
						ibc = 0;
					}
					ib[ibc] = f;
					ibc++;
				} else {
					bfree(f);
				}
		}
#endif
	iput(&in, &ibc, ib);
}

iput(ip, aibc, ib)
register struct inode *ip;
register int *aibc;
daddr_t *ib;
{
	register struct dinode *dp;
	struct s5inode *s5ip = (struct s5inode *) ip->i_fsptr;
	daddr_t d;
	register int i,j,k;
	daddr_t ib2[NIDIR];	/* a double indirect block */
	daddr_t ib3[NIDIR];	/* a triple indirect block */

	filsys->s_tinode--;
	d = itod(ip->i_number);
	if(d >= filsys->s_isize) {
		if(error == 0)
			fprintf(stderr, "%s: ilist too small\n", arg0);
		error = 1;
		return;
	}
	rdfs(d, buf);
	dp = (struct dinode *)buf;
	dp += itoo(ip->i_number);

	dp->di_mode = (ip->i_ftype | s5ip->s5i_mode) ;
	dp->di_nlink = ip->i_nlink;
	dp->di_uid = ip->i_uid;
	dp->di_gid = ip->i_gid;
	dp->di_size = ip->i_size;
	dp->di_atime = utime;
	dp->di_mtime = utime;
	dp->di_ctime = utime;

	switch(ip->i_ftype) {

	case IFDIR:
	case IFREG:
		/* handle direct pointers */
		for(i=0; i<*aibc && i<LADDR; i++) {
			s5ip->s5i_faddr[i] = ib[i];
			ib[i] = 0;
		}
		/* handle single indirect block */
		if(i < *aibc)
		{
			for(j=0; i<*aibc && j<NIDIR; j++, i++)
				ib[j] = ib[i];
			for(; j<NIDIR; j++)
				ib[j] = 0;
			s5ip->s5i_faddr[LADDR] = alloc();
			wtfs(s5ip->s5i_faddr[LADDR], (char *)ib);
		}
		/* handle double indirect block */
		if(i < *aibc)
		{
			for(k=0; k<NIDIR && i<*aibc; k++)
			{
				for(j=0; i<*aibc && j<NIDIR; j++, i++)
					ib[j] = ib[i];
				for(; j<NIDIR; j++)
					ib[j] = 0;
				ib2[k] = alloc();
				wtfs(ib2[k], (char *)ib);
			}
			for(; k<NIDIR; k++)
				ib2[k] = 0;
			s5ip->s5i_faddr[LADDR+1] = alloc();
			wtfs(s5ip->s5i_faddr[LADDR+1], (char *)ib2);
		}
		/* handle triple indirect block */
		if(i < *aibc)
		{
			fprintf(stderr, "%s: triple indirect blocks not handled\n", arg0);
		}
		break;

	case IFBLK:
		break;

	case IFCHR:
		break;

#ifdef RT
	case IFEXT:
	case IF1EXT:
			/* copy extent list */
		for ( i = 0; i < LADDR; i++ ) 
			s5ip->s5i_faddr[i] = ib[i];
		break;
#endif

	default:
		fprintf(stderr, "%s: bad ftype %o\n", arg0, ip->i_ftype);
		exit(1);
	}

	ltol3(dp->di_addr, s5ip->s5i_faddr, NADDR);
	wtfs(d, buf);
}

badblk(bno)
daddr_t bno;
{

	return(0);
}
errusage()
{
	fprintf(stderr, "usage: mkfs filsys proto [gap blocks/cyl] [-b blocksize]\n       mkfs filsys blocks[:inodes] [gap blocks/cyl] [-b blocksize]\n");
		exit(1);
}
#ifdef RT		/*  the following functions are all RT-only  */
duped(bno)
daddr_t bno;
{
	daddr_t d;
	register m, n;

	d = bno - filsys->s_isize;
	m = 1 << (d%BITS);
	n = (d/BITS);
	if(bmap[n] & m)
		return(1);
	bmap[n] =| m;
	return(0);
}

addfree()
{
	char flg[MAXFN];
	int adr[MAXFN];
	int i, j;
	daddr_t f, d, nxt;

	for(i=0; i<f_n; i++)
		flg[i] = 0;
	i = 0;
	for(j=0; j<f_n; j++) {
		while(flg[i])
			i = (i+1)%f_n;
		adr[j] = i+1;
		flg[i]++;
		i = (i+f_m)%f_n;
	}

	bfree((daddr_t)0);
addloop:
	j = 0;
	d = filsys->s_nxtblk + NHWAT;
	if(d >= filsys->s_fsize)
		d = filsys->s_fsize-1;
	while(d%f_n)
		d++;
	nxt = d;
	if(nxt > filsys->s_fsize)
		nxt = filsys->s_fsize;
	for(; d > filsys->s_nxtblk; d =- f_n)
	for(i=0; i<f_n; i++) {
		f = d - adr[i];
		if(f < filsys->s_fsize && f >= filsys->s_nxtblk) {
			if(!duped(f)) {
				bfree(f );
				j++;
			}
		}
	}
	filsys->s_nxtblk = nxt;
	if(j == 0 && filsys->s_nxtblk < filsys->s_fsize)
		goto addloop;
}

daddr_t 
cf_alloc(nblkp)
daddr_t *nblkp;
{
	daddr_t sblk, nblk, maxcnt, i, mxnblk, mxsblk;
	int *ptr;

	sblk = filsys->s_nxtblk - filsys->s_isize;
	maxcnt = filsys->s_fsize - filsys->s_isize;
	ptr = bmap + (sblk>>4);
	nblk = mxnblk = mxsblk = 0;
	if(sblk == maxcnt)
		goto errxt;
	for(i = sblk; i < maxcnt; ) {
		if((i&(BITS-1)) || *ptr != 0) {
			do {
				if(((*ptr)&(1<<(i&(BITS-1)))) == 0)
					nblk++;
				else {
					if(mxnblk < nblk) {
						mxnblk = nblk;
						mxsblk = sblk;
					}
					nblk = 0;
					sblk = i+1;
				}
				i++;
				if(nblk >= *nblkp)
					break;
			} while(i&(BITS-1));
		} else if(*ptr == 0) {
			nblk =+ BITS;
			i =+ BITS;
		}
		ptr++;
		if(i > maxcnt)
			nblk =- (i-maxcnt);
		if(nblk >= *nblkp)
			break;
	}
	if(mxnblk > nblk) {
		nblk = mxnblk;
		sblk = mxsblk;
	}
	ptr = bmap + (sblk>>4);
	if(nblk > *nblkp)
		nblk = *nblkp;
	maxcnt = sblk+nblk;
	if(sblk == maxcnt)
		goto errxt;
	for(i = sblk; i < maxcnt; ) {
		if((i&(BITS-1)) || (maxcnt-i) < BITS) {
			do {
				*ptr =| (1<<(i&(BITS-1)));
				i++;
				if(i >= maxcnt)
					break;
			} while(i&(BITS-1));
		} else {
			*ptr = -1;
			i =+ BITS;
		}
		ptr++;
	}
	*nblkp = nblk;
	if(nblk == 0) {
errxt:
		fprintf(stderr,"%s: out of free extents %s\n", arg0, sp);
		exit(1);
	}
	sblk =+ filsys->s_isize;
	if(sblk == filsys->s_nxtblk)
		filsys->s_nxtblk = sblk + nblk;
	return(sblk);
}

daddr_t ibn[3];
daddr_t ib[3][NIDIR];
int	ibc[3];

daddr_t 
bitmap(ip, blkn)
register struct inode *ip;
daddr_t blkn;
{
	register i;
	struct s5inode *s5ip = ip->i_fsptr;
	struct buf *bp, *nbp;
	int	ind;
	int j,k, sh;
	daddr_t nb, *bap, *bapn;
	dev_t dev;

	if(blkn < 0) {
		return((daddr_t)0);
	}
	dev = ip->i_dev;

	/*
	 * blocks 0..NADDR-4 are direct blocks
	 */

	if(blkn < NADDR-3) {
		i = blkn;
		nb = s5ip->s5i_addr[i];
		if(nb == 0) {
			s5ip->s5i_addr[i] = nb = alloc(dev);
		}
		return(nb);
	}

	/*
	 * addresses NADDR-3, NADDR-2, and NADDR-1
	 * have single, double, triple indirect blocks.
	 * the first step is to determine
	 * how many levels of indirection.
	 */

	sh = 0;
	nb = 1;
	blkn =- NADDR-3;
	for(j=3; j>0; j--) {
		sh =+ NSHIFT;
		nb =<< NSHIFT;
		if(blkn < nb)
			break;
		blkn =- nb;
	}
	if(j == 0) {
		return((daddr_t)0);
	}

	/*
	 * fetch the address from the inode
	 */

	nb = s5ip->s5i_addr[NADDR-j];
	if(nb == 0) {
		ibn[0] = nb = alloc(dev);
		ibc[0] = 0;
		if(nb == NULL)
			return((daddr_t)0);
		clrbuf(&ib[0][0]);
		s5ip->s5i_addr[NADDR-j] = nb;
	}

	/*
	 * fetch through the indirect blocks
	 */

	k = j;
	for(; j<=3; j++) {
		ind = j - k;
		bap = &ib[ind][0];
		sh =- NSHIFT;
		i = (blkn>>sh) & NMASK;
		nb = bap[i];
		if(nb == 0) {
			nb = alloc(dev);
			if(nb == NULL) {
				return((daddr_t)0);
			}
			bap[i] = nb;
			ibc[ind] =+ 1;
			if(j != 3)
				ibn[ind+1] = nb;
			if(ibc[ind] >= NIDIR){
				wtfs(ibn[ind],bap);
				ibc[ind] = 0;
				ibn[ind] = (daddr_t)0;
				clrbuf(&ib[ind][0]);
			}
		} 
	}
	return(nb);
}

clrbuf(p)
register char *p;
{
	register i;

	for(i=0;i<512;i++)
		*p++ = 0;
}

flushbuf(i)
{
	if(ibn[i]){
		wtfs(ibn[i],&ib[i][0]);
		clrbuf(&ib[i][0]);
	}
	ibn[i] = 0;
	ibc[i] = 0;
}
#endif
