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
/*	@(#)find.c	4.7	*/
/*	find	COMPILE:	cc -o find -s -O -i find.c -lS	*/
#include	<stdio.h>
#include	<pwd.h>
#include	<grp.h>

#define	UID	1
#define	GID	2

#ifdef RT
#include "rt/types.h"
#include "rt/dir.h"
#include "rt/stat.h"
#else
#include "sys/types.h"
#include "sys/dir.h"
#include "sys/stat.h"
#include "ctype.h"
#endif
#define PATHLEN	256
#define A_DAY	86400L /* a day full of seconds */
#define EQ(x, y)	(strcmp(x, y)==0)
#define BUFSIZE	512	/* In u370 I can't use BUFSIZ nor BSIZE */
#define CPIOBSZ	4096
#define Bufsize	5120

int	Randlast;
char	Pathname[PATHLEN];

struct anode {
	int (*F)();
	struct anode *L, *R;
} Node[100];
int Nn;  /* number of nodes */
char	*Fname;
long	Now;
int	Argc,
	Ai,
	Pi;
char	**Argv;
/* cpio stuff */
int	Cpio;
short	*SBuf, *Dbuf, *Wp;
char	*Buf, *Cbuf, *Cp;
char	Strhdr[500],
	*Chdr = Strhdr;
int	Wct = Bufsize / 2, Cct = Bufsize;
int	Cflag;
int	depthf = 0;

long	Newer;

struct stat Statb;

struct	anode	*exp(),
		*e1(),
		*e2(),
		*e3(),
		*mk();
char	*nxtarg();
char	Home[128];
long	Blocks;
char *strrchr();
char *sbrk();
	struct header {
		short	h_magic,
			h_dev;
		ushort	h_ino,
			h_mode,
			h_uid,
			h_gid;
		short	h_nlink,
			h_rdev,
			h_mtime[2],
			h_namesize,
			h_filesize[2];
		char	h_name[256];
	} hdr;
#ifdef RT
short	Extend;
#endif

main(argc, argv) char *argv[];
{
	struct anode *exlist;
	int paths;
	register char *cp, *sp = 0;
	FILE *pwd, *popen();

	time(&Now);
	pwd = popen("pwd", "r");
	fgets(Home, 128, pwd);
	if(pclose(pwd) & 0377) {
		fprintf(stderr,"Cannot execute `pwd'\n");
		exit(2);
	}
	Home[strlen(Home) - 1] = '\0';
	Argc = argc; Argv = argv;
	if(argc<3) {
usage:		fprintf(stderr,"Usage: find path-list predicate-list\n");
		exit(1);
	}
	for(Ai = paths = 1; Ai < (argc-1); ++Ai, ++paths)
		if(*Argv[Ai] == '-' || EQ(Argv[Ai], "(") || EQ(Argv[Ai], "!"))
			break;
	if(paths == 1) /* no path-list */
		goto usage;
	if(!(exlist = exp())) { /* parse and compile the arguments */
		fprintf(stderr,"find: parsing error\n");
		exit(1);
	}
	if(Ai<argc) {
		fprintf(stderr,"find: missing conjunction\n");
		exit(1);
	}
	for(Pi = 1; Pi < paths; ++Pi) {
		sp = "\0";
		strcpy(Pathname, Argv[Pi]);
		if(*Pathname != '/')
			chdir(Home);
		if(cp = strrchr(Pathname, '/')) {
			sp = cp + 1;
			*cp = '\0';
			if(chdir(*Pathname? Pathname: "/") == -1) {
				fprintf(stderr,"find: bad starting directory\n");
				exit(2);
			}
			*cp = '/';
		}
		Fname = *sp? sp: Pathname;
		descend(Pathname, Fname, exlist); /* to find files that match  */
	}
	if(Cpio) {
		strcpy(Pathname, "TRAILER!!!");
		Statb.st_size = 0;
		cpio();
	}
	exit(0);
}

/* compile time functions:  priority is  exp()<e1()<e2()<e3()  */

struct anode *exp() { /* parse ALTERNATION (-o)  */
	int or();
	register struct anode * p1;

	p1 = e1() /* get left operand */ ;
	if(EQ(nxtarg(), "-o")) {
		Randlast--;
		return(mk(or, p1, exp()));
	}
	else if(Ai <= Argc) --Ai;
	return(p1);
}
struct anode *e1() { /* parse CONCATENATION (formerly -a) */
	int and();
	register struct anode * p1;
	register char *a;

