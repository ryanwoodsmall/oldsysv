static char sccsid[] = "@(#)ls.c	1.12";
/*
* 	list file or directory;
* 	define DOTSUP to suppress listing of files beginning with dot
*/

#include	<sys/param.h>
#include	<sys/types.h>
#include	<sys/sysmacros.h>
#include	<sys/stat.h>
#include	<sys/dir.h>
#include	<stdio.h>
#if u3b
#include	<sys/macro.h>
#endif

#define	DOTSUP	1
#define ISARG   0100000 /* this bit equals 1 in lflags of structure lbuf 
                        *  if *namep is to be used;
                        */
#define DIRECT	10	/* Number of direct blocks */

#ifdef u370
	/* Number of pointers in an indirect block */
#define INDIR	(BSIZE/sizeof(daddr_t))
	/* Number of right shifts to divide by INDIR */
#define INSHFT	10
#else
	/* Number of pointers in an indirect block */
#define INDIR	128
	/* Number of right shifts to divide by INDIR */
#define INSHFT	7
#endif


struct	lbuf	{
	union	{
		char	lname[DIRSIZ];   /* used for filename in a directory */
		char	*namep;          /* for name in ls-command; */
	} ln;
	char	ltype;  /* filetype */
	unsigned short	lnum;   /* inode number of file */
	short	lflags; /* 0777 bits are used as various r,w,x permissions */
	short	lnl;    /* number of links to file */
	unsigned short	luid;
	unsigned short	lgid;
	long	lsize;  /* filesize or major/minor device numbers */
	long	lmtime;
};

int	nfiles = 0;	/* number of flist entries in current use */
int	nargs = 0;	/* number of flist entries used for arguments */
int	maxfils = 0;	/* number of flist/lbuf entries allocated */
int	maxn = 0;	/* number of flist entries with lbufs assigned */
int	quantn = 1024;	/* allocation growth quantum */

struct	lbuf	*nxtlbf;	/* pointer to next lbuf to be assigned */
struct	lbuf	**flist;	/* pointer to list of lbuf pointers */
struct	lbuf	*gstat();

FILE	*pwdfu, *pwdfg, *dirf;

int	aflg, dflg, lflg, sflg, tflg, uflg, iflg, fflg, gflg, cflg, oflg;
int	rflg = 1;   /* initialized to 1 for special use in compar() */
int	flags;
int	err = 0;	/* Contains return code */
int	pflg = 0;	/* Flag for -p option. */
char	*dmark;	/* Used if -p option active. Contains "/" or NULL. */
unsigned	lastuid	= -1, lastgid = -1;
int	statreq;    /* is > 0 if any of sflg, lflg, tflg are on */

char	*dotp = ".";
char	*makename();
char	tbufu[16], tbufg[16];   /* assumed 15 = max. length of user/group name */
char	*ctime();
char	stdbuf[BUFSIZ];

long	nblock();
long	tblocks;  /* total number of blocks of files in a directory */
long	year, now;

