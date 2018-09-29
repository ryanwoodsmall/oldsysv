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
/* @(#)prfsnap.c	1.2 */
/*
 *	prfsnap - dump profile data to a log file
 */

# define PRF_ON   1
# define PRF_VAL  2
# define PRFMAX 1024

int	buf[PRFMAX * 2 + 1];
int	prfmax;

main(argc, argv)
	int	argc;
	char	**argv;
{
	register  int  prf, log;
	int	tvec;

	if(argc != 2)
		error("usage: prfsnap  logfile");
	if((prf = open("/dev/prf", 0)) < 0)
		error("cannot open /dev/prf");
	if((log = open(argv[1], 1)) < 0)
		if((log = creat(argv[1], 0666)) < 0)
			error("cannot creat log file");

	lseek(log, 0, 2);
	if(ioctl(prf, 3, PRF_ON))
		error("cannot activate profiling");
	prfmax = ioctl(prf, 2, 0);
	time(&tvec);
	read(prf, buf, (prfmax * 2 + 1) * sizeof (int));
	if(lseek(log, 0, 1) == 0) {
		write(log, &prfmax, sizeof prfmax);
		write(log, buf, prfmax * sizeof (int));
	}
	write(log, &tvec, sizeof tvec);
	write(log, &buf[prfmax], (prfmax + 1) * sizeof (int));
}

error(s)
	char	*s;
{
	write(2, "prfsnap: ", 9);
	write(2, s, strlen(s));
	write(2, "\n", 1);
	exit(1);
}
