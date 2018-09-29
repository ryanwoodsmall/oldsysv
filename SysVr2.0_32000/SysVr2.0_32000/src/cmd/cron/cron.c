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
/*	@(#)cron.c	1.9	*/
#include <sys/types.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <pwd.h>
#include <stdio.h>
#include <fcntl.h>
#include <time.h>
#include <ctype.h>
#include <signal.h>
#include <errno.h>
#include "cron.h"

#define MAIL		"/bin/mail"	/* mail program to use */
#define CONSOLE		"/dev/console"	/* where to write error messages when cron dies	*/

#define TMPINFILE	"/tmp/crinXXXXXX"  /* file to put stdin in for cmd  */
#define	TMPDIR		"/tmp"
#define	PFX		"crout"
#define TMPOUTFILE	"/tmp/croutXXXXXX" /* file to place stdout, stderr */

#define INMODE		00400		/* mode for stdin file	*/
#define OUTMODE		00600		/* mode for stdout file */
#define ISUID		06000		/* mode for verifing at jobs */

#define INFINITY	2147483647L	/* upper bound on time	*/
#define CUSHION		120L
#define	MAXRUN		25		/* max total jobs allowed in system */
#define ZOMB		100		/* proc slot used for mailing output */

#define	JOBF		'j'
#define	NICEF		'n'
#define	USERF		'u'
#define WAITF		'w'

#define BCHAR		'>'
#define	ECHAR		'<'

#define	DEFAULT		0
#define	LOAD		1

#define BADCD		"can't change directory to the crontab directory."
#define NOREADDIR	"can't read the crontab directory."

#define BADJOBOPEN	"unable to read your at job."
#define BADSHELL	"because your login shell isn't /bin/sh, you can't use cron."
#define BADSTAT		"can't access your crontab file.  Resubmit it."
#define CANTCDHOME	"can't change directory to your home directory.\nYour commands will not be executed."
#define CANTEXECSH	"unable to exec the shell for one of your commands."
#define EOLN		"unexpected end of line"
#define NOREAD		"can't read your crontab file.  Resubmit it."
#define NOSTDIN		"unable to create a standard input file for one of your crontab commands.\nThat command was not executed."
#define OUTOFBOUND	"number too large or too small for field"
#define STDERRMSG	"\n\n*************************************************\nCron: The previous message is the standard output\n      and standard error of one of your cron commands.\n"
#define STDOUTERR	"one of your commands generated output or errors, but cron was unable to mail you this output.\nRemember to redirect standard output and standard error for each of your commands."
#define UNEXPECT	"unexpected symbol found"

struct event {	
	time_t time;	/* time of the event	*/
	short etype;	/* what type of event; 0=cron, 1=at	*/
	char *cmd;	/* command for cron, job name for at	*/
	struct usr *u;	/* ptr to the owner (usr) of this event	*/
	struct event *link; 	/* ptr to another event for this user */
	union { 
		struct { /* for crontab events */
			char *minute;	/*  (these	*/
			char *hour;	/*   fields	*/
			char *daymon;	/*   are	*/
			char *month;	/*   from	*/
			char *dayweek;	/*   crontab)	*/
			char *input;	/* ptr to stdin	*/
		} ct;
		struct { /* for at events */
			short exists;	/* for revising at events	*/
			int eventid;	/* for el_remove-ing at events	*/
		} at;
	} of; 
};

struct usr {	
	char *name;	/* name of user (e.g. "kew")	*/
	char *home;	/* home directory for user	*/
	int uid;	/* user id	*/
	int gid;	/* group id	*/
#ifdef ATLIMIT
	int aruncnt;	/* counter for running jobs per uid */
#endif
#ifdef CRONLIMIT
	int cruncnt;	/* counter for running cron jobs per uid */
#endif
	int ctid;	/* for el_remove-ing crontab events */
	short ctexists;	/* for revising crontab events	*/
	struct event *ctevents;	/* list of this usr's crontab events */
	struct event *atevents;	/* list of this usr's at events */
	struct usr *nextusr; 
};	/* ptr to next user	*/

struct	queue
{
	int njob;	/* limit */
	int nice;	/* nice for execution */
	int nwait;	/* wait time to next execution attempt */
	int nrun;	/* number running */
}	
	qd = {100, 2, 60},		/* default values for queue defs */
	qt[NQUEUE];

struct	queue	qq;
int	wait_time = 60;

struct	runinfo
{
	int	pid;
	short	que;
	struct  usr *rusr;		/* pointer to usr struct */
	char 	*outfile;	/* file where stdout & stderr are trapped */
}	rt[MAXRUN];

int msgfd;		/* file descriptor for fifo queue */
int ecid=1;		/* for giving event classes distinguishable id names 
			   for el_remove'ing them.  MUST be initialized to 1 */
short jobtype;		/* at or batch job */
int delayed;		/* is job being rescheduled or did it run first time */
int notexpired;		/* time for next job has not come */
int cwd;		/* current working directory */
int running;		/* zero when no jobs are executing */
struct event *next_event;	/* the next event to execute	*/
struct usr *uhead;	/* ptr to the list of users	*/
struct usr *ulast;	/* ptr to last usr table entry */
time_t init_time,num(),time();
char *strcpy(),*strncpy(),*strcat();
extern char *xmalloc();

/* user's default environment for the shell */
char homedir[100]="HOME=";
char logname[50]="LOGNAME=";
char *envinit[]={
	homedir,logname,"PATH=:/bin:/usr/bin:/usr/lbin","SHELL=/bin/sh",0};
extern char **environ;


