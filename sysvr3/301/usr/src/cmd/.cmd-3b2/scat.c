/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)cmd-3b2:scat.c	1.2"
/*
 * Simplified version of cat. Avoids stdio to help in
 * the first-3B2-restore-floppy space crunch.
 *
 */
#include <fcntl.h>


#define BUFSIZE  512
#define	STDIN	0
#define STDOUT  1
#define STDERR  2

void	filecopy();

main(argc,argv)
int	argc ;
char	**argv ;
{
	int	fd ;

	if (argc == 1) { /* no arguments; copy standard input */
		fd = STDIN ;
		filecopy(fd, argv) ;
	} else
		while (--argc > 0)
			if ( (fd = open(*++argv, O_RDONLY)) < 0) {
				fatal("scat: can't open", *argv) ;
				exit(1) ;
			} else {
				filecopy(fd, argv) ;
				close(fd) ;
			}
	exit(0);
}


static void
filecopy(fds, argv)
int	fds ;
char	**argv ;
{
	char buf[BUFSIZE] ;
	int n ;
	for (;;)   {
	n = read(fds, buf, BUFSIZE);
	if (n <= 0)	break;
	if (write(STDOUT, buf, n) != n) 
		fatal("scat: output error", *argv);
	}
}

fatal(why, what)
char	*why;
char	*what;
{
	static char	between[] = "   ";
	static char	after[] = "\n";

	write(STDERR, why, (unsigned int) strlen(why));
	write(STDERR, between, (unsigned int) strlen(between));
	write(STDERR, what, (unsigned int) strlen(what));
	write(STDERR, after, (unsigned int) strlen(after));
	exit(1);
}
