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
/* @(#)mail.c	1.14 */

/*
 *	Mail to a remote machine will normally use the uux command.
 *	If available, it may be better to send mail via nusend
 *	or usend, although delivery is not as reliable as uux.
 *	Mail may be compiled to take advantage
 *	of these other networks by adding:
 *		#define USE_NUSEND  for nusend
 *	and
 *		#define USE_USEND   for usend.
 *
 *	NOTE:  If either or both defines are specified, that network
 *	will be tried before uux.
 */

#include	<stdio.h>
#include	<sys/types.h>
#include	<signal.h>
#include	<pwd.h>
#include	<time.h>
#include	<utmp.h>
#include	<sys/stat.h>
#include	<setjmp.h>
#include	<sys/utsname.h>

#define LOCKON	1	/* lock set */
#define LOCKOFF	0	/* lock released */
#define C_NOACCESS	0	/* no access to file */
#define CHILD	0
#define SAME	0

#define CERROR		-1
#define CSUCCESS	0

#define TRUE	1
#define FALSE	0

#define E_FLGE	1	/* exit flge error */
#define E_FILE	2	/* exit file error */
#define E_SPACE	3	/* exit no space */
#define E_FRWD	3	/* exit cannot forward */
/*
 * copylet flags
 */
#define	REMOTE		1		/* remote mail, add rmtmsg */
#define ORDINARY	2
#define ZAP		3		/* zap header and trailing empty line */
#define FORWARD		4

#define	LSIZE		BUFSIZ		/* maximum size of a line */
#define	MAXLET		300		/* maximum number of letters */
#define FROMLEVELS	20		/* maxium number of forwards */
#define MAXFILENAME	128

#ifndef	MFMODE
#define	MFMODE		0660		/* create mode for `/usr/mail' files */
#endif
#define MAILGRP		6

#define A_OK		0		/* return value for access */
#define A_EXIST		0		/* access check for existence */
#define A_WRITE		2		/* access check for write permission */
#define A_READ		4		/* access check for read permission */


struct	let	{
	long	adr;
	char	change;
} let[MAXLET];

struct	passwd	*getpwuid(), getpwent();
struct	utsname utsn;

FILE	*tmpf, *malf;

char	lettmp[] = "/tmp/maXXXXX";
char	from[] = "From ";
char	TO[] = "To: ";
char	maildir[] = "/usr/mail/";
char	sendto[256];

/*
 * fowrding address
 */
char	*mailfile;
char	maillock[] = ".lock";
char	dead[] = "/dead.letter";
char	rmtbuf[] = "/tmp/marXXXXXX";
#define RMTMSG	" remote from %s\n"
#define FORWMSG	" forwarded by %s\n"
char	frwrd[] = "Forward to ";
char	nospace[] = "mail: no space for temp file\n";
char	*thissys;
char	mbox[] = "/mbox";
char	curlock[50];
char	line[LSIZE];
char	resp[LSIZE];
char	*hmbox;
char	*hmdead;
char	*home;
char	*my_name;
char	*getlogin();
char	*malloc();
char	lfil[50];
char	*ctime();
char	*getenv();
char	*strrchr();
char	*mktemp();

int	error;
int	fromlevel = 0;
int	nlet	= 0;
int	locked;
int	changed;
int	forward;
int	delete();
int	flgf = 0;
int	flgp = 0;
int	flge = 0;
int	delflg = 1;
int	toflg = 0;
int	savdead();
int	(*saveint)();
int	(*setsig())();
int	strln();

long	ftell();
long	iop;

jmp_buf	sjbuf;

#ifdef USE_NUSEND
jmp_buf nusendfail;
int	nusendjmp = FALSE;
#endif

#ifdef USE_USEND
jmp_buf usendfail;
int	usendjmp = FALSE;
#endif

unsigned umsave;
char	*help[] = {
	"q\t\tquit\n",
	"x\t\texit without changing mail\n",
	"p\t\tprint\n",
	"s [file]\tsave (default mbox)\n",
	"w [file]\tsame without header\n",
	"-\t\tprint previous\n",
	"d\t\tdelete\n",
	"+\t\tnext (no delete)\n",
	"m [user]\tmail to user\n",
	"! cmd\t\texecute cmd\n",
	0
};

/*
 *	mail [ + ] [ -irpqet ]  [ -f file ]
 *	mail [ -t ] persons 
 *	rmail [ -t ] persons 
 */
