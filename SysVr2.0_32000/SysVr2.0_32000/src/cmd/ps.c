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
/*	@(#)ps.c	1.32	*/

/*
 *	ps - process status
 *	examine and print certain things about processes
 */
#include <stdio.h>
#include <fcntl.h>
#include "sys/types.h"
#include <ctype.h>

#include "nlist.h"
#include "pwd.h"
#include "sys/param.h"
#ifdef u3b
#include "sys/page.h"
#include "sys/seg.h"
#include "sys/sbr.h"
#include "sys/region.h"
#endif
#include "sys/sysmacros.h"
#ifdef u3b2
#include "sys/sbd.h"
#include "sys/immu.h"
#endif
#include "sys/proc.h"
#include "sys/tty.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/stat.h"
#include "sys/user.h"
#include "sys/var.h"
#if pdp11 || vax || u3b || ns32000
#include "core.h"
#endif

#define NTTYS	20
#define SIZ 	30
#define TSIZE	100
#define ARGSIZ	30

#ifndef MAXLOGIN
#define MAXLOGIN	8
#endif

/* Structure for storing user info */
struct udata {
	unsigned short uid;	/* numeric user id */
	char name[MAXLOGIN];	/* character user id, may not be null terminated */
};

/* udata granularity for structure allocation */
#define UDQ	50

/* Pointer to user data */
struct udata *ud;
int	nud = 0;	/* number of valid ud structures */
int	maxud = 0;	/* number of ud's allocated */

struct udata uid_tbl[SIZ];	/* table to store selected uid's */
int	nut = 0;	/* counter for uid_tbl */

struct nlist nl[] = {
#if u3b || u3b5 || u3b2
	"proc", (long)0, (short)0, (unsigned short)0, (char)0, (char)0,
#ifdef u3b
	"swaplow", (long)0, (short)0, (unsigned short)0, (char)0, (char)0,
#endif
#if u3b5 || u3b2
	"swplo", (long)0, (short)0, (unsigned short)0, (char)0, (char)0,
#endif
	"v", (long)0, (short)0, (unsigned short)0, (char)0, (char)0,
#ifdef u3b
	"_sbrpte", (long)0, (short)0, (unsigned short)0, (char)0, (char)0,
#endif
	0, (long)0, (short)0, (unsigned short)0, (char)0, (char)0
#endif
#if pdp11 || vax
	{ "_proc" },
	{ "_swplo" },
	{ "_v" },
	{ "_sbrpte" },
	{ 0 },
#endif
#if ns32000
	{ "_proc" },
	{ "_swplo" },
	{ "_v" },
	{ "_kpte2" },
	{ 0 },
#endif
};

#if pdp11 || vax || u3b5 || u3b2 || ns32000
union {
	struct proc mprc;
	struct xproc zprc;
	} prc;
#define mproc prc.mprc
#define zproc prc.zprc
#endif
#ifdef u3b
struct proc prc;
#define	mproc	prc
#define	zproc	prc
#endif /* u3b */
struct	var  v;
#if pdp11 || vax
int	sbrpte;
int	upte[128];
int	spte;
#endif
#if ns32000
struct {
	int pte, pte2;
} udotpte[USIZE];
#endif
#if pdp11 || u3b || u3b5 || u3b2
struct	user u;
#endif
#ifdef vax
union { struct user yy;
	int xx[128] [USIZE];
	} zz;
#define u zz.yy
int	mf;
#endif
#ifdef ns32000
union { struct user yy;
	int xx[256] [USIZE];
	} zz;
#define u zz.yy
#endif
#if u3b || u3b5 || u3b2
int	file;
#endif


int	retcode=1;
int	c;
int	lflg;
int	eflg;
int	uflg;
int	aflg;
int	dflg;
int	pflg;
int	fflg;
int	gflg;
int	nflg;
int	tflg;
int	sflg;
int	errflg;
char	*gettty();
char	*ttyname();
int	memfd;
int	swmem;
int	swap;
daddr_t	swplo;
char	argbuf[ARGSIZ];
char	*parg;
char	*p1;
char	*coref;
char	*memf;
char	*system;
long lseek();
static char stdbuf[BUFSIZ];

#define NDEV	1024
int	ndev;
struct devl {
	char	dname[DIRSIZ];
	dev_t	dev;
} devl[NDEV];

char	*tty[NTTYS];	/* for t option */
int	ntty = 0;
int	pid[SIZ];	/* for p option */
int	npid = 0;
int	grpid[SIZ];	/* for g option */
int	ngrpid = 0;

