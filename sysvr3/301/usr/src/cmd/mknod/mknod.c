/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mknod:mknod.c	1.4"
#include <stdio.h>
#ifdef RT
#include <rt/types.h>
#include <rt/stat.h>
#else
#include <sys/types.h>
#include <sys/stat.h>
#endif

#define MAXMAJOR 127

main(argc, argv)
int argc;
char **argv;
{
	register  int  m, a, b;

	if(argc == 3 && !strcmp(argv[2], "p")) { /* fifo */
		a = mknod(argv[1], S_IFIFO|0666, 0);
		chown(argv[1], getuid(), getgid());
		if(a)
			perror("mknod");
		exit(a == 0? 0: 2);
	}
	if(getuid()) {
		fprintf(stderr, "mknod: must be super-user\n");
		exit(2);
	}
	if(argc != 5) {
		fprintf(stderr,"mknod: arg count\n");
		usage();
	}
	if(*argv[2] == 'b')
		m = S_IFBLK|0666;
	else if(*argv[2] == 'c')
		m = S_IFCHR|0666;
#ifdef RT
	else if(*argv[2] == 'r')
		m = S_IFREC|0666;
#endif
	else
		usage();
	a = number(argv[3]);
	if(a < 0)
		usage();
	if(a > MAXMAJOR)
		major_too_big();
	b = number(argv[4]);
	if(b < 0)
		usage();
	if(mknod(argv[1], m, ((a&0xFF)<<8)|(b&0xFF)) < 0)
		perror("mknod");
	exit(0);
}

number(s)
register  char  *s;
{
	register  int  n, c;

	n = 0;
	if(*s == '0') {
		while(c = *s++) {
			if(c < '0' || c > '7')
				return(-1);
			n = n * 8 + c - '0';
		}
	} else {
		while(c = *s++) {
			if(c < '0' || c > '9')
				return(-1);
			n = n * 10 + c - '0';
		}
	}
	return(n);
}
usage()
{
#ifdef RT
	fprintf(stderr,"usage: mknod name b/c/r major minor\n");
#else
	fprintf(stderr,"usage: mknod name b/c major minor\n");
#endif
	exit(2);
}
major_too_big()
{
	fprintf(stderr,"mknod: major number must be less than or equal to %d\n",MAXMAJOR);
	exit(2);
}