	p1 = e2();
	a = nxtarg();
	if(EQ(a, "-a")) {
And:
		Randlast--;
		return(mk(and, p1, e1()));
	} else if(EQ(a, "(") || EQ(a, "!") || (*a=='-' && !EQ(a, "-o"))) {
		--Ai;
		goto And;
	} else if(Ai <= Argc) --Ai;
	return(p1);
}
struct anode *e2() { /* parse NOT (!) */
	int not();

	if(Randlast) {
		fprintf(stderr,"find: operand follows operand\n");
		exit(1);
	}
	Randlast++;
	if(EQ(nxtarg(), "!"))
		return(mk(not, e3(), (struct anode *)0));
	else if(Ai <= Argc) --Ai;
	return(e3());
}
struct anode *e3() { /* parse parens and predicates */
	int exeq(), ok(), glob(),  mtime(), atime(), ctime(), user(),
		group(), size(), csize(), perm(), links(), print(),
		type(), ino(), depth(), cpio(), newer();
	struct anode *p1;
	struct anode *mkret;
	int i;
	register char *a, *b, s;

	a = nxtarg();
	if(EQ(a, "(")) {
		Randlast--;
		p1 = exp();
		a = nxtarg();
		if(!EQ(a, ")")) goto err;
		return(p1);
	}
	else if(EQ(a, "-print")) {
		return(mk(print, (struct anode *)0, (struct anode *)0));
	}
	else if(EQ(a, "-depth")) {
		depthf = 1;
		return(mk(depth, (struct anode *)0, (struct anode *)0));
	}
	b = nxtarg();
	s = *b;
	if(s=='+') b++;
	if(EQ(a, "-name"))
		return(mk(glob, (struct anode *)b, (struct anode *)0));
	else if(EQ(a, "-mtime"))
		return(mk(mtime, (struct anode *)atoi(b), (struct anode *)s));
	else if(EQ(a, "-atime"))
		return(mk(atime, (struct anode *)atoi(b), (struct anode *)s));
	else if(EQ(a, "-ctime"))
		return(mk(ctime, (struct anode *)atoi(b), (struct anode *)s));
	else if(EQ(a, "-user")) {
		if((i=getunum(UID, b)) == -1) {
			if(gmatch(b, "[0-9][0-9][0-9]*")
			|| gmatch(b, "[0-9][0-9]")
			|| gmatch(b, "[0-9]"))
				return mk(user, (struct anode *)atoi(b), (struct anode *)s);
			fprintf(stderr,"find: cannot find -user name\n");
			exit(1);
		}
		return(mk(user, (struct anode *)i, (struct anode *)s));
	}
	else if(EQ(a, "-inum"))
		return(mk(ino, (struct anode *)atoi(b), (struct anode *)s));
	else if(EQ(a, "-group")) {
		if((i=getunum(GID, b)) == -1) {
			if(gmatch(b, "[0-9][0-9][0-9]*")
			|| gmatch(b, "[0-9][0-9]")
			|| gmatch(b, "[0-9]"))
				return mk(group, (struct anode *)atoi(b), (struct anode *)s);
			fprintf(stderr,"find: cannot find -group name\n");
			exit(1);
		}
		return(mk(group, (struct anode *)i, (struct anode *)s));
	} else if(EQ(a, "-size")) {
		mkret = mk(size, (struct anode *)atoi(b), (struct anode *)s);
		if(*b == '+' || *b == '-')b++;
		while(isdigit(*b))b++;
		if(*b == 'c') Node[Nn-1].F = csize;
		return(mkret);
	} else if(EQ(a, "-links"))
		return(mk(links, (struct anode *)atoi(b), (struct anode *)s));
	else if(EQ(a, "-perm")) {
		for(i=0; *b ; ++b) {
			if(*b=='-') continue;
			i <<= 3;
			i = i + (*b - '0');
		}
		return(mk(perm, (struct anode *)i, (struct anode *)s));
	}
	else if(EQ(a, "-type")) {
		i = s=='d' ? S_IFDIR :
		    s=='b' ? S_IFBLK :
		    s=='c' ? S_IFCHR :
		    s=='p' ? S_IFIFO :
		    s=='f' ? S_IFREG :
#ifdef RT
		    s=='r' ? S_IFREC :
		    s=='m' ? S_IFEXT :
		    s=='1' ? S_IF1EXT:
#endif
		    0;
		return(mk(type, (struct anode *)i, (struct anode *)0));
	}
	else if (EQ(a, "-exec")) {
		i = Ai - 1;
		while(!EQ(nxtarg(), ";"));
		return(mk(exeq, (struct anode *)i, (struct anode *)0));
	}
	else if (EQ(a, "-ok")) {
		i = Ai - 1;
		while(!EQ(nxtarg(), ";"));
		return(mk(ok, (struct anode *)i, (struct anode *)0));
	}
	else if(EQ(a, "-cpio")) {
		if((Cpio = creat(b, 0666)) < 0) {
			fprintf(stderr,"find: cannot create %s\n", b);
			exit(1);
		}
		SBuf = (short *)sbrk(CPIOBSZ);
		Wp = Dbuf = (short *)sbrk(Bufsize);
#ifdef RT
		setio(-1,1);	/* turn on physio */
#endif
		depthf = 1;
		return(mk(cpio, (struct anode *)0, (struct anode *)0));
	}
	else if(EQ(a, "-ncpio")) {
		if((Cpio = creat(b, 0666)) < 0) {
			fprintf(stderr,"find: cannot create %s\n", b);
			exit(1);
		}
		Buf = (char*)sbrk(CPIOBSZ);
		Cp = Cbuf = (char *)sbrk(Bufsize);
#ifdef RT
		setio(-1,1);	/* turn on physio */
#endif
		Cflag++;
		depthf = 1;
		return(mk(cpio, (struct anode *)0, (struct anode *)0));
	}
	else if(EQ(a, "-newer")) {
		if(stat(b, &Statb) < 0) {
			fprintf(stderr,"find: cannot access %s\n", b);
			exit(1);
		}
		Newer = Statb.st_mtime;
		return mk(newer, (struct anode *)0, (struct anode *)0);
	}
err:	fprintf(stderr,"find: bad option %s\n", a);
	exit(1);
}
struct anode *mk(f, l, r)
int (*f)();
struct anode *l, *r;
{
	Node[Nn].F = f;
	Node[Nn].L = l;
	Node[Nn].R = r;
	return(&(Node[Nn++]));
}