main(argc, argv)
char	**argv;
{
	register i;

	umsave = umask(7);
	setbuf(stdout, malloc(BUFSIZ));
	uname(&utsn);
	thissys = utsn.nodename;

	/* 
	 * Use environment variables
	 * logname is used for from
	 */
	if(((home = getenv("HOME")) == NULL) || (strlen(home) == 0))
		home = ".";
	if (((my_name = getenv("LOGNAME")) == NULL) || (strlen(my_name) == 0))
		my_name = getlogin();
	if ((my_name == NULL) || (strlen(my_name) == 0))
		my_name = getpwuid(geteuid())->pw_name;
	/*
	 * Catch signals for cleanup
	 */
	if(setjmp(sjbuf)) 
		done();
	for (i=SIGINT; i<SIGCLD; i++)
		setsig(i, delete);
	setsig(SIGHUP, done);
	/*
	 * Make temporary file for letter
	 */
	mktemp(lettmp);
	unlink(lettmp);
	if((tmpf = fopen(lettmp, "w")) == NULL){
		fprintf(stderr, "mail: can't open %s for writing\n", lettmp);
		error = E_FILE;
		done();
	}
	/*
	 * Rmail always invoked to send mail
	 */
	if (argv[0][0] != 'r' &&	
		(argc == 1 || argv[1][0] == '-' || argv[1][0] == '+'))
		printmail(argc, argv);
	else
		sendmail(argc, argv);
	done();
}

/*
 * Signal reset
 * signals that are not being ignored will be 
 * caught by function f
 *	i	-> signal number
 *	f	-> signal routine
 * return
 *	rc	-> former signal
 */
int (*setsig(i, f))()
int	i;
int	(*f)();
{
	register int (*rc)();

	if((rc = signal(i, SIG_IGN))!=SIG_IGN)
		signal(i, f);
	return(rc);
}

/*
 * Print mail entries
 *	argc	-> argument count
 *	argv	-> arguments
 */