/*************/
main(argc,argv)
/*************/
char **argv;
{
	time_t t,t_old;
	time_t last_time;
	time_t ne_time;		/* amt of time until next event execution */
	time_t next_time();
	time_t lastmtime = 0L;
	struct usr *u,*u2;
	struct event *e,*e2,*eprev;
	struct stat buf;
	long seconds;
	int rfork;

begin:
	if (rfork=fork()) {
		if (rfork==-1) {
			sleep(30);
			goto begin; 
		}
		exit(0); 
	}

	umask(022);
	setpgrp();	/* detach cron from console */
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);

	initialize();
	quedefs(DEFAULT);	/* load default queue definitions */
	msg("*** cron started ***   pid = %d",getpid());
	timeout();	/* set up alarm clock trap */
	t_old = time((long *) 0);
	last_time = t_old;
	while (TRUE) {			/* MAIN LOOP	*/
		t = time((long *) 0);
		if((t_old > t) || (t-last_time > CUSHION)) {
			/* the time was set backwards or forward */
			el_delete();
			u = uhead;
			while (u!=NULL) {
				rm_ctevents(u);
				e = u->atevents;
				while (e!=NULL) {
					free(e->cmd);
					e2 = e->link;
					free(e);
					e = e2; 
				}
				u2 = u->nextusr;
				u = u2; 
			}
			close(msgfd);
			initialize();
			t = time((long *) 0); 
		}
		t_old = t;
		if (next_event == NULL)
			if (el_empty()) ne_time = INFINITY;
			else {	
				next_event = (struct event *) el_first();
				ne_time = next_event->time - t; 
			}
		else ne_time = next_event->time - t;
#ifdef DEBUG
		if(next_event != NULL)
			printf("next_time=%ld  %s",
				next_event->time,ctime(&next_event->time));
#endif
		seconds = (ne_time < (long) 0) ? (long) 0 : ne_time;
		if(ne_time > (long) 0)
			idle(seconds);
		if(notexpired) {
			notexpired = 0;
			last_time = INFINITY;
			continue;
		}
		if(stat(QUEDEFS,&buf))
			msg("cannot stat QUEDEFS file");
		else
			if(lastmtime != buf.st_mtime) {
				quedefs(LOAD);
				lastmtime = buf.st_mtime;
			}
		last_time = next_event->time;	/* save execution time */
		ex(next_event);
		switch(next_event->etype) {
		/* add cronevent back into the main event list */
		case CRONEVENT:
			if(delayed) {
				delayed = 0;
				break;
			}
			next_event->time = next_time(next_event);
			el_add( next_event,next_event->time,
			    (next_event->u)->ctid ); 
			break;
		/* remove at or batch job from system */
		default:
			eprev=NULL;
			e=(next_event->u)->atevents;
			while (e != NULL)
				if (e == next_event) {
					if (eprev == NULL)
						(e->u)->atevents = e->link;
					else	eprev->link = e->link;
					free(e->cmd);
					free(e);
					break;	
				}
				else {	
					eprev = e;
					e = e->link; 
				}
			break;
		}
		next_event = NULL; 
	}
}



/**********/
initialize()
/**********/
{

	static int flag = 0;

#ifdef DEBUG
	printf("in initialize\n");
#endif
	init_time = time((long *) 0);
	el_init(8,init_time,(long)(60*60*24),10);
	if(access(FIFO,04)==-1 && mknod(FIFO,S_IFIFO|0600,0)!=0)
		crabort("cannot access fifo queue");
	if((msgfd = open(FIFO, O_RDWR)) < 0) {
		perror("! open");
		crabort("cannot create fifo queue");
	}

	/* read directories, create users list,
	   and add events to the main event list	*/
	uhead = NULL;
	read_dirs();
	next_event = NULL;
	if(flag)
		return;
	if(freopen(ACCTFILE,"a",stdout) == NULL)
		fprintf(stderr,"cannot open %s\n",ACCTFILE);
	close(fileno(stderr));
	dup(1);
	/* this must be done to make popen work....i dont know why */
	freopen("/dev/null","r",stdin);
	flag = 1;
}


/******************/
read_dirs()
/******************/
{
	int dir;
	int mod_ctab(), mod_atjob();

#ifdef DEBUG
	printf("Read Directories\n");
#endif
	if (chdir(CRONDIR) == -1) crabort(BADCD);
	cwd = CRON;
	if ((dir=open(".",O_RDONLY))==-1)
		crabort(NOREADDIR);
	dscan(dir,mod_ctab);
	close(dir);
	if(chdir(ATDIR) == -1) {
		msg("cannot chdir to at directory");
		return;
	}
	cwd = AT;
	if ((dir=open(".",O_RDONLY))==-1) {
		msg("cannot read at at directory");
		return; 
	}
	dscan(dir,mod_atjob);
	close(dir);
}

/****************/
dscan(df,fp)
/****************/
int	df;
int	(*fp)();
{

	register	i, dn;
	register	struct	direct	*dp;
	char		name[DIRSIZ+1];
	struct		direct	db[BSIZE/sizeof(struct direct)];
	long		lseek();

	lseek(df, (long) 0, 0);
	while((dn=read(df, (char *)db, sizeof db) / sizeof *db) > 0)
	for(dp = db; dn-- > 0; ++dp) {
		if(dp->d_ino == 0)
			continue;
		for(i=0; i<DIRSIZ; ++i)
			name[i] = dp->d_name[i];
		name[i] = '\0';
		(*fp) (name);
	}
}

