/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)wall:wall.c	1.13"

#define	FAILURE	(-1)
#define	TRUE	1
#define	FALSE	0

#include <signal.h>

char	mesg[3000];

#include <grp.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <utmp.h>
#include <sys/utsname.h>
#include <dirent.h>
#include <pwd.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <errno.h>

extern	unsigned alarm();
extern	unsigned short getuid();
int	entries;
extern	int errno;
extern	char *sys_errlist[];
extern	void exit();
extern	void endgrent();
char	*infile;
int	group;
struct	group *pgrp;
extern	struct group *getgrnam();
char	*grpname;
char	line[MAXNAMLEN+1] = "???";
extern	void perror();
extern	char *strcpy();
extern	char *strncpy();
char	*systm;
char	*timstr;
long	tloc;
extern	time_t time();
extern	char *ttyname();
unsigned int usize;
struct	utmp *utmp;
struct	utsname utsn;
char	who[9]	= "???";

#define equal(a,b)		(!strcmp( (a), (b) ))

main(argc, argv)
int	argc;
char	*argv[];
{
	int	i=0, fd;
	register	struct utmp *p;
	FILE	*f;
	struct	stat statbuf;
	struct	dirent *direntry, *readdir();
	DIR *devp = NULL, *opendir();
	void closdir();
	register	char *ptr;
	struct	passwd *pwd;
	extern	struct passwd *getpwuid();
	char	*malloc();

	if(uname(&utsn) == -1) {
		fprintf(stderr, "wall: uname() failed, %s\n", sys_errlist[errno]);
		exit(2);
	}
	systm = utsn.nodename;

	if(stat(UTMP_FILE, &statbuf) < 0) {
		fprintf(stderr, "stat failed on %s\n", UTMP_FILE);
		exit(1);
	}
	/*
		get usize (an unsigned int) for malloc call
 		and check that there is no truncation (for those 16 bit CPUs)
 	*/
	usize = statbuf.st_size;
	if(usize != statbuf.st_size) {
		fprintf(stderr, "'%s' too big.\n", UTMP_FILE);
		exit(1);
	}
	entries = usize / sizeof(struct utmp);
	if((utmp=(struct utmp *)malloc(usize)) == NULL) {
		fprintf(stderr, "cannot allocate memory for '%s'.\n", UTMP_FILE);
		exit(1);
	}

	if((fd=open(UTMP_FILE, O_RDONLY)) < 0) {
		fprintf(stderr,"cannot open '%s'\n", UTMP_FILE);
		exit(1);
	}
	if(read(fd, (char *) utmp, usize) != usize) {
		fprintf(stderr, "cannot read '%s'\n", UTMP_FILE);
		exit(1);
	}
	close(fd);
	readargs(argc, argv);

	/*
		Get the name of the terminal wall is running from.
	*/

	if (isatty(fileno(stderr)) && fstat(fileno(stderr),&statbuf) !=
		FAILURE && ((devp = opendir("/dev")) != NULL)) {


			/*
				Read in directory entries for /dev and look for
				ones with an inode number equal to the inode 
				number of our standard error output.
			*/
			while ((direntry = readdir(devp)) != NULL) {
				if (direntry->d_ino != statbuf.st_ino) 
					direntry->d_ino = 0;
				else { 
					strcpy(line, direntry->d_name);
					break;
				}
			}

			/*
				If we've reached the end of the dev directory 
				and can't find another name for this terminal, 
				finally give up
			*/

	} 
	if (who[0] == '?') {
		if (pwd = getpwuid((int) getuid()))
			strncpy(&who[0],pwd->pw_name,sizeof(who));
	}
	if (devp != NULL) closedir(devp);

	f = stdin;
	if(infile) {
		f = fopen(infile, "r");
		if(f == NULL) {
			fprintf(stderr,"%s??\n", infile);
			exit(1);
		}
	}
	for(ptr= &mesg[0]; fgets(ptr,&mesg[sizeof(mesg)]-ptr, f) != NULL
		; ptr += strlen(ptr));
	fclose(f);
	time(&tloc);
	timstr = ctime(&tloc);
	for(i=0;i<entries;i++) {
		if((p=(&utmp[i]))->ut_type != USER_PROCESS) continue;
		sendmes(p);
	}
	alarm(60);
	do {
		i = wait((int *)0);
	} while(i != -1 || errno != ECHILD);
	exit(0); /* NOTREACHED */
}

sendmes(p)
struct utmp *p;
{
	register i;
	register char *s;
	static char device[] = "/dev/123456789012";
	FILE *f;

	if(group)
		if(!chkgrp(p->ut_user))
			return;
	while((i=fork()) == -1) {
		alarm(60);
		wait((int *)0);
		alarm(0);
	}

	if(i)
		return;

	signal(SIGHUP, SIG_IGN);
	alarm(60);
	s = &device[0];
	sprintf(s,"/dev/%s",&p->ut_line[0]);
#ifdef DEBUG
	f = fopen("wall.debug", "a");
#else
	f = fopen(s, "w");
#endif
	if(f == NULL) {
		fprintf(stderr,"Cannot send to %.-8s on %s\n", &p->ut_user[0],s);
		perror("open");
		exit(1);
	}
	fprintf(f, "\07\07\07Broadcast Message from %s (%s) on %s %19.19s",
		 who, line, systm, timstr);
	if(group)
		fprintf(f, " to group %s", grpname);
	fprintf(f, "...\n");
#ifdef DEBUG
	fprintf(f,"DEBUG: To %.8s on %s\n", p->ut_user, s);
#endif
	fprintf(f, "%s\n", mesg);
	fclose(f);
	exit(0);
}

readargs(ac, av)
int ac;
char **av;
{
	register int i;

	for(i = 1; i < ac; i++) {
		if(equal(av[i], "-g")) {
			if(group) {
				fprintf(stderr, "Only one group allowed\n");
				exit(1);
			}
			i++;
			if((pgrp=getgrnam(grpname= av[i])) == NULL) {
				fprintf(stderr, "Unknown group %s\n", grpname);
				exit(1);
			}
			endgrent();
			group++;
		}
		else
			infile = av[i];
	}
}
#define BLANK		' '
chkgrp(name)
register char *name;
{
	register int i;
	register char *p;

	for(i = 0; pgrp->gr_mem[i] && pgrp->gr_mem[i][0]; i++) {
		for(p=name; *p && *p != BLANK; p++);
		*p = 0;
		if(equal(name, pgrp->gr_mem[i]))
			return(1);
	}

	return(0);
}