main(argc, argv)
char *argv[];
{
	extern char	*optarg;
	extern int	optind;
	int	amino, opterr=0;
	int	c;
	register struct lbuf *ep;
	struct	lbuf	lb;
	int	i, j;
	int	compar();
	long	time();
	char *malloc();
	void qsort(), exit();

#ifdef STANDALONE
	if (argv[0][0] == '\0')
		argc = getargv("ls", &argv, 0);
#endif
#ifndef	STANDALONE
	setbuf(stdout, stdbuf);
#endif
	lb.lmtime = time((long *) NULL);
	year = lb.lmtime - 6L*30L*24L*60L*60L; /* 6 months ago */
	now = lb.lmtime + 60;
	while ((c=getopt(argc, argv, "logtasdrucifp")) != EOF) switch(c) {
		case 'a':
			aflg++;
			continue;

		case 's':
			sflg++;
			statreq++;
			continue;

		case 'd':
			dflg++;
			continue;

		case 'g':
			gflg++;
			oflg-=2;
			lflg++;
			statreq++;
			continue;

		case 'l':
			lflg++;
			oflg++;
			gflg++;
			statreq++;
			continue;

		case 'r':
			rflg = -1;
			continue;

		case 't':
			tflg++;
			statreq++;
			continue;

		case 'u':
			uflg++;
			continue;

		case 'c':
			cflg++;
			continue;

		case 'i':
			iflg++;
			continue;

		case 'f':
			fflg++;
			continue;

		case 'o':
			oflg++;
			gflg-=2;
			lflg++;
			statreq++;
			continue;

		case 'p':
			pflg++;
			statreq++;
			continue;

		case '?':
			opterr++;
			continue;
		}
	if(opterr) {
		fprintf(stderr,"usage: ls -logtasdrucifp file . . .\n");
		exit(2);
	}
	if (fflg) {
		aflg++;
		lflg = 0;
		sflg = 0;
		tflg = 0;
		statreq = 0;
	}
	if(lflg) {
		if (oflg > 0)
			if ((pwdfu = fopen("/etc/passwd", "r")) == NULL) {
				fprintf(stderr,"%s file cannot be opened for reading","/etc/passwd");
				exit(2);
			}
		if (gflg > 0)
			if ((pwdfg = fopen("/etc/group", "r")) == NULL) {
				fprintf(stderr,"%s file cannot be opened for reading","/etc/group");
				exit(2);
			}
	}

/* allocate space for flist and the associated data structures (lbufs) */
	maxfils = quantn;
	if((flist=(struct lbuf **)malloc((unsigned)(maxfils * sizeof(struct lbuf *)))) == NULL
	|| (nxtlbf = (struct lbuf *)malloc((unsigned)(quantn * sizeof(struct lbuf)))) == NULL) {
		fprintf(stderr, "ls: out of memory\n");
		exit(2);
	}
	if ((amino=(argc-optind))==0) { 
/* case when no names are given in ls-command and current 
 * directory is to be used 
 */
		argv[optind] = dotp;
	}
	for (i=0; i < (amino ? amino : 1); i++) {
		if ((ep = gstat((*argv[optind] ? argv[optind] : dotp), 1))==NULL)
		{
			err = 2;
			optind++;
			continue;
		}
		ep->ln.namep = (*argv[optind] ? argv[optind] : dotp);
		ep->lflags |= ISARG;
		optind++;
		nargs++;	/* count good arguments stored in flist */
	}
	qsort(flist, (unsigned)nargs, sizeof(struct lbuf *), compar);
	for (i=0; i<nargs; i++) {
		ep = flist[i];
		if (ep->ltype=='d' && dflg==0 || fflg) {
			if (amino>1)
				printf("\n%s:\n", ep->ln.namep);
			nfiles = nargs;
			readdir(ep->ln.namep);
		/* the resulting data lies between nargs and
		 * nfiles in flist.
		 */
			if (fflg==0)
				qsort(&flist[nargs], (unsigned)(nfiles - nargs),sizeof(struct lbuf *),compar);
			if (lflg || sflg)
				printf("total %ld\n", tblocks);
			for (j=nargs; j<nfiles; j++) {
				pentry(flist[j]);
			}
		} else 
			pentry(ep);
	}
	exit(err);
}

/* print one output line;
 *  if uid/gid is not found in the appropriate
 *  file (passwd/group), then print uid/gid instead of 
 *  user/group name;
 */
pentry(ap)
struct lbuf *ap;
{
	struct	{
		char	dminor,
			dmajor;
	};
	register struct lbuf *p;
	register char *cp;

	p = ap;
	if (iflg)
		printf("%5u ", p->lnum);
	if (sflg)  {
           if (p->ltype != 'b' && p->ltype != 'c')
		   printf("%4ld ", nblock(p->lsize));
           else printf("%4d ",0);  /* for special files of type 'b', 'c'; */
        }
	if (lflg) {
		putchar(p->ltype);
		pmode(p->lflags);
		printf("%4d ", p->lnl);
		if (oflg > 0)
			if(getname(p->luid, tbufu, 0)==0)
				printf("%-9.9s", tbufu);
			else
				printf("%-9u", p->luid);
		if (gflg > 0)
			if(getname(p->lgid, tbufg, 1)==0)
				printf("%-9.9s", tbufg);
			else
				printf("%-9u", p->lgid);
		if (p->ltype=='b' || p->ltype=='c')
			printf("%3d,%3d", major((int)p->lsize), minor((int)p->lsize));
		else
			printf("%7ld", p->lsize);
		cp = ctime(&p->lmtime);
		if((p->lmtime < year) || (p->lmtime > now))
			printf(" %-7.7s %-4.4s ", cp+4, cp+20);
		else
			printf(" %-12.12s ", cp+4);
	}
	if(pflg & p->ltype=='d')dmark = "/"; else dmark = "";
	if (p->lflags&ISARG)
		printf("%s%s\n", p->ln.namep,dmark);
	else
		printf("%.14s%s\n", p->ln.lname,dmark);  /* 14 = DIRSIZ */
}

/* get name from passwd/group file for a given uid/gid
 *  and store it in buf; lastuid is set to uid;
 *  returns -1 if uid is not in file
 */