char *nxtarg() { /* get next arg from command line */
	static strikes = 0;

	if(strikes==3) {
		fprintf(stderr,"find: incomplete statement\n");
		exit(1);
	}
	if(Ai>=Argc) {
		strikes++;
		Ai = Argc + 1;
		return("");
	}
	return(Argv[Ai++]);
}

/* execution time functions */
and(p)
register struct anode *p;
{
	return(((*p->L->F)(p->L)) && ((*p->R->F)(p->R))?1:0);
}
or(p)
register struct anode *p;
{
	 return(((*p->L->F)(p->L)) || ((*p->R->F)(p->R))?1:0);
}
not(p)
register struct anode *p;
{
	return( !((*p->L->F)(p->L)));
}
glob(p)
register struct { int f; char *pat; } *p; 
{
	return(gmatch(Fname, p->pat));
}
print()
{
	puts(Pathname);
	return(1);
}
mtime(p)
register struct { int f, t, s; } *p; 
{
	return(scomp((int)((Now - Statb.st_mtime) / A_DAY), p->t, p->s));
}
atime(p)
register struct { int f, t, s; } *p; 
{
	return(scomp((int)((Now - Statb.st_atime) / A_DAY), p->t, p->s));
}
ctime(p)
register struct { int f, t, s; } *p; 
{
	return(scomp((int)((Now - Statb.st_ctime) / A_DAY), p->t, p->s));
}
user(p)
register struct { int f, u, s; } *p; 
{
	return(scomp(Statb.st_uid, p->u, p->s));
}
ino(p)
register struct { int f, u, s; } *p;
{
	return(scomp((int)Statb.st_ino, p->u, p->s));
}
group(p)
register struct { int f, u; } *p; 
{
	return(p->u == Statb.st_gid);
}
links(p)
register struct { int f, link, s; } *p; 
{
	return(scomp(Statb.st_nlink, p->link, p->s));
}
size(p)
register struct { int f, sz, s; } *p; 
{
	return(scomp((int)((Statb.st_size+(BUFSIZE - 1))/BUFSIZE), p->sz, p->s));
}
csize(p)
register struct { int f, sz, s; } *p; 
{
	return(scomp((int)Statb.st_size, p->sz, p->s));
}
perm(p)
register struct { int f, per, s; } *p; 
{
	register i;
	i = (p->s=='-') ? p->per : 07777; /* '-' means only arg bits */
	return((Statb.st_mode & i & 07777) == p->per);
}
type(p)
register struct { int f, per, s; } *p;
{
	return((Statb.st_mode&S_IFMT)==p->per);
}
exeq(p)
register struct { int f, com; } *p;
{
	fflush(stdout); /* to flush possible `-print' */
	return(doex(p->com));
}
ok(p)
struct { int f, com; } *p;
{
	int c, yes=0;