printmail(argc, argv)
char	**argv;
{
	register int c;
	int	flg, i, j, print, aret, stret, goerr = 0;
	char	*p, *getarg();
	char	frwrdbuf[256];
	struct	stat stbuf;
	extern	char *optarg;

	/*
	 * Print in reverse order
	 */
	if (argv[1][0] == '+') {
		forward = 1;
		argc--;
		argv++;
	}
	while((c = getopt(argc, argv, "f:rpqiet")) != EOF)
		switch(c) {

		/*
		 * use file input for mail
		 */
		case 'f':
			flgf = 1;
			mailfile = optarg;
			break;
		/* 
		 * print without prompting
		 */
		case 'p':
			flgp++;
		/* 
		 * terminate on deletes
		 */
		case 'q':
			delflg = 0;
		case 'i':
			break;

		/* 
		 * print by first in, first out order
		 */
		case 'r':
			forward = 1;
			break;
		/*
		 * do  not print mail
 		 */
		case 'e':
			flge = 1;
			break;

		case 't':
			toflg = 1;
			break;
		/*
		 * bad option
		 */
		case '?':
			goerr++;
	}
	if(goerr) {
		fprintf(stderr, "usage: mail [-erpqt] [-f file] [persons]\n");
		error = E_FILE;
		done();

	}
	if (toflg == 1 && argc > 2) {
		argv++;
		sendmail(--argc, argv);
		done();
	}

	/*
	 * create working directory mbox name
	 */
	if((hmbox = malloc(strlen(home) + strlen(mbox) + 1)) == NULL){
		fprintf(stderr, "mail: can't allocate hmbox");
		error = E_FILE;
		return;
	}
	cat(hmbox, home, mbox);
	if(!flgf) {
		if(((mailfile = getenv("MAIL")) == NULL) || (strlen(mailfile) == 0)) {
			if((mailfile = malloc(strlen(maildir) + strlen(my_name) + 1)) == NULL){
				fprintf(stderr, "mail: can't allocate mailfile");
				error = E_FILE;
				return;
			}
			cat(mailfile, maildir, my_name);
		}
	}
	/*
	 * Check accessibility of mail file
	 */
	stret = stat(mailfile, &stbuf);
	if((aret=access(mailfile, A_READ)) == A_OK)
		malf = fopen(mailfile, "r");
	if (stret == CSUCCESS && aret == CERROR) {
		fprintf(stderr, "mail: permission denied!\n");
		error = E_FILE;
		return;
	}else 
	if (flgf && (aret == CERROR || (malf == NULL))) {
		fprintf(stderr, "mail: cannot open %s\n", mailfile);
		error = E_FILE;
		return;
	}else 
	if(aret == CERROR || (malf == NULL) || (stbuf.st_size == 0)) {
		if(!flge) printf("No mail.\n");
		error = E_FLGE;
		return;
	}
	lock(mailfile);
	/*
	 * See if mail is to be forwarded to 
	 * another system
	 */
	if(areforwarding(mailfile)) {
		if(flge) {
			unlock();
			error = E_FLGE;
			return;
		}
		if (!((stbuf.st_gid == MAILGRP) && ((stbuf.st_mode & 0777)== MFMODE))) {
			printf("Your mail cannot be forwarded.\n");
			printf("Check permissions and group id of mail file.\n");
			unlock();
			error = E_FRWD;
			return;
		}
		printf("Your mail is being forwarded to ");
		fseek(malf, (long)(sizeof(frwrd) - 1), 0);
		fgets(frwrdbuf, sizeof(frwrdbuf), malf);
		printf("%s", frwrdbuf);
		if(getc(malf) != EOF)
			printf("and your mailbox contains extra stuff\n");
		unlock();
		return;
	}
	if(flge) {
		unlock();
		return;
	}
	/*
	 * copy mail to temp file and mark each
	 * letter in the let array
	 */
	copymt(malf, tmpf);
	fclose(malf);
	fclose(tmpf);
	unlock();
	if((tmpf = fopen(lettmp, "r")) == NULL) {
		fprintf(stderr, "mail: can't open %s\n", lettmp);
		error = E_FILE;
		return;
	}
	changed = 0;
	print = 1;
	for (i = 0; i < nlet; ) {

		/*
		 * reverse order ?
		 */
		j = forward ? i : nlet - i - 1;
		if( setjmp(sjbuf) == 0 && print != 0 )
				copylet(j, stdout, ORDINARY);
		
		/*
		 * print only
		 */
		if(flgp) {
			i++;
			continue;
		}
		/*
		 * Interactive
		 */
		setjmp(sjbuf);
		printf("? ");
		fflush(stdout);
		if (fgets(resp, sizeof(resp), stdin) == NULL)
			break;
		print = 1;
		switch (resp[0]) {

		default:
			printf("usage\n");

		/*
		 * help
		 */
		case '?':
			print = 0;
			{
				register i;

				for(i=0;help[i];i++)
					printf("%s", help[i]);
			}
			break;
		/*
		 * skip entry
		 */
		case '+':

		case 'n':
		case '\n':
			i++;
		case 'p':
			break;
		case 'x':
			changed = 0;
		case 'q':
			goto donep;
		/*
		 * Previous entry
		 */
		case '^':
		case '-':
			if (--i < 0)
				i = 0;
			break;
		/*
		 * Save in file without header
		 */
		case 'y':
		case 'w':
		/*
		 * Save mail with header
		 */
		case 's':
			if (resp[1] == '\n' || resp[1] == '\0')
				cat(resp+1, hmbox, "");
			else if(resp[1] != ' ') {
				printf("invalid command\n");
				print = 0;
				continue;
			}
			umask(umsave);
			flg = 0;
			p = resp+1;
			if (getarg(lfil, p) == NULL)
				cat(resp+1, hmbox, "");
			for (p = resp+1; (p = getarg(lfil, p)) != NULL; ) {
				if((aret=legal(lfil)))
					malf = fopen(lfil, "a");
				if ((malf == NULL) || (aret == 0)) {
					fprintf(stderr, "mail: cannot append to %s\n", lfil);
					flg++;
					continue;
				}
				if(aret==2)
					chown(lfil, geteuid(), getgid());
				if (copylet(j, malf, resp[0]=='w'? ZAP: ORDINARY) == FALSE) {
					fprintf(stderr, "mail: cannot save mail\n");
					flg++;
				}
				fclose(malf);
			}
			umask(7);
			if (flg)
				print = 0;
			else {
				let[j].change = 'd';
				changed++;
				i++;
			}
			break;
		/*
		 * Mail letter to someone else
		 */
		case 'm':
			if (resp[1] == '\n' || resp[1] == '\0') {
				i++;
				continue;
			}
			if (resp[1] != ' ') {
				printf("invalid command\n");
				print = 0;
				continue;
			}
			flg = 0;
			p = resp+1;
			if (getarg(lfil, p) == NULL) {
				i++;
				continue;
			}
			for (p = resp+1; (p = getarg(lfil, p)) != NULL; ) {
				if (lfil[0] == '$' && !(getenv(&lfil[1]))) {
					fprintf(stderr,"%s has no value or is \
not exported.\n",lfil);
					flg++;
				}
				else if (sendrmt(j, lfil) == FALSE)
 					flg++;
			}
			if (flg)
				print = 0;
			else {
				let[j].change = 'd';
				changed++;
				i++;
			}
			break;
		/*
		 * Escape to shell
		 */
		case '!':
			system(resp+1);
			printf("!\n");
			print = 0;
			break;
		/*
		 * Delete an entry
		 */
		case 'd':
			let[j].change = 'd';
			changed++;
			i++;
			if (resp[1] == 'q')
				goto donep;
			break;
		}
	}
	/*
	 * Copy updated mail file back
	 */
   donep:
	if (changed)
		copyback();
}