getname(uid, buf, type)
unsigned uid;
int type;
char buf[];
{
        int c;
        register i, j, n;

	if (uid==(type ? lastgid : lastuid))
		return(0);
	rewind(type ? pwdfg : pwdfu);
	if(type)
		lastgid = -1;
	else	lastuid = -1;
	do {
		i = 0;
		j = 0;
		n = 0;
                while((c=fgetc(type ? pwdfg : pwdfu)) != '\n') {  /* '\n' indicates end of 
                                                  *  a per user/group record
                                                  *  in passwd/group file;
                                                  */
                     if (c==EOF)
                        return(-1);  
                     else if (c==':') j++;
                          else if (j==0) buf[i++] = c;
                               else if (j==2)
                                       n = n*10 + (c-'0');
		}
	} while (n != uid);
	buf[i] = '\0';
	if (type)
		lastgid = uid;
	else	lastuid = uid;
	return(0);
}

long nblock(size)
long size;
{
	long blocks, tot;

	blocks = tot = (size + BSIZE - 1) >> BSHIFT;
	if(blocks > DIRECT)
		tot += ((blocks - DIRECT - 1) >> INSHFT) + 1;
	if(blocks > DIRECT + INDIR)
		tot += ((blocks - DIRECT - INDIR - 1) >> (INSHFT * 2)) + 1;
	if(blocks > DIRECT + INDIR + INDIR*INDIR)
		tot++;
	return(tot);
}

/* print various r,w,x permissions 
 */
pmode(aflag)
{
        /* these arrays are declared static to allow initializations */
	static int	m0[] = { 1, S_IREAD>>0, 'r', '-' };
	static int	m1[] = { 1, S_IWRITE>>0, 'w', '-' };
	static int	m2[] = { 3, S_ISUID|S_IEXEC, 's', S_IEXEC, 'x', S_ISUID, 'S', '-' };
	static int	m3[] = { 1, S_IREAD>>3, 'r', '-' };
	static int	m4[] = { 1, S_IWRITE>>3, 'w', '-' };
	static int	m5[] = { 3, S_ISGID|(S_IEXEC>>3),'s', S_IEXEC>>3,'x', S_ISGID,'S', '-'};
	static int	m6[] = { 1, S_IREAD>>6, 'r', '-' };
	static int	m7[] = { 1, S_IWRITE>>6, 'w', '-' };
	static int	m8[] = { 3, S_ISVTX|(S_IEXEC>>6),'t', S_IEXEC>>6,'x', S_ISVTX,'T', '-'};

        static int  *m[] = { m0, m1, m2, m3, m4, m5, m6, m7, m8};

	register int **mp;

	flags = aflag;
	for (mp = &m[0]; mp < &m[sizeof(m)/sizeof(m[0])];)
		select(*mp++);
}

select(pairp)
register int *pairp;
{
	register int n;

	n = *pairp++;
	while (n-->0) {
		if((flags & *pairp) == *pairp) {
			pairp++;
			break;
		}else {
			pairp += 2;
		}
	}
	putchar(*pairp);
}

/* returns pathname of the form dir/file;
 *  dir is a null-terminated string;
 */
char *
makename(dir, file) 
char *dir, *file;
{
	static char dfile[100+DIRSIZ];  /* 100 is assumed to be max. length of a
                                        *  file/dir name in ls-command;
                                        *  dfile is static as this is returned
                                        *  by makename();
                                        */
	register char *dp, *fp;
	register int i;

	dp = dfile;
	fp = dir;
	while (*fp)
		*dp++ = *fp++;
	*dp++ = '/';
	fp = file;
	for (i=0; i<DIRSIZ; i++)
		*dp++ = *fp++;
	*dp = '\0';  /* filenames in a directory were not null-terminated */
	return(dfile);
}

/* read each filename in directory dir and store its
 *  status in flist[nfiles] 
 *  use makename() to form pathname dir/filename;
 */
readdir(dir)
char *dir;
{
	struct direct dentry;
	register int j;
	register struct lbuf *ep;

	if ((dirf = fopen(dir, "r")) == NULL) {
		fflush(stdout);
		fprintf(stderr, "%s unreadable\n", dir);
		err = 2;
		return;
	}
        else {
          	tblocks = 0;
          	for(;;) {
          		if (fread((char *) &dentry, sizeof(dentry), 1, dirf) != 1)
          			break;  /* end of directory */
          		if (dentry.d_ino==0
          			|| aflg==0 && dentry.d_name[0]=='.' 
# ifndef DOTSUP
          			&& (dentry.d_name[1]=='\0' || dentry.d_name[1]=='.'
          			&& dentry.d_name[2]=='\0')
# endif
          			)  /* check for directory items '.', '..', 
                                   *  and items without valid inode-number;
                                   */
          			continue;
          		ep = gstat(makename(dir, dentry.d_name), 0);
          		if (ep==NULL)
          			continue;
                        else {
          		     ep->lnum = dentry.d_ino;
                             for (j=0; j<DIRSIZ; j++)
          		         ep->ln.lname[j] = dentry.d_name[j];
                        }
          	}
          	fclose(dirf);
	}
}