/****************/
mod_ctab(name)
/****************/
char	*name;
{

	struct	passwd	*pw,*getpwnam(),*getpwuid();
	struct	stat	buf;
	struct	usr	*u,*find_usr();
	char	namebuf[132];
	char	*pname;

	if((pw=getpwnam(name)) == NULL)
		return;
	if(cwd != CRON) {
		strcat(strcat(strcpy(namebuf,CRONDIR),"/"),name);
		pname = namebuf;
	} else
		pname = name;
	/* a warning message is given by the crontab command so there is
	   no need to give one here......use this code if you only want users
	   with a login shell of /bin/sh to use cron
	if((strcmp(pw->pw_shell,"")!=0) && (strcmp(pw->pw_shell,SHELL)!=0)){
			mail(name,BADSHELL,2);
			unlink(pname);
			return;
	}
	*/
	if(stat(pname,&buf)) {
		mail(name,BADSTAT,2);
		unlink(pname);
		return;
	}
	if((u=find_usr(name)) == NULL) {
#ifdef DEBUG
		printf("new user (%s) with a crontab\n",name);
#endif
		u = (struct usr *) xmalloc(sizeof(struct usr));
		u->name = xmalloc(strlen(name)+1);
		strcpy(u->name,name);
		u->home = xmalloc(strlen(pw->pw_dir)+1);
		strcpy(u->home,pw->pw_dir);
		u->uid = pw->pw_uid;
		u->gid = pw->pw_gid;
		u->ctexists = TRUE;
		u->ctid = ecid++;
		u->ctevents = NULL;
		u->atevents = NULL;
#ifdef ATLIMIT
		u->aruncnt = 0;
#endif
#ifdef CRONLIMIT
		u->cruncnt = 0;
#endif
		u->nextusr = uhead;
		uhead = u;
		readcron(u);
	} else {
		u->uid = pw->pw_uid;
		u->gid = pw->pw_gid;
		if(strcmp(u->home,pw->pw_dir) != 0) {
			free(u->home);
			u->home = xmalloc(strlen(pw->pw_dir)+1);
			strcpy(u->home,pw->pw_dir);
		}
		u->ctexists = TRUE;
		if(u->ctid == 0) {
#ifdef DEBUG
			printf("%s now has a crontab\n",u->name);
#endif
			/* user didnt have a crontab last time */
			u->ctid = ecid++;
			u->ctevents = NULL;
			readcron(u);
			return;
		}
#ifdef DEBUG
		printf("%s has revised his crontab\n",u->name);
#endif
		rm_ctevents(u);
		el_remove(u->ctid,0);
		readcron(u);
	}
}


/****************/
mod_atjob(name)
/****************/
char	*name;
{

	char	*ptr;
	time_t	tim;
	struct	passwd	*pw,*getpwnam(),*getpwuid();
	struct	stat	buf;
	struct	usr	*u,*find_usr();
	struct	event	*e;
	char	namebuf[132];
	char	*pname;

	ptr = name;
	if(((tim=num(&ptr)) == 0) || (*ptr != '.'))
		return;
	ptr++;
	if(!isalpha(*ptr))
		return;
	jobtype = *ptr - 'a';
	if(cwd != AT) {
		strcat(strcat(strcpy(namebuf,ATDIR),"/"),name);
		pname = namebuf;
	} else
		pname = name;
	if(stat(pname,&buf) || jobtype >= NQUEUE-1) {
		unlink(pname);
		return;
	}
	if(!(buf.st_mode & ISUID)) {
		unlink(pname);
		return;
	}
	if((pw=getpwuid(buf.st_uid)) == NULL)
		return;
	/* a warning message is given by the at command so there is no
	   need to give one here......use this code if you only want users
	   with a login shell of /bin/sh to use cron
	if((strcmp(pw->pw_shell,"")!=0) && (strcmp(pw->pw_shell,SHELL)!=0)){
			mail(pw->pw_name,BADSHELL,2);
			unlink(pname);
			return;
	}
	*/
	if((u=find_usr(pw->pw_name)) == NULL) {
#ifdef DEBUG
		printf("new user (%s) with an at job = %s\n",pw->pw_name,name);
#endif
		u = (struct usr *) xmalloc(sizeof(struct usr));
		u->name = xmalloc(strlen(pw->pw_name)+1);
		strcpy(u->name,pw->pw_name);
		u->home = xmalloc(strlen(pw->pw_dir)+1);
		strcpy(u->home,pw->pw_dir);
		u->uid = pw->pw_uid;
		u->gid = pw->pw_gid;
		u->ctexists = FALSE;
		u->ctid = 0;
		u->ctevents = NULL;
		u->atevents = NULL;
#ifdef ATLIMIT
		u->aruncnt = 0;
#endif
#ifdef CRONLIMIT
		u->cruncnt = 0;
#endif
		u->nextusr = uhead;
		uhead = u;
		add_atevent(u,name,tim);
	} else {
		u->uid = pw->pw_uid;
		u->gid = pw->pw_gid;
		if(strcmp(u->home,pw->pw_dir) != 0) {
			free(u->home);
			u->home = xmalloc(strlen(pw->pw_dir)+1);
			strcpy(u->home,pw->pw_dir);
		}
		e = u->atevents;
		while(e != NULL)
			if(strcmp(e->cmd,name) == 0) {
				e->of.at.exists = TRUE;
				break;
			} else
				e = e->link;
		if (e == NULL) {
#ifdef DEBUG
			printf("%s has a new at job = %s\n",u->name,name);
#endif
			add_atevent(u,name,tim);
		}
	}
}



/****************/
add_atevent(u,job,tim)
/****************/
struct usr *u;
char *job;
time_t tim;
{
	struct event *e;

	e=(struct event *) xmalloc(sizeof(struct event));
	e->etype = jobtype;
	e->cmd = xmalloc(strlen(job)+1);
	strcpy(e->cmd,job);
	e->u = u;
#ifdef DEBUG
	printf("add_atevent: user=%s, job=%s, time=%ld\n",
		u->name,e->cmd, e->time);
#endif
	e->link = u->atevents;
	u->atevents = e;
	e->of.at.exists = TRUE;
	e->of.at.eventid = ecid++;
	if(tim < init_time)		/* old job */
		e->time = init_time;
	else
		e->time = tim;
	el_add(e, e->time, e->of.at.eventid); 
}


char line[CTLINESIZE];		/* holds a line from a crontab file	*/
int cursor;			/* cursor for the above line	*/