/*
 * copy temp or whatever back to /usr/mail
 */
copyback()
{
	register i, n, c;
	int new = 0, aret;
	struct stat stbuf;

	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	lock(mailfile);
	stat(mailfile, &stbuf);
	/*
	 * Has new mail arrived?
	 */
	if (stbuf.st_size != let[nlet].adr) {
		if((malf = fopen(mailfile, "r")) == NULL) {
			fprintf(stderr, "mail: can't re-read %s\n", mailfile);
			error = E_FILE;
			done();
		}
		fseek(malf, let[nlet].adr, 0);
		fclose(tmpf);
		if((tmpf = fopen(lettmp, "a")) == NULL) {
			fprintf(stderr, "mail: can't re-write %s\n", mailfile);
			error = E_FILE;
			done();
		}

		/*
		 * append new mail
		 * assume only one
		 * new letter
		 */
		while ((c = fgetc(malf)) != EOF)
			if (fputc(c, tmpf) == EOF) {
				fclose(malf);
				fclose(tmpf);
				fprintf(stderr, nospace);
				error = E_SPACE;
				done();
			}
		fclose(malf);
		fclose(tmpf);
		if((tmpf = fopen(lettmp, "r")) == NULL) {
			fprintf(stderr, "mail: can't re-read %s\n", lettmp);
			error = E_FILE;
			done();
		}
		if(nlet == (MAXLET-2)){
			fprintf(stderr, "copyback:Too many letters\n");
			error = E_SPACE;
			done();
		}
		let[++nlet].adr = stbuf.st_size;
		new = 1;
	}
	/*
	 * Copy mail back to mail file
	 */
	if((aret=access(mailfile, A_WRITE)) == A_OK)
		malf = fopen(mailfile, "w");
	if ((malf == NULL) || (aret == CERROR)) {
		fprintf(stderr, "mail: can't rewrite %s\n", mailfile);
		error = E_FILE;
		done();
	}
	n = 0;
	for (i = 0; i < nlet; i++)
		if (let[i].change != 'd') {
			if (copylet(i, malf, ORDINARY) == FALSE) {
				fprintf(stderr, "mail: cannot copy mail back\n");
			}
			n++;
		}
	fclose(malf);
	/*
	 * Empty mailbox?
	 */
	if ((n == 0) && ((stbuf.st_mode & 0777)== MFMODE))
		unlink(mailfile);
	if (new && !flgf)
		printf("new mail arrived\n");
	unlock();
}

/*
 * copy mail (f1) to temp (f2)
 */
copymt(f1, f2)	
register FILE *f1, *f2;
{
	long nextadr;
	int n;
	int mesg = 0;

	nlet = nextadr = 0;
	let[0].adr = 0;
	while (fgets(line, sizeof(line), f1) != NULL) {

		/*
		 * bug nlet should be checked
		 */
		if(line[0] == from[0])
			if (isfrom(line)){
				if(nlet >= (MAXLET-2)) {
					if (!mesg) {
						fprintf(stderr,"Warning: Too \
many letters, overflowing letters concatenated\n\n");
						mesg++;
					}
				}
				else
					let[nlet++].adr = nextadr;
			}
		n = strln(line);
		nextadr += n;
		if (write(f2->_file,line,n) != n) {
			fclose(f1);
			fclose(f2);
			fprintf(stderr, nospace);
			error = E_SPACE;
			done();
		}
	}

	/*
	 * last plus 1
	 */
	let[nlet].adr = nextadr;
}

