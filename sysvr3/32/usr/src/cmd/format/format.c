/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)format:format.c	1.1"

#include "sys/extbus.h"
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>

#define	ADDON_PREFIX	"/usr/lib"
#define	BUFSIZE		512
#define	CMDNAME		"format"
#define	ERREXIT		1
#define	USAGE		2

char		*Cmdname;
char		Command[BUFSIZE];
char		*Devicefile;
extern char	*sys_errlist[];
extern int	errno;
void		error(),
		usage();

main(argc, argv)
int 	argc;
char 	*argv[];
{
	extern char	*optarg;
	int		c;
	extern int	optind;

	if (argc < 1)
		Cmdname = CMDNAME;
	else
		Cmdname = argv[0];

	/* Parse command line arguments */
	while ((c = getopt(argc, argv, "Sinr:v")) != EOF)
		switch (c) {
		case 'S' :
		case 'i' :
		case 'n' :
		case 'r' :
		case 'v' :
			break;
		case '?' :
			usage();
			break;
		}

	/* Check for the correct number of arguments */
	if (argc - optind != 1)
		/* Not enough or too many arguments */
		usage();

	/* The last argument must be the device file */
	Devicefile = argv[optind];

	if (gen_cmdname()) {
		argv[0] = Command;

		/* Exec command */
		execv(argv[0], argv);
		error("%s exec failed\n", Command);
	} else
		error("not supported for device %s\n", Devicefile);
}

gen_cmdname()
{
	register int	fd;
	struct bus_type	bus_info;

	if ((fd = open(Devicefile, O_RDONLY)) < 0)
		error("%s open failed\n", Devicefile);

	if (ioctl(fd, B_GETTYPE, &bus_info) < 0) {
		errno = 0;
		return(0);
	} else
		sprintf(Command, "%s/%s/%s", ADDON_PREFIX, bus_info.bus_name, CMDNAME);

	close(fd);
	return(1);
}

void
usage()
{
	fprintf(stderr, "Usage: %s [ -n ] [ -v ] device\n", Cmdname);
	exit(USAGE);
}

void
error(message, data)
char	*message;	/* Message to be reported */
long	data;
{
	(void) fprintf(stderr, "%s: ", Cmdname);
	(void) fprintf(stderr, message, data);
	if (errno)
		(void) fprintf(stderr, "%s: %s\n", Cmdname, sys_errlist[errno]);
	exit(ERREXIT);
}	/* error() */
