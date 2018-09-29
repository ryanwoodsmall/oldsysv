/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)uucp:nttysrv.c	1.1"
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/times.h>
#include <time.h>
#include <sys/stat.h>
#include <string.h>
#include <signal.h>
#include <sys/tiuser.h>
#include <sys/termio.h>
#include <sys/stropts.h>
#include <ctype.h>
#include <utmp.h>
#include <memory.h>

extern int errno;

int fd, fd0, fd1, fd2;
struct termio term;
int pid;			/* process id for child */
char dev[40];
char *devname, *timep;
struct utmp utmp;
struct utmp ut;
struct utmp *u = &utmp;
unsigned int status;
struct stat statd, statlog;
time_t cur_time;

FILE *Logfd;

main(argc, argv)
int argc;
char *argv[];
{
	extern void hangup();
	extern char *getenv();


	signal(SIGQUIT, SIG_IGN);
	signal(SIGHUP, hangup);
	signal(SIGTERM, SIG_IGN);
	signal(SIGINT, SIG_IGN);


	if ( stat("/usr/tmp/nttysrv.log", &statlog) >= 0 ) {
		if ( statlog.st_size >= 10*1024 ) {
			link("/usr/tmp/nttysrv.log","/usr/tmp/Onttysrv.log");
			unlink("/usr/tmp/nttysrv.log");
		}
	}

	Logfd = fopen("/usr/tmp/nttysrv.log", "a");
	setbuf(Logfd, NULL);

	if ( ( cur_time = time((long *)0) ) < 0 ) {
			fprintf( Logfd, "error: time system call failed\n");
			exit(1);
	} 

	fprintf( Logfd, "nttysrv: server started at %s", ctime(&cur_time));

	setpgrp();
	if (argc == 1) {
		devname = getenv("NLSPROVIDER");
		if ( devname == NULL ) {
			fprintf( Logfd, "error: environment variable NLSPROVIDER not set\n");
			exit(1);
		}
	} else if ( argc >= 2 ) 
		devname = argv[1];
	else if ( argc > 2 ) {
		fprintf( Logfd, "warning: more than one argument to nttysrv\n");
	}

	fd = 0;


/* find out what device to open for file descriptors 1 and 2 */

	if (tsgetdev() < 0) {
		exit(1);
	}

	close(1);
	close(2);

	if ((fd1 = open(dev, O_WRONLY)) != 1) {
		fprintf(Logfd, "error: open of %s for stdout failed, errno = %d\n", dev, errno);
		exit(1);
	}

	if ((fd2 = open(dev, O_WRONLY)) != 2) {
		fprintf(Logfd, "error: open of %s for stderr failed, errno = %d\n", dev, errno);
		exit(1);
	}

	fdopen(fd1, "w");
	fdopen(fd2, "w");

	setvbuf(stdout, NULL, _IOLBF, 0);
	setvbuf(stderr, NULL, _IONBF, 0);

	close(0);

	if ((fd0 = open(dev, O_RDONLY)) != 0) {
		fprintf(Logfd, "error: open of %s for stdin failed, errno = %d\n", dev, errno);
		exit(1);
	}

	fdopen(fd0, "r");

	if ( tsexec() < 0 ) 
		exit(1);
	else
		exit(0);
}

tsgetdev()
{
	int nodename;
	extern char *getenv();

	if ( fstat(0, &statd) < 0) {

		fprintf(Logfd, "error: fstat failed, errno = %d\n", errno);

		exit(-1);
	}
	nodename = statd.st_rdev & 0xff;

	sprintf(dev, "/dev/%s%02d", devname, nodename);

	fprintf(Logfd, "nttysrv: device = %s\n", dev);

	/* Take status on the device so that a chown and chmod can be
	   done */

	if ( stat(dev, &statd) < 0) {

		fprintf(Logfd, "error: stat on device failed, errno = %d\n", errno);
		return(-1);
	}
	return(0);
}

int
tsexec()
{
	struct tms ctime;
	struct tms curtime;
	char *chrptr;
	extern void hangup();

	if ( (pid = fork()) < 0) {
		fprintf(Logfd, "error: fork failed, errno = %d\n", errno);
		return(-1);
	}

	if (pid != 0) {	/* parent sets up utmp entry */
		if (utmp_setup() < 0) {
			kill(0, SIGTERM);
			exit(1);
		}
	}

	/* child process will execute the command */

	if(pid == 0) {
		signal(SIGINT, SIG_DFL);
		signal(SIGHUP, SIG_DFL);
		signal(SIGQUIT, SIG_DFL);
		signal(SIGTERM, SIG_DFL);

		chrptr = strrchr(dev, '/');
		chrptr++;
		if (execl("/etc/getty", "/etc/getty", "-t","90", chrptr, "9600", 0) < 0) {

			fprintf(Logfd, "error: exec of getty failed, errno = %d\n", errno);
			return(-1);
		}
		return(0);
	} else {
		times(&curtime);
		wait(&status);
		times(&ctime);

		if (clean_entries() != 0) {
			return(-1);
		}
	}
	return(0);
}

void
hangup()
{
	if (clean_entries() == 0 )
		exit(0);
	else
		exit(1);
}

utmp_setup()
{
	char *chrptr;
	int i, j;
	extern struct utmp *getutid();

	chrptr = strrchr(dev, '/');
	chrptr++;
	memset(u->ut_user, NULL, sizeof(u->ut_user));
	ut.ut_id[0] = u->ut_id[0] = 'a';
	ut.ut_id[1] = u->ut_id[1] = 'a';
	ut.ut_id[2] = u->ut_id[2] = dev[strlen(dev) - 1];
	ut.ut_id[3] = u->ut_id[3] = dev[strlen(dev) - 2];
	(void) strncpy(u->ut_line, chrptr, sizeof(u->ut_line));
	ut.ut_type = u->ut_type = INIT_PROCESS;
	u->ut_exit.e_termination = 0;
	u->ut_exit.e_exit = 0;
	u->ut_pid = pid;
	(void) time(&u->ut_time);
	for (i = 1; i < 26; i++) {
		for (j = 1; j < 26; j++) {
			if (getutid(&ut) == (struct utmp *)NULL)
				goto done;
			ut.ut_id[1]++;
			u->ut_id[1]++;
		}
		ut.ut_id[0]++;
		u->ut_id[0]++;
	}
done:
	if ((i >= 26) && (j >= 26)) {
		fprintf(Logfd, "error: couldn't make utmp entry\n");
		return(-1);
	}
	pututline(u);
	endutent();
	return(0);
}


clean_entries()
{

	memset(u->ut_user, 0, sizeof(u->ut_user));
	u->ut_type = DEAD_PROCESS;
	u->ut_exit.e_termination = status & 0xff;
	u->ut_exit.e_exit = ((status >> 8) & 0xff);
	time(&u->ut_time);
	if (getutid(&ut) == (struct utmp *)NULL) {
		fprintf(Logfd, "error: utmp entry not there to delete\n");
		return(-1);
	}
	pututline(u);
	endutent();
	return(0);
}