main(argc, argv)
char **argv;
{
	register char **ttyp = tty;
	char *name;
	char *p;
	int puid, ppid, ppgrp;
	int i, found;
	extern char *optarg;
	extern int optind;
	char	*getstr;
	char *usage="ps [ -edalf ] [ -c corefile ] [ -s swapdev ] [ -n namelist ] [ -t tlist ]";
	char *usage2="	[ -p plist ] [ -u ulist ] [ -g glist ]";
	char *malloc();
	unsigned size;

	getstr = "lfeadn:s:c:t:p:g:u:";
	system = "/unix";
#if vax || u3b || u3b5 || u3b2 || ns32000
		coref = "/dev/kmem";
		memf = "/dev/mem";
#endif
#ifdef pdp11
		coref = "/dev/mem";
		memf = coref;
#endif

	setbuf(stdout, stdbuf);
	while ((c = getopt(argc,argv,getstr)) != EOF)
		switch(c) {
		case 'l':		/* long listing */
			lflg++;
			break;

		case 'f':		/* full listing */
			fflg++;
			break;

		case 'e':		/* list for every process */
			eflg++;
			tflg = uflg = pflg = gflg = 0;
			break;

		case 'a':		/* same as e except no proc grp leaders */
			aflg++; 	/* and no non-terminal processes     */
			break;

		case 'd':		/* same as e except no proc grp leaders */
			dflg++;
			break;

		case 'n':		/* alternate namelist */
			nflg++;
			system = optarg;
			break;

		case 'c':		/* core file given */
			coref = optarg;	
			memf = coref;
			break;
		case 's':		/* swap device given */
			sflg++;
			if ((swap = open(optarg, 0)) < 0 ) {
				fprintf(stderr, "ps: cannot open %s\n",optarg);
				done(1);
			}
			break;

		case 't':		/* terminals */
			tflg++;
			p1 = optarg;
			do {
				parg = argbuf;
				if (ntty >= NTTYS)
					break;
				getarg();
		        	if ((p = malloc(14)) == NULL) {
					fprintf(stderr,"ps: no memory\n");
					done(1);
				}
		        	size = 14;
		        	if(isdigit(*parg)) {
		        	        strcpy(p, "tty");
		        	        size -= 3;
		        	}
		        	strncat(p, parg, size);
				*ttyp++ = p;
				ntty++;
			}
			while (*p1);
			break;

		case 'p':		/* proc ids */
			pflg++;
			p1 = optarg;
			parg = argbuf;
			do {
				if (npid >= SIZ)
					break;
				getarg();
				pid[npid++] = atoi(parg);
			}
			while (*p1);
			break;

		case 'g':		/* proc group */
			gflg++;
			p1 = optarg;
			parg = argbuf;
			do {
				if (ngrpid >= SIZ)
					break;
				getarg();
				grpid[ngrpid++] = atoi(parg);
			}
			while (*p1);
			break;

		case 'u':		/* user name or number */
			uflg++;
			p1 = optarg;
			parg = argbuf;
			do {
				getarg();
				if(nut < SIZ)
					strncpy(uid_tbl[nut++].name,parg,MAXLOGIN);
			}
			while (*p1);
			break;

		case '?':		/* error */
			errflg++;
			break;
		}
	if ( errflg || (optind < argc)) {
		fprintf(stderr,"usage: %s\n%s\n",usage,usage2);
		done(1);
	}
	if (tflg)
		*ttyp = 0;
	/* if specifying options not used, current terminal is default */
	if ( !(aflg || eflg || dflg || uflg || tflg || pflg || gflg )) {
		name = ttyname(2);
		*ttyp++ = name+5;
		*ttyp = 0;
		ntty++;
		tflg++;
	}
	if (eflg)
		tflg = uflg = pflg = gflg = aflg = dflg = 0;
	if (aflg || dflg)
		tflg = 0;
	/* get data from psfile */
	if(!readata()) {
		getdev();
		getpass();
		getnl();
		if(!nflg)
			wrdata();
	}else {
		if(nflg)
			getnl();
	}
	uconv();
	if(nl[0].n_value==0||nl[1].n_value==0||nl[2].n_value==0) {
		fprintf(stderr, "ps: no namelist\n");
		done(1);
	}
#ifdef	vax
	nl[0].n_value = ((int)nl[0].n_value & 0x3fffffff);
	nl[1].n_value = ((int)nl[1].n_value & 0x3fffffff);
	nl[2].n_value = ((int)nl[2].n_value & 0x3fffffff);
	nl[3].n_value = ((int)nl[3].n_value & 0x3fffffff);
#endif
	if ((memfd = open(coref, 0)) < 0) {
		fprintf(stderr, "ps: no mem\n");
		done(1);
	}
	if ((swmem = open(memf,0)) < 0) {
		fprintf(stderr, "ps: no mem\n");
		done(1);
	}
	/*
	 * Find base of swap
	 */
	l_lseek(memfd, (long)nl[1].n_value, 0);
	r_read(memfd, (char *)&swplo, sizeof(swplo));
	/*
	 * read to find proc table size
	 */
	l_lseek(memfd, (long)nl[2].n_value, 0);
	r_read(memfd, (char *)&v, sizeof(v));
#ifdef vax
	/*
	 * read to find system page tables
	 */
	l_lseek(memfd, (long)nl[3].n_value, 0);
	r_read(memfd, (char *)&sbrpte, sizeof(sbrpte));
	sbrpte &= 0x3fffffff;
#endif
	/*
	 * Locate proc table
	 */
	l_lseek(memfd, (long)nl[0].n_value, 0);
/* this opens swap */
	if ( !sflg ) {
		if ((swap = open("/dev/swap", 0)) < 0) {
			fprintf(stderr, "ps: cannot open /dev/swap\n");
			done(1);
		}
	}
	if (fflg && lflg )
		printf("  F S     UID   PID  PPID  C PRI NI   ADDR  SZ    WCHAN    STIME TTY      TIME COMD\n");
	else if (fflg)
		printf("     UID   PID  PPID  C    STIME TTY      TIME COMMAND\n");
	else if (lflg)
		printf("  F S   UID   PID  PPID  C PRI NI   ADDR  SZ    WCHAN TTY      TIME COMD\n");
	else
		printf("   PID TTY      TIME COMMAND\n");
	/* determine which processes to print info about */
	for (i=0; i<v.v_proc; i++) {
		found = 0;
		r_read(memfd, (char *)&mproc, sizeof(mproc));
		if (mproc.p_stat == 0)	
			continue;
		puid = mproc.p_uid;
		ppid = mproc.p_pid;
		ppgrp = mproc.p_pgrp;
		if ((ppid == ppgrp) && (dflg || aflg))
			continue;
		if (eflg || dflg)
			found++;
		else if (pflg && search(pid, npid, ppid))
			found++;
		else if (uflg && ufind(puid))
			found++;
		else if (gflg && search(grpid, ngrpid, ppgrp))
			found++;
		if ( !found && !tflg && !aflg )
			continue;
		if (prcom(puid,found)) {
			printf("\n");
			retcode =0;
		}
	}
	done(retcode);
}