/*
 * check to see if mail is being forwarded
 *	s	-> mail file
 * returns
 *	TRUE	-> forwarding
 *	FALSE	-> local
 */
areforwarding(s)
char *s;
{
	register char *p;
	register int c;
	FILE *fd;
	char fbuf[256];
	if((fd = fopen(s, "r")) == NULL) 
		return(FALSE);
	fbuf[0] = '\0';
	fread(fbuf, sizeof(frwrd) - 1, 1, fd);
	if(strncmp(fbuf, frwrd, sizeof(frwrd) - 1) == SAME) {
		for(p = sendto; (c = getc(fd)) != EOF && c != '\n';)
			if(c != ' ') 
			*p++ = c;
		*p = '\0';
		fclose(fd);
		return(TRUE);
	}
	fclose(fd);
	return(FALSE);
}

/*
 * copy letter 
 *	ln	-> index into: letter table
 *	f	-> file descrptor to copy file to
 *	type	-> copy type
 */
copylet(ln, f, type) 
register FILE *f;
{
	register int i;
	register char *s;
	char	buf[512], lastc, ch;
	int	n, j;
	long	k;
	int	num;

	fseek(tmpf, let[ln].adr, 0);
	k = let[ln+1].adr - let[ln].adr;
	if(k)
		k--;
	for(i=0;i<k;){
		s = buf;
		num = ((k-i) > sizeof(buf))?sizeof(buf):(k-i);
		if((n = read(tmpf->_file, buf, num)) <= 0){
			return(FALSE);
		}
		lastc = buf[n-1];
		if(i == 0){
			for(j=0;j<n;j++,s++){
				if(*s == '\n')
					break;
			}
			if(type != ZAP)
				if(write(f->_file,buf,j) == CERROR){
				return(FALSE);
				}
			i += j+1;
			n -= j+1;
			ch = *s++;
			switch(type){
			case REMOTE:
				fprintf(f, RMTMSG, thissys);
				break;
			case ORDINARY:
				fprintf(f, "%c", ch);
				break;
			case FORWARD:
				fprintf(f, FORWMSG, my_name);
				break;
			}
			fflush(f);
		}
		if(write(f->_file, s, n) != n){
			return(FALSE);
		}
		i += n;
	}
	if(type != ZAP || lastc != '\n'){
		read(tmpf->_file, buf, 1);
		write(f->_file, buf, 1);
	}
	return(TRUE);
}

/*
 * check for "from" string
 *	lp	-> string to be checked
 * returns
 *	TRUE	-> match
 *	FALSE	-> no match
 */
isfrom(lp)
register char *lp;

{
	register char *p;

	for (p = from; *p; )
		if (*lp++ != *p++)
			return(FALSE);
	return(TRUE);
}

/*
 * Send mail
 *	argc	-> argument count
 *	argv	-> argument list
 */