	fflush(stdout); /* to flush possible `-print' */
	fprintf(stderr,"< %s ... %s >?   ", Argv[p->com], Pathname);
	fflush(stderr);
	if((c=getchar())=='y') yes = 1;
	while(c!='\n')
		if(c==EOF)
			exit(2);
		else
			c = getchar();
	return(yes? doex(p->com): 0);
}

#define MKSHORT(v, lv) {U.l=1L;if(U.c[0]) U.l=lv, v[0]=U.s[1], v[1]=U.s[0]; else U.l=lv, v[0]=U.s[0], v[1]=U.s[1];}
union { long l; short s[2]; char c[4]; } U;
long mklong(v)
short v[];
{
	U.l = 1;
	if(U.c[0] /* VAX */)
		U.s[0] = v[1], U.s[1] = v[0];
	else
		U.s[0] = v[0], U.s[1] = v[1];
	return U.l;
}

depth()
{
	return(1);
}

cpio()
{
#define MAGIC 070707
#define HDRSIZE	(sizeof hdr - 256)
#define CHARS	76
	register ifile, ct;
	static long fsz;
	register i;

	strcpy(hdr.h_name, !strncmp(Pathname, "./", 2)? Pathname+2: Pathname);
	hdr.h_magic = MAGIC;
	hdr.h_namesize = strlen(hdr.h_name) + 1;
	hdr.h_uid = Statb.st_uid;
	hdr.h_gid = Statb.st_gid;
	hdr.h_dev = Statb.st_dev;
	hdr.h_ino = Statb.st_ino;
	hdr.h_mode = Statb.st_mode;
	hdr.h_nlink = Statb.st_nlink;
	hdr.h_rdev = Statb.st_rdev;
	MKSHORT(hdr.h_mtime, Statb.st_mtime);
	fsz = (hdr.h_mode & S_IFMT) == S_IFREG? Statb.st_size: 0L;
#ifdef RT
	if ((hdr.h_mode & S_IFMT) == S_IFEXT
		|| (hdr.h_mode & S_IFMT) == S_IF1EXT) {
		Extend = 1;
		fsz = Statb.st_size;
	} else Extend = 0;
#endif
	MKSHORT(hdr.h_filesize, fsz);

	if (Cflag)
		bintochar(fsz);

	if(EQ(hdr.h_name, "TRAILER!!!")) {
		Cflag? writehdr(Chdr, CHARS + hdr.h_namesize):
			bwrite((short *)&hdr, HDRSIZE + hdr.h_namesize);
		for (i = 0; i < 10; ++i)
			Cflag? writehdr(Buf, BUFSIZE): bwrite(SBuf, BUFSIZE);
		return;
	}
	if(!mklong(hdr.h_filesize)) {
		Cflag? writehdr(Chdr, CHARS + hdr.h_namesize):
			bwrite((short *)&hdr, HDRSIZE + hdr.h_namesize);
#ifdef RT
		if (Extend)
			Cflag? writehdr(Chdr, CHARS + hdr.h_namesize):
				bwrite((short *)&hdr, HDRSIZE + hdr.h_namesize);
#endif
		return;
	}
	if((ifile = open(Fname, 0)) < 0) {
cerror:
		fprintf(stderr,"find: cannot copy %s\n", hdr.h_name);
		return;
	}
	Cflag? writehdr(Chdr, CHARS + hdr.h_namesize):
		bwrite((short *)&hdr, HDRSIZE+hdr.h_namesize);
#ifdef RT
	if (Extend)
		Cflag? writehdr(Chdr, CHARS + hdr.h_namesize):
			bwrite((short *)&hdr, HDRSIZE + hdr.h_namesize);
#endif
	for(fsz = mklong(hdr.h_filesize); fsz > 0; fsz -= CPIOBSZ) {
		ct = fsz>CPIOBSZ? CPIOBSZ: fsz;
		if(read(ifile, Cflag? Buf: (char *)SBuf, ct) < 0)  {
			fprintf(stderr,"Cannot read %s\n", hdr.h_name);
			continue;
		}
		Cflag? writehdr(Buf, ct): bwrite(SBuf, ct);
	}
	close(ifile);
	return 1;
}