/* readata reads in the open devices (terminals) and stores */
/* info in the devl structure. */

static char psfile[] = "/etc/ps_data";

int readata()
{
	struct stat sbuf1, sbuf2;
	int fd;

	if (stat(psfile, &sbuf1) < 0
	    || (stat("/dev", &sbuf2) < 0 || sbuf1.st_mtime <= sbuf2.st_mtime || sbuf1.st_mtime <= sbuf2.st_ctime)
	    || (stat("/unix", &sbuf2) < 0 || sbuf1.st_mtime <= sbuf2.st_mtime || sbuf1.st_mtime <= sbuf2.st_ctime)
	    || (stat("/etc/passwd", &sbuf2) < 0 || sbuf1.st_mtime <= sbuf2.st_mtime || sbuf1.st_mtime <= sbuf2.st_ctime)) {
		return(0);
	}
	if(( fd = open(psfile, O_RDONLY)) < 0)
		return(0);

	/* read /dev data from psfile */
    	psread(fd, &ndev, sizeof(ndev));
	psread(fd, devl, ndev * sizeof(*devl));

	/* read /etc/passwd data from psfile */
	psread(fd, &nud, sizeof(nud));
	if((ud = (struct udata *)malloc(nud * sizeof(*ud))) == NULL) {
		fprintf(stderr, "ps: not enough memory\n");
		exit(1);
	}
	psread(fd, ud, nud * sizeof(*ud));

	/* read /unix data from psfile */
	if(!nflg)
		psread(fd, nl, sizeof(nl));

	close(fd);
	return(1);
}


getdev()
{
	register FILE *df;
	struct direct dbuf;
	struct stat sbuf1;

	if ((df = fopen("/dev", "r")) == NULL) {
		fprintf(stderr, "ps: cannot open /dev\n");
		done(1);
	}
	if (chdir("/dev") < 0) {
		fprintf(stderr, "ps: cannot change to /dev\n");
		done(1);
	}
	ndev = 0;
	while (fread((char *)&dbuf, sizeof(dbuf), 1, df) == 1) {
		if(dbuf.d_ino == 0)
			continue;
		if(stat(dbuf.d_name, &sbuf1) < 0)
			continue;
		if ((sbuf1.st_mode&S_IFMT) != S_IFCHR)
			continue;
		strcpy(devl[ndev].dname, dbuf.d_name);
		devl[ndev].dev = sbuf1.st_rdev;
		ndev++;
	}
	fclose(df);
}

