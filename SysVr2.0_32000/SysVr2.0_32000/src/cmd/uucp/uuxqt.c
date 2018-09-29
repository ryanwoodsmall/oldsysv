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

#include "uucp.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/dir.h>
#include <time.h>
#include <stdio.h>
#include <signal.h>

#define APPCMD(d) {\
char *p;\
for (p = d; *p != '\0';) *cmdp++ = *p++;\
*cmdp++ = ' ';\
*cmdp = '\0';}

/*
 * execute commands set up by a uux command,
 * usually from a remote machine - set by uucp.
 */
FILE *Cfile;
void exit();
char *strcpy();
int notiok = 1;

#define PATH	"PATH=/bin:/usr/bin:/usr/lbin "
#define LOGNAME	"LOGNAME=uucp "
#define XBUFSIZ	350

/*  to remove restrictions from uuxqt
 *  define ALLOK 1
 *  to add allowable commands, add to the file CMDFILE
 */
int	statop;
char	Sfile[MAXFULLNAME];
char xfile[MAXFULLNAME], user[NAMEBUF], buf[XBUFSIZ];
long	time();
char	**Env;
int	mask;
main(argc, argv, envp)
char	**envp;
char *argv[];
{
	FILE *xfp, *dfp, *fp;
	int gid, uid, ret, cret, badfiles=0;
	int stcico = 0;
	int argnok;
	char xcmd[XBUFSIZ];
	char lbuf[30];
	char cfile[NAMESIZE], dfile[MAXFULLNAME];
	char file[NAMESIZE];
	char fin[MAXFULLNAME], sysout[NAMESIZE], fout[MAXFULLNAME];
	char path[MAXFULLNAME];
	char cmd[XBUFSIZ];
	char *cmdp, prm[XBUFSIZ], *ptr;
	char *getprm(), *strrchr(), *lxp;
	char retstat[30];
	struct stat sb;
	extern onintr();

	signal(SIGILL, onintr);
	signal(SIGTRAP, onintr);
	signal(SIGIOT, onintr);
	signal(SIGEMT, onintr);
	signal(SIGFPE, onintr);
	signal(SIGBUS, onintr);
	signal(SIGSEGV, onintr);
	signal(SIGSYS, onintr);
	signal(SIGPIPE, onintr);
	signal(SIGTERM, onintr);
	/*
	 * get local system name
	 */
	Env = envp;
	nstat.t_qtime = time((long *)0);
	strcpy(Progname, "uuxqt");
	Pchar = 'Q';
	uucpname(Myname);
	mask = umask(WFMASK);
	Ofn = 1;
	Ifn = 0;
	while (argc>1 && argv[1][0] == '-') {
		switch(argv[1][1]){

		/*
		 * debugging level
		 */
		case 'x':
			Debug = atoi(&argv[1][2]);
			if (Debug <= 0)
				Debug = 1;
			break;
		default:
			fprintf(stderr, "unknown flag %s\n", argv[1]);
				break;
		}
		--argc;  argv++;
	}

	DEBUG(4, "\n\n** %s **\n", "START");

	/*
	 * change to spool directory
	 */
	cret = chdir(Spool);
	ASSERT(cret == 0, "CANNOT CHDIR TO SPOOL - ", Spool, cret);
	if(cret != 0) {
		DEBUG(1, "No spool directory - %s\n", Spool);
		exit(0);
	}
	strcpy(Wrkdir, Spool);

	/*
	 * determine user who started uuxqt
	 */
	gid = getegid();
	uid = geteuid();
	guinfo(uid, User, path);
	setuucp(User);
	DEBUG(4, "User - %s\n", User);
	guinfo(getuid(), Loginuser, buf);
	dlogent("uuxqt - Loginuser",Loginuser);
	if (ulockf(X_LOCK, (time_t)  X_LOCKTIME) != 0)
		exit(0);

	DEBUG(4, "process %s\n", "");
	while (gtxfile(xfile) > 0) {
		statop = 0;
		DEBUG(4, "xfile - %s\n", xfile);

		xfp = fopen(xfile, "r");
		if (xfp == NULL) {
			char file1[100];
			sprintf(file1,"%s/badxfile",Spool);
			xcp(xfile,file1);
			unlink(xfile);
		}
		ASSERT(xfp != NULL, "CAN'T OPEN", xfile, 0);

		if(stat(xfile, &sb) != -1)
			nstat.t_qtime = sb.st_mtime;
		/*
		 * initialize to default
		 */
		strcpy(user, User);
		strcpy(fin, "/dev/null");
		strcpy(fout, "/dev/null");
		sprintf(sysout, "%.6s", Myname);
		badfiles = 0;
		cmd[0] = '\0';
		while (fgets(buf, XBUFSIZ, xfp) != NULL) {

			/*
			 * interpret JCL
			 */
			switch (buf[0]) {

			/*
			 * user name
			 */
			case X_USER:
				sscanf(&buf[1], "%s%s", user, Rmtname);
				break;

			/*
			 * standard input
			 */
			case X_STDIN:
				sscanf(&buf[1], "%s", fin);
				expfile(fin);
				if (chkpth("", "", fin)) {
					DEBUG(4,"badfile - in: %s\n",fin);
					badfiles = 1;
					}
				break;

			/*
			 * standard output
			 */
			case X_STDOUT:
				sscanf(&buf[1], "%s%s", fout, sysout);
				sysout[SYSNSIZE] = '\0';
#ifdef FOWARD
				if (fout[0] != '!')
				{
#endif
				if (fout[0] != '~' || prefix(sysout, Myname))
					expfile(fout);
				if (chkpth("", "", fout)) {
					badfiles = 1;
					DEBUG(4,"badfile - out: %s\n",fout);
					}
#ifdef FOWARD
				}
#endif
				break;

			/*
			 * command to execute
			 */
			case X_CMD:
				strcpy(cmd, &buf[2]);
				if (*(cmd + strlen(cmd) - 1) == '\n')
					*(cmd + strlen(cmd) - 1) = '\0';
				break;
			case X_MAILF:
				statop++;
				sscanf(&buf[1], "%s", Sfile);
				break;

			/*
			 * no notification
			 */
			case X_NONOTI:
				notiok = 0;
				break;
			default:
				break;
			}
		}

		fclose(xfp);
		DEBUG(4, "fin - %s, ", fin);
		DEBUG(4, "fout - %s, ", fout);
		DEBUG(4, "sysout - %s, ", sysout);
		DEBUG(4, "user - %s\n", user);
		DEBUG(4, "cmd - %s\n", cmd);
		if (cmd[0] == '\0')
			continue;

		/*
		 * command execution
		 * generate a temporary file (if necessary)
		 * to hold output to be shipped back
		 */
		if (strcmp(fout, "/dev/null") == SAME)
			strcpy(dfile,"/dev/null");
		else
			gename(DATAPRE, sysout, 'O', dfile);

		/*
		 * expand file names where necessary
		 */
		expfile(dfile);
		strcpy(buf, PATH);
		strcat(buf, LOGNAME);
		cmdp = buf + strlen(buf);
		ptr = cmd;
		xcmd[0] = '\0';
		argnok = 0;
		while ((ptr = getprm(ptr, prm)) != NULL) {
			if (prm[0] == ';' || prm[0] == '^'
			  || prm[0] == '&'  || prm[0] == '|'
			  || prm[0] == '`' ) {
				xcmd[0] = '\0';
				APPCMD(prm);
				continue;
			}


				/*
				 * command not valid
				 */
			if ((argnok = argok(xcmd, prm)) != 0) 
				break;

			if (prm[0] == '~')
				expfile(prm);
			APPCMD(prm);
		}

		/*
		 * check to see if command can be executed
		 */
		if (argnok || badfiles) {
			sprintf(lbuf, "%s XQT DENIED", user);
			logent(cmd, lbuf);
			DEBUG(4, "bad command %s\n", prm);
			notify(user, Rmtname, cmd, "DENIED");
			goto rmfiles;
		}
		sprintf(lbuf, "%s XQT", user);
		logent(buf, lbuf);
		DEBUG(4, "cmd %s\n", buf);

		/*
		 * move files to execute directory
		 * and change to that directory
		 */
		mvxfiles(xfile);
		cret = chdir(XQTDIR);
		ASSERT(cret == 0, "CANNOT CHDIR TO - ", XQTDIR, cret);
		if(cret != 0) {
			DEBUG(1, "No XQTDIR - %s\n", XQTDIR);
			cleanup(0);
		}

		/*
		 * invok shell to execute command
		 */
		mask = umask(0);
		ret = shio(buf, fin, dfile, CNULL);
		sprintf(retstat, "exit %d, signal %d",
		  (ret>>8) & 0377, ret & 0377);
		if (strcmp(xcmd, "rmail") != SAME
		  && strcmp(xcmd, "mail") != SAME) {

		/*
		 * see if user wants respcifiction
		 */
		  if (notiok)
			notify(user, Rmtname, cmd, retstat);
		} else 
			if (ret !=0) {

			/*
			 * mail failed 
			 * return letter to sender
			 */
			retosndr(user, Rmtname, fin, buf);
			sprintf(buf, "ret (%o) from %s!%s", ret, Rmtname, user);
			logent("MAIL FAIL", buf);
		}
		umask(mask);

		/*
		 * change back to spool directory
		 */
		DEBUG(4, "exit cmd - %d\n", ret);
		cret = chdir(Spool);
		ASSERT(cret == 0, "CANNOT CHDIR TO SPOOL - ", Spool, cret);
		if(cret != 0) {
			DEBUG(1, "No spool directory - %s\n", Spool);
			exit(0);
		}

		/*
		 * remove files
		 */
		rmxfiles(xfile);
		if (ret != 0) {

			/*
			 * exit status not zero, so append to
			 * returned data
			 */
			dfp = fopen(dfile, "a");
			ASSERT(dfp != NULL, "CAN'T OPEN", dfile, 0);
			fprintf(dfp, "exit status %d", ret);
			fclose(dfp);
		}
		if (strcmp(fout, "/dev/null") != SAME) {

			/*
			 * if output is on this machine
			 * copy output these otherwise
			 * spawn job to send to send output
			 * elsewhere
			 */
			if (prefix(sysout, Myname)) {
				xmv(dfile, fout);
			} else {
				gename(CMDPRE, sysout, 'O', cfile);
				fp = fopen(cfile, "w");
				ASSERT(fp != NULL, "OPEN", cfile, 0);
				fprintf(fp, "S %s %s %s - %s 0666\n",
				dfile, fout, user, (lxp=strrchr(dfile, '/'))?lxp+1:dfile);
				fclose(fp);
			}
		}
	rmfiles:

		/*
		 * delete job files in spool directory
		 */
		xfp = fopen(xfile, "r");
		ASSERT(xfp != NULL, "CAN'T OPEN - to remove dfiles", xfile, 0);
		while (fgets(buf, XBUFSIZ, xfp) != NULL) {
			if (buf[0] != X_RQDFILE)
				continue;
			sscanf(&buf[1], "%s", file);
			unlink(file);
		}
		unlink(xfile);
		fclose(xfp);
	}

	if (stcico)
		xuucico("");
	cleanup(0);
}


