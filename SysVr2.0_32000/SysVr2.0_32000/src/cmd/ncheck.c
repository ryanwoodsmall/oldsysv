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
/*	@(#)ncheck.c	1.5	*/
/*
 * ncheck -- obtain file names from reading filesystem
 */

/* number of inode blocks to process at a time */
#define	NI	16
#define	NB	300
#ifdef u370
#define HSIZE	8017
#else
#ifdef small
#define	HSIZE	1740
#else
#if vax || ns32000
#define HSIZE	8017
#else
#define HSIZE	2503
#endif
#endif
#endif

#include <stdio.h>
#include <ctype.h>
#include <sys/param.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/sysmacros.h>
#include <sys/inode.h>
#include <sys/ino.h>
#include <sys/dir.h>
#include <sys/filsys.h>
#include <sys/fblk.h>

/* file system block size */
#if (vax || ns32000 || u3b || u3b5) && (FsTYPE == 3)
#define FSBSIZE	(BSIZE*2)
#else
#define	FSBSIZE BSIZE
#endif

#define	NIDIR	(FSBSIZE/sizeof(daddr_t))
#define	NBINODE	(FSBSIZE/sizeof(struct dinode))
#define	NDIR	(FSBSIZE/sizeof(struct direct))

char	*checklist = "/etc/checklist";
struct	filsys	sblock;
struct	dinode	itab[NBINODE*NI];
daddr_t	iaddr[NADDR];
ino_t	ilist[NB];
struct	htab
{
	ino_t	h_ino;
	ino_t	h_pino;
	char	h_name[DIRSIZ];
} htab[HSIZE];

int	aflg;
int	sflg;
int	fi;
ino_t	ino;
int	nhent;
int	nxfile;

int	bsize, physblks;
int	nidir;
int	nbinode;
int	ndir;

int	nerror;
daddr_t	bmap();
long	atol();
struct htab *lookup();

main(argc, argv)
char *argv[];
{
	register i;
	register FILE *fp;
	char filename[50];
	long n;

	while (--argc) {
		argv++;
		if (**argv=='-')
		switch ((*argv)[1]) {

		case 'a':
			aflg++;
			continue;

		case 'i':
			for(i=0; i<NB; i++) {
				n = atol(argv[1]);
				if(n == 0)
					break;
				ilist[i] = n;
				nxfile = i;
				argv++;
				argc--;
			}
			continue;

		case 's':
			sflg++;
			continue;

		default:
			fprintf(stderr, "ncheck: bad flag %c\n", (*argv)[1]);
			nerror++;
		}
		else break;
	}
	if(argc) {		/* arg list has file names */
		while(argc-- > 0)
			check(*argv++);
	}
	else {			/* use default checklist */
		if((fp = fopen(checklist,"r")) == NULL) {
			fprintf(stderr,"Can't open checklist file: %s\n",checklist);
			exit(2);
		}
		while(getline(fp,filename,sizeof(filename)) != EOF)
			check(filename);
		fclose(fp);
	}
	return(nerror);
}

check(file)
char *file;
{
	register i, j;
	ino_t mino;

	fi = open(file, 0);
	if(fi < 0) {
		fprintf(stderr, "ncheck: cannot open %s\n", file);
		nerror++;
		return;
	}
	nhent = 0;
	printf("%s:\n", file);
	sync();
	lseek(fi, (long)SUPERBOFF, 0);
	if (read(fi, (char *)&sblock, sizeof(sblock)) != sizeof(sblock)) {
		fprintf(stderr, "ncheck: read error super-block\n");
		return;
	}
	if(sblock.s_magic == FsMAGIC) {
		if(sblock.s_type == Fs1b) {
			physblks = 1;
			bsize = BSIZE;
		} else if(sblock.s_type == Fs2b) {
			physblks = 2;
			bsize = BSIZE*2;
		} else {
			printf("(%-10s): bad block type\n", file);
			return;
		}
	} else {
		physblks = 1;
		bsize = BSIZE;
	}
	nidir = bsize / sizeof(daddr_t);
	nbinode = bsize / sizeof(struct dinode);
	ndir = bsize / sizeof(struct direct);
	mino = (sblock.s_isize-2) * nbinode;
	ino = 0;
	for(i=2;; i+=NI) {
		if(ino >= mino)
			break;
		bread((daddr_t)i, (char *)itab, (sizeof(struct dinode) * nbinode * NI));
		for(j=0; j<nbinode*NI; j++) {
			if(ino >= mino)
				break;
			ino++;
			pass1(&itab[j]);
		}
	}
	ilist[nxfile+1] = 0;
	ino = 0;
	for(i=2;; i+=NI) {
		if(ino >= mino)
			break;
		bread((daddr_t)i, (char *)itab, (sizeof(struct dinode) * nbinode * NI));
		for(j=0; j<nbinode*NI; j++) {
			if(ino >= mino)
				break;
			ino++;
			pass2(&itab[j]);
		}
	}
	ino = 0;
	for(i=2;; i+=NI) {
		if(ino >= mino)
			break;
		bread((daddr_t)i, (char *)itab, (sizeof(struct dinode) * nbinode * NI));
		for(j=0; j<nbinode*NI; j++) {
			if(ino >= mino)
				break;
			ino++;
			pass3(&itab[j]);
		}
	}
}

