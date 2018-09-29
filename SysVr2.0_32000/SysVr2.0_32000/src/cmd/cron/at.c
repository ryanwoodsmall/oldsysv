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
/*	@(#)at.c	1.7	*/
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <fcntl.h>
#include <pwd.h>
#include <stdio.h>
#include <ctype.h>
#include <time.h>
#include <signal.h>
#include <errno.h>
#include "cron.h"

#define TMPFILE		"_at"	/* prefix for temporary files	*/
#define ATMODE		06444	/* Mode for creating files in ATDIR.
Setuid bit on so that if an owner of a file gives that file 
away to someone else, the setuid bit will no longer be set.  
If this happens, atrun will not execute the file	*/
#define ROOT		0	/* user-id of super-user */
#define BUFSIZE		512	/* for copying files */
#define LINESIZE	130	/* for listing jobs */
#define	MAXTRYS		100	/* max trys to create at job file */

#define BADDATE		"bad date specification"
#define BADFIRST	"bad first argument"
#define BADHOURS	"hours field is too large"
#define BADMD		"bad month and day specification"
#define BADMINUTES	"minutes field is too large"
#define BADSHELL	"because your login shell isn't /bin/sh, you can't use at"
#define WARNSHELL	"warning: commands will be executed using /bin/sh\n"
#define CANTCD		"can't change directory to the at directory"
#define CANTCHOWN	"can't change the owner of your job to you"
#define CANTCREATE	"can't create a job for you"
#define INVALIDUSER	"you are not a valid user (no entry in /etc/passwd)"
#define	NONUMBER	"proper syntax is:\n\tat -ln\nwhere n is a number"
#define NOREADDIR	"can't read the at directory"
#define NOTALLOWED	"you are not authorized to use at.  Sorry."
#define NOTHING		"nothing specified"
#define PAST		"it's past that time"

char identif[]="at: version 1.5, date 7/2/82";

/*
	this data is used for parsing time
*/
#define	dysize(A)	(((A)%4) ? 365 : 366)
int	gmtflag = 0;
int	dflag = 0;
extern	time_t	timezone;
extern	char	*argp;
char	login[UNAMESIZE];
char	argpbuf[80];
char	pname[80];
char	pname1[80];
char	argbuf[80];
time_t	when, now, gtime();
struct	tm	*tp, at, rt, *localtime();
int	mday[12] =
{
	31,38,31,
	30,31,30,
	31,31,30,
	31,30,31,
};
int	mtab[12] =
{
	0,   31,  59,
	90,  120, 151,
	181, 212, 243,
	273, 304, 334,
};
int     dmsize[12] = {
	31,28,31,30,31,30,31,31,30,31,30,31};

/* end of time parser */

short	jobtype = ATEVENT;		/* set to 1 if batch job */
char	*tfname;
extern	char *xmalloc();
extern int per_errno;
time_t num();

