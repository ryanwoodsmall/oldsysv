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
/**************************************************************************
 ***			C r o n t a b . c				***
 **************************************************************************

	date:	7/2/82
	description:	This program implements crontab (see cron(1)).
			This program should be set-uid to root.
	files:
		/usr/lib/cron drwxr-xr-x root sys
		/usr/lib/cron/cron.allow -rw-r--r-- root sys
		/usr/lib/cron/cron.deny -rw-r--r-- root sys

 **************************************************************************/

char identif[] = "crontab - version  @(#)crontab.c	1.4, date 9/2/83";

#include <sys/types.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <ctype.h>
#include "cron.h"

#define TMPFILE		"_cron"		/* prefix for tmp file */
#define CRMODE		0444	/* mode for creating crontabs */

#define BADCREATE	"can't create your crontab file in the crontab directory."
#define BADOPEN		"can't open your crontab file."
#define BADSHELL	"because your login shell isn't /bin/sh, you can't use cron."
#define WARNSHELL	"warning: commands will be executed using /bin/sh\n"
#define BADUSAGE	"proper usage is: \n	crontab [file] [-r]"
#define INVALIDUSER	"you are not a valid user (no entry in /etc/passwd)."
#define NOTALLOWED	"you are not authorized to use cron.  Sorry."
#define EOLN		"unexpected end of line."
#define UNEXPECT	"unexpected character found in line."
#define OUTOFBOUND	"number out of bounds."

extern int per_errno;
int err,cursor,catch();
char *cf,*tnam,line[CTLINESIZE];
extern char *xmalloc();

main(argc,argv)
char **argv;
{
	char login[UNAMESIZE],*getuser(),*strcat(),*strcpy();
	char *pp;
	FILE *fp;

	if (argc>2) crabort(BADUSAGE);
	pp = getuser(getuid());
	if(pp == NULL) {
		if (per_errno==2)
			crabort(BADSHELL);
		else
			crabort(INVALIDUSER); 
	}
	strcpy(login,pp);
	if (!allowed(login,CRONALLOW,CRONDENY)) crabort(NOTALLOWED);

	cf = xmalloc(strlen(CRONDIR)+strlen(login)+2);
	strcat(strcat(strcpy(cf,CRONDIR),"/"),login);
	if ((argc==2) && (strcmp(argv[1],"-r")==0)) {
		unlink(cf);
		sendmsg(DELETE,login);
		exit(0); 
	}
	if ((argc==2) && (strcmp(argv[1], "-l") ==0)) {
		if((fp = fopen(cf,"r")) == NULL)
			crabort(BADOPEN);
		while(fgets(line,CTLINESIZE,fp) != NULL)
			fputs(line,stdout);
		fclose(fp);
		exit(0);
	}

	if (argc==1) copycron(stdin);
	else if ((fp=fopen(argv[1],"r"))==NULL) crabort(BADOPEN);
	else copycron(fp);
	sendmsg(ADD,login);
	if(per_errno == 2)
		fprintf(stderr,WARNSHELL);
}


/******************/
copycron(fp)
/******************/
FILE *fp;
{
	FILE *tfp,*fdopen();
	char pid[6],*strcat(),*strcpy();
	int t;

	sprintf(pid,"%-5d",getpid());
	tnam=xmalloc(strlen(CRONDIR)+strlen(TMPFILE)+7);
	strcat(strcat(strcat(strcpy(tnam,CRONDIR),"/"),TMPFILE),pid);
	/* catch SIGINT, SIGHUP, SIGQUIT signals */
	if (signal(SIGINT,catch) == SIG_IGN) signal(SIGINT,SIG_IGN);
	if (signal(SIGHUP,catch) == SIG_IGN) signal(SIGHUP,SIG_IGN);
	if (signal(SIGQUIT,catch) == SIG_IGN) signal(SIGQUIT,SIG_IGN);
	if (signal(SIGTERM,catch) == SIG_IGN) signal(SIGTERM,SIG_IGN);
	if ((t=creat(tnam,CRMODE))==-1) crabort(BADCREATE);
	if ((tfp=fdopen(t,"w"))==NULL) {
		unlink(tnam);
		crabort(BADCREATE); 
	}
	err=0;	/* if errors found, err set to 1 */
	while (fgets(line,CTLINESIZE,fp) != NULL) {
		cursor=0;
		while(line[cursor] == ' ' || line[cursor] == '\t')
			cursor++;
		if(line[cursor] == '#')
			goto cont;
		if (next_field(0,59)) continue;
		if (next_field(0,23)) continue;
		if (next_field(1,31)) continue;
		if (next_field(1,12)) continue;
		if (next_field(0,06)) continue;
		if (line[++cursor] == '\0') {
			cerror(EOLN);
			continue; 
		}
cont:
		if (fputs(line,tfp) == EOF) {
			unlink(tnam);
			crabort(BADCREATE); 
		}
	}
	fclose(fp);
	fclose(tfp);
	if (!err) {
		/* make file tfp the new crontab */
		unlink(cf);
		if (link(tnam,cf)==-1) {
			unlink(tnam);
			crabort(BADCREATE); 
		} 
	}
	unlink(tnam);
}


/*****************/
next_field(lower,upper)
/*****************/
int lower,upper;
{
	int num,num2;

	while ((line[cursor]==' ') || (line[cursor]=='\t')) cursor++;
	if (line[cursor] == '\0') {
		cerror(EOLN);
		return(1); 
	}
	if (line[cursor] == '*') {
		cursor++;
		if ((line[cursor]!=' ') && (line[cursor]!='\t')) {
			cerror(UNEXPECT);
			return(1); 
		}
		return(0); 
	}
	while (TRUE) {
		if (!isdigit(line[cursor])) {
			cerror(UNEXPECT);
			return(1); 
		}
		num = 0;
		do { 
			num = num*10 + (line[cursor]-'0'); 
		}			while (isdigit(line[++cursor]));
		if ((num<lower) || (num>upper)) {
			cerror(OUTOFBOUND);
			return(1); 
		}
		if (line[cursor]=='-') {
			if (!isdigit(line[++cursor])) {
				cerror(UNEXPECT);
				return(1); 
			}
			num2 = 0;
			do { 
				num2 = num2*10 + (line[cursor]-'0'); 
			}				while (isdigit(line[++cursor]));
			if ((num2<lower) || (num2>upper)) {
				cerror(OUTOFBOUND);
				return(1); 
			}
		}
		if ((line[cursor]==' ') || (line[cursor]=='\t')) break;
		if (line[cursor]=='\0') {
			cerror(EOLN);
			return(1); 
		}
		if (line[cursor++]!=',') {
			cerror(UNEXPECT);
			return(1); 
		}
	}
	return(0);
}


/**********/
cerror(msg)
/**********/
char *msg;
{
	fprintf(stderr,"%scrontab: error on previous line; %s\n",line,msg);
	err=1;
}


/**********/
catch()
/**********/
{
	unlink(tnam);
	exit(1);
}


/**********/
crabort(msg)
/**********/
char *msg;
{
	fprintf(stderr,"crontab: %s\n",msg);
	exit(1);
}

/***********/
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
	pmsg->etype = CRON;
	pmsg->action = action;
	strncpy(pmsg->fname,fname,FLEN);
	if(write(msgfd,pmsg,sizeof(struct message)) != sizeof(struct message))
		fprintf(stderr,"at: error in message send\n");
}