/* Get the passwd file data into the ud structure */
getpass()
{
	struct passwd *pw, *getpwent();
	char *malloc(), *realloc();

	ud = NULL;
	nud = 0;
	maxud = 0;

	while((pw=getpwent()) != NULL) {
		while(nud >= maxud) {
			maxud += UDQ;
			ud = (struct udata *) ((ud == NULL) ?
				malloc(sizeof(struct udata) * maxud) :
				realloc(ud, sizeof(struct udata) * maxud));
			if(ud == NULL) {
				fprintf(stderr,"ps: not enough memory for %d users\n", maxud);
				exit(1);
			}
		}
		/* copy fields from pw file structure to udata */
		ud[nud].uid = pw->pw_uid;
		strncpy(ud[nud].name, pw->pw_name, MAXLOGIN);
		nud++;
	}
	endpwent();
}

/* Get name list data into nl structure */
getnl()
{
	nlist(system, nl);
}

wrdata()
{
	int fd;

	umask(02);
	unlink(psfile);
	if((fd = open(psfile, O_WRONLY | O_CREAT | O_EXCL, 0664)) > -1) {
		/* write /dev data */
		pswrite(fd, &ndev, sizeof(ndev));
		pswrite(fd, devl, ndev * sizeof(*devl));

		/* write /etc/passwd data */
		pswrite(fd, &nud, sizeof(nud));
		pswrite(fd, ud, nud * sizeof(*ud));

		/* write /unix data */
		pswrite(fd, nl, sizeof(nl));

		close(fd);
	}
}


/* getarg finds next argument in list and copies arg into argbuf  */
/* p1 first pts to arg passed back from getopt routine.  p1 is then */
/* bumped to next character that is not a comma or blank - p1 null */
/* indicates end of list    */

getarg()
{
	char *parga;
	parga = argbuf;
	while(*p1 && *p1 != ',' && *p1 != ' ')
		*parga++ = *p1++;
	*parga = '\0';

	while( *p1 && ( *p1 == ',' || *p1 == ' ') )
		p1++;
}

/* gettty returns the user's tty number or ? if none  */
char *gettty()
{
	register i;
	register char *p;

	if (u.u_ttyp==0)
		return("?");
	for (i=0; i<ndev; i++) {
		if (devl[i].dev == u.u_ttyd) {
			p = devl[i].dname;
			return(p);
		}
	}
	return("?");
}

#ifdef pdp11
long	round(a,b)
	long		a, b;
{
	long		w = ((a+b-1)/b)*b;

	return(w);
}

typedef unsigned pos;

struct map {
	long	b1, e1; long f1;
	long	b2, e2; long f2;
};
struct map datmap;
#endif

#if pdp11 || u3b5 
int	file;
#endif