/******************/
readcron(u)
/******************/
struct usr *u;
{
	/* readcron reads in a crontab file for a user (u).
	   The list of events for user u is built, and 
	   u->events is made to point to this list.
	   Each event is also entered into the main event list. */

	FILE *fopen(),*cf;		/* cf will be a user's crontab file */
	time_t next_time();
	struct event *e;
	int start,i;
	char *next_field();
	char namebuf[132];
	char *pname;

	/* read the crontab file */
	if(cwd != CRON) {
		strcat(strcat(strcpy(namebuf,CRONDIR),"/"),u->name);
		pname = namebuf;
	} else
		pname = u->name;
	if ((cf=fopen(pname,"r")) == NULL) {
		mail(u->name,NOREAD,2);
		return; 
	}
	while (fgets(line,CTLINESIZE,cf) != NULL) {
		/* process a line of a crontab file */
		cursor = 0;
		while(line[cursor] == ' ' || line[cursor] == '\t')
			cursor++;
		if(line[cursor] == '#')
			continue;
		e = (struct event *) xmalloc(sizeof(struct event));
		e->etype = CRONEVENT;
		if ((e->of.ct.minute=next_field(0,59,u)) == NULL) goto badline;
		if ((e->of.ct.hour=next_field(0,23,u)) == NULL) goto badline;
		if ((e->of.ct.daymon=next_field(1,31,u)) == NULL) goto badline;
		if ((e->of.ct.month=next_field(1,12,u)) == NULL) goto badline;
		if ((e->of.ct.dayweek=next_field(0,6,u)) == NULL) goto badline;
		if (line[++cursor] == '\0') {
			mail(u->name,EOLN,1);
			goto badline; 
		}
		/* get the command to execute	*/
		start = cursor;
again:
		while ((line[cursor]!='%')&&(line[cursor]!='\n')
		    &&(line[cursor]!='\0') && (line[cursor]!='\\')) cursor++;
		if(line[cursor] == '\\') {
			cursor += 2;
			goto again;
		}
		e->cmd = xmalloc(cursor-start+1);
		strncpy(e->cmd,line+start,cursor-start);
		e->cmd[cursor-start] = '\0';
		/* see if there is any standard input	*/
		if (line[cursor] == '%') {
			e->of.ct.input = xmalloc(strlen(line)-cursor+1);
			strcpy(e->of.ct.input,line+cursor+1);
			for (i=0; i<strlen(e->of.ct.input); i++)
				if (e->of.ct.input[i] == '%') e->of.ct.input[i] = '\n'; 
		}
		else e->of.ct.input = NULL;
		/* have the event point to it's owner	*/
		e->u = u;
		/* insert this event at the front of this user's event list   */
		e->link = u->ctevents;
		u->ctevents = e;
		/* set the time for the first occurance of this event	*/
		e->time = next_time(e);
		/* finally, add this event to the main event list	*/
		el_add(e,e->time,u->ctid);
#ifdef DEBUG
		printf("inserting cron event %s at %ld (%s)",
			e->cmd,e->time,ctime(&e->time));
#endif
		continue;

badline: 
		free(e); 
	}

	fclose(cf);
}


/*****************/
mail(usrname,msg,format)
/*****************/
char *usrname,*msg;
int format;
{
	/* mail mails a user a message.	*/

	FILE *pipe,*popen();
	char *temp,*i,*strrchr();

#ifdef TESTING
	return;
#endif
	temp = xmalloc(strlen(MAIL)+strlen(usrname)+2);
	pipe = popen(strcat(strcat(strcpy(temp,MAIL)," "),usrname),"w");
	if (pipe!=NULL) {
		if (format == 1) {
			fprintf(pipe,"Your crontab file has an error in it.\n");
			i = strrchr(line,'\n');
			if (i != NULL) *i = ' ';
			fprintf(pipe, "\t%s\n\t%s\n",line,msg);
			fprintf(pipe, "This entry has been ignored.\n"); 
		}
		else fprintf(pipe, "Cron: %s\n",msg);
		/* dont want to do pclose because pclose does a wait */
		fclose(pipe); 
		/* decremented in idle() */
		running++;
	}
	free(temp);
}


/*****************/
char *next_field(lower,upper,u)
/*****************/
int lower,upper;
struct usr *u;
{
	/* next_field returns a pointer to a string which holds 
	   the next field of a line of a crontab file.
	   if (numbers in this field are out of range (lower..upper),
	       or there is a syntax error) then
			NULL is returned, and a mail message is sent to
			the user telling him which line the error was in.     */

	char *s;
	int num,num2,start;

	while ((line[cursor]==' ') || (line[cursor]=='\t')) cursor++;
	start = cursor;
	if (line[cursor] == '\0') {
		mail(u->name,EOLN,1);
		return(NULL); 
	}
	if (line[cursor] == '*') {
		cursor++;
		if ((line[cursor]!=' ') && (line[cursor]!='\t')) {
			mail(u->name,UNEXPECT,1);
			return(NULL); 
		}
		s = xmalloc(2);
		strcpy(s,"*");
		return(s); 
	}
	while (TRUE) {
		if (!isdigit(line[cursor])) {
			mail(u->name,UNEXPECT,1);
			return(NULL); 
		}
		num = 0;
		do { 
			num = num*10 + (line[cursor]-'0'); 
		}			while (isdigit(line[++cursor]));
		if ((num<lower) || (num>upper)) {
			mail(u->name,OUTOFBOUND,1);
			return(NULL); 
		}
		if (line[cursor]=='-') {
			if (!isdigit(line[++cursor])) {
				mail(u->name,UNEXPECT,1);
				return(NULL); 
			}
			num2 = 0;
			do { 
				num2 = num2*10 + (line[cursor]-'0'); 
			}				while (isdigit(line[++cursor]));
			if ((num2<lower) || (num2>upper)) {
				mail(u->name,OUTOFBOUND,1);
				return(NULL); 
			}
		}
		if ((line[cursor]==' ') || (line[cursor]=='\t')) break;
		if (line[cursor]=='\0') {
			mail(u->name,EOLN,1);
			return(NULL); 
		}
		if (line[cursor++]!=',') {
			mail(u->name,UNEXPECT,1);
			return(NULL); 
		}
	}
	s = xmalloc(cursor-start+1);
	strncpy(s,line+start,cursor-start);
	s[cursor-start] = '\0';
	return(s);
}


