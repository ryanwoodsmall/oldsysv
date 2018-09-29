/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ps:ps.c	1.62"

/*	ps - process status					*/
/*	examine and print certain things about processes	*/

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <ftw.h>
#include "sys/types.h"
#include <ctype.h>
#include "nlist.h"
#include "pwd.h"
#include "sys/param.h"

#if u3b2 || u3b5
#include "sys/immu.h"		    /* sys/immu.h must precede sys/region.h for 3b2 */
#endif

#if u3b
#include "sys/page.h"
#include "sys/seg.h"
#include "sys/sbr.h"		/* sbr, system base register, 3B20 specific hardware*/
#include "sys/region.h"
#endif

#if u3b2
#include "sys/psw.h"					/* psw, program status word */
#include "sys/pcb.h"					/* pcb, program control blk */
#include "sys/region.h"
#include "sys/sbd.h"					/* need for MAINSTORE	    */
#endif

#include "sys/sysmacros.h"
#include "sys/proc.h"
#include "sys/tty.h"
#include "sys/dir.h"
#include "sys/signal.h"
#include "sys/stat.h"
#include "sys/user.h"
#include "sys/var.h"

#if pdp11		    /* pdp11 core file info formerly of /usr/include/core.h */
#define TXTRNDSIZ 8192L				      /* available in sys/machdep.h */
#define stacktop(siz) (0x10000L)
#define stackbas(siz) (0x10000L-siz)
#endif

#define NTTYS	20	/* max number ttys that can be specified with the -t option  */
#define SIZ 	30	/* max number processes that can be specified with -p and -g */
#define ARGSIZ	30	/* size of buffer which holds args for -t, -p & -u options   */

#ifndef MAXLOGIN
#define MAXLOGIN	8	/*max number of char in userid that will be printed  */
#endif
			/* NUMSLPS, number of sleep semaphores and hash chains.      */
			/* This does not limit the number of processes which can be  */
			/* sleeping.  They just sleep on the same semaphore.         */
			/* The following #define (NUMSLPS) must match the one in the */
			/* slp.c file in the kernel.  In future releases, the kernel */
			/* should put this value in a variable (var.h ?) so that ps  */
			/* can read it.  For now though, the kernel is frozen. 	     */
#if u3b
#define	NUMSLPS	64
#endif

/* Structure for storing user info */
struct udata {
	unsigned short uid;	/* numeric user id */
	char name[MAXLOGIN];	/* character user id, may not be null terminated */
};

/* udata and devl granularity for structure allocation */
#define UDQ	50

/* Pointer to user data */
struct udata *ud;
int	nud = 0;	/* number of valid ud structures */
int	maxud = 0;	/* number of ud's allocated */

struct udata uid_tbl[SIZ];	/* table to store selected uid's */
int	nut = 0;		/* counter for uid_tbl */

					/* see nlist(3c)			 */
					/*    semaphores on 3b20, 3b5 and vax	 */
					/* no semaphores on 3b2 or pdp11	 */
					/* swaplo(w): from /unix, 1st block # on */
struct nlist nl[] = {			/* 	      device to be used for swap */
#if u3b || u3b2 || u3b5
	"proc", (long)0, (short)0, (unsigned short)0, (char)0, (char)0,
#if u3b
	"swaplow", (long)0, (short)0, (unsigned short)0, (char)0, (char)0,
#else
	"swplo", (long)0, (short)0, (unsigned short)0, (char)0, (char)0,
#endif
	"v", (long)0, (short)0, (unsigned short)0, (char)0, (char)0,
#if u3b
	"_sbrpte", (long)0, (short)0, (unsigned short)0, (char)0, (char)0,
	"slpsemas", (long)0, (short)0, (unsigned short)0, (char)0, (char)0,
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
};

#if pdp11 || vax
union {
	struct proc mprc;
	struct xproc zprc;
	} prc;
#define mproc prc.mprc
#define zproc prc.zprc
#endif