cleanup(code)
int code;
{
	logcls();
	rmlock(CNULL);
	exit(code);
}

/*
 * catch signal then cleanup and exit
 */
onintr(inter)
register int inter;
{
	char str[30];
	signal(inter, SIG_IGN);
	sprintf(str, "QSIGNAL %d", inter);
	logent(str, "QCAUGHT");
	cleanup(-inter);
}

/*
 * get a file to execute
 *	file	-> a read to return filename in
 * returns:
 *	0	-> no file
 *	1	-> file to execute
 */
gtxfile(file)
register char *file;
{
	static  FILE *pdir;
	char pre[2];

	/*
	 * open spool directory on first pass
	 */
	if (pdir == NULL) {
		pdir = fopen(Spool, "r");
		ASSERT(pdir != NULL, "GTXFILE CAN'T OPEN", Spool, 0);
	}

	pre[0] = XQTPRE;
	pre[1] = '\0';

	/*
	 * scan spool directory looking for execute file
	 */
	while (gnamef(pdir, file) != 0) {
		DEBUG(4, "file - %s\n", file);

		/*
		 * look for x prefix
		 */
		if (!prefix(pre, file))
			continue;

		/*
		 * check to see if associated files have arrived
		 */
		if (gotfiles(file))

			return(1);
	}

	fclose(pdir);
	return(0);
}