/* print info about the process */
prcom(puid,found)
int puid,found;
{
#ifdef u3b
	char line[80];
	int page;
#endif
	register int *ip;
	register char *cp, *cp1;
	register char *tp;
	long addr;
	char *ctime();
	time_t time();
	time_t *clock, *tloc;
	time_t tim;
	char timbuf[26];
	char *curtim = timbuf;
	char *sttim, *s1;
	long tm;
#if pdp11 || u3b5 || u3b2
	int abuf[BSIZE*2/sizeof(int)];
	int nbad, badflg;
#endif
#ifdef pdp11
	long	txtsiz, datsiz, stksiz;
	int	septxt;
	int	lw=(lflg?35:80);
#endif
	int	match, i;
	int	nbytes;
	int	uzero = 0;
	register char **ttyp, *str;
#if pdp11 || vax || u3b5 || u3b2 || ns32000
	int	ww;
	int	adrv0;
	int	*ip1;
#endif
#if pdp11 || u3b5 || u3b2
	int	cnt;
#endif

	/* if process is zombie, call print routine and return */
	if (mproc.p_stat==SZOMB) {
		if ( tflg && !found)
			return(0);
		else {
			przom(puid);
			return(1);
		}
	}
	/* Determine if process is in memory or swap space */
	/* and read in user block */
#if u3b5 || u3b2
	if (mproc.p_flag & SLOAD || mproc.p_flag & SSPART) {
#ifdef u3b2
		addr = ctob((int)mproc.p_addr) & (~MAINSTORE);
#else
#ifdef u3b5
		addr = ctob((int)mproc.p_addr) & (~VUSER);
#endif
#endif
		file = swmem;
	} else {
#ifdef u3b2
		addr = ( swplo + mproc.p_swaddr + ctod(mproc.p_swsize-USIZE-mproc.p_ssize) ) << BSHIFT;
#else
#ifdef u3b5
		addr = ( swplo + mproc.p_swaddr + ctod(mproc.p_swsize-USIZE) ) << BSHIFT;
#endif
#endif
		file = swap;
	}
	l_lseek(file, addr, 0);
	if ((nbytes = read(file, (char *)&u, sizeof(u))) != sizeof(u))
		if ((nbytes == 0) && sflg && (file == swap)) {
			uzero++;
			for ( i = 0, tp = (char *)&u; i < sizeof(u); i++)
				*tp++ = '\0';
		} else
			return(0);
#endif
#ifdef u3b
	cp1 = (char *)&u + sizeof(u);
	for(i = 0, cp = (char *) &u; cp < cp1; i++, cp += NBPP) {
		l_lseek(swmem, mproc.p_uptbl[i]&PG_ADDR, 0);
		if (read(swmem, cp, cp1-cp > NBPP ? NBPP : cp1-cp) < 0)
			fprintf(stderr, "ps: cannot read %s\n", swmem);
		}
#endif /* u3b */
#ifdef vax
	ww = (int)(mproc.p_spt + (mproc.p_nspt-1)*128);
	ww = sbrpte + ((ww&0x3fffffff)>>9)*4;
	l_lseek(swmem,(long)ww&0x3fffffff,0);
	r_read(swmem,&spte, 4);
	l_lseek(swmem,(spte&0x1fffff)<<9,0);
	r_read(swmem, upte, sizeof upte);
	for(c=0; c<USIZE; c++) {
		l_lseek(swmem, (upte[128-USIZE+c]&0x1fffff)<<9, 0);
		if (read(swmem,(char *)(((int *)&u)+128*c),512) != 512)	/* get u page */
			return(0);
		if(sizeof(struct user) - (c + 1) * 512 <= 0)
			break;
	}
#endif
#ifdef ns32000
	ww = (mproc.p_addr << 10) + 512;
	l_lseek(swmem,(long)ww,0);
	r_read(swmem,udotpte, sizeof(udotpte));
	for(c=0; c<USIZE; c++) {
		l_lseek(swmem, udotpte[c].pte & 0xfffc00, 0);
		if (read(swmem,(char *)(((int *)&u)+256*c),1024) != 1024)	/* get u page */
			return(0);
		if(sizeof(struct user) - (c + 1) * 1024 <= 0)
			break;
	}
#endif
#ifdef pdp11
	if (mproc.p_flag&SLOAD) {
		addr = ctob((long)mproc.p_addr);
		file = swmem;
	} else {
		addr = (mproc.p_addr+swplo)<<9;
		file = swap;
	}
	l_lseek(file, addr, 0);
	if ((nbytes = read(file, (char *)&u, sizeof(u))) != sizeof(u))
		if (( nbytes == 0) && sflg && (file == swap)) {
			uzero++;
			for ( i = 0, tp = (char *) &u; i < sizeof(u); i++)
				*tp++ = '\0';
		} else 
			return(0);
#endif

	/* get current terminal - if none (?) and aflg is set */
	/* then don't print info - if tflg is set, check if term */
	/* is in list of desired terminals and print if so   */
	tp = gettty();
	if ( aflg && (*tp == '?' ))
		return(0);
	if(tflg && !found) {	/* the t option */
		for (ttyp=tty, match=0; (str = *ttyp) !=0 && !match; ttyp++)
			if (strcmp(tp,str) == 0)
				match++;
		if(!match)
			return(0);
	}

	if (lflg)
#ifndef u3b
		printf("%3o %c", mproc.p_flag&0377, "OSWRIZTXX"[mproc.p_stat]);	/* F S */
#else
		printf("%3o %c", mproc.p_flag&0377, "-SRZTIO"[mproc.p_stat]);	/* F S */
#endif
	if (fflg) {
		i = getunam(puid);
		if (i >= 0)
			printf("%8.8s", ud[i].name);
		else
			printf("%8.8u", puid);
	}
	else if (lflg)
		printf("%6u", puid);
	printf("%6u",mproc.p_pid);	/* PID */
	if (lflg || fflg)
		printf("%6u%3d", mproc.p_ppid, mproc.p_cpu&0377);	/* PPID  CPU */
	if (lflg) {
		printf("%4d%3d",mproc.p_pri, mproc.p_nice);	/* PRI  NICE */
#if vax || u3b || u3b5 || u3b2 || ns32000
		printf("%7x%4d",
#if vax || u3b5 || u3b2 || ns32000
			mproc.p_addr,
#endif
#if u3b
			(mproc.p_sbr & S_STBL)<<2,
#endif
			mproc.p_size);	/* ADDR  SZ */
		if (mproc.p_wchan)
			printf("%9x",mproc.p_wchan);	/* WCHAN */
		else
			printf("         ");
#endif
#ifdef pdp11
		printf("%8o%3d", mproc.p_addr, (mproc.p_size+7)>>3);
		if (mproc.p_wchan)
			printf("%9o", mproc.p_wchan);
		else
			printf("         ");
#endif
	}
	if (uzero) 		/* u-block zeroed out so return */
		return(1);
	if (fflg) {		  /* STIME*/
		clock = &u.u_start;
		tim = time((time_t *) 0);
		tloc = &tim;
		s1 = ctime(tloc);
		strcpy(curtim,s1);
		sttim = ctime(clock);
		prtim(curtim, sttim);
	}
	printf(" %-7.7s", tp);  /* TTY */
	tm = (u.u_utime + u.u_stime + HZ/2)/HZ;	/* TIME */
	printf(" %2ld:%.2ld", tm/60, tm%60);
#ifndef u3b
	if(mproc.p_pid == 0) {
		printf(" swapper");
		return(1);
	}
#endif
	/* if fflg not set, print command from u_block */

	if (!fflg) {		/* CMD */
		printf(" %.8s", u.u_comm);
		return(1);
	}
	/* set up address maps for user pcs */
#ifdef pdp11
	txtsiz = ctob(u.u_tsize);
	datsiz = ctob(u.u_dsize);
	stksiz = ctob(u.u_ssize);
	septxt = u.u_sep;
	datmap.b1 = (septxt ? 0 : round(txtsiz,TXTRNDSIZ));
	datmap.e1=datmap.b1+datsiz;
	datmap.f1 = ctob(USIZE)+addr;
	datmap.b2 = stackbas(stksiz);
	datmap.e2 = stacktop(stksiz);
	datmap.f2 = ctob(USIZE)+(datmap.e1-datmap.b1)+addr;

	/* determine if process is a shell or not */ 
	/* if last word is null then shell */
	
	addr += ctob(mproc.p_size) - sizeof(abuf);
	l_lseek(file, addr+sizeof(abuf)-sizeof(int), 0);
	if (read(file, (char *)abuf, sizeof(int)) != sizeof(int))
		return(1);
	if (abuf[0]&0177400) {
		char b[82];
		char *bp = b;
		char **ap = abuf[0];
		char *cp;
	
		*bp++ = ' ';
		badflg = 0;
		while((cp=(char *)getword(ap++)) != -1 && cp && (bp < b+lw)){
			nbad = 0;
			while((c = getbyte(cp++)) && (int)(cp) != 0177777 && (bp < b + lw)){
				badflg++;
				if (c < ' ' || c > '~') {
					if (nbad++ > 3)
						break;
					continue;
				}
				*bp++ = c;
			}
			*bp++ = ' ';
		}
		*bp++ = '\0';
		if ( badflg == 0 || nbad != 0 || *b == '\0' )
			printf(" [ %.8s ]",u.u_comm);
		else
			printf("%.20s",b);
		return(1);
	}
	l_lseek(file, addr, 0);
	if (read(file, (char *)abuf, sizeof(abuf)) != sizeof(abuf))
		return(1);
	badflg = 0;
	for (ip = &abuf[BSIZE*2/sizeof(int)-sizeof(int)]; ip > abuf;) {
		if (*--ip == -1 || *ip == 0) {
			cp = (char *)(ip+1);
			if (*cp==0)
				cp++;
			nbad = 0;
			/* find argc */
			cnt = 0;
			adrv0 = (int)(ip+1) - (int)(abuf) + (int)(USRSTACK - sizeof(abuf));
			for (ip1 = ip; ip1 > abuf;) {
				if (*--ip1 == adrv0) {
					cnt = *(ip1 - 3);
					break;
				}
			}
			for (cp1 = cp; cp1 < (char *)&abuf[BSIZE*2/sizeof(int)]; cp1++) {
				badflg++;
				c = *cp1&0177;
				if (c==0) {
					if (--cnt <= 0) {
						*cp1++ = 0;
						break;
					}
					else {
						*cp1 = ' ';
					}
				}
				else if (c < ' ' || c > '~') {
					if (++nbad >= 3) {
						*cp1++ = ' ';
						break;
					}
					*cp1 = '?';
				}
			}
			while (*--cp1==' ')
				*cp1 = 0;
			if ( cnt != 0 ||  badflg == 0 || nbad != 0 || *cp == '\0')
				printf(" [ %.8s ]",u.u_comm);
			else
				printf(lflg?" %.35s":" %.80s", cp);
			return(1);
		}
	}
	printf(" [ %.8s ]",u.u_comm);
#endif
#if u3b || vax || ns32000
	u.u_psargs[PSARGSZ] = 0;
	for (cp=u.u_psargs; cp<&u.u_psargs[PSARGSZ]; cp++) {
		if (*cp == 0) break;
		if (*cp < ' ' || *cp > '~') {
			printf(" [ %.8s ]", u.u_comm);
			return(1);
		}
	}
	printf(lflg ? " %.35s" : " %.80s", u.u_psargs);
#endif
#if u3b5 || u3b2
	if ( mproc.p_flag & SLOAD || mproc.p_flag & SSPART )
		/* in memory */
#ifdef u3b2
		addr = u.u_sdt.seg[STKSEG].wd2.address & (~MAINSTORE);
#else
#ifdef u3b5
		addr = ctob( (int)u.u_segdata.seg[STKSEG].base ) & (~VUSER);
#endif
#endif
	else
		/* on swapdev */
#ifdef u3b2
		addr = (swplo + mproc.p_swaddr + ctod(mproc.p_swsize-mproc.p_ssize)) << BSHIFT;
#else
#ifdef u3b5
		addr = (swplo + mproc.p_swaddr + ctod(mproc.p_swsize-mproc.p_ssize-USIZE)) << BSHIFT;
#endif
#endif
	l_lseek( file, addr, 0 );
	if ( read( file, (char*)abuf, sizeof(abuf) ) != sizeof(abuf) )
		return( 1 );
	badflg = 0;
	for ( ip=abuf; ip<&abuf[BSIZE*2/sizeof(int)]; ++ip)
		{
		/* Since the maximum number of argv[] and environ[] characters
		 * is 5120 (os/sys1.c), then there can be at most 5120/2 arguments
		 * since each argument must be terminated by a NULL.  Therefore,
		 * if an int is found whose value is <= 5120/2, then it is
		 * assumed to be argc.  It is possible that the arguments end in
		 * such a way that the NULL terminating the last argument is the
		 * first byte in a word (this word is zero); argc is then in the
		 * next word.  So, if we find a word of zero, we will skip it and
		 * check the next word.
		 */
		if ( (unsigned)( cnt = *ip ) <= 5120/2 )
			{
			if ( cnt == 0 )
				if ( (unsigned)( cnt = *(ip+1) ) > 5120/2 )
					break;
			nbad = 0;
			for ( cp=(char*)abuf; cp<(char*)&abuf[BSIZE*2/sizeof(int)]; ++cp)
				{
				badflg++;
				c = *cp&0177;
				if (c==0)
					if (--cnt <= 0)
						break;
					else
						*cp = ' ';
				else
					if (c < ' ' || c > '~')
						if (++nbad >= 3)
							{
							*cp = ' ';
							break;
							}
					else
						*cp = '?';
				}
			while (*--cp==' ')
				*cp = 0;
			if ( cnt != 0 ||  badflg == 0 || nbad != 0 || *cp == '\0')
				printf(" [ %.8s ]",u.u_comm);
			else
				printf(lflg?" %.35s":" %.80s", abuf);
			return(1);
			}
		}
	printf(" [ %.8s ]",u.u_comm);
#endif /* u3b5  || u3b2 */
	return(1);
}