#if u3b || u3b2 || u3b5
struct proc prc;
#define	mproc	prc
#define	zproc	prc
#endif /*u3b,u3b2,u3b5*/

struct	var  v;

#if pdp11 || vax
int	sbrpte;
int	upte[128];
int	spte;
#endif

#if pdp11 || u3b || u3b2 || u3b5
struct	user u;
#endif

#if vax
union { struct user yy;
	int xx[128] [USIZE];
	} zz;
#define u zz.yy
#endif

#if pdp11
int	sflg;
int	swap;
#endif /*pdp11*/

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
int	errflg;
int	isatty();
char	*gettty();
char	*ttyname();
int	memfd;
int	swmem;
daddr_t	swplo;		   /* disk block number, physical, type long, see sys/types.h */
char	argbuf[ARGSIZ];
char	*parg;
char	*p1;				     /* points to successive option arguments */
char	*coref;
char	*memf;
char	*sysname;
long lseek();
static char stdbuf[BUFSIZ];

int	ndev;			/* number of devices */
int	maxdev;			/* number of devl structures allocated */
				/* DIRSIZ: max char per directory, 14, see sys/dir.h  */
struct devl {			/* device list	 */		    /* and sys/param.h*/
	char	dname[DIRSIZ];	/* device name	 */
	dev_t	dev;		/* device number */
} *devl;

char	*tty[NTTYS];	/* for t option */
int	ntty = 0;
int	pid[SIZ];	/* for p option */
int	npid = 0;
int	grpid[SIZ];	/* for g option */
int	ngrpid = 0;

#if u3b || u3b2 || u3b5				/* State flag symbols, see sys/proc.h */
	static char states[] =	"-SRZTIOX";
#else
	static char states[] =	"-SWRIZTX";
#endif

extern int	chown();
extern unsigned short getegid();
extern int	errno;
extern char	*sys_errlist[];
extern void	exit();
extern char	*malloc();
extern char	*realloc();