sendmail(argc, argv)
char **argv;
{
	int	aret;
	char	**args;
	int	fromflg = 0;
	char	*asctime();
	struct	tm *bp, *localtime();
	char	*tp, *zp;
	int	n;
	char buf[128];

	/*
	 * Format time
	 */
	time(&iop);
	bp = localtime(&iop);
	tp = asctime(bp);
	zp = tzname[bp->tm_isdst];
	sprintf(buf, "%s%s %.16s %.3s %.5s", from, my_name, tp, zp, tp+20);
	write(tmpf->_file,buf,strlen(buf));
	/*
	 * Copy to list in mail entry?
	 */
	if (toflg == 1 && argc > 1) {
		aret = argc;
		args = argv;
		sprintf(buf,"%s ",TO);
		write(tmpf->_file,buf,strlen(buf));
		while(--aret > 0) {
			sprintf(buf,"%s ",*++args);
			write(tmpf->_file,buf,strlen(buf));
		}
		write(tmpf->_file,"\n",1);
	}
	iop = ftell(tmpf);
	flgf = 1;
	/*
	 * Read mail message
	 */
	saveint = setsig(SIGINT, savdead);
	fromlevel = 0;
	while (fgets(line, sizeof(line), stdin) != NULL) {
		if (line[0] == '.' && line[1] == '\n')
			break;
		/*
		 * If "from" string is present prepend with
		 * a ">" so it is no longer interpreted as
		 * the last system fowarding the mail.
		 */
		if(line[0] == from[0])
			if (isfrom(line))
				write(tmpf->_file,">",1);
		/*
		 * Find out how many "from" lines
		 */
		if (fromflg == 0 && (strncmp(line, "From", 4) == SAME || strncmp(line, ">From", 5) == SAME))
			fromlevel++;
		else
			fromflg = 1;
		n = strln(line);
		if (write(tmpf->_file,line,n) != n) {
			fclose(tmpf);
			fprintf(stderr, nospace);
			error = E_SPACE;
			return;
		}
		flgf = 0;
	}
	setsig(SIGINT, saveint);
	write(tmpf->_file,"\n",1);
	/*
	 * In order to use some of the subroutines that
	 * are used to read mail, the let array must be set up
	 */
	nlet = 1;
	let[0].adr = 0;
	let[1].adr = ftell(tmpf);
	if (fclose(tmpf) == EOF) {
			fclose(tmpf);
			fprintf(stderr, nospace);
			error = E_SPACE;
			return;
	}
	if (flgf)
		return;
	if((tmpf = fopen(lettmp, "r")) == NULL) {
		fprintf(stderr, "mail: cannot reopen %s for reading\n", lettmp);
		error = E_FILE;
		return;
	}
	/*
	 * Send a copy of the letter to the specified users
	 */
	if (error == 0)
		while (--argc > 0)
			if (send(0, *++argv, 0) == FALSE) 
				error++;
	/*
	 * Save a copy of the letter in dead.letter if
	 * any letters can't be sent
	 */
	if (error) {
		/*
		 * Try to create dead letter in current directory
		 * or in home directory
		 */
		umask(umsave);
		if((aret=legal(&dead[1])))
			malf = fopen(&dead[1], "w");
		if ((malf == NULL) || (aret == 0)) {

			/*
			 * try to create in $HOME
			 */
			if((hmdead = malloc(strlen(home) + strlen(dead) + 1)) == NULL) {
				fprintf(stderr, "mail: can't malloc\n");
				goto out;
			}
			cat(hmdead, home, dead);
			if((aret=legal(hmdead)))
				malf = fopen(hmdead, "w");
			if ((malf == NULL) || (aret == 0)) {
				fprintf(stderr, "mail: cannot create %s\n", &dead[1]);
			out:
				fclose(tmpf);
				error = E_FILE;
				umask(7);
				return;
			}  else {
				chmod(hmdead, 0600);
				printf("Mail saved in %s\n", hmdead);
			}
		} else {
			chmod(&dead[1], 0600);
			printf("Mail saved in %s\n", &dead[1]);
		}
		/*
		 * Copy letter into dead letter box
		 */
		umask(7);
		if (copylet(0, malf, ZAP) == FALSE) {
			fprintf(stderr, "mail: cannot save in dead letter\n");
		}
		fclose(malf);
	}
	fclose(tmpf);
}

savdead()
{
	setsig(SIGINT, saveint);
	error++;
}

/*
 * send mail to remote system taking fowarding into account
 *	n	-> index into mail table
 *	name	-> mail destination
 * returns
 *	TRUE	-> sent mail
 *	FALSE	-> can't send mail
 */