/*****************/
time_t next_time(e)
/*****************/
struct event *e;
{
	/* returns the integer time for the next occurance of event e.
	   the following fields have ranges as indicated:
	PRGM  | min	hour	day of month	mon	day of week
	------|-------------------------------------------------------
	cron  | 0-59	0-23	    1-31	1-12	0-6 (0=sunday)
	time  | 0-59	0-23	    1-31	0-11	0-6 (0=sunday)
	   NOTE: this routine is hard to understand. */

	struct tm *tm,*localtime();
	int tm_mon,tm_mday,tm_wday,wday,m,min,h,hr,carry,day,days,
	d1,day1,carry1,d2,day2,carry2,daysahead,mon,yr,db,wd,today;
	time_t t;

	t = time((long *) 0);
	tm = localtime(&t);

	tm_mon = next_ge(tm->tm_mon+1,e->of.ct.month) - 1;	/* 0-11 */
	tm_mday = next_ge(tm->tm_mday,e->of.ct.daymon);		/* 1-31 */
	tm_wday = next_ge(tm->tm_wday,e->of.ct.dayweek);	/* 0-6  */
	today = TRUE;
	if ( (strcmp(e->of.ct.daymon,"*")==0 && tm->tm_wday!=tm_wday)
	    || (strcmp(e->of.ct.dayweek,"*")==0 && tm->tm_mday!=tm_mday)
	    || (tm->tm_mday!=tm_mday && tm->tm_wday!=tm_wday)
	    || (tm->tm_mon!=tm_mon)) today = FALSE;

	m = tm->tm_min+1;
	min = next_ge(m%60,e->of.ct.minute);
	carry = (min < m) ? 1:0;
	h = tm->tm_hour+carry;
	hr = next_ge(h%24,e->of.ct.hour);
	carry = (hr < h) ? 1:0;
	if ((!carry) && today) {
		/* this event must occur today	*/
		if (tm->tm_min>min)
			t +=(time_t)(hr-tm->tm_hour-1)*HOUR + 
			    (time_t)(60-tm->tm_min+min)*MINUTE;
		else t += (time_t)(hr-tm->tm_hour)*HOUR +
			(time_t)(min-tm->tm_min)*MINUTE;
		return(t-(long)tm->tm_sec); 
	}

	min = next_ge(0,e->of.ct.minute);
	hr = next_ge(0,e->of.ct.hour);

	/* calculate the date of the next occurance of this event,
	   which will be on a different day than the current day.	*/

	/* check monthly day specification	*/
	d1 = tm->tm_mday+1;
	day1 = next_ge((d1-1)%days_in_mon(tm->tm_mon,tm->tm_year)+1,e->of.ct.daymon);
	carry1 = (day1 < d1) ? 1:0;

	/* check weekly day specification	*/
	d2 = tm->tm_wday+1;
	wday = next_ge(d2%7,e->of.ct.dayweek);
	if (wday < d2) daysahead = 7 - d2 + wday;
	else daysahead = wday - d2;
	day2 = (d1+daysahead-1)%days_in_mon(tm->tm_mon,tm->tm_year)+1;
	carry2 = (day2 < d1) ? 1:0;

	/* based on their respective specifications,
	   day1, and day2 give the day of the month
	   for the next occurance of this event.	*/

	if ((strcmp(e->of.ct.daymon,"*")==0) && (strcmp(e->of.ct.dayweek,"*")!=0)) {
		day1 = day2;
		carry1 = carry2; 
	}
	if ((strcmp(e->of.ct.daymon,"*")!=0) && (strcmp(e->of.ct.dayweek,"*")==0)) {
		day2 = day1;
		carry2 = carry1; 
	}

	yr = tm->tm_year;
	if ((carry1 && carry2) || (tm->tm_mon != tm_mon)) {
		/* event does not occur in this month	*/
		m = tm->tm_mon+1;
		mon = next_ge(m%12+1,e->of.ct.month)-1;		/* 0..11 */
		carry = (mon < m) ? 1:0;
		yr += carry;
		/* recompute day1 and day2	*/
		day1 = next_ge(1,e->of.ct.daymon);
		db = days_btwn(tm->tm_mon,tm->tm_mday,tm->tm_year,mon,1,yr) + 1;
		wd = (tm->tm_wday+db)%7;
		/* wd is the day of the week of the first of month mon	*/
		wday = next_ge(wd,e->of.ct.dayweek);
		if (wday < wd) day2 = 1 + 7 - wd + wday;
		else day2 = 1 + wday - wd;
		if ((strcmp(e->of.ct.daymon,"*")!=0) && (strcmp(e->of.ct.dayweek,"*")==0))
			day2 = day1;
		if ((strcmp(e->of.ct.daymon,"*")==0) && (strcmp(e->of.ct.dayweek,"*")!=0))
			day1 = day2;
		day = (day1 < day2) ? day1:day2; 
	}
	else { /* event occurs in this month	*/
		mon = tm->tm_mon;
		if (!carry1 && !carry2) day = (day1 < day2) ? day1 : day2;
		else if (!carry1) day = day1;
		else day = day2;
	}

	/* now that we have the min,hr,day,mon,yr of the next
	   event, figure out what time that turns out to be.	*/