/* file handling and access routines */

#ifdef pdp11
getbyte(adr)
	pos	adr;
{
	return((int)access(adr,1));
}

getword(adr)
	pos	adr;
{
	return((int)access(adr,sizeof(int*)));
}

access(aadr,size)
pos aadr;
int size;
{
	int *word = 0;
	register struct map *amap = &datmap;
	long adr = aadr;

	if(!within(aadr,amap->b1,amap->e1)) {
		if(within(aadr,amap->b2,amap->e2)) 
			adr += (amap->f2)-(amap->b2);
		else
			return(0);
	}
	else {
		adr += (amap->f1)-(amap->b1);
	}
	if(lseek(file,adr,0)==-1 || read(file,(char *)&word,size)<size) {
		return(0);
	}

	return((int)word);
}


within(adr,lbd,ubd)
	pos	adr;
	long	lbd, ubd;
{
	return(adr>=lbd && adr<ubd);
}
#endif

done(exitno)
{
	exit(exitno);
}

/* search returns 1 if arg is found in array arr */
/* which has length num, and returns 0 if not found */

search(arr, num, arg)
int arr[];
int num;
int arg;
{
	int i;
	for (i = 0; i < num; i++)
		if (arg == arr[i])
			return(1);
	return(0);
}

/* after the u option */

