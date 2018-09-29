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
static char	sccsid[] = "@(#)cut.c	1.4";
#
/* cut : cut and paste columns of a table (projection of a relation) */
/* Release 1.5; handles single backspaces as produced by nroff    */
# include <stdio.h>	/* make: cc cut.c */
# define NFIELDS 1024	/* max no of fields or resulting line length */
# define USAGE "Usage: cut [-s] [-d<char>] {-c<list> | -f<list>} file ..."
# define CFLIST "bad list for c/f option"
# define BACKSPACE 8


main(argc, argv)
int	argc;
char	**argv;
{
	int	del = '\t';
	char *d;
	int num, j, count, poscnt, r, s, t;
	int	endflag, supflag, cflag, fflag, backflag, filenr;
	int	sel[NFIELDS];
	char 	buf[BUFSIZ];
	register int	c;
	register char	*p1, *rbuf;
	char	*p2, outbuf[NFIELDS];
	extern char *optarg;
	extern int optind;
	register char *p, *list;
	FILE *inptr;
	endflag = supflag = cflag = fflag = 0;


	while((c = getopt(argc, argv, "c:d:f:s")) != EOF)
		switch(c) {
			case 'c':
				if (fflag)
					diag(CFLIST);
				cflag++;
				list = optarg;
				break;
			case 'd':
				if (strlen(optarg) > 1)
					diag("no delimiter");
				else
					del = (int)*optarg;
				break;
			case 'f':
				if (cflag)
					diag(CFLIST);
				fflag++;
				list = optarg;
				break;
			case 's':
				supflag++;
				break;
			case '?':
				diag(USAGE);
		}

	argv = &argv[optind];
	argc -= optind;

	if (!(cflag || fflag))
		diag(CFLIST);

	do {
		p = list;
		switch(*p) {
			case '-':
				if (r)
					diag(CFLIST);
				r = 1;
				if (num == 0)
					s = 1;
				else {
					s = num;
					num = 0;
					break;
				}
			case '\0' :
			case ','  :
				if (num >= NFIELDS)
					diag(CFLIST);
				if (r) {
					if (num == 0)
						num = NFIELDS - 1;
					if (num < s)
						diag(CFLIST);
					for (j = s; j <= num; j++)
						sel[j] = 1;
				} else
					sel[num] = (num > 0 ? 1 : 0);
				s = num = r = 0;
				if (*p == '\0')
					continue;
				else
					break;
			default:
				if (*p < '0' || *p > '9')
					diag(CFLIST);
				num = 10 * num + *p - '0';
				break;
		}
		list++;
	}while (*p != '\0');
	for (j=t=0; j < NFIELDS; j++)
		t += sel[j];
	if (t == 0)
		diag("no fields");

	filenr = 0;
	do {	/* for all input files */
		if (argc == 0)
			inptr = stdin;
		else
			if ((inptr = fopen(argv[filenr], "r")) == NULL) {
				fprintf(stderr, "cut: cannot open %s\n", argv[filenr]);
				exit(2);
			}
		endflag = 0;
		do {	/* for all lines of a file */
			count = poscnt = backflag = 0;
			p1 = &outbuf[0] - 1 ;
			p2 = p1;
			rbuf = buf;
			if ((fgets(buf, BUFSIZ, inptr)) == NULL) {
				endflag = 1;
				continue;
			}
			do { 	/* for all char of the line */
				if (count == NFIELDS - 1)
					diag("line too long");
				if (*rbuf != '\n')
					*++p1 = *rbuf;
				if (cflag && (c == BACKSPACE))
					backflag++;
				else if (!backflag)
					poscnt += 1;
				else
					backflag--;
				if ( backflag > 1 )
					diag("cannot handle multiple adjacent backspaces\n");
				if (*rbuf == '\n' && count > 0  || *rbuf == del || cflag) {
					count += 1;
					if (fflag)
						poscnt = count;
					if (sel[poscnt])
						p2 = p1;
					else
						p1 = p2;
				}
			} while (*rbuf++ != '\n');
			if ( !endflag && (count > 0 || !supflag)) {
				if (*p1 == del && !sel[count])
					*p1 = '\0'; /*suppress trailing delimiter*/
				else
					*++p1 = '\0';
				puts(outbuf);
			}
		} while (!endflag);
		fclose(inptr);
	} while (++filenr < argc);
}


diag(s)
char	*s;
{
	fprintf(stderr, "cut: %s\n", s);
	exit(2);
}


diagl()
{
	diag("bad list for c/f option\n");
}