pass1(ip)
register struct dinode *ip;
{
	if((ip->di_mode & IFMT) != IFDIR) {
		if (sflg != 1) return;
		if (nxfile >= NB){
			++sflg;
			fprintf(stderr,"Too many special files (increase ilist array)\n");
			return;
		}
		if ((ip->di_mode&IFMT)==IFBLK || (ip->di_mode&IFMT)==IFCHR
		  || ip->di_mode&(ISUID|ISGID))
			ilist[nxfile++] = ino;
			return;
	}
	lookup(ino, 1);
}

pass2(ip)
register struct dinode *ip;
{
	struct direct dbuf[NDIR];
	long doff;
	struct direct *dp;
	register i, j;
	int k;
	struct htab *hp;
	daddr_t d;
	ino_t kno;

	if((ip->di_mode&IFMT) != IFDIR)
		return;
	l3tol(iaddr, ip->di_addr, NADDR);
	doff = 0;
	for(i=0;; i++) {
		if(doff >= ip->di_size)
			break;
		d = bmap(i);
		if(d == 0)
			break;
		bread(d, (char *)dbuf, (sizeof(struct direct) * ndir));
		for(j=0; j<ndir; j++) {
			if(doff >= ip->di_size)
				break;
			doff += sizeof(struct direct);
			dp = dbuf+j;
			kno = dp->d_ino;
			if(kno == 0)
				continue;
			hp = lookup(kno, 0);
			if(hp == 0)
				continue;
			if(dotname(dp))
				continue;
			hp->h_pino = ino;
			for(k=0; k<DIRSIZ; k++)
				hp->h_name[k] = dp->d_name[k];
		}
	}
}

pass3(ip)
register struct dinode *ip;
{
	struct direct dbuf[NDIR];
	long doff;
	struct direct *dp;
	register i, j;
	int k;
	daddr_t d;
	ino_t kno;

	if((ip->di_mode&IFMT) != IFDIR)
		return;
	l3tol(iaddr, ip->di_addr, NADDR);
	doff = 0;
	for(i=0;; i++) {
		if(doff >= ip->di_size)
			break;
		d = bmap(i);
		if(d == 0)
			break;
		bread(d, (char *)dbuf, (sizeof(struct direct) * ndir));
		for(j=0; j<ndir; j++) {
			if(doff >= ip->di_size)
				break;
			doff += sizeof(struct direct);
			dp = dbuf+j;
			kno = dp->d_ino;
			if(kno == 0)
				continue;
			if(aflg==0 && dotname(dp))
				continue;
			if(ilist[0] == 0 && sflg==0)
				goto pr;
			for(k=0; ilist[k] != 0; k++)
				if(ilist[k] == kno)
					goto pr;
			continue;
		pr:
			printf("%u	", kno);
			pname(ino, 0);
			printf("/%.14s", dp->d_name);
			if (lookup(kno, 0))
				printf("/.");
			printf("\n");
		}
	}
}

dotname(dp)
register struct direct *dp;
{

	if (dp->d_name[0]=='.')
		if (dp->d_name[1]==0 || (dp->d_name[1]=='.' && dp->d_name[2]==0))
			return(1);
	return(0);
}

pname(i, lev)
ino_t i;
{
	register struct htab *hp;

	if (i==ROOTINO)
		return;
	if ((hp = lookup(i, 0)) == 0) {
		printf("???");
		return;
	}
	if (lev > 10) {
		printf("...");
		return;
	}
	pname(hp->h_pino, ++lev);
	printf("/%.14s", hp->h_name);
}

struct htab *
lookup(i, ef)
ino_t i;
{
	register struct htab *hp;

	for (hp = &htab[i%HSIZE]; hp->h_ino;) {
		if (hp->h_ino==i)
			return(hp);
		if (++hp >= &htab[HSIZE])
			hp = htab;
	}
	if (ef==0)
		return(0);
	if (++nhent >= HSIZE) {
		fprintf(stderr, "ncheck: out of core-- increase HSIZE\n");
		exit(1);
	}
	hp->h_ino = i;
	return(hp);
}

bread(bno, buf, cnt)
daddr_t bno;
char *buf;
{
	register i;

	lseek(fi, bno*bsize, 0);
	if (read(fi, buf, cnt) != cnt) {
		fprintf(stderr, "ncheck: read error %d\n", bno);
		for(i=0; i<bsize; i++)
			buf[i] = 0;
	}
}

daddr_t
bmap(i)
{
	daddr_t ibuf[NIDIR];

	if(i < NADDR-3)
		return(iaddr[i]);
	i -= NADDR-3;
	if(i > nidir) {
		fprintf(stderr, "ncheck: %u - huge directory\n", ino);
		return((daddr_t)0);
	}
	bread(iaddr[NADDR-3], (char *)ibuf, (sizeof(daddr_t) * nidir));
	return(ibuf[i]);
}
getline(fp,loc,maxlen)
FILE *fp;
char *loc;
{
	register n;
	register char *p, *lastloc;

	p = loc;
	lastloc = &p[maxlen-1];
	while((n = getc(fp)) != '\n') {
		if(n == EOF)
			return(EOF);
		if(!isspace(n) && p < lastloc)
			*p++ = n;
	}
	*p = 0;
	return(p - loc);
}