/*
 * check for needed files
 *	file	-> name of file to check
 * return: 
 *	0	-> not ready
 *	1	-> all files ready
 */
gotfiles(file)
register char *file;
{
	register FILE *fp;
	struct stat stbuf;
	char buf[XBUFSIZ], rqfile[MAXFULLNAME];

	fp = fopen(file, "r");
	if (fp == NULL)
		return(FALSE);

	while (fgets(buf, XBUFSIZ, fp) != NULL) {
		DEBUG(4, "%s\n", buf);

		/*
		 * look at requried files
		 */
		if (buf[0] != X_RQDFILE)
			continue;
		sscanf(&buf[1], "%s", rqfile);

		/*
		 * expand file name 
		 */
		expfile(rqfile);

		/*
		 * see if file exists
		 */
		if (stat(rqfile, &stbuf) == -1) {
			fclose(fp);
			return(FALSE);
		}
	}

	fclose(fp);
	return(TRUE);
}


/*
 * remove execute files to x-directory
 * return:
 *	none
 */
rmxfiles(xfile)
char *xfile;
{
	register FILE *fp;
	char buf[XBUFSIZ], file[NAMESIZE], tfile[NAMESIZE];
	char tfull[MAXFULLNAME];

	if((fp = fopen(xfile, "r")) == NULL)
		return;

	/*
	 * unlink each file belonging to job
	 */
	while (fgets(buf, XBUFSIZ, fp) != NULL) {
		if (buf[0] != X_RQDFILE)
			continue;
		if (sscanf(&buf[1], "%s%s", file, tfile) < 2)
			continue;
		sprintf(tfull, "%s/%s", XQTDIR, tfile);
		unlink(tfull);
	}
	fclose(fp);
	return;
}