	days = days_btwn(tm->tm_mon,tm->tm_mday,tm->tm_year,mon,day,yr);
	t += (time_t)(23-tm->tm_hour)*HOUR + (time_t)(60-tm->tm_min)*MINUTE
	    + (time_t)hr*HOUR + (time_t)min*MINUTE + (time_t)days*DAY;
	return(t-(long)tm->tm_sec);
}



#define	DUMMY	100
/****************/
next_ge(current,list)
/****************/
int current;
char *list;
{
	/* list is a character field as in a crontab file;
	   	for example: "40,20,50-10"
	   next_ge returns the next number in the list that is
	   greater than or equal to current.
	   if no numbers of list are >= current, the smallest
	   element of list is returned.
	   NOTE: current must be in the appropriate range.	*/

	char *ptr;
	int n,n2,min,min_gt;

	if (strcmp(list,"*") == 0) return(current);
	ptr = list;
	min = DUMMY; 
	min_gt = DUMMY;
	while (TRUE) {
		if ((n=(int)num(&ptr))==current) return(current);
		if (n<min) min=n;
		if ((n>current)&&(n<min_gt)) min_gt=n;
		if (*ptr=='-') {
			ptr++;
			if ((n2=(int)num(&ptr))>n) {
				if ((current>n)&&(current<=n2))
					return(current); 
			}
			else {	/* range that wraps around */
				if (current>n) return(current);
				if (current<=n2) return(current); 
			}
		}
		if (*ptr=='\0') break;
		ptr += 1; 
	}
	if (min_gt!=DUMMY) return(min_gt);
	else return(min);
}

/****************/
del_atjob(name,usrname)
/****************/
char	*name;
char	*usrname;
{

	struct	event	*e, *eprev;
	struct	usr	*u;

	if((u = find_usr(usrname)) == NULL)
		return;
	e = u->atevents;
	eprev = NULL;
	while(e != NULL)
		if(strcmp(name,e->cmd) == 0) {
			if(next_event == e)
				next_event = NULL;
			if(eprev == NULL)
				u->atevents = e->link;
			else
				eprev->link = e->link;
			el_remove(e->of.at.eventid, 1);
			free(e->cmd);
			free(e);
			break;
		} else {
			eprev = e;
			e = e->link;
		}
	if(!u->ctexists && u->atevents == NULL) {
#ifdef DEBUG
		printf("%s removed from usr list\n",usrname);
#endif
		if(ulast == NULL)
			uhead = u->nextusr;
		else
			ulast->nextusr = u->nextusr;
		free(u->name);
		free(u->home);
		free(u);
	}
}

/*****************/
del_ctab(name)
/*****************/
char	*name;
{

	struct	usr	*u;

	if((u = find_usr(name)) == NULL)
		return;
	rm_ctevents(u);
	el_remove(u->ctid, 0);
	u->ctid = 0;
	u->ctexists = 0;
	if(u->atevents == NULL) {
#ifdef DEBUG
		printf("%s removed from usr list\n",name);
#endif
		if(ulast == NULL)
			uhead = u->nextusr;
		else
			ulast->nextusr = u->nextusr;
		free(u->name);
		free(u->home);
		free(u);
	}
}


/*****************/
rm_ctevents(u)
/*****************/
struct usr *u;
{
	struct event *e2,*e3;

	/* see if the next event (to be run by cron)
	   is a cronevent owned by this user.		*/
	if ( (next_event!=NULL) && 
	    (next_event->etype==CRONEVENT) &&
	    (next_event->u==u) )
		next_event = NULL;
	e2 = u->ctevents;
	while (e2 != NULL) {
		free(e2->cmd);
		free(e2->of.ct.minute);
		free(e2->of.ct.hour);
		free(e2->of.ct.daymon);
		free(e2->of.ct.month);
		free(e2->of.ct.dayweek);
		if (e2->of.ct.input != NULL) free(e2->of.ct.input);
		e3 = e2->link;
		free(e2);
		e2 = e3; 
	}
	u->ctevents = NULL;
}


/****************/
struct usr *find_usr(uname)
/****************/
char *uname;
{
	struct usr *u;

	u = uhead;
	ulast = NULL;
	while (u != NULL) {
		if (strcmp(u->name,uname) == 0) return(u);
		ulast = u;
		u = u->nextusr; 
	}
	return(NULL);
}


/*****************/
ex(e)
/*****************/
struct event *e;
{

	register i,j;
	short sp_flag;
	int fd,rfork;
	char *at_cmdfile, *cron_infile;
	char *mktemp();
	char *tempnam();
	struct stat buf;
	struct queue *qp;
	struct runinfo *rp;

	qp = &qt[e->etype];	/* set pointer to queue defs */
	if(qp->nrun >= qp->njob) {
		msg("%c queue max run limit reached",e->etype+'a');
		resched(qp->nwait);
		return;
	}
	for(rp=rt; rp < rt+MAXRUN; rp++) {
		if(rp->pid == 0)
			break;
	}
	if(rp >= rt+MAXRUN) {
		msg("MAXRUN (%d) procs reached",MAXRUN);
		resched(qp->nwait);
		return;
	}
#ifdef ATLIMIT
	if((e->u)->uid != 0 && (e->u)->aruncnt >= ATLIMIT) {
		msg("ATLIMIT (%d) reached for uid %d",ATLIMIT,(e->u)->uid);
		resched(qp->nwait);
		return;
	}
#endif
#ifdef CRONLIMIT
	if((e->u)->uid != 0 && (e->u)->cruncnt >= CRONLIMIT) {
		msg("CRONLIMIT (%d) reached for uid %d",CRONLIMIT,(e->u)->uid);
		resched(qp->nwait);
		return;
	}
#endif

