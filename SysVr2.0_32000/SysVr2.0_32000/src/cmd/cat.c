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
/*
**	Concatenate files.
*/

static char catvers[] = "@(#)cat.c	1.6";

#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>

char	buffer[BUFSIZ];

int	silent = 0;
int	vflag = 0;
int	tflag = 0;
int	eflag = 0;

main(argc, argv)
int    argc;
char **argv;
{
	register FILE *fi;
	register int c;
	extern	int optind;
	int	errflg = 0;
	int	stdinflg = 0;
	int	status = 0;
	int	dev, ino = -1;
	struct	stat	statb;

#ifdef STANDALONE
	if (argv[0][0] == '\0')
		argc = getargv ("cat", &argv, 0);
#endif
	while( (c=getopt(argc,argv,"usvte")) != EOF ) {
		switch(c) {
		case 'u':
#ifndef	STANDALONE
			setbuf(stdout, (char *)NULL);
#endif
			continue;
		case 's':
			silent++;
			continue;
		case 'v':
			vflag++;
			continue;
		case 't':
			tflag++;
			continue;
		case 'e':
			eflag++;
			continue;
		case '?':
			errflg++;
			break;
		}
		break;
	}

	if (errflg) {
		if (!silent)
			fprintf(stderr,"usage: cat -usvte [-|file] ...\n");
		exit(2);
	}
	if(fstat(fileno(stdout), &statb) < 0) {
		if(!silent)
			fprintf(stderr, "cat: Cannot stat stdout\n");
		exit(2);
	}
	statb.st_mode &= S_IFMT;
	if (statb.st_mode!=S_IFCHR && statb.st_mode!=S_IFBLK) {
		dev = statb.st_dev;
		ino = statb.st_ino;
	}
	if (optind == argc) {
		argc++;
		stdinflg++;
	}
	for (argv = &argv[optind];
	     optind < argc && !ferror(stdout); optind++, argv++) {
		if ((*argv)[0]=='-' && (*argv)[1]=='\0' || stdinflg)
			fi = stdin;
		else {
			if ((fi = fopen(*argv, "r")) == NULL) {
				if (!silent)
				   fprintf(stderr, "cat: cannot open %s\n",
								*argv);
				status = 2;
				continue;
			}
		}
		if(fstat(fileno(fi), &statb) < 0) {
			if(!silent)
			   fprintf(stderr, "cat: cannot stat %s\n", *argv);
			status = 2;
			continue;
		}
		if (statb.st_dev==dev && statb.st_ino==ino) {
			if(!silent)
			   fprintf(stderr, "cat: input %s is output\n",
						stdinflg?"-": *argv);
			if (fclose(fi) != 0 ) 
				fprintf(stderr, "cat: close error\n");
			status = 2;
			continue;
		}

		if (vflag)
			status = vcat(fi);
		else
			status = cat(fi);

		if (fi!=stdin)
			fflush(stdout);
		if (fclose(fi) != 0) 
			if (!silent)
				fprintf(stderr, "cat: close error\n");
	}
	fflush(stdout);
	if (ferror(stdout)) {
		if (!silent)
			fprintf (stderr, "cat: output error\n");
		status = 2;
	}
	exit(status);
}

int
cat(fi)
	FILE *fi;
{
	register int fi_desc;
	register int nitems;

	fi_desc = fileno(fi);

	for (;;) {
		nitems=read(fi_desc,buffer,BUFSIZ);
		if (nitems <= 0) break;
		if (write(1,buffer,(unsigned)nitems) != nitems) {
			if (!silent)
				fprintf(stderr, "cat: output error\n");
			return(2);
		}
	}

	return(0);
}


/* character type table */

#define PLAIN	0
#define CONTRL	1
#define TAB	2
#define BACKSP	3
#define NEWLIN	4

char ctype[128] = {
	CONTRL,	CONTRL,	CONTRL,	CONTRL,	CONTRL,	CONTRL,	CONTRL,	CONTRL,
	CONTRL, TAB,	NEWLIN,	CONTRL,	TAB,	CONTRL,	CONTRL,	CONTRL,
	CONTRL,	CONTRL,	CONTRL,	CONTRL,	CONTRL,	CONTRL,	CONTRL,	CONTRL,
	CONTRL,	CONTRL,	CONTRL,	CONTRL,	CONTRL,	CONTRL,	CONTRL,	CONTRL,
	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,
	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,
	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,
	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,
	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,
	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,
	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,
	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,
	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,
	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,
	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,
	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	PLAIN,	CONTRL,
};

int
vcat(fi)
	FILE *fi;
{
	register int c;

	while ((c = getc(fi)) != EOF) {
		if (c & 0200) {
			putchar('M');
			putchar('-');
			c-= 0200;
		}

		switch(ctype[c]) {
			case PLAIN:
				putchar(c);
				break;
			case TAB:
				if (!tflag)
					putchar(c);
				else {
					putchar('^');
					putchar (c^0100);
				}
				break;
			case CONTRL:
				putchar('^');
				putchar(c^0100);
				break;
			case NEWLIN:
				if (eflag)
					putchar('$');
				putchar(c);
				break;
			}
		}
	return(0);
}