/*
 * move execute files to x-directory
 *	xfile	-> excute file name
 * return: 
 *	none
 */
mvxfiles(xfile)
char *xfile;
{
	register FILE *fp;
	int ret;
	char buf[XBUFSIZ], ffile[MAXFULLNAME], tfile[NAMESIZE];
	char tfull[MAXFULLNAME];

	if((fp = fopen(xfile, "r")) == NULL)
		return;

	while (fgets(buf, XBUFSIZ, fp) != NULL) {
		if (buf[0] != X_RQDFILE)
			continue;
		if (sscanf(&buf[1], "%s%s", ffile, tfile) < 2)
			continue;

		/*
		 * expand file names and move to
		 * execute directory
		 */
		expfile(ffile);
		sprintf(tfull, "%s/%s", XQTDIR, tfile);
		unlink(tfull);
		ret = link(ffile, tfull);
		ASSERT(ret == 0, "LINK ERROR", "", ret);
		unlink(ffile);
	}
	fclose(fp);
	return;
}


/*
 * check for valid command/argumanet
 * NOTE: side effect is to set xc to the
 *	 command to be executed.
 * return:
 *	0	-> success
 *	1	-> failure
 */
argok(xc, cmd)
register char *xc, *cmd;
{
	register char *gret;
	char ptr[XBUFSIZ];
	char *strchr();

#ifndef ALLOK

	/*
	 * don't allow sh command strings `....`
	 * don't allow redirection of standard in or our
	 */
	if (strchr(cmd, '`') != NULL
	  || strchr(cmd, '>') != NULL
	  || strchr(cmd, ';') != NULL
	  || strchr(cmd, '^') != NULL
	  || strchr(cmd, '&') != NULL
	  || strchr(cmd, '|') != NULL
	  || strchr(cmd, '<') != NULL) {
		DEBUG(5,"Illegal shell character in command string\n","");
		return(TRUE);
	}
#endif

	if (xc[0] != '\0')
		return(FALSE);

	DEBUG(5, "cmd = %s\n", cmd);
#ifndef ALLOK

	/*
	 * check for valid command
	 */


	if (cmdchk(cmd,Rmtname,CMDFILE) < 0)
		return(TRUE);
#endif
	strcpy(xc, cmd);
	return(FALSE);
}

/*
 * Check to see if command is valid for a specific machine.
 * Will check L.sys file. The format is: cmd,machine1,machine2,
 * If no machine is specified for a given command then the command
 * can be used for all remote machines.  Otherwise, if machine 
 * names are specified, only those machines can use that command.
 * As usual if the desired command is not in L.sys then execution
 * will be denied.
 * Return codes:	
 *	ok: 0
 *	fail: <0
 */