uconv()
{
	int found;
	int pwuid;
	int i, j;

	/* search thru name array for oarg */
	for (i=0; i<nut; i++) {
		found = -1;
		for(j=0; j<nud; j++) {
			if (strncmp(uid_tbl[i].name, ud[j].name, MAXLOGIN)==0) {
				found = j;
				break;
			}
		}
		/* if not found and oarg is numeric */
		/* then search through number array */
		if (found < 0 && (uid_tbl[i].name[0] >= '0' && uid_tbl[i].name[0] <= '9')) {
			pwuid = atoi(uid_tbl[i].name);
			for (j=0; j<nud; j++) {
				if (pwuid == ud[j].uid) {
					found = j;
					break;
				}
			}
		}

		/* if found then enter found index into tbl array */
		if ( found != -1 ) {
			uid_tbl[i].uid = ud[found].uid;
			strncpy(uid_tbl[i].name,ud[found].name,MAXLOGIN);
		}else {
			fprintf(stderr,"ps: unknown user %s\n",uid_tbl[i].name);
			for(j=i+1; j<nut; j++) {
				strncpy(uid_tbl[j-1].name,uid_tbl[j].name,MAXLOGIN);
			}
			nut--;
			if (nut <= 0) exit(1);
			i--;
		}
	}
	return;
}