main(argc, argv)
char **argv;
{
	register char **ttyp = tty;
	char *getstr;
	char *name;
	char *p;
	int puid, ppid, ppgrp;		/* puid: 	process user id	   */
	int i, found;			/* ppid: parent process id, see sys/proc.h */
	extern char *optarg;		/* ppgrp:	process group id	   */
	extern int optind;
	extern int num();		/* function to determine whether numeric    */
	int pgerrflg = 0;		/* err flg: non-numeric arg w/p & g options */
	void getdev();

#if pdp11
	char *usage="ps [ -edalf ] [ -c corefile ] [ -s swapdev ] [ -n namelist ] [ -t tlist ]";
#else
#if u3b2 || u3b5
	char *usage="ps [ -edalf ] [ -n namelist ] [ -t tlist ]";
#else
	char *usage="ps [ -edalf ] [ -c corefile ] [ -n namelist ] [ -t tlist ]";
#endif /*u3b2||u3b5*/
#endif /*pdp11*/

	char *usage2="	[ -p plist ] [ -u ulist ] [ -g glist ]";
	char *malloc();
	unsigned size;

	sysname = "/unix";

#if pdp11
	getstr = "lfeadn:s:c:t:p:g:u:";				/* note s & c options */
	coref = "/dev/mem";
	memf = coref;
#endif
#if vax || u3b
	getstr = "lfeadn:c:t:p:g:u:";				/* no   s option */
	coref = "/dev/kmem";
	memf = "/dev/mem";
#endif
#if u3b2 || u3b5
	getstr = "lfeadn:t:p:g:u:";				/* no s or c options */
	coref = "/dev/kmem";
	memf = "/dev/mem";
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
			sysname = optarg;
			break;

#if !(u3b2 || u3b5)
		case 'c':		/* core file given */
			coref = optarg;	/* c option unavailable on 3b2/5    */
			memf = coref;	/* until virtual to physical memory */
			break;		/* address translation library	    */
					/* routine is available		    */
#endif /*!(u3b2||u3b5)*/

#if pdp11
		case 's':		/* swap device given */
			sflg++;
			if ((swap = open(optarg, 0)) < 0 ) {
				fprintf(stderr, "ps: cannot open %s\n",optarg);
				done(1);
			}
			break;
#endif /*pdp11*/

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
				if (!num(parg)) {
					pgerrflg++;
					fprintf(stderr,
						"ps: %s in an invalid non-numeric argument for -p option\n", parg);
				}
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
				if (!num(parg)) {
					pgerrflg++;
					fprintf(stderr,
						"ps: %s is an invalid non-numeric argument for -g option\n", parg);
				}
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

		default:			/* error on ? and s, if not pdp11 */
			errflg++;
			break;
		}

	if (errflg || (optind < argc) || pgerrflg) {
		fprintf(stderr,"usage: %s\n%s\n",usage,usage2);
		done(1);
	}
	if (tflg)
		*ttyp = 0;
						/* if specifying options not used, */
						/*    current terminal is default  */
	if ( !(aflg || eflg || dflg || uflg || tflg || pflg || gflg )) {
		name = 0;
		for (i = 2; i > -1; i--)
			if (isatty(i)) {
				name = ttyname(i);
				break;
			}
		if (name == 0) {
			fprintf(stderr, "ps: can't find controlling terminal\n");
			done(1);
		}
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

#if vax
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

	l_lseek(memfd, (long)nl[1].n_value, 0);		      /* Find base of swap */
	r_read(memfd, (char *)&swplo, (int)sizeof(swplo));
	l_lseek(memfd, (long)nl[2].n_value, 0);		   /* Find proc table size */
	r_read(memfd, (char *)&v, (int)sizeof(v));

#if vax							/* Find system page tables */
	l_lseek(memfd, (long)nl[3].n_value, 0);
	r_read(memfd, (char *)&sbrpte, (int)sizeof(sbrpte));
	sbrpte &= 0x3fffffff;
#endif /*vax*/

	l_lseek(memfd, (long)nl[0].n_value, 0);		      /* Locate proc table */

#if pdp11						      /* This opens swap   */
	if ( !sflg ) {
		if ((swap = open("/dev/swap", 0)) < 0) {
			fprintf(stderr, "ps: cannot open /dev/swap\n");
			done(1);
		}
	}
#endif /*pdp11*/

	if (fflg && lflg )
		printf(" F S     UID   PID  PPID  C PRI NI     ADDR     SZ    WCHAN    STIME TTY      TIME COMD\n");
	else if (fflg)
		printf("     UID   PID  PPID  C    STIME TTY      TIME COMMAND\n");
	else if (lflg)
		printf(" F S   UID   PID  PPID  C PRI NI     ADDR     SZ    WCHAN TTY      TIME COMD\n");
	else
		printf("   PID TTY      TIME COMMAND\n");
	/* determine which processes to print info about */
	for (i=0; i<v.v_proc; i++) {
		found = 0;
		r_read(memfd, (char *)&mproc, (int)sizeof(mproc));
		if (mproc.p_stat == 0)	
			continue;
		puid = mproc.p_uid;
		ppid = mproc.p_pid;
		ppgrp = mproc.p_pgrp;
		if ((ppid == ppgrp) && (dflg || aflg))	/* omit process group leaders */
			continue;			/* for a and d options	      */
		if (eflg || dflg)
			found++;
		else if (pflg && search(pid, npid, ppid)) /*ppid in p option arg list?*/
			found++;
		else if (uflg && ufind(puid))		  /*puid in u option arg list?*/
			found++;
		else if (gflg && search(grpid, ngrpid, ppgrp)) /* grpid in g option arg list?*/
			found++;
		if ( !found && !tflg && !aflg )
			continue;
#if u3b2
		if (prcom(puid,found, nl[0].n_value + i * sizeof(mproc))) {
#else
		if (prcom(puid,found)) {
#endif
			printf("\n");
			retcode =0;
		}
	}
	done(retcode);	/*NOTREACHED*/
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
	if((devl = (struct devl *)malloc(ndev * sizeof(*devl))) == NULL) {
		fprintf(stderr, "ps: malloc() for device table failed, %s\n",
			sys_errlist[errno]);
		exit(1);
	}
	psread(fd, devl, ndev * sizeof(*devl));

	/* read /etc/passwd data from psfile */
	psread(fd, &nud, sizeof(nud));
	if((ud = (struct udata *)malloc(nud * sizeof(*ud))) == NULL) {
		fprintf(stderr, "ps: not enough memory for udata table\n");
		exit(1);
	}
	psread(fd, ud, nud * sizeof(*ud));

	/* read /unix data from psfile */
	if(!nflg)
		psread(fd, nl, sizeof(nl));

	close(fd);
	return(1);
}

void
getdev()				/* getdev() uses ftw() to pass	  */
{					/* pathnames under /dev to gdev() */
	int gdev();			/* along with a status buffer	  */
	int rcode;

	rcode = ftw ("/dev", gdev, 17);

	switch(rcode) {

	case 0:	 return;		/* successful return, devl populated */

	case 1:  fprintf(stderr, "ps: ftw() encountered problem\n");
		 done(1);

	case -1: fprintf(stderr, "ps: ftw() failed, %s\n", sys_errlist[errno]);
		 done(1);

	default: fprintf(stderr, "ps: ftw() unexpected return, rcode=%d\n", rcode);
		 done(1);
	}
}

int
gdev(objptr, statp, numb)		/* gdev() puts device names and ID into	*/ 
char *objptr;				/* the devl structure for character	*/
struct stat *statp;			/* special files in /dev.  The "/dev/"	*/
int numb;				/* string is stripped from the name and	*/
{					/* if the resulting pathname exceeds	*/
	register int i;			/* DIRSIZ in length then the highest	*/
	int leng, start;		/* level directory names are stripped	*/
	static struct devl ldevl[2];	/* until the pathname is DIRSIZ or less	*/
	static int lndev, consflg;

	switch (numb) {

	case FTW_F:	if ((statp->st_mode&S_IFMT) == S_IFCHR) {
					/* get more and be ready for syscon & systty */
				while ((ndev + lndev) >= maxdev) {
					maxdev += UDQ;
					devl = (struct devl *) ((devl == NULL) ?
						malloc(sizeof(struct devl) * maxdev) :
						realloc(devl, sizeof(struct devl) * maxdev));
					if (devl == NULL) {
						fprintf(stderr,
							"ps: not enough memory for %d devices\n", maxdev);
						exit(1);
					}
				}
						/* save systty & syscon entries if    */
						/* the console entry hasn't been seen */
				if (!consflg &&
				   (strcmp("/dev/systty", objptr) == 0 ||
				    strcmp("/dev/syscon", objptr) == 0   )) {
					strncpy(ldevl[lndev].dname, &objptr[5], DIRSIZ);
					ldevl[lndev].dev = statp->st_rdev;
					lndev++;
					return(0);
				}

				leng = strlen(objptr);
								/* strip off /dev/ */
				if (leng < (DIRSIZ + 4)) 	/* strip off /dev/ */
					strcpy(devl[ndev].dname, &objptr[5]);
				else {
					start = leng - DIRSIZ - 1;
			
					for (i = start; (i < leng) && (objptr[i] != '/'); i++) ;
					if (i == leng )
						strncpy(devl[ndev].dname, &objptr[start], DIRSIZ);
					else
						strncpy(devl[ndev].dname, &objptr[i+1], DIRSIZ);
				}
				devl[ndev].dev = statp->st_rdev;
				ndev++;
						/* put systty & syscon entries	 */
						/* in devl when console is found */
				if (strcmp("/dev/console", objptr) == 0) {
					consflg++;
					for (i=0; i < lndev; i++) {
						strncpy(devl[ndev].dname, ldevl[i].dname, DIRSIZ);
						devl[ndev].dev = ldevl[i].dev;
						ndev++;
					}
					lndev = 0;
				}
			}
			return(0);

	case FTW_D:
	case FTW_DNR:
	case FTW_NS:	return(0);

	default:	fprintf(stderr, "ps: gdev() error, %d, encountered\n", numb);
			return(1);
	}
			/* FTW_D:   Directory,			  FTW_F:  File	    */
			/* FTW_DNR: Directory-no read permission, FTW_NS: no status */
}

/* Get the passwd file data into the ud structure */
getpass()
{
	struct passwd *pw, *getpwent();

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
	nlist(sysname, nl);
}

wrdata()		/* put data in /etc/ps_data file */
{
	int fd;

	umask(02);
	unlink(psfile);
	if((fd = open(psfile, O_WRONLY | O_CREAT | O_EXCL, 0664)) == -1) {
		fprintf(stderr, "ps: open() for write failed\n");
		fprintf(stderr, "ps: /etc/ps_data, %s\n", sys_errlist[errno]);
		fprintf(stderr, "ps: Please notify your System Administrator\n");
	}
	else {
		/* make owner root, group sys */
		chown(psfile, (int)0, (int)getegid());

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

#if pdp11
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
int	file;
#endif /*pdp11*/


#if u3b2
prcom(puid,found, pp)			/* print info about the process */
int puid,found;
struct proc	*pp;
#else
prcom(puid,found)			/* print info about the process */
int puid,found;
#endif
{
#if u3b5
	pde_t pde;
	sde_t sde;
	int abuf[NBPSCTR*2/sizeof(int)];	/* BSIZE changed to NBPSCTR for FSS */
#endif						/* FSS - file system switch	    */

#if pdp11
	int abuf[BSIZE*2/sizeof(int)];		/* BSIZE unchanged, no FSS on pdp11 */
#endif /*pdp11*/

#if pdp11 || u3b5 
	register int *ip;
	int nbad, badflg;
	int	nbytes;
	int	cnt;
#endif

#if pdp11 || u3b2 || u3b5
	long addr;
#endif

	register char *cp;

#if pdp11 || u3b || u3b2
	register char *cp1;
#endif

	register char *tp;
	char *ctime();
	time_t time();
	time_t *clock, *tloc;
	time_t tim;
	char timbuf[26];
	char *curtim = timbuf;
	char *sttim, *s1;
	long tm;
	int	match, i;
	int	uzero = 0;
	register char **ttyp, *str;

#if pdp11
	long	txtsiz, datsiz, stksiz;
	int	septxt;
	int	lw=(lflg?35:80);
	int	adrv0;
	int	*ip1;
#endif

#if vax
	int	ww;
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
				/* SIDL: intermediate state in process creation  */
				/* if SIDL then user block address may be bad or */
				/*	contents of user block may be residue	 */
	if (mproc.p_stat==SIDL) {
		return(1);
		}

	/* If paging system,    process is in memory, read in user block	 */
	/* If non-paged system, determine if process is in memory or swap space, */
	/*						then read in user block  */

	/* Pages in the user page table are contiguous on 3b5 but not 3b2/3b20.  */
	/* Pages in the segment page table are not contiguous for each segment   */
	/*								on 3b5   */
#if u3b5
	addr = (mproc.p_addr << PNUMSHFT);
	l_lseek(swmem, addr, 0);
	if((nbytes = read(swmem, (char *)&u, sizeof (u))) != sizeof (u)){
		if ( strcmp(coref,memf) != 0 )
			printf("ps can not read %s\n", memf);
		return(0);
		}
#endif /*u3b5*/
						/* uptbl, user page table	  */
						/* NBPP, number of bytes per page */
						/*       see sys/page.h           */
#if u3b 
	cp1 = (char *)&u + sizeof(u);
	for(i = 0, cp = (char *) &u; cp < cp1; i++, cp += NBPP) {
		l_lseek(swmem, mproc.p_uptbl[i]&PG_ADDR, 0);
		if (read(swmem, cp, cp1-cp > NBPP ? NBPP : cp1-cp) < 0)
			fprintf(stderr, "ps: cannot read %d\n", swmem);
		}
#endif /*u3b*/

#if u3b2
	cp1 = (char *)&u + sizeof(u);
	i = ((char *)ubptbl(pp) - (char *)pp -
	    ((char *)mproc.p_ubptbl - (char *)&mproc))>>2;
	for(cp = (char *) &u; cp < cp1; i++, cp += NBPP) {
		l_lseek(swmem,
			(mproc.p_ubptbl[i].pgm.pg_pfn << 11) - MAINSTORE, 0);
		if (read(swmem, cp, cp1-cp > NBPP ? NBPP : cp1-cp) < 0)
			fprintf(stderr, "ps: cannot read %d\n", swmem);
		}
#endif /*u3b2*/

#if vax
	ww = (int)(mproc.p_spt + (mproc.p_nspt-1)*128);
	ww = sbrpte + ((ww&0x3fffffff)>>9)*4;
	l_lseek(swmem,(long)ww&0x3fffffff,0);
	r_read(swmem,&spte, 4);
	l_lseek(swmem,(spte&0x1fffff)<<9,0);
	r_read(swmem, upte, (int)sizeof upte);
	for(c=0; c<USIZE; c++) {
		l_lseek(swmem, (upte[128-USIZE+c]&0x1fffff)<<9, 0);
		if (read(swmem,(char *)(((int *)&u)+128*c),512) != 512)	/* get u page */
			return(0);
		if((int)sizeof(struct user) - (c + 1) * 512 <= 0)
			break;
	}
#endif /*vax*/

#if pdp11
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
#endif /*pdp11*/

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
		printf("%2x %c", mproc.p_flag&0377, states[mproc.p_stat]);    /* F S */
	if (fflg) {
		i = getunam(puid);
		if (i >= 0)
			printf("%8.8s", ud[i].name);
		else
			printf("%8.8u", puid);
	}
	else if (lflg)
		printf("%6u", puid);
	printf("%6u",mproc.p_pid);					/* PID */
	if (lflg || fflg)
		printf("%6u%3d", mproc.p_ppid, mproc.p_cpu&0377);	/* PPID  CPU */
	if (lflg) {
		printf("%4d%3d",mproc.p_pri, mproc.p_nice);		/* PRI  NICE */

							/* S_STBL, see sys/sbr.h,    */
							/* word addr of segment table*/

						/* slpsemas undefined should never   */
						/* happen, running ps on machine     */
						/* other than the one compiled for   */
#if u3b
		printf("%9x%7d",(mproc.p_sbr & S_STBL)<<2, mproc.p_size); /*ADDR SZ */
		if (mproc.p_wchan){
			if(nl[4].n_value <= 0){
				printf("slpsemas undefined.\n");
			} else if(mproc.p_wchan >= (char *)nl[4].n_value  &&
				  mproc.p_wchan < (char *)(nl[4].n_value) +
						  NUMSLPS * sizeof(sema_t)) /* for AP */
				printf("%9x",mproc.p_w2chan);
			else
			printf("%9x",mproc.p_wchan);			/* WCHAN */
		} else {
			printf("         ");
		}
#endif /*u3b*/

#if u3b2
		printf("%9x%7d", ubptbl(pp), mproc.p_size);		/* ADDR SZ */
		if (mproc.p_wchan)
			printf("%9x",mproc.p_wchan);			/* WCHAN */
		else 
			printf("         ");
#endif /*u3b2*/

#if vax || u3b5
		printf("%9x%7d", mproc.p_addr, mproc.p_size);		/* ADDR SZ */
		if (mproc.p_wchan)
			printf("%9x",mproc.p_wchan);			/* WCHAN */
		else 
			printf("         ");
#endif /*vax,u3b5*/

#if pdp11
		printf("%9x%7d", mproc.p_addr, (mproc.p_size+7)>>3);	/* ADDR SZ */
		if (mproc.p_wchan)
			printf("%9x", mproc.p_wchan);			/* WCHAN */
		else
			printf("         ");
#endif /*pdp11*/

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
	printf(" %-7.14s", tp);						/* TTY */
	tm = (u.u_utime + u.u_stime + HZ/2)/HZ;				/* TIME */
	printf(" %2ld:%.2ld", tm/60, tm%60);

	/* if fflg not set or system process, print command from u_block */
				/* SSYS, system process, today 0 or 2    */
	if (!fflg || mproc.p_flag & SSYS) {				/* CMD */
		if (u.u_comm[0])
			printf(" %.8s", u.u_comm);
		else
			printf(" swapper");
		return(1);
	}

	/* set up address maps for user pcs */
#if pdp11
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
#endif /*pdp11*/
					/* PSARGSZ, length of cmd arg string	  */
					/* today 40, see sys/user.h		  */
					/* tommorrow 80?, hope so for consistency */
#if vax || u3b || u3b2
	for (cp=u.u_psargs; cp<&u.u_psargs[PSARGSZ]; cp++) {
		if (*cp == 0) break;
		if (*cp < ' ' || *cp > '~') {
			printf(" [ %.8s ]", u.u_comm);
			return(1);
		}
	}
	if (lflg)
		printf(" %.35s", u.u_psargs);
	else
		printf(" %.*s", PSARGSZ, u.u_psargs);
#endif /*u3b,vax,u3b2*/

					/* no u_psargs on 3b5, look in user stack */

					/* Active Process: (SLOAD) Process has a  */
					/*      page present in memory.  Get      */
					/*      command line arguments from stack.*/

					/* Inactive Process: (!SLOAD) Process has */
					/*      no page present in memory.  Print */
					/*      command name from u.u_comm in	  */
					/*      user block.			  */

					/* SEXEC, execing process:  can't get args*/
#if u3b5
	if( mproc.p_flag & SEXEC ){
		printf(" [ %.8s ]", u.u_comm);
		return(1);
		}
	addr = u.u_sectaddr[2] + 8;
	for( i = 0; i < 2; i++ ){
		l_lseek(swmem, addr, 0);
		if((nbytes = read(swmem, (char *)&sde, sizeof (sde)))
		    != sizeof (sde)){
			printf("Can not access SDE table\n");
			return(0);
			}
		addr = (long) sde.wd2.address & 0xfffffff8;
		}

	l_lseek(swmem, addr, 0);
	if((nbytes = read(swmem, (char *)&pde, sizeof (pde)))
	    != sizeof (pde)){
		printf("Can not access PDE table\n");
		return(0);
		}
	if( mproc.p_flag & SLOAD && pde.pgm.pg_pres == 1){	/* Active Process   */
		addr = pde.pgm.pg_pfn << PNUMSHFT;
		}
	else{
		printf(" [ %.8s ] ", u.u_comm);			/* Inactive Process */
		return(1);
		}

	l_lseek(swmem, addr, 0);
	if ( read( swmem, (char*)abuf, sizeof(abuf) ) != sizeof(abuf) ) {
		printf(" [ %.8s ] ", u.u_comm);
		return( 1 );
		}
	badflg = 0;
	for ( ip=abuf; ip<&abuf[NBPSCTR*2/sizeof(int)]; ++ip)
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
			for ( cp=(char*)abuf; cp<(char*)&abuf[NBPSCTR*2/sizeof(int)]; ++cp)
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
#endif /*u3b5*/

	return(1);
}


/* file handling and access routines */

#if pdp11
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
#endif /*pdp11*/

done(exitno)
{
	exit(exitno);
}

/* search returns 1 if arg is found in array arr */
/* which has length num, and returns 0 if not found */

search(arr, number, arg)
int arr[];
int number;
int arg;
{
	int i;
	for (i = 0; i < number; i++)
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
		fprintf(stderr, "ps: l_lseek() error on lseek, %s\n", sys_errlist[errno]);
		done(1);
	}
}

/* read with error checking */
r_read (fd, buf, nbytes)
int	fd, nbytes;
char	*buf;
{
	int rbytes;
	rbytes = read(fd, buf, (unsigned)nbytes);
	if (rbytes != nbytes) {
		fprintf(stderr, "ps: r_read() error on read, rbytes=%d, nbytes=%d\n",
			rbytes, nbytes);
		done(1);
	}
}

/* special read unlinks psfile on read error */
psread(fd, bp, bs)
int fd;
char *bp;
unsigned bs;
{
	int rbs;

	rbs = read(fd, bp, bs);
	if(rbs != bs) {
		fprintf(stderr, "ps: psread() error on read, rbs=%d, bs=%d\n", rbs, bs);
		unlink(psfile);
	}
}

/* special write unlinks psfile on read error */
pswrite(fd, bp, bs)
int fd;
char *bp;
unsigned bs;
{
	int wbs;

	wbs = write(fd, bp, bs);
	if(wbs != bs) {
		fprintf(stderr, "ps: pswrite() error on write, wbs=%d, bs=%d\n",
			wbs, bs);
		unlink(psfile);
	}
}

/* print starting time of process unless process started more */
/* than 24 hours ago in which case date is printed   */
/* sttim is start time and it is compared to curtim (current time ) */

prtim(curtim, sttim)
char *curtim, *sttim;
{
	char *ptr1, *ptr2;
	char dayst[3], daycur[3];
	if ( strncmp(curtim, sttim, 11) == 0) {
		ptr1 = sttim + 11;
		ptr2 = ptr1 + 8;
	}
	else {
		ptr1 = sttim + 4;
		ptr2 = ptr1 + 7;
		/* if time is < 24 hours different, then print time */
		if (strncmp(curtim+4, sttim+4, 3) == 0) {
			strncpy(dayst,sttim+8, 2);
			strcat(dayst,"");
			strncpy(daycur,curtim+8,2);
			strcat(daycur,"");
			if ((atoi(dayst) +1 == atoi(daycur)) &&
				 (strncmp(curtim+11,sttim+11,8)<=0)) {
				ptr1 = sttim + 11;
				ptr2 = ptr1 + 8;
			}
		}
	}
	*ptr2 = '\0';
	printf("%9.9s",ptr1);
}

#if u3b || u3b2 || u3b5
#define xp_flag p_flag
#define xp_stat	p_stat
#define xp_pid	p_pid
#define xp_ppid	p_ppid
#define xp_cpu	p_cpu
#define xp_pri	p_pri
#define xp_nice	p_nice
#define xp_utime p_utime
#define xp_stime p_stime
#endif /*u3b,u3b2,u3b5*/

przom(puid)
/* print zombie process - zproc overlays mproc */
int puid;
{
	int i;
	long tm;

	if (lflg)
		printf("%2x %c", zproc.xp_flag&0377, states[zproc.xp_stat]);  /* F S */
	if (fflg) {
		i = getunam(puid);
		if (i >= 0)
			printf("%8.8s", ud[i].name);
		else
			printf("%8.8u", puid);
	}
	else if (lflg)
		printf("%6u", puid);
	printf("%6u",zproc.xp_pid);					/* PID */
	if (lflg || fflg)
		printf("%6u%3d", zproc.xp_ppid, zproc.xp_cpu&0377);	/* PPID CPU */
	if (lflg)
		printf("%4d%3d",zproc.xp_pri, zproc.xp_nice);		/* PRI NICE */
        if (fflg)
                printf("         ");					/* STIME */
        if (lflg)
                printf("                         ");			/* ADDR SZ WCHAN */
	tm = (zproc.xp_utime + zproc.xp_stime + HZ/2)/HZ;
	printf("         %2ld:%.2ld", tm/60, tm%60);			/* TTY TIME */
	printf(" <defunct>");
	return;
}