cmdchk(cmd, machine, file)
char	*cmd, *machine;
char	*file;
{
	FILE *cmdfile;
	char buf[XBUFSIZ+1];
	register char *s, *i;
	char *strchr();
	char *strpbrk();

	cmdfile = fopen(file, "r");
	ASSERT(cmdfile != NULL, "CAN'T OPEN", CMDFILE,0);
	while (fgets(buf, sizeof(buf)-1, cmdfile) != NULL) {
	/*
	 * Any machines specified
	 */
	if ((s = strchr(buf, ',')) != NULL) {
		/*
	 	 * check for case: cmd,machine,machine
	 	 */
		*s++ = '\0';
		if (strcmp(buf, cmd) == SAME) {
			DEBUG(5,"matched - command: %s\n",buf);
			while (*s != '\n') {
				if ((i = strpbrk(s,",\n\0")) != NULL){
					if (*i == ',')
						*i++ = '\0';
					else
					if (*i == '\n') {
						*i++ = '\0';
						*i = '\n';
					} else 
						return(-4);
				}
				DEBUG(5, "machine: %s\n",s);
				if (strcmp(machine, s) == SAME) {
					DEBUG(5,"machine match: %s\n",s);
					fclose(cmdfile);
					return(0);
				}
				s = i;
			}
			fclose(cmdfile);
			DEBUG(5, "machine invalid: %s\n",machine);
			return(-2);
		}else 
			continue;
	}else
	/*
	 * machine permission not required - check command permission 
	 *  case: command
	 */
	if ((i = strpbrk(buf,"\n\0")) != NULL)
		*i='\0';
	else 
		return(-4);
	if (strcmp(buf, cmd)==SAME) {
		fclose(cmdfile);
		return(0);
	}else 
		continue;
 	}
	fclose(cmdfile);
	return(-1);
}






stmesg(u, f, m)
char	*u, *f, *m;
{
	struct tm *tp;
	FILE	*Cf;
	extern struct tm *localtime();
	time_t	time();
	time_t	clock;
	long	td,th,tm,ts;

	if((Cf = fopen(Sfile, "a+")) == NULL){
		return;
	}
	chmod(Sfile, 0666);
DEBUG(4,"STM %d\n",Cf);
	fprintf(Cf, "uucp job %.4s ", &xfile[strlen(xfile)-4]);
	time(&clock);
	tp = localtime(&clock);
	fprintf(Cf, "(%d/%d-%d:%2.2d:%2.2d) ", tp->tm_mon + 1,
		tp->tm_mday, tp->tm_hour, tp->tm_min, tp->tm_sec);
	td = clock - nstat.t_qtime;
	ts = td%60;
	td /= 60;
	tm = td%60;
	td /= 60;
	th = td;
	fprintf(Cf, "(%ld:%ld:%ld) ", th, tm, ts);
	fprintf(Cf, "%s %s\n", f, m);
	fclose(Cf);

}
/*
 * send mail to user giving execution results
 * assumesnew mail command send remote mail
 *	user	-> user to notify
 *	rmt	-> system name where user resides
 *	cmd	-> command executed
 *	str	-> message
 * return: 
 *	none
 */
notify(user, rmt, cmd, str)
char *user, *rmt, *cmd, *str;
{
	char text[XBUFSIZ];
	char ruser[XBUFSIZ];

	sprintf(text, "uuxqt cmd (%.50s) status (%s)", cmd, str);
	if (prefix(rmt, Myname)){
		strcpy(ruser, user);
		if(statop){
			stmesg(user, text, "");
			return;
		}
	}else
		sprintf(ruser, "%s!%s", rmt, user);
	mailst(ruser, text, "");
	return;
}

/*
 * return mail to sender
 *	user	-> user to notify
 *	rmt	-> system name where user resides
 *	file	-> file to return
 *	cmd	-> command name
 * return:
 *	none
 */
retosndr(user, rmt, file, cmd)
char *user, *rmt, *file, *cmd;
{
	char ruser[XBUFSIZ];
	char msg[XBUFSIZ];

	if (strncmp(rmt, Myname, SYSNSIZE) == SAME)
		strcpy(ruser, user);
	else
		sprintf(ruser, "%s!%s", rmt, user);

	sprintf(msg, "Mail failed (%s).  Letter returned to sender.\n", cmd);

	if (anyread(file) == 0)
		mailst(ruser, msg, file);
	else
		mailst(ruser, msg, "");
	return;
}