bintochar(t)
long t;
{
#ifdef u370
	sprintf(Chdr,"%06ho%06ho%06ho%06ho%06ho%06ho%06ho%06ho%011lo%06ho%011lo%s",
#else
	sprintf(Chdr, "%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.6ho%.11lo%.6ho%.11lo%s",
#endif
		MAGIC,Statb.st_dev,Statb.st_ino,Statb.st_mode,Statb.st_uid,
		Statb.st_gid,Statb.st_nlink,Statb.st_rdev & 00000177777,
		Statb.st_mtime,(short)strlen(hdr.h_name)+1,t,hdr.h_name);
}

newer()
{
	return Statb.st_mtime > Newer;
}

/* support functions */
scomp(a, b, s) /* funny signed compare */
register a, b;
register char s;
{
	if(s == '+')
		return(a > b);
	if(s == '-')
		return(a < -(b));
	return(a == b);
}

doex(com)
{
	register np;
	register char *na;
	static char *nargv[50];
	static ccode;
	static pid;

	ccode = np = 0;
	while (na=Argv[com++]) {
		if(strcmp(na, ";")==0) break;
		if(strcmp(na, "{}")==0) nargv[np++] = Pathname;
		else nargv[np++] = na;
	}
	nargv[np] = 0;
	if (np==0) return(9);
	if(pid = fork())
		while(wait(&ccode) != pid);
	else { /*child*/
		chdir(Home);
		execvp(nargv[0], nargv, np);
		exit(1);
	}
	return(ccode ? 0:1);
}

getunum(t, s)
int	t;
char	*s;
{
	register i;
	struct	passwd	*getpwnam(), *pw;
	struct	group	*getgrnam(), *gr;

	i = -1;
	if( t == UID ){
		if( ((pw = getpwnam( s )) != (struct passwd *)NULL) && pw != (struct passwd *)EOF )
			i = pw->pw_uid;
	} else {
		if( ((gr = getgrnam( s )) != (struct group *)NULL) && gr != (struct group *)EOF )
			i = gr->gr_gid;
	}
	return(i);
}

descend(name, fname, exlist)
struct anode *exlist;
char *name, *fname;
{
	int	dir = 0, /* open directory */
		dsize,
		entries,
		cdval = 0;
	struct direct dentry[32];
	register struct direct	*dp;
	register char *c1, *c2;
	register long	offset, dirsize;
	register i;
	char *endofname;

	if(stat(fname, &Statb)<0) {
		fprintf(stderr,"find: bad status-- %s\n", name);
		return(0);
	}
	if(!depthf)
		(*exlist->F)(exlist);
	if((Statb.st_mode&S_IFMT)!=S_IFDIR) {
		if(depthf)
			(*exlist->F)(exlist);
		return(1);
	}

	for(c1 = name; *c1; ++c1);
	if((int)(c1-name) >= PATHLEN-sizeof dentry[0].d_name) {
		fprintf(stderr,"Pathname too long");
		exit(2);
	}
	if(*(c1-1) == '/')
		--c1;
	endofname = c1;
	if(Statb.st_size > 32000)
		fprintf(stderr,"Huge directory %s --call administrator\n", name);
	dirsize = Statb.st_size;

	if((cdval=chdir(fname)) == -1) {
		fprintf(stderr,"find: cannot chdir to %s\n", name);
	} else {
		for(offset=0 ; offset < dirsize;) { /* each block */
			dsize = BUFSIZE<(dirsize-offset)? BUFSIZE: (dirsize-offset);
			if(!dir) {
				if((dir=open(".", 0))<0) {
					fprintf(stderr,"find: cannot open %s\n", name);
					break;
				}
				if(offset) lseek(dir, (long)offset, 0);
			}
			if((i = read(dir, (char *)dentry, dsize)) <= 0) {
				fprintf(stderr,"find: cannot read %s\n", name);
				break;
			}
			offset += i;
			if(dir > 10 || offset == dirsize) {
				close(dir);
				dir = 0;
			}
			for(dp=dentry, entries=dsize>>4; entries; --entries, ++dp) { /* each directory entry */
				if(dp->d_ino==0
				|| (dp->d_name[0]=='.' && dp->d_name[1]=='\0')
				|| (dp->d_name[0]=='.' && dp->d_name[1]=='.' && dp->d_name[2]=='\0'))
					continue;
				c1 = endofname;
				*c1++ = '/';
				c2 = dp->d_name;
				for(i=0; i<14; ++i)
					if(*c2)
						*c1++ = *c2++;
					else
						break;
				*c1 = '\0';
				if(c1 == (endofname + 1)) { /* ?? */
					break;
				}
				Fname = endofname+1;
				descend(name, Fname, exlist);
			}
		}
	}
	if(dir)
		close(dir);
	c1 = endofname;
	*c1 = '\0';
	if(cdval == -1 || chdir("..") == -1) {
		if((endofname=strrchr(Pathname,'/')) == Pathname)
			chdir("/");
		else {
			if(endofname != NULL)
				*endofname = '\0';
			chdir(Home);
			if(chdir(Pathname) == -1) {
				fprintf(stderr,"find: bad directory tree\n");
				exit(1);
			}
			if(endofname != NULL)
				*endofname = '/';
		}
	}
	if(depthf) {
		if(stat(fname, &Statb) < 0) {
			fprintf(stderr,"cannot stat %s\n", fname);
		}
		(*exlist->F)(exlist);
	}
/*	*c1 = '/';	*/
	return;
}

gmatch(s, p)
register char	*s, *p;
{
	register int	scc;
	char		c;

	if(scc = *s++) {
		if((scc &= 0177) == 0) {
			scc = 0200;
		}
	}
	switch(c = *p++) {

	case '[':
		{
			int ok; 
			int lc; 
			int notflag = 0;

			ok = 0; 
			lc = 077777;
			if( *p == '!' ) {
				notflag = 1; 
				p++; 
			}
			while( c = *p++ ) {
				if(c == ']') {
					return(ok?gmatch(s,p):0);
				} else if (c == '-') {
					if(notflag) {
						if(lc > scc || scc > *(p++)) {
							ok++;
						} else { 
							return(0);
						}
					} else { 
						if( lc <= scc && scc <= (*p++)) {
							ok++;
						}
					}
				} else {
					if(notflag) {
						if(scc != (lc = (c&0177))) {
							ok++;
						} else {
							return(0);
						}
					} else { 
						if(scc == (lc = (c&0177))) { 
							ok++;
						}
					}
				}
			}
			return(0);
		}
	case '?':
		return(scc?gmatch(s,p):0);

	case '*':
		if(*p == 0) {
			return(1);
		}
		--s;
		while(*s) {
			if(gmatch(s++,p)) {
				return(1);
			} 
		}
		return(0);

	case 0:
		return(scc == 0);

	default:
		if((c&0177) != scc) {
			return(0);
		}

	}
	return(gmatch(s,p)?1:0);
}

bwrite(rp, c)
register short *rp;
register c;
{
	register short *wp = Wp;

	c = (c+1) >> 1;
	while(c--) {
		if(!Wct) {
again:
			if(write(Cpio, (char *)Dbuf, Bufsize)<0) {
				Cpio = chgreel(1, Cpio);
				goto again;
			}
			Wct = Bufsize >> 1;
			wp = Dbuf;
			++Blocks;
		}
		*wp++ = *rp++;
		--Wct;
	}
	Wp = wp;
}

writehdr(rp, c)
register char *rp;
register c;
{
	register char *cp = Cp;

	while (c--)  {
		if (!Cct)  {
again:
			if(write(Cpio, Cbuf, Bufsize) < 0)  {
				Cpio = chgreel(1, Cpio);
				goto again;
			}
			Cct = Bufsize;
			cp = Cbuf;
			++Blocks;
		}
		*cp++ = *rp++;
		--Cct;
	}
	Cp = cp;
}

chgreel(x, fl)
{
	register f;
	char str[22];
	FILE *devtty;
	struct stat statb;

	fprintf(stderr,"find: can't %s\n", (x? "write output": "read input"));
	fstat(fl, &statb);
	if((statb.st_mode&S_IFMT) != S_IFCHR)
		exit(1);
again:
	fprintf(stderr,"If you want to go on, type device/file name when ready\n");
	devtty = fopen("/dev/tty", "r");
	fgets(str, 20, devtty);
	str[strlen(str) - 1] = '\0';
	if(!*str)
		exit(1);
	close(fl);
	if((f = open(str, x? 1: 0)) < 0) {
		fprintf(stderr,"That didn't work");
		fclose(devtty);
		goto again;
	}
	return f;
}