sendrmt(n, name)
register char *name;
{
# define NSCCONS	"/usr/nsc/cons/"
	register char *p;
	register local;
	FILE *rmf, *popen();
	char rsys[64], cmd[200];
	char remote[30];

	/*
	 * assume mail is for remote
	 * look for bang to confirm that
	 * assumption
	 */
	local = 0;
	while (*name=='!')
		name++;
	for(p=rsys; *name!='!'; *p++ = *name++)
		if (*name=='\0') {
			local++;
			break;
		}
	*p = '\0';
	if ((!local && *name=='\0') || (local && *rsys=='\0')) {
		fprintf(stderr, "null name\n");
		return(FALSE);
	}
	if (local)
		sprintf(cmd, "mail %s", rsys);
	if (strcmp(thissys, rsys) == SAME) {
		local++;
		sprintf(cmd, "mail %s", name+1);
	}

	/*
	 * send local mail or remote via uux
	 */
	if (!local) {
		if (fromlevel > FROMLEVELS)
			return(FALSE);

#ifdef USE_NUSEND
		/*
		 * If mail can't be sent over NSC network
		 * use uucp.
		 */
		if (setjmp(nusendfail) == 0) {
			nusendjmp = TRUE;
			sprintf(remote, "%s%s", NSCCONS, rsys);
			if (access(remote, A_EXIST) != CERROR) {
				/*
				 * Send mail over NSC network
				 */
				sprintf(cmd, "nusend -d %s -s -e -!'rmail %s' - 2>/dev/null",
					rsys, name);
#ifdef DEBUG
printf("%s\n", cmd);
#endif
				if ((rmf=popen(cmd, "w")) != NULL) {
					copylet(n, rmf, local? FORWARD: REMOTE);
					if (pclose(rmf) == 0) {
						nusendjmp = FALSE;
						return(TRUE);
					}
				}
			}
		}
		nusendjmp = FALSE;
#endif

#ifdef USE_USEND
		if (setjmp(usendfail) == 0) {
			usendjmp = TRUE;
			sprintf(cmd, "usend -s -d%s -uNoLogin -!'rmail %s' - 2>/dev/null",
				rsys, name);
#ifdef DEBUG
printf("%s\n", cmd);
#endif
			if ((rmf=popen(cmd, "w")) != NULL) {
				copylet(n, rmf, local? FORWARD: REMOTE);
				if (pclose(rmf) == 0) {
					usendjmp = FALSE;
					return(TRUE);
				}
			}
		}
		usendjmp = FALSE;
#endif

		/*
		 * Use uux to send mail
		 */
		if (strchr(name+1, '!'))
			sprintf(cmd, "/usr/bin/uux - %s!rmail \\(%s\\)", rsys, name+1);
		else
			sprintf(cmd, "/usr/bin/uux - %s!rmail %s", rsys, name+1);
	}
#ifdef DEBUG
printf("%s\n", cmd);
#endif
	/*
	 * copy letter to pipe
	 */
	if ((rmf=popen(cmd, "w")) == NULL)
		return(FALSE);
	if (copylet(n, rmf, local? FORWARD: REMOTE) == FALSE) {
		fprintf(stderr, "mail: cannot pipe to mail command\n");
		pclose(rmf);
		return(FALSE);
	}

	/*
	 * check status
	 */
	return(pclose(rmf)==0 ? TRUE : FALSE);
}

/*
 * send letter n to name
 *	n	-> letter number
 *	name	-> mail destination
 *	level	-> depth of recursion for forwarding
 * returns
 *	TRUE	-> mail sent
 *	FALSE	-> can't send mail
 */
send(n, name, level)
int	n;
char	*name;
{
	register char *p;
	char	file[MAXFILENAME];
	struct	passwd	*pwd, *getpwname();
	int (*istat)(), (*qstat)(), (*hstat)();

	if(level > 20) {
		fprintf(stderr, "unbounded forwarding\n");
		return(FALSE);
	}
	if (strcmp(name, "-") == SAME)
		return(TRUE);
	/*
	 * See if mail is to be fowarded
	 */
	for(p=name; *p!='!' &&*p!='\0'; p++)
		;
	if (*p == '!')
		return(sendrmt(n, name));
	/*
	 * See if user has specified that mail is to be fowarded
	 */
	cat(file, maildir, name);
	if(areforwarding(file))
		return(send(n, sendto, level+1));
	/*
	 * see if user exists on this system
	 */
	setpwent();	
	if((pwd = getpwnam(name)) == NULL){
		fprintf(stderr, "mail: can't send to %s\n", name);
		return(FALSE);
	}
	cat(file, maildir, name);
	lock(file);
	/*
	 * If mail file does not exist create it
	 * with the correct uid and gid
	 */
	if(access(file, A_EXIST) == CERROR) {
		umask(0);
		istat = signal(SIGINT, SIG_IGN);
		qstat = signal(SIGQUIT, SIG_IGN);
		hstat = signal(SIGHUP, SIG_IGN);
		close(creat(file, MFMODE));
		umask(7);
		chown(file, pwd->pw_uid, getegid());
		signal(SIGINT, istat);
		signal(SIGQUIT, qstat);
		signal(SIGHUP, hstat);
	}
	/*
	 * Append letter to mail box
	 */
	if((malf = fopen(file, "a")) == NULL){
		fprintf(stderr, "mail: cannot append to %s\n", file);
		unlock();
		return(FALSE);
	}
	if (copylet(n, malf, ORDINARY) == FALSE) {
		fprintf(stderr, "mail: cannot append to %s\n", file);
		unlock();
		return(FALSE);
	}
	fclose(malf);
	unlock();
	return(TRUE);
}

/*
 * signal catching routine
 * reset signals on quits and interupts
 * exit on other signals
 *	i	-> signal #
 */