/* get status of file and recomputes tblocks;
 * argfl = 1 if file is a name in ls-command and  = 0
 * for filename in a directory whose name is an
 * argument in the command;
 * stores a pointer in flist[nfiles] and
 * returns that pointer;
 * returns NULL if failed;
 */
struct lbuf *
gstat(file, argfl)
char *file;
{
	struct stat statb;
	register struct lbuf *rep;
	static int nomocore;
	char *malloc(), *realloc();

	if (nomocore)
		return(NULL);
	else if (nfiles >= maxfils) { 
/* all flist/lbuf pair assigned files time to get some more space */
		maxfils += quantn;
		if((flist=(struct lbuf **)realloc((char *)flist, (unsigned)(maxfils * sizeof(struct lbuf *)))) == NULL
		|| (nxtlbf = (struct lbuf *)malloc((unsigned)(quantn * sizeof(struct lbuf)))) == NULL) {
			fprintf(stderr, "ls: out of memory\n");
			nomocore = 1;
			return(NULL);
		}
	}

/* nfiles is reset to nargs for each directory
 * that is given as an argument maxn is checked
 * to prevent the assignment of an lbuf to a flist entry
 * that already has one assigned.
 */
	if(nfiles >= maxn) {
		rep = nxtlbf++;
		flist[nfiles++] = rep;
		maxn = nfiles;
	}else {
		rep = flist[nfiles++];
	}
	rep->lflags = 0;
	if (argfl || statreq) {
		if (stat(file, &statb)<0) {
			fprintf(stderr, "%s not found\n", file);
			nfiles--;
			return(NULL);
		}
                else {
	            	rep->lnum = statb.st_ino;
	            	rep->lsize = statb.st_size;
	            	switch(statb.st_mode&S_IFMT) {

	            	case S_IFDIR:
	            		rep->ltype = 'd';
	            		break;

	            	case S_IFBLK:
	            		rep->ltype = 'b';
	            		rep->lsize = statb.st_rdev;
	            		break;

	            	case S_IFCHR:
	            		rep->ltype = 'c';
	            		rep->lsize = statb.st_rdev;
	            		break;

	            	case S_IFIFO:
                 		rep->ltype = 'p';
                 		break;
                        default:
                                rep->ltype = '-';
                 	}
	          	rep->lflags = statb.st_mode & ~S_IFMT;
                                    /* mask ISARG and other file-type bits */
	          	rep->luid = statb.st_uid;
	          	rep->lgid = statb.st_gid;
	          	rep->lnl = statb.st_nlink;
	          	if(uflg)
	          		rep->lmtime = statb.st_atime;
	          	else if (cflg)
	          		rep->lmtime = statb.st_ctime;
	          	else
	          		rep->lmtime = statb.st_mtime;
                        if (rep->ltype != 'b' && rep->ltype != 'c')
	          	   tblocks += nblock(statb.st_size);
                }
	}
        return(rep);
}

compar(pp1, pp2)  /* return >0 if item pointed by pp2 should appear first */
struct lbuf **pp1, **pp2;
{
	register struct lbuf *p1, *p2;

	p1 = *pp1;
	p2 = *pp2;
	if (dflg==0) {
/* compare two names in ls-command one of which is file
 *  and the other is a directory;
 *  this portion is not used for comparing files within
 *  a directory name of ls-command;
 */
		if (p1->lflags&ISARG && p1->ltype=='d') {
			if (!(p2->lflags&ISARG && p2->ltype=='d'))
				return(1);
                }
                else {
			if (p2->lflags&ISARG && p2->ltype=='d')
				return(-1);
		}
	}
	if (tflg) {
		if(p2->lmtime == p1->lmtime)
			return(0);
		else if(p2->lmtime > p1->lmtime)
			     return(rflg);
		else return(-rflg);
	}
        else
             return(rflg * strcmp(p1->lflags&ISARG? p1->ln.namep: p1->ln.lname,
				p2->lflags&ISARG? p2->ln.namep: p2->ln.lname));
}