/* for full command (-f flag) print user name instead of number */
/* search thru existing table of userid numbers and if puid is found, */
/* return corresponding name.  Else search thru /etc/passwd */

getunam(puid)
int puid;
{
	int i;

	for(i=0; i<nud; i++)
		if(ud[i].uid == puid)
			return(i);
	return(-1);
}

/* ufind will return 1 if puid is in table ; if not return 0 */
ufind(puid)
int puid;
{
	int i;

	for(i=0; i<nut; i++)
		if(uid_tbl[i].uid == puid)
			return(1);
	return(0);
}

/* lseek with error checking */
l_lseek(fd, offset, whence)
int fd, whence;
long	offset;
{
	if (lseek(fd, offset, whence) == -1) {
		fprintf(stderr, "ps: error on lseek\n");
		done(1);
	}
}

/* read with error checking */
r_read (fd, buf, nbytes)
int	fd, nbytes;
char	*buf;
{
	if (read(fd, buf, nbytes) != nbytes) {
		fprintf(stderr, "ps: error on read\n");
		done(1);
	}
}

/* special read unlinks psfile on read error */
psread(fd, bp, bs)
int fd;
char *bp;
unsigned bs;
{
	if(read(fd, bp, bs) != bs) {
		fprintf(stderr, "ps: error on read\n");
		unlink(psfile);
	}
}

/* special write unlinks psfile on read error */
pswrite(fd, bp, bs)
int fd;
char *bp;
unsigned bs;
{
	if(write(fd, bp, bs) != bs) {
		fprintf(stderr, "ps: error on write\n");
		unlink(psfile);
	}
}

/* print starting time of process unless process started more */
/* than 24 hours ago in which case date is printed   */
/* sttim is start time and it is compared to curtim (current time ) */

prtim(curtim, sttim)
char *curtim, *sttim;
{
	char *p1, *p2;
	char dayst[3], daycur[3];
	if ( strncmp(curtim, sttim, 11) == 0) {
		p1 = sttim + 11;
		p2 = p1 + 8;
	}
	else {
		p1 = sttim + 4;
		p2 = p1 + 7;
		/* if time is < 24 hours different, then print time */
		if (strncmp(curtim+4, sttim+4, 3) == 0) {
			strncpy(dayst,sttim+8, 2);
			strcat(dayst,"");
			strncpy(daycur,curtim+8,2);
			strcat(daycur,"");
			if ((atoi(dayst) +1 == atoi(daycur)) &&
				 (strncmp(curtim+11,sttim+11,8)<=0)) {
				p1 = sttim + 11;
				p2 = p1 + 8;
			}
		}
	}
	*p2 = '\0';
	printf("%9.9s",p1);
}

#ifdef u3b
#define xp_flag p_flag
#define xp_stat	p_stat
#define xp_pid	p_pid
#define xp_ppid	p_ppid
#define xp_cpu	p_cpu
#define xp_pri	p_pri
#define xp_nice	p_nice
#define xp_utime p_utime
#define xp_stime p_stime
#endif
przom(puid)
/* print zombie process - zproc overlays mproc */
int puid;
{
	int i;
	long tm;

	if (lflg)
#ifndef u3b
		printf("%3o %c", zproc.xp_flag&0377, "OSWRIZT"[zproc.xp_stat]);	/* F S */
#else
		printf("%3o %c", zproc.xp_flag&0377, "-SRZTIO"[zproc.xp_stat]);	/* F S */
#endif
	if (fflg) {
		i = getunam(puid);
		if (i >= 0)
			printf("%8.8s", ud[i].name);
		else
			printf("%8.8u", puid);
	}
	else if (lflg)
		printf("%6u", puid);
	printf("%6u",zproc.xp_pid);	/* PID */
	if (lflg || fflg)
		printf("%6u%3d", zproc.xp_ppid, zproc.xp_cpu&0377);	/* PPID  CPU */
	if (lflg)
		printf("%4d%3d",zproc.xp_pri, zproc.xp_nice);	/* PRI  NICE */
        if (fflg)
                printf("         ");
        if (lflg)
                printf("                    ");
	tm = (zproc.xp_utime + zproc.xp_stime + HZ/2)/HZ;	/* TIME */
	printf("         %2ld:%.2ld", tm/60, tm%60);
	printf(" <defunct>");
	return;
}