main(argc,argv)
char **argv;
{
	FILE *fopen();
	struct passwd *pw,*getpwuid();
	struct direct dentry;
	struct stat buf;
	int user,i,j,len,dir,fd,catch();
	char *ptr,*job,line[LINESIZE],pid[6];
	char *pp;
	char *cmdname;
	char *asctime(),*fgets(),*strcat(),*strcpy(),*strrchr();
	char *mkjobname(),*getuser();
	time_t t,time();
	int  st = 1;

	pp = getuser((user=getuid()));
	if(pp == NULL) {
		if(per_errno == 2)
			atabort(BADSHELL);
		else
			atabort(INVALIDUSER);
	}
	strcpy(login,pp);
	if (!allowed(login,ATALLOW,ATDENY)) atabort(NOTALLOWED);

	if (strcmp(argv[1],"-r")==0) {
		/* remove jobs that are specified */
		if (chdir(ATDIR)==-1) atabort(CANTCD);
		for (i=2; i<argc; i++)
			if (stat(argv[i],&buf))
				fprintf(stderr,"at: %s does not exist\n",argv[i]);
			else if ((user!=buf.st_uid) && (user!=ROOT))
				fprintf(stderr,"at: you don't own %s\n",argv[i]);
			else {
				sendmsg(DELETE,argv[i]);
				unlink(argv[i]);
			}
		exit(0); 
	}

	if (strncmp(argv[1],"-l",2)==0) {
		/* list jobs for user */
		if (chdir(ATDIR)==-1) atabort(CANTCD);
		if (argc==2) {
			/* list all jobs for a user */
			if ((dir=open(ATDIR,O_RDONLY))==-1) atabort(NOREADDIR);
			lseek(dir,(long)2*sizeof(dentry),0);
			for (;;) {
				if (read(dir,&dentry,sizeof(dentry))!=sizeof(dentry))
					break;	/* end of directory */
				if (dentry.d_ino==0) continue;
				if (stat(dentry.d_name,&buf)) {
					unlink(dentry.d_name);
					continue; 
				}
				if ((user!=ROOT) && (buf.st_uid!=user))
					continue;
				ptr = dentry.d_name;
				if (((t=num(&ptr))==0) || (*ptr!='.'))
					continue;
				if ((user==ROOT) && ((pw=getpwuid(buf.st_uid))!=NULL))
					printf("user = %s\t%s\t%s",pw->pw_name,dentry.d_name,asctime(localtime(&t)));
				else	printf("%s\t%s",dentry.d_name,asctime(localtime(&t)));
			}
			close(dir);
		}
		else	/* list particular jobs for user */
			for (i=2; i<argc; i++) {
				ptr = argv[i];
				if (((t=num(&ptr))==0) || (*ptr!='.'))
					fprintf(stderr,"at: invalid job name %s\n",argv[i]);
				else if (stat(argv[i],&buf))
					fprintf(stderr,"at: %s does not exist\n",argv[i]);
				else if ((user!=buf.st_uid) && (user!=ROOT))
					fprintf(stderr,"at: you don't own %s\n",argv[i]);
				else 
					printf("%s\t%s",argv[i],asctime(localtime(&t)));
			}
		exit(0);
	}

	if(strncmp(argv[1], "-q",2) == 0) {
		if(argv[1][2] == NULL)
			atabort("no queue specified");
		jobtype = argv[1][2] - 'a';
		st = 2;
	}

	/* figure out what time to run the job */

	if(argc == 1 && jobtype != BATCHEVENT)
		atabort(NOTHING);
	time(&now);
	if(jobtype != BATCHEVENT) {	/* at job */
		argp = argpbuf;
		i = st;
		while(i < argc) {
			strcat(argp,argv[i]);
			strcat(argp, " ");
			i++;
		}
		tp = localtime(&now);
		mday[1] = 28 + leap(tp->tm_year);
		yyparse();
		atime(&at, &rt);
		when = gtime(&at);
		if(!gmtflag) {
			when += timezone;
			if(localtime(&when)->tm_isdst)
				when -= 60 * 60;
		}
	} else		/* batch job */
		when = now;
	if(when < now)	/* time has already past */
		atabort("too late");

	sprintf(pid,"%-5d",getpid());
	tfname=xmalloc(strlen(ATDIR)+strlen(TMPFILE)+7);
	strcat(strcat(strcat(strcpy(tfname,ATDIR),"/"),TMPFILE),pid);
	/* catch SIGINT, HUP, and QUIT signals */
	if (signal(SIGINT, catch) == SIG_IGN) signal(SIGINT,SIG_IGN);
	if (signal(SIGHUP, catch) == SIG_IGN) signal(SIGHUP,SIG_IGN);
	if (signal(SIGQUIT,catch) == SIG_IGN) signal(SIGQUIT,SIG_IGN);
	if (signal(SIGTERM,catch) == SIG_IGN) signal(SIGTERM,SIG_IGN);
	if((fd = open(tfname,O_CREAT|O_EXCL|O_WRONLY,ATMODE)) < 0)
		atabort(CANTCREATE);
	if (chown(tfname,user,getgid())==-1) {
		unlink(tfname);
		atabort(CANTCHOWN);
	}
	close(1);
	dup(fd);
	close(fd);
	sprintf(pname,"%s",PROTO);
	sprintf(pname1,"%s.%c",PROTO,'a'+jobtype);
	copy();
	if (link(tfname,job=mkjobname(when))==-1) {
		unlink(tfname);
		atabort(CANTCREATE);
	}
	unlink(tfname);
	sendmsg(ADD,strrchr(job,'/')+1);
	if(per_errno == 2)
		fprintf(stderr,WARNSHELL);
	fprintf(stderr,"job %s at %.24s\n",strrchr(job,'/')+1,ctime(&when));
	if (when-t-MINUTE < HOUR) fprintf(stderr,
	    "at: this job may not be executed at the proper time.\n");
}


/***************/
find(elem,table,tabsize)
/***************/
char *elem,**table;
int tabsize;
{
	int i;

	for (i=0; i<strlen(elem); i++)
		elem[i] = tolower(elem[i]);
	for (i=0; i<tabsize; i++)
		if (strcmp(elem,table[i])==0) return(i);
		else if (strncmp(elem,table[i],3)==0) return(i);
	return(-1);
}


/****************/
char *mkjobname(t)
/****************/
time_t t;
{
	int i;
	char *name;
	struct  stat buf;
	name=xmalloc(200);
	for (i=0;i < MAXTRYS;i++) {
		sprintf(name,"%s/%ld.%c",ATDIR,t,'a'+jobtype);
		if (stat(name,&buf)) 
			return(name);
		t += 1;
	}
	atabort("queue full");
}


