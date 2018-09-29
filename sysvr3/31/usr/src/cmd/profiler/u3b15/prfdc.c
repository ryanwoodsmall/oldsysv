/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)profil-3b15:prfdc.c	1.4.1.1"
/*
 *	prfdc - profiler data collector
 */

# include "time.h"
# include "signal.h"
# include "errno.h"
# include "stdio.h"
# include "sys/types.h"
# include "sys/stat.h"
# include "fcntl.h"

# define PRF_ON    1
# define PRF_VAL   2

int	*buf;			/* Symbols, Kernel ctrs, and User ctr */
int	prfmax;			/* number of text addresses */

sigalrm()
{
	signal(SIGALRM, sigalrm);
}

main(argc, argv)
	char	**argv;
{
	register  int  prf, log;
	register  int  rate = 10, first = 1, toff = 17;
	int	tvec;
	int	*getspace();
	struct	tm	*localtime();


	switch(argc) {
		default:
			error("usage: prfdc  logfile  [ rate  [ off_hour ] ]");
		case 4:
			toff = atoi(argv[3]);
		case 3:
			rate = atoi(argv[2]);
		case 2:
			;
	}
	if(rate <= 0)
		error("invalid sampling rate");
	if((prf = open("/dev/prf", 0)) < 0)
		error("cannot open /dev/prf");
	if(open(argv[1], 0) >= 0)
		error("existing file would be truncated");
	if((log = creat(argv[1], 0666)) < 0)
		error("cannot creat log file");

	if(ioctl(prf, 3, PRF_ON))
		error("cannot activate profiling");
	if(fork())
		exit(0);
	setpgrp();
	sigalrm();
	buf = getspace();

	prfmax = ioctl(prf, 2, 0);
	write(log, &prfmax, sizeof prfmax);

	for(;;) {
		alarm(60 * rate);
		time(&tvec);
		read(prf, buf, (prfmax * 2 + 1) * sizeof (int));
		if(first) {
			write(log, buf, prfmax * sizeof (int));
			first = 0;
		}
		write(log, &tvec, sizeof tvec);
		write(log, &buf[prfmax], (prfmax + 1) * sizeof (int));
		if(localtime(&tvec)->tm_hour == toff)
			exit(0);
		pause();
	}
}

error(s)
	char	*s;
{
	write(2, "prfdc: ", 6);
	write(2, s, strlen(s));
	write(2, "\n", 1);
	exit(1);
}

 int *
getspace()
	{
	void perror();
	char *malloc();
	int *space;
	int f, maxprf;
	char *flnm = "/tmp/prf.adrfl";
	struct stat ubuf,syb;

	stat ("/unix",&syb);
	if ((stat(flnm,&ubuf) == -1) ||
		 (ubuf.st_mtime <= syb.st_ctime)) {
		perror("Bad address file, execute  prfld");
		exit(2);
	}
	if((f = open(flnm, O_RDONLY)) == -1) {
		perror("Cannot open /tmp/prf.adrfl");
		exit(errno);
	}
	if (read (f,&maxprf, sizeof (int)) == -1) {
		perror("Cannot read /tmp/prf.adrfl");
		exit(errno);
	}
	close(f);
	if ((space = (int *)malloc((maxprf * 2 + 1) * sizeof (int))) == NULL) {
		perror("Cannot malloc space for symbol table");
		exit(2);
	}
	return(space);
	}