	rp->outfile = tempnam(TMPDIR,PFX);
	if((rfork = fork()) == -1) {
		msg("cannot fork");
		resched(wait_time);
		sleep(30);
		return;
	}
	if(rfork) {	/* parent process */
		++qp->nrun;
		++running;
		rp->pid = rfork;
		rp->que = e->etype;
#ifdef ATLIMIT
		if(e->etype != CRONEVENT)
			(e->u)->aruncnt++;
#endif
#if ATLIMIT && CRONLIMIT
		else
			(e->u)->cruncnt++;
#else
#ifdef CRONLIMIT
		if(e->etype == CRONEVENT)
			(e->u)->cruncnt++;
#endif
#endif
		rp->rusr = (e->u);
		logit((char)BCHAR,rp,0);
		return;
	}

	for (i=0; i<20; i++) close(i);

	if (e->etype != CRONEVENT ) {
		/* open jobfile as stdin to shell */
		at_cmdfile = xmalloc(strlen(ATDIR)+strlen(e->cmd)+2);
		strcat(strcat(strcpy(at_cmdfile,ATDIR),"/"),e->cmd);
		if (stat(at_cmdfile,&buf)) exit(1);
		if (!(buf.st_mode&ISUID)) {
			/* if setuid bit off, original owner has 
			   given this file to someone else	*/
			unlink(at_cmdfile);
			exit(1); 
		}
		if (open(at_cmdfile,O_RDONLY) == -1) {
			mail((e->u)->name,BADJOBOPEN,2);
			unlink(at_cmdfile);
			exit(1); 
		}
		unlink(at_cmdfile); 
	}

	/* set correct user and group identification	*/
	if ((setgid((e->u)->gid)<0) || (setuid((e->u)->uid)<0))
		exit(1);
	sp_flag = FALSE;
	if (e->etype == CRONEVENT)
		/* check for standard input to command	*/
		if (e->of.ct.input != NULL) {
			cron_infile = mktemp(TMPINFILE);
			if ((fd=creat(cron_infile,INMODE)) == -1) {
				mail((e->u)->name,NOSTDIN,2);
				exit(1); 
			}
			if (write(fd,e->of.ct.input,strlen(e->of.ct.input))
			    != strlen(e->of.ct.input)) {
				mail((e->u)->name,NOSTDIN,2);
				unlink(cron_infile);
				exit(1); 
			}
			close(fd);
			/* open tmp file as stdin input to sh	*/
			if (open(cron_infile,O_RDONLY)==-1) {
				mail((e->u)->name,NOSTDIN,2);
				unlink(cron_infile);
				exit(1); 
			}
			unlink(cron_infile); 
		}
		else if (open("/dev/null",O_RDONLY)==-1) {
			open("/",O_RDONLY);
			sp_flag = TRUE; 
		}

	/* redirect stdout and stderr for the shell	*/
	if (creat(rp->outfile,OUTMODE)!=-1) dup(1);
	else if (open("/dev/null",O_WRONLY)!=-1) dup(1);
	if (sp_flag) close(0);
	strcat(homedir,(e->u)->home);
	strcat(logname,(e->u)->name);
	environ = envinit;
	if (chdir((e->u)->home)==-1) {
		mail((e->u)->name,CANTCDHOME,2);
		exit(1); 
	}
#ifdef TESTING
	exit(1);
#endif
	if((e->u)->uid != 0)
		nice(qp->nice);
	if (e->etype == CRONEVENT)
		execl(SHELL,"sh","-c",e->cmd,0);
	else /* type == ATEVENT */
		execl(SHELL,"sh",0);
	mail((e->u)->name,CANTEXECSH,2);
	exit(1);
}


/***************/
idle(tyme)
/***************/
long tyme;
{

	long t;
	time_t	now;
	int	pid;
	int	prc;
	long	alm;
	struct	runinfo	*rp;

	t = tyme;
	while(t > 0L) {
		if(running) {
			if(t > wait_time)
				alm = wait_time;
			else
				alm = t;
#ifdef DEBUG
			printf("in idle - setting alarm for %ld sec\n",alm);
#endif
			alarm((unsigned) alm);
			pid = wait(&prc);
			alarm(0);
			if(pid == -1) {
				if(msg_wait())
					return;
			} else {
				for(rp=rt;rp < rt+MAXRUN; rp++)
					if(rp->pid == pid)
						break;
				if(rp >= rt+MAXRUN) {
					msg("unexpected pid returned %d (ignored)",pid);
					/* incremented in mail() */
					running--;
				}
				else
					if(rp->que == ZOMB) {
						running--;
						rp->pid = 0;
						free(rp->outfile);
						unlink(rp->outfile);
					}
					else
						cleanup(rp,prc);
			}
		} else {
			msg_wait();
			return;
		}
		now = time((char *) 0);
		t = next_event->time - now;
	}
}


/*****************/
cleanup(pr,rc)
/*****************/
struct	runinfo	*pr;
{

	int	fd;
	char	line[5+UNAMESIZE+CTLINESIZE];
	struct	usr	*p;
	struct	stat	buf;

	logit((char)ECHAR,pr,rc);
	--qt[pr->que].nrun;
	pr->pid = 0;
	--running;
	p = pr->rusr;
#ifdef ATLIMIT
	if(pr->que != CRONEVENT)
		--p->aruncnt;
#endif
#if ATLIMIT && CRONLIMIT
	else
		--p->cruncnt;
#else
#ifdef CRONLIMIT
	if(pr->que == CRONEVENT)
		--p->cruncnt;
#endif
#endif
	if(!stat(pr->outfile,&buf)) {
		if(buf.st_size > 0) {
			if((fd=open(pr->outfile,O_WRONLY)) == -1)
				mail(p->name,STDOUTERR,2);
			else {
				lseek(fd,(long) 0,2);
				write(fd,STDERRMSG,strlen(STDERRMSG));
				close(fd);
				/* mail user stdout and stderr */
				sprintf(line,"%s %s < \"%s\"\n",
					MAIL,p->name,pr->outfile);
				if((pr->pid = fork()) == 0) {
					execl("/bin/sh","sh","-c",line,0);
					exit(127);
				}
				pr->que = ZOMB;
				running++;

			}
		} else {
			free(pr->outfile);
			unlink(pr->outfile);
		}
	}
}

