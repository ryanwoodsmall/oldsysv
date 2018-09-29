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
/*	@(#)tee.c	1.3	*/
/*
 * tee-- pipe fitting
 */

#include <signal.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <fcntl.h>
int openf[20] = { 1 };
int n = 1;
int t = 0;
int errflg;
int aflag;

char in[BUFSIZ];

char out[BUFSIZ];

extern errno;
long	lseek();

main(argc,argv)
char **argv;
{
	int register w;
	extern int optind;
	int c;
	struct stat buf;
	while ((c = getopt(argc, argv, "ai")) != EOF)
		switch(c) {
			case 'a':
				aflag++;
				break;
			case 'i':
				signal(SIGINT, SIG_IGN);
				break;
			case '?':
				errflg++;
		}
	if (errflg) {
		fprintf(stderr, "usage: tee [ -i ] [ -a ] [file ] ...\n");
		exit(2);
	}
	argc -= optind;
	argv = &argv[optind];
	fstat(1,&buf);
	t = (buf.st_mode&S_IFMT)==S_IFCHR;
	if(lseek(1,0L,1)==-1&&errno==ESPIPE)
		t++;
	while(argc-->0) {
		openf[n++] = open(argv[0],O_WRONLY|O_CREAT|
			(aflag?O_APPEND:O_TRUNC), 0666);
		if(stat(argv[0],&buf)>=0) {
			if((buf.st_mode&S_IFMT)==S_IFCHR)
				t++;
		} else {
			fprintf(stderr, "tee: cannot open %s\n", argv[0]);
			n--;
		}
		argv++;
	}
	w = 0;
	for(;;) {
		w = read(0, in, BUFSIZ);
		if (w > 0)
			stash(w);
		else
			break;
	}
}

stash(p)
{
	int k;
	int i;
	int d;
	d = t ? 16 : p;
	for(i=0; i<p; i+=d)
		for(k=0;k<n;k++)
			write(openf[k], in+i, d<p-i?d:p-i);
}