delete(i)
register int i;
{
	setsig(i, delete);

#ifdef USE_NUSEND
	if (i == SIGPIPE && nusendjmp == TRUE)
		longjmp(nusendfail, 1);
#endif

#ifdef USE_USEND
	if (i == SIGPIPE && usendjmp == TRUE)
		longjmp(usendfail, 1);
#endif

	if(i>SIGQUIT){
		fprintf(stderr, "mail: error signal %d\n", i);
	}else
		fprintf(stderr, "\n");
	if(delflg && (i==SIGINT || i==SIGQUIT))
		longjmp(sjbuf, 1);
	done();
}

/*
 * clean up lock files and exit
 */
done()
{
	unlock();
	unlink(lettmp);
	unlink(rmtbuf);
	exit(error);
}

/*
 * create mail lock file
 *	file	-> lock file name
 * returns:
 *	none
 */
lock(file)
char *file;
{
	register int f, i;

	if (locked == LOCKON)
		return;
	cat(curlock, file, maillock);
	for (i=0; i<10; i++) {
		if((f = creat(curlock, C_NOACCESS)) != CERROR){
			close(f);
			locked = LOCKON;
			return;
		}
		sleep(2);
	}
	fprintf(stderr, "mail: %s not creatable after %d tries\n", curlock, i);
	error = E_FILE;
	done();
}

unlock()
{
	unlink(curlock);
	locked = LOCKOFF;
}

/*
 * concatenate from1 and from2 to to
 *	to	-> destination string
 *	from1	-> source string
 *	from2	-> source string
 * return:
 *	none
 */
cat(to, from1, from2)
register char *to, *from1, *from2;
{

	for (; *from1; )
		*to++ = *from1++;
	for (; *from2; )
		*to++ = *from2++;
	*to = '\0';
}

/*
 * get next token
 *	p	-> string to be searched
 *	s	-> area to return token
 * returns:
 *	p	-> updated string pointer
 *	s	-> token
 *	NULL	-> no token
 */
char *getarg(s, p)	
register char *s, *p;
{
	while (*p == ' ' || *p == '\t')
		p++;
	if (*p == '\n' || *p == '\0')
		return(NULL);
	while (*p != ' ' && *p != '\t' && *p != '\n' && *p != '\0')
		*s++ = *p++;
	*s = '\0';
	return(p);
}

/*
 * check existence of file
 *	file	-> file to check
 * returns:
 *	0	-> exists unwriteable
 *	1	-> exists writeable
 *	2	-> does not exist
 */
legal(file)
register char *file;
{
	register char *sp;
	char dfile[MAXFILENAME];

	/*
	 * If file does not exist then
	 * try "." if file name has no "/"
	 * For file names that have a "/", try check
	 * for existence of previous directory
	 */
	if(access(file, A_EXIST) == A_OK)
		if(access(file, A_WRITE) == A_OK)
			return(1);
		else	return(0);
	else {
		if((sp=strrchr(file, '/')) == NULL)
			cat(dfile, ".", "");
		else {
			strncpy(dfile, file, sp - file);
			dfile[sp - file] = '\0';
		}
		if(access(dfile, A_WRITE) == CERROR) 
			return(0);
		return(2);
	}
}

/*
 * invok shell to execute command waiting for
 * command to terminate
 *	s	-> command string
 * return:
 *	status	-> command exit status
 */
system(s)
char *s;
{
	register int pid, w;
	int status;
	int (*istat)(), (*qstat)();

	/*
	 * Spawn the shell to execute command, however,
	 * since the mail command runs setgid mode
	 * reset the effective group id to the real
	 * group id so that the command does not
	 * acquire any special privileges
	 */
	if ((pid = fork()) == CHILD) {
		setuid(getuid());
		setgid(getgid());
		execl("/bin/sh", "sh", "-c", s, NULL);
		_exit(127);
	}

	/*
	 * Parent temporarily ignores signals so it 
	 * will remain around for command to finish
	 */
	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while ((w = wait(&status)) != pid && w != CERROR)
		;
	if (w == CERROR)
		status = CERROR;
	signal(SIGINT, istat);
	signal(SIGQUIT, qstat);
	return(status);
}

/*
*  strln - determine length of line (terminated by '\n')
*/

strln (s)
char *s;
{
	int i;

	for (i=0 ; i < LSIZE && s[i] != '\n' ; i++);
	return(i+1);
}