#define	MSGSIZE	sizeof(struct message)

/*****************/
msg_wait()
/*****************/
{

	long	t;
	time_t	now;
	struct	stat msgstat;
	struct	message	*pmsg;
	int	cnt;

	if(fstat(msgfd,&msgstat) != 0)
		crabort("cannot stat fifo queue");
	if(msgstat.st_size == 0 && running)
		return(0);
	if(next_event == NULL)
		t = INFINITY;
	else {
		now = time((long *) 0);
		t = next_event->time - now;
		if(t <= 0L)
			t = 1L;
	}
#ifdef DEBUG
	printf("in msg_wait - setting alarm for %ld sec\n", t);
#endif
	alarm((unsigned) t);
	pmsg = &msgbuf;
	if((cnt=read(msgfd,pmsg,MSGSIZE)) != MSGSIZE) {
		if(errno != EINTR) {
			perror("! read");
			notexpired = 1;
		}
		if(next_event == NULL)
			notexpired = 1;
		return(1);
	}
	alarm(0);
	if(pmsg->etype != NULL) {
		switch(pmsg->etype) {
		case AT:
			if(pmsg->action == DELETE)
				del_atjob(pmsg->fname,pmsg->logname);
			else
				mod_atjob(pmsg->fname);
			break;
		case CRON:
			if(pmsg->action == DELETE)
				del_ctab(pmsg->fname);
			else
				mod_ctab(pmsg->fname);
			break;
		default:
			msg("message received - bad format");
			break;
		}
		if (next_event != NULL) {
			if (next_event->etype == CRONEVENT)
				el_add(next_event,next_event->time,(next_event->u)->ctid);
			else /* etype == ATEVENT */
				el_add(next_event,next_event->time,next_event->of.at.eventid);
			next_event = NULL;
		}
		fflush(stdout);
		pmsg->etype = NULL;
		notexpired = 1;
		return(1);
	}
}


/*****************/
timeout()
/*****************/
{
	signal(SIGALRM, timeout);
}

/*****************/
crabort(mssg)
/*****************/
char *mssg;
{
	/* crabort handles exits out of cron */
	int c;

	/* write error msg to console */
	if ((c=open(CONSOLE,O_WRONLY))>=0) {
		write(c,"cron aborted: ",14);
		write(c,mssg,strlen(mssg));
		write(c,"\n",1);
		close(c); 
	}
	msg(mssg);
	msg("******* CRON ABORTED ********");
	exit(1);
}

/*****************/
msg(fmt,a,b)
/*****************/
char *fmt;
{

	time_t	t;

	t = time((long *) 0);
	printf("! ");
	printf(fmt,a,b);
	printf(" %s",ctime(&t));
	fflush(stdout);
}


/*****************/
logit(cc,rp,rc)
/*****************/
char	cc;
struct	runinfo	*rp;
{
	time_t t;
	int    ret;
	char   *ctime();

	t = time((long *) 0);
	if(cc == BCHAR)
		printf("%c  CMD: %s\n",cc, next_event->cmd);
	printf("%c  %.8s %u %c %.24s",
		cc,(rp->rusr)->name, rp->pid, QUE(rp->que),ctime(&t));
	if((ret=TSTAT(rc)) != 0)
		printf(" ts=%d",ret);
	if((ret=RCODE(rc)) != 0)
		printf(" rc=%d",ret);
	putchar('\n');
	fflush(stdout);
}

/***************/
resched(delay)
/***************/
int	delay;
{
	time_t	nt;

	/* run job at a later time */
	nt = next_event->time + delay;
	if(next_event->etype == CRONEVENT) {
		next_event->time = next_time(next_event);
		if(nt < next_event->time)
			next_event->time = nt;
		el_add(next_event,next_event->time,(next_event->u)->ctid);
		delayed = 1;
		msg("rescheduling a cron job");
		return;
	}
	add_atevent(next_event->u, next_event->cmd, nt);
	msg("rescheduling at job");
}

#define	QBUFSIZ		80

/**********/
quedefs(action)
/**********/
int	action;
{
	register i;
	int	j;
	char	name[DIRSIZ+1];
	char	qbuf[QBUFSIZ];
	FILE	*fd;

	/* set up default queue definitions */
	for(i=0;i<NQUEUE;i++) {
		qt[i].njob = qd.njob;
		qt[i].nice = qd.nice;
		qt[i].nwait = qd.nwait;
	}
	if(action == DEFAULT)
		return;
	if((fd = fopen(QUEDEFS,"r")) == NULL) {
		msg("cannot open quedefs file");
		msg("using default queue definitions");
		return;
	}
	while(fgets(qbuf, QBUFSIZ, fd) != NULL) {
		if((j=qbuf[0]-'a') < 0 || j >= NQUEUE || qbuf[1] != '.')
			continue;
		i = 0;
		while(qbuf[i] != NULL)
			name[i] = qbuf[i++];
		parsqdef(&name[2]);
		qt[j].njob = qq.njob;
		qt[j].nice = qq.nice;
		qt[j].nwait = qq.nwait;
	}
	fclose(fd);
}

/**********/
parsqdef(name)
/**********/
char *name;
{
	register i;

	qq = qd;
	while(*name) {
		i = 0;
		while(isdigit(*name)) {
			i *= 10;
			i += *name++ - '0';
		}
		switch(*name++) {
		case JOBF:
			qq.njob = i;
			break;
		case NICEF:
			qq.nice = i;
			break;
		case WAITF:
			qq.nwait = i;
			break;
		}
	}
}
