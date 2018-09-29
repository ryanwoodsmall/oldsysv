/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
	rmnttry - attempt to perform queued mounts
*/

#ident	"@(#)rmount:rmnttry.c	1.1.3.1"
#include <sys/types.h>
#include <sys/stat.h>
#include <nserve.h>
#include <fcntl.h>
#include <signal.h>
#include <mnttab.h>
#include <stdio.h>

#define	TIMEOUT	180	/* time limit (seconds) for mount to complete */

char *cmd;

main(argc, argv)
int argc;
char **argv;
{
	char *fqn(), fdev[MAXDNAME], fres[MAXDNAME];	/* fully qualified names */
	struct mnttab *rstart, *last, *rp, *rd_rmnttab();
	struct mnttab *mntstart, *mntlast, *mp, *rd_mnttab();
	int fails = 0;		/* number of failed mounts */
	int chg = 0;		/* change flag for rmnttab */
	int i, j;		/* indices for command line names */
	char **ulist;		/* list of unused resources */

	cmd = argv[0];
	if (geteuid() !=  0) {
		fprintf(stderr, "%s: must be super-user\n", cmd);
		exit(2);
	}

	lock();		/* lock file for entire session */

	/* if RFS is not running, then delete the rmnttab table */
	if (rfs_up() != 0) {
		if (!wr_rmnttab((char *)0, (char *)0))
			fails = 2;	/* set error return */
		unlock();
		return fails;
	}

	if (!(rstart = rd_rmnttab(&last))) {
		unlock();
		return 2;
	}

	mntstart = rd_mnttab(&mntlast);		/* read /etc/mnttab */
	if (!mntstart)			/* keep quiet in case of error */
		mntlast = mntstart;

	/* scan through rmnttab, trying either each entry or selected ones */

	for (rp=rstart; rp < last; ++rp)
		if (argc == 1 || request(rp->mt_dev, argv)) {

			fqn(rp->mt_dev, fres);	/* generate a fully qualified name */

			/*  attempt to mount this resource but first see if it
			    or its mount point is already in /etc/mnttab  */

			for(mp = mntstart; mp < mntlast; ++mp)
				if (strcmp(fqn(mp->mt_dev, fdev), fres) == 0) {
					printf("%s: warning: %s already mounted on %s\n",
						cmd, rp->mt_dev, mp->mt_filsys);
					chg++;
					*(rp->mt_dev) = '\0';
					break;
				}
			    	else if (strncmp(mp->mt_filsys, rp->mt_filsys, 32) == 0) {
					printf("%s: warning: mount point %s used by %s\n",
						cmd, rp->mt_filsys, mp->mt_dev);
					chg++;
					*(rp->mt_dev) = '\0';
					break;
				}

			if (mp == mntlast)	/* it's not in mnttab */
				/* try the mount */
				if (!trymount(rp->mt_dev, rp->mt_filsys, rp->mt_ro_flg)) {
					/* success ... delete the entry */
					*(rp->mt_dev) = '\0';
					chg++;
				}
				else
					/* mark fail for proper return code */
					fails++;
		}
	
	/* check for unused command line resources */
	for (ulist = &argv[1], i=1, j=0; i < argc; ++i)
		if (argv[i] && *argv[i])
			ulist[j++] = argv[i];
	if (j) {
		fprintf(stderr, "%s: resources not queued:", cmd);
		for (i=0; i < j; ++i)
			fprintf(stderr, " %s", ulist[i]);
		fputc('\n', stderr);
	}

	/* if rmnttab was not altered, don't write it out */
	if (!chg) {
		unlock();
		return fails?1:0;
	}

	if (fails)
		fails = 1;	/* set proper return code */
	/* write rmnttab */

	if (!wr_rmnttab(rstart, last))
		fails = 2;	/* set error return */
	unlock();
	return fails;
}


/*
	request - if resource is a command line argument, return 1
		and set the that command line argument to an  empty string.
		otherwise, return 0
*/

request(resource, av)
char *resource, **av;
{
	while (*++av)
		if (strncmp(resource, *av, 32) == 0) {
			**av = '\0';
			return 1;
		}
	return 0;
}


/*
	rd_mnttab - read the mount table
		a block is allocated that is large enough to hold the table 
	return:
		pointer to the start of the mount table,
		NULL if error
		parameter last points to the entry past the last one
*/

#define MNTTAB	"/etc/mnttab"

struct mnttab *
rd_mnttab(last)
struct mnttab **last;
{
	char *malloc();
	int mfd, mtab_size;
	struct mnttab *mtab;
	struct stat stbuf;

	if ((mfd = open(MNTTAB, O_RDONLY)) < 0) {
		fprintf(stderr, "%s: cannot open %s\n", cmd, MNTTAB);
		return(NULL);
	}
	if (fstat(mfd, &stbuf) < 0) {
		fprintf(stderr, "%s: cannot stat %s\n", cmd, MNTTAB);
		return(NULL);
	}

	mtab_size = stbuf.st_size;

	if (!(mtab = (struct mnttab *)malloc(mtab_size))) {
		fprintf(stderr, "%s: cannot allocate space for %s\n",
			cmd, MNTTAB);
		return(NULL);
	}

	if (read(mfd, mtab, stbuf.st_size) != stbuf.st_size) {
		fprintf(stderr, "%s: read error on %s\n", cmd, MNTTAB);
		return(NULL);
	}

	*last = (struct mnttab *)((char *)mtab + stbuf.st_size);
	close(mfd);
	return(mtab);
}


/*
	trymount - perform an /etc/mount; return its exit status.

	TIMEOUT is the maximum time in seconds for /etc/mount to complete
*/

extern int fork(), execl(), wait();
static char *flg[] = { "", "-r", "-d", "-dr" };
int pid;		/* PID of forked /etc/mount. Kill it when the time's up */
/* pointers to arguments to mount to print useful error message */
char *pflags;		/* flags for mount */
char *pdev;		/* device for mount */
char *pfs;		/* file system for mount */

trymount(dev, fs, flag)
char *dev, *fs;
short flag;
{
	int status, w;
	void (*istat)(), (*qstat)(), (*astat)();
	void killpid();

	pflags = flg[flag&0x03];	/* convert flag to command line flag string */
	pdev = dev;
	pfs = fs;
	astat = signal(SIGALRM, killpid);
	if ((pid = fork()) == 0) {
		int fd;

		if ((fd = open("/dev/null", O_WRONLY)) < 0)
			fprintf(stderr, "%s: Can't open /dev/null\n", cmd);
		else {
			close(1);
			fcntl(fd, F_DUPFD, 1);		/* redirect stdout */
			close(2);
			fcntl(fd, F_DUPFD, 2);		/* redirect stderr */
			close(fd);
			/* perform the mount */
			(void) execl("/etc/mount", "mount", pflags, dev, fs, (char *)0);
		}
		_exit(127);
	}
	else {
		alarm(TIMEOUT);		/* set mount time limit */
		(void) signal(SIGALRM, killpid);
	}

	istat = signal(SIGINT, SIG_IGN);
	qstat = signal(SIGQUIT, SIG_IGN);
	while((w = wait(&status)) != pid && w != -1)
		;
	alarm(0);
	(void) signal(SIGALRM, astat);
	(void) signal(SIGINT, istat);
	(void) signal(SIGQUIT, qstat);
	return((w == -1)? w: status);
}


/*
	killpid - kill the /etc/mount command when it's been running too long
*/

void
killpid()
{
	kill(pid, SIGKILL);
	fprintf(stderr, "%s: \"/etc/mount %s %s %s\" timed out.\n",
		cmd, pflags, pdev, pfs);
}
