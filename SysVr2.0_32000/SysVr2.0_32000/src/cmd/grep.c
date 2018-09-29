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
static char sccsid[] = "@(#)grep.c	1.8";
/*
 * grep -- print lines matching (or not matching) a pattern
 *
 *	status returns:
 *		0 - ok, and some matches
 *		1 - ok, but no matches
 *		2 - some error
 */

#include <stdio.h>
#include <ctype.h>

/*
 * define some macros for rexexp.h
 */

#define INIT	register char *sp = instring;	/* 1st arg pts to RE string */
#define GETC()		(*sp++)
#define PEEKC()		(*sp)
#define UNGETC(c)	(--sp)
#define RETURN(c)	return;
#define ERROR(c)	regerr(c)

#include <regexp.h>

char errstr[][64] = {
	"Range endpoint too large." ,
	"Bad number." ,
	"``\\digit'' out of range." ,
	"Illegal or missing delimiter." ,
	"No remembered search string." ,
	"\\( \\) imbalance." ,
	"Too many \\(." ,
	"More than 2 numbers given in \\{ \\}." ,
	"} expected after \\." ,
	"First number exceeds second in \\{ \\}." ,
	"[ ] imbalance." ,
	"Regular expression overflow." ,
	"Unknown regexp error code!!" ,
	NULL
};

#define	errmsg(msg, arg)	fprintf(stderr, msg, arg)
#define ESIZE	256
#define	BLKSIZE	512

char	*strrchr();
FILE	*temp;
long	lnum;
char	linebuf[BUFSIZ];
char	prntbuf[BUFSIZ];
char	expbuf[ESIZE];
int	nflag;
int	bflag;
int	lflag;
int	cflag;
int	vflag;
int	sflag;
int	iflag;
int	errflg;
int	nfile;
long	tln;
int	nsucc;
int	nchars;
int	nlflag;

main(argc, argv)
register argc;
char **argv;
{
	register	c;
	register char	*arg;
	extern int	optind;

	while((c=getopt(argc, argv, "blcnsvi")) != EOF)
		switch(c) {
		case 'v':
			vflag++;
			break;
		case 'c':
			cflag++;
			break;
		case 'n':
			nflag++;
			break;
		case 'b':
			bflag++;
			break;
		case 's':
			sflag++;
			break;
		case 'l':
			lflag++;
			break;
		case 'i':
			iflag++;
			break;
		case '?':
			errflg++;
		}

	if(errflg || (optind >= argc)) {
		errmsg("Usage: grep -blcnsvi pattern file . . .\n",
			(char *)NULL);
		exit(2);
	}

	argv = &argv[optind];
	argc -= optind;
	nfile = argc - 1;

	if (strrchr(*argv,'\n'))
		ERROR(41);

	if (iflag) {
		for(arg = *argv; *arg != NULL; ++arg)
			*arg = (char)tolower((int)(*arg));
	}

	compile(*argv, expbuf, &expbuf[ESIZE], '\0');

	if (--argc == 0)
		execute(NULL);
	else
		while (argc-- > 0)
			execute(*++argv);

	exit(nsucc == 2 ? 2 : nsucc == 0);
}

execute(file)
register char *file;
{
	register char *lbuf;
	register i;

	if (file == NULL)
		temp = stdin;
	else if ( (temp = fopen(file, "r")) == NULL) {
		if (!sflag)
			errmsg("grep: can't open %s\n", file);
		nsucc = 2;
		return;
	}

	lnum = 0;
	tln = 0;

	while((nchars = fgetl(prntbuf, BUFSIZ, temp)) != 0) {
		if(nchars == BUFSIZ - 1  &&  prntbuf[nchars-1] != '\n')
			continue;

		if(prntbuf[nchars-1] == '\n') {
			nlflag = 1;
			prntbuf[nchars-1] = '\0';
		} else
			nlflag = 0;

		lnum++;

		if (iflag) {
			for(i=0, lbuf=linebuf; i < nchars; i++, lbuf++)
				*lbuf = (char)tolower((int)prntbuf[i]);
			*lbuf = '\0';
			lbuf = linebuf;
		} else
			lbuf = prntbuf;

		if((step(lbuf, expbuf) ^ vflag) && succeed(file) == 1)
			break;	/* lflag only once */
	}
	fclose(temp);

	if (cflag) {
		if (nfile>1)
			fprintf(stdout, "%s:", file);
		fprintf(stdout, "%ld\n", tln);
	}
	return;
}

succeed(f)
register char *f;
{
	nsucc = (nsucc == 2) ? 2 : 1;
	if (cflag) {
		tln++;
		return(0);
	}
	if (lflag) {
		fprintf(stdout, "%s\n", f);
		return(1);
	}

	if (nfile > 1)	/* print filename */
		fprintf(stdout, "%s:", f);

	if (bflag)	/* print block number */
		fprintf(stdout, "%ld:", (ftell(temp)-1)/BLKSIZE);

	if (nflag)	/* print line number */
		fprintf(stdout, "%ld:", lnum);

	if (nlflag)
		prntbuf[nchars-1] = '\n';

	fwrite(prntbuf, 1, nchars, stdout);
	return(0);
}

regerr(err)
register err;
{
	errmsg("grep: RE error %d: ", err);
	switch(err) {
		case 11:
			err = 0;
			break;
		case 16:
			err = 1;
			break;
		case 25:
			err = 2;
			break;
		case 36:
			err = 3;
			break;
		case 41:
			err = 4;
			break;
		case 42:
			err = 5;
			break;
		case 43:
			err = 6;
			break;
		case 44:
			err = 7;
			break;
		case 45:
			err = 8;
			break;
		case 46:
			err = 9;
			break;
		case 49:
			err = 10;
			break;
		case 50:
			err = 11;
			break;
		default:
			err = 12;
			break;
	}

	errmsg("%s\n", errstr[err]);
	exit(2);
}

/*
 * The following code is a modified version of the fgets() stdio
 * routine.  The reason why it is used instead of fgets() is that
 * we need to know how many characters we read into the buffer.
 * Thus that value is returned here instead of the value of s1.
 */
#define MIN(x, y)	(x < y ? x : y)
#define _BUFSYNC(iop)	if (_bufend(iop) - iop->_ptr <   \
				( iop->_cnt < 0 ? 0 : iop->_cnt ) )  \
					_bufsync(iop)

extern int _filbuf();
extern char *memccpy();

fgetl(ptr, size, iop)
char *ptr;
register int size;
register FILE *iop;
{
	char *p, *ptr0 = ptr;
	register int n;

	for (size--; size > 0; size -= n) {
		if (iop->_cnt <= 0) { /* empty buffer */
			if (_filbuf(iop) == EOF) {
				if (ptr0 == ptr)
					return (NULL);
				break; /* no more data */
			}
			iop->_ptr--;
			iop->_cnt++;
		}
		n = MIN(size, iop->_cnt);
		if ((p = memccpy(ptr, (char *) iop->_ptr, '\n', n)) != NULL)
			n = p - ptr;
		ptr += n;
		iop->_cnt -= n;
		iop->_ptr += n;
		_BUFSYNC(iop);
		if (p != NULL)
			break; /* found '\n' in buffer */
	}
	*ptr = '\0';
	return (ptr-ptr0);
}