/****************/
catch()
/****************/
{
	unlink(tfname);
	exit(1);
}


/****************/
atabort(msg)
/****************/
char *msg;
{
	fprintf(stderr,"at: %s\n",msg);
	exit(1);
}

yywrap()
{
	return 1;
}

yyerror()
{
	atabort(BADDATE);
}

/*
 * add time structures logically
 */
atime(a, b)
register
struct tm *a, *b;
{
	if ((a->tm_sec += b->tm_sec) >= 60) {
		b->tm_min += a->tm_sec / 60;
		a->tm_sec %= 60;
	}
	if ((a->tm_min += b->tm_min) >= 60) {
		b->tm_hour += a->tm_min / 60;
		a->tm_min %= 60;
	}
	if ((a->tm_hour += b->tm_hour) >= 24) {
		b->tm_mday += a->tm_hour / 24;
		a->tm_hour %= 24;
	}
	a->tm_year += b->tm_year;
	if ((a->tm_mon += b->tm_mon) >= 12) {
		a->tm_year += a->tm_mon / 12;
		a->tm_mon %= 12;
	}
	a->tm_mday += b->tm_mday;
	while (a->tm_mday > mday[a->tm_mon]) {
		a->tm_mday -= mday[a->tm_mon++];
		if (a->tm_mon > 11) {
			a->tm_mon = 0;
			mday[1] = 28 + leap(++a->tm_year);
		}
	}
}

leap(year)
{
	return year % 4 == 0;
}

/*
 * return time from time structure
 */
time_t
gtime(tp)
register
struct	tm *tp;
{
	register i;
	long	tv;
	extern int dmsize[];

	tv = 0;
	for (i = 1970; i < tp->tm_year+1900; i++)
		tv += dysize(i);
	if (dysize(tp->tm_year) == 366 && tp->tm_mon >= 2)
		++tv;
	for (i = 0; i < tp->tm_mon; ++i)
		tv += dmsize[i];
	tv += tp->tm_mday - 1;
	tv = 24 * tv + tp->tm_hour;
	tv = 60 * tv + tp->tm_min;
	tv = 60 * tv + tp->tm_sec;
	return tv;
}

/*
 * make job file from proto + stdin
 */
copy()
{
	register c;
	register FILE *pfp;
	register FILE *xfp;
	char	dirbuf[60];
	register char **ep;
	char *strchr();
	long ulimit();
	int umask();
	unsigned um;
	char *val;
	extern char **environ;

	printf(": %s job\n",jobtype ? "batch" : "at");
	for (ep=environ; *ep; ep++) {
		if ( strchr(*ep,'\'')!=NULL )
			continue;
		if ((val=strchr(*ep,'='))==NULL)
			continue;
		*val++ = '\0';
		printf("export %s; %s='%s'\n",*ep,*ep,val);
		*--val = '=';
	}
	if((pfp = fopen(pname1,"r")) == NULL && (pfp=fopen(pname,"r"))==NULL)
		atabort("no prototype");
	um = umask(0);
	while ((c = getc(pfp)) != EOF) {
		if (c != '$')
			putchar(c);
		else switch (c = getc(pfp)) {
		case EOF:
			goto out;
		case 'd':
			dirbuf[0] = NULL;
			if((xfp=popen("/bin/pwd","r")) != NULL) {
				fscanf(xfp,"%s",dirbuf);
				pclose(xfp);
			}
			printf("%s", dirbuf);
			break;
		case 'l':
			printf("%ld",ulimit(1,-1L));
			break;
		case 'm':
			printf("%o", um);
			break;
		case '<':
			while ((c = getchar()) != EOF)
				putchar(c);
			break;
		case 't':
			printf(":%lu", when);
			break;
		default:
			putchar(c);
		}
	}
out:
	fclose(pfp);
}

/****************/
sendmsg(action,fname)
/****************/
char action;
char *fname;
{

	static	int	msgfd = -2;
	struct	message	*pmsg;

	pmsg = &msgbuf;
	if(msgfd == -2)
		if((msgfd = open(FIFO,O_WRONLY|O_NDELAY)) < 0) {
			if(errno == ENXIO || errno == ENOENT)
				fprintf(stderr,"cron may not be running - call your system administrator\n");
			else
				fprintf(stderr,"at: error in message queue open\n");
			return;
		}
	pmsg->etype = AT;
	pmsg->action = action;
	strncpy(pmsg->fname,fname,FLEN);
	strncpy(pmsg->logname,login,LLEN);
	if(write(msgfd,pmsg,sizeof(struct message)) != sizeof(struct message))
		fprintf(stderr,"at: error in message send\n");
}
