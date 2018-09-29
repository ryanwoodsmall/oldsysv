/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/setupterm.c	1.29"
#ifndef NOBLIT
#include <sys/jioctl.h>
#endif
#include "curses.ext"
#include "uparm.h"
#ifndef termpath
#define termpath(file) "/usr/lib/terminfo/file"
#endif

char *getenv();
char *malloc();
char ttytype[128];
char _frst_tblstr[2048];

TERMINAL		_first_term;
TERMINAL *		cur_term = &_first_term;
struct _bool_struct	_frst_bools;
struct _bool_struct *	cur_bools = &_frst_bools;
struct _num_struct	_frst_nums;
struct _num_struct *	cur_nums = &_frst_nums;
struct _str_struct	_frst_strs;
struct _str_struct *	cur_strs = &_frst_strs;

int _called_before = 0;	/* To check for first time. Used/cleared by restart.c */

#ifdef DUMPTI
extern char
	*boolfnames[], *boolnames[], *boolcodes[],
	*numfnames[], *numnames[], *numcodes[],
	*strfnames[], *strnames[], *strcodes[];

main (argc, argv)
int argc;
char **argv;
{
    if (argc > 1)
	setupterm(argv[1], 1, (int *)0);
    else
	setupterm((char *)0,1,(int *)0);
    return 0;
}

Pr(ch)
register int ch;
{
    if (ch >= 0200)
	{
	printf("M-");
	ch -= 0200;
	}
    if ( (ch < ' ') || (ch == 0177) )
	printf("^%c", ch ^ 0100);
    else
	printf("%c", ch);
}

Sprint(n, string)
int n;
register char *string;
{
    register int ch;
    if (n == -1)
	{
	printf(".\n");
	return;
	}
    printf(", string = '");
    while (ch = *string++)
	Pr(ch&0377);

    printf("'.\n");
}

Mprint(n, memory)
register int n;
register char *memory;
{
    register unsigned char ch;
    while (ch = *memory++, n-- > 0)
	Pr(ch&0377);
}

# define Vr2getshi()	Vr2getsh(ip-2)

# if vax || pdp11
#  define Vr2getsh(ip)	(* (short *) (ip))
# endif

# ifndef Vr2getsh
/*
 * Here is a more portable version, which does not assume byte ordering
 * in shorts, sign extension, etc.
 */
Vr2getsh(p)
register char *p;
{
	register int rv;
	if (*p == 0377)
		return -1;
	rv = *p++;
	rv += *p * 256;
	return rv;
}
# endif

#endif /* DUMPTI */

#define getshi()	getsh(ip) ; ip += 2

/*
 * "function" to get a short from a pointer.  The short is in a standard
 * format: two bytes, the first is the low order byte, the second is
 * the high order byte (base 256).  The only negative number allowed is
 * -1, which is represented as 255, 255.  This format happens to be the
 * same as the hardware on the pdp-11 and vax, making it fast and
 * convenient and small to do this on a pdp-11.
 */

#if vax || pdp11
# define getsh(ip)	(* (short *) ip)
#endif
/*
 * The following macro is partly due to Mike Laman, laman@sdcsvax
 *	NCR @ Torrey Pines.		- Tony Hansen
 */
#if u3b || u3b15 || u3b2 || m68000
# define getsh(ip)	((short) (*((unsigned char *) ip) | (*(ip+1) << 8)))
#endif

#ifndef getsh
/*
 * Here is a more portable version, which does not assume byte ordering
 * in shorts, sign extension, etc. It does assume that the C preprocessor
 * does sign-extension the same as on the machine being compiled for.
 * When ANSI C comes along, this should be changed to check <limits.h>
 * to see if the low character value is negative.
 */
static getsh(p)
register char *p;
{
	register int rv, rv2;

# if	-1 == '\377'			/* sign extension occurs */
	rv = (*p++) & 0377;
	rv2 = (*p) & 0377;
# else					/* no sign extension */
	rv = *p++;
	rv2 = *p;
# endif
	if ((rv2 == 0377) &&
	    ((rv == 0377) || (rv == 0376)))
		return -1;
	return rv + (rv2 * 256);
}
#endif

/*
 * setupterm: low level routine to dig up terminfo from database
 * and read it in.  Parms are terminal type (0 means use getenv("TERM"),
 * file descriptor all output will go to (for ioctls), and a pointer
 * to an int into which the error return code goes (0 means to bomb
 * out with an error message if there's an error).  Thus,
 * setupterm((char *)0, 1, (int *)0) is a reasonable way for a simple
 * program to set up.
 */
setupterm(term, filenum, errret)
char *term;
int filenum;	/* This is a UNIX file descriptor, not a stdio ptr. */
int *errret;
{
	char tiebuf[4096];
	char fname[512];
	register char *ip;
	register char *cp;
	int n, tfd;
	char *lcp, *ccp;
	int snames, nbools, nints, nstrs, sstrtab;
	char *strtab;
#ifdef DUMPTI
	int Vr2val;
#endif /* DUMPTI */

	if (term == NULL)
		term = getenv("TERM");
	if (term == NULL || *term == '\0')
		term = "unknown";
	tfd = -1;
	if ((cp=getenv("TERMINFO")) && *cp) {
		if (strlen(cp) + 2 + strlen(term) > 512) {
			if (errret == 0) {
				write(2, "TERMINFO pathname for device exceeds 512 characters\r\n", 53);
				exit(-5);
			} else {
				*errret = -1;
				return -1;
			}
		}
		strcpy(fname, cp);
		cp = fname + strlen(fname);
		*cp++ = '/';
		*cp++ = *term;
		*cp++ = '/';
		strcpy(cp, term);
		tfd = open(fname, 0);
#ifdef DUMPTI
		printf("looking in file %s\n", fname);
#endif /* DUMPTI */
	}
	if (tfd < 0) {
		strcpy(fname, termpath(a/));
		cp = fname + strlen(fname);
		cp[-2] = *term;
		strcpy(cp, term);
		tfd = open(fname, 0);
#ifdef DUMPTI
		printf("looking in file %s\n", fname);
#endif /* DUMPTI */
	}

	if (tfd < 0) {
		if (access(termpath(.), 0)) {
			if (errret == 0)
				perror(termpath(.));
			else
				*errret = -1;
		} else {
			if (errret == 0) {
				write(2, "No such terminal: ", 18);
				write(2, term, strlen(term));
				write(2, "\r\n", 2);
			} else
				*errret = 0;
		}
		if (errret == 0)
			exit(-2);
		else
			return -1;
	}

	n = read(tfd, tiebuf, sizeof tiebuf);
	close(tfd);
	if (n <= 0) {
corrupt:
		if (errret == 0) {
			write(2, "corrupted term entry\r\n", 22);
			exit(-3);
		} else
			return -1;
	} else if (n == sizeof tiebuf) {
		if (errret == 0) {
			write(2, "term entry too long\r\n", 21);
			exit(-4);
		} else
			return -1;
	}
	cp = ttytype;
	ip = tiebuf;

	/* Pick up header */
	snames = getshi();
#ifdef DUMPTI
	Vr2val = Vr2getshi();
	printf("Magic number = %d, %#o [%d,%#o].\n",
		snames, snames, Vr2val, Vr2val);
#endif /* DUMPTI */
	if (snames != MAGNUM) {
		goto corrupt;
	}
	snames = getshi();
#ifdef DUMPTI
	Vr2val = Vr2getshi();
	printf("Size of names = %d, %#o [%d,%#o].\n",
		snames, snames, Vr2val, Vr2val);
#endif /* DUMPTI */

	nbools = getshi();
#ifdef DUMPTI
	Vr2val = Vr2getshi();
	printf("Number of bools = %d, %#o [%d,%#o].\n",
		nbools, nbools, Vr2val, Vr2val);
#endif /* DUMPTI */

	nints = getshi();
#ifdef DUMPTI
	Vr2val = Vr2getshi();
	printf("Number of ints = %d, %#o [%d,%#o].\n",
		nints, nints, Vr2val, Vr2val);
#endif /* DUMPTI */

	nstrs = getshi();
#ifdef DUMPTI
	Vr2val = Vr2getshi();
	printf("Number of strings = %d, %#o [%d,%#o].\n",
		nstrs, nstrs, Vr2val, Vr2val);
#endif /* DUMPTI */

	sstrtab = getshi();
#ifdef DUMPTI
	Vr2val = Vr2getshi();
	printf("Size of string table = %d, %#o [%d,%#o].\n",
		sstrtab, sstrtab, Vr2val, Vr2val);
	printf("Names are: %.*s.\n", snames, ip);
#endif /* DUMPTI */

	/* allocate all of the space */
	strtab = NULL;
	if (_called_before) {
		/* 2nd or more times through */
		cur_term = (TERMINAL *) malloc(sizeof (TERMINAL));
		if (!cur_term)
			goto badmalloc;
		cur_bools = (struct _bool_struct *) malloc(sizeof (struct _bool_struct));
		if (!cur_bools)
			goto freeterminal;
		cur_nums = (struct _num_struct *) malloc(sizeof (struct _num_struct));
		if (!cur_nums)
			goto freebools;
		cur_strs = (struct _str_struct *) malloc(sizeof (struct _str_struct));
		if (!cur_strs) {
			freenums:	free((char *)cur_nums);
			freebools:	free((char *)cur_bools);
			freeterminal:	free((char *)cur_term);
			badmalloc:
				if (errret == 0) {
					write (2, "malloc returned NULL in setupterm\n", 34);
					exit(-6);
				} else {
					*errret = -1;
					return -1;
				}
		}
	} else {
		/* First time through */
		_called_before = TRUE;
		cur_term = &_first_term;
		cur_bools = &_frst_bools;
		cur_nums = &_frst_nums;
		cur_strs = &_frst_strs;
		if (sstrtab < sizeof(_frst_tblstr))
			strtab = _frst_tblstr;
	}

	if (strtab == NULL) {
		strtab = (char *) malloc(sstrtab);
		if (strtab == NULL) {
			if (cur_strs != &_frst_strs)
				free((char *)cur_strs);
			goto freenums;
		}
	}

	cur_term->_bools = cur_bools;
	cur_term->_nums = cur_nums;
	cur_term->_strs = cur_strs;
	cur_term->_strtab = strtab;
	cur_term->sgr_mode = cur_term->sgr_faked = A_NORMAL;

	/* no more catchable errors */
	if (errret)
		*errret = 1;

	if (filenum == 1 && !isatty(filenum))
		filenum = 2;	/* Allow output redirect */
	cur_term -> Filedes = filenum;

#ifndef DUMPTI
# ifdef SYSV
	cur_term->Nttyb.c_cflag = 0;
# else
	cur_term->Nttyb.sg_ispeed = 0;
# endif /* SYSV */
	def_shell_mode();
#endif /* DUMPTI */

	/* Skip names of terminals */
	memcpy((char *)cp, (char *)ip, snames*sizeof(*cp));
	ip += snames;

	/*
	 * Pull out the booleans.
	 * Inner blocks are to share this register among two variables.
	 */
	{
		char *fp = &cur_bools->Sentinel;
		register char s;
#ifdef DUMPTI
		register int tempindex = 0;
#endif /* DUMPTI */
		for (cp= &cur_bools->_auto_left_margin; nbools--; ) {
			s = *ip++;
#ifdef DUMPTI
			printf("Bool %s [%s] (%s) = %d.\n",
				boolfnames[tempindex],
				boolnames[tempindex],
				boolcodes[tempindex], s);
			tempindex++;
#endif /* DUMPTI */
			if (cp < fp)
				*cp++ = s & 01;
		}
	}

	/* Force proper alignment */
	if (((unsigned int) ip) & 1)
		ip++;

	/*
	 * Pull out the numbers.
	 */
	{
		register short *sp;
		short *fp = &cur_nums->Sentinel;
		register int s;
#ifdef DUMPTI
		register int tempindex = 0;
#endif /* DUMPTI */

		for (sp= &cur_nums->_columns; nints--; ) {
			s = getshi();
#ifdef DUMPTI
			Vr2val = Vr2getshi();
			printf("Num %s [%s] (%s) = %d [%d].\n",
				numfnames[tempindex],
				numnames[tempindex],
				numcodes[tempindex], s, Vr2val);
			tempindex++;
#endif /* DUMPTI */
			if (sp < fp)
				if (s < 0)
					*sp++ = -1;
				else
					*sp++ = s;
		}
	}

#ifdef JWINSIZE
	/*
	 * ioctls for Blit - you may need to #include <jioctl.h>
	 * This ioctl defines the window size and overrides what
	 * it says in terminfo.
	 */
	{
		struct jwinsize w;

		if (ioctl(filenum, JWINSIZE, &w) != -1 &&
			w.bytesy != 0 && w.bytesx != 0) {
			cur_nums->_lines = (unsigned char) w.bytesy;
			cur_nums->_columns = (unsigned char) w.bytesx;
#ifdef DUMPTI
	printf("ioctl JWINSIZE override: (lines,columns)=(%d,%d)\n",
		w.bytesy, w.bytesx);
#endif /* DUMPTI */
		}
	}
#endif

#ifdef TIOCGSIZE
	/*
	 * ioctls for Sun.
	 */
	{
		struct ttysize w;

		if (ioctl(filenum, TIOCGSIZE, &w) != -1 &&
			w.ts_lines > 0 && w.ts_cols > 0) {
			cur_nums->_lines = w.ts_lines;
			cur_nums->_columns = w.ts_cols;
#ifdef DUMPTI
	printf("ioctl TIOCGSIZE override: (lines,columns)=(%d,%d)\n",
		w.ts_lines, w.ts_cols);
#endif /* DUMPTI */
		}
	}
#endif

	/*
	 * Check $LINES and $COLUMNS.
	 */
	{
		int ilines = 0, icolumns = 0;
		lcp = getenv("LINES");
		ccp = getenv("COLUMNS");
		if (lcp)
			if ((ilines = atoi(lcp)) > 0) {
				cur_nums->_lines = ilines;
#ifdef DUMPTI
	printf("$LINES override: lines=%d\n", ilines);
#endif /* DUMPTI */
			}
		if (ccp)
			if ((icolumns = atoi(ccp)) > 0) {
				cur_nums->_columns = icolumns;
#ifdef DUMPTI
	printf("$COLUMNS override: columns=%d\n", icolumns);
#endif /* DUMPTI */
			}
	}

	/*
	 * Pull out the strings.
	 */
	{
		register char **pp;
		char **fp = &cur_strs->strs3.Sentinel;
#ifdef DUMPTI
		register int tempindex = 0;
		register char *startstr = ip + 2*nstrs;

		printf("string table = '");
		Mprint(sstrtab, startstr);
		printf("'\n");
#endif /* DUMPTI */

		for (pp= &cur_strs->strs._back_tab; nstrs--; ) {
			n = getshi();
#ifdef DUMPTI
			Vr2val = Vr2getshi();
			printf("String %s [%s] (%s) offset = %d [%d]",
				strfnames[tempindex],
				strnames[tempindex],
				strcodes[tempindex], n, Vr2val);
			tempindex++;
#endif /* DUMPTI */
			if (pp < fp) {
#ifdef DUMPTI
				Sprint(n, startstr+n);
#endif /* DUMPTI */
				if (n < 0)
					*pp++ = NULL;
				else
					*pp++ = strtab+n;
			}
#ifdef DUMPTI
			else
				Sprint(-1, (char *)0);
#endif /* DUMPTI */
		}
	}

	memcpy(strtab, ip, sstrtab);

#ifndef DUMPTI

	/*
	 * If tabs are being expanded in software, turn this off
	 * so output won't get messed up.  Also, don't use tab
	 * or backtab, even if the terminal has them, since the
	 * user might not have hardware tabs set right.
	 */
# ifdef SYSV
	if ((PROGTTY.c_oflag & TABDLY) == TAB3) {
		PROGTTY.c_oflag &= ~TABDLY;
		cur_strs->strs2._tab = NULL;
		cur_strs->strs._back_tab = NULL;
		reset_prog_mode();
	}
# else
	if ((PROGTTY.sg_flags & XTABS) == XTABS) {
		PROGTTY.sg_flags &= ~XTABS;
		cur_strs->strs2._tab = NULL;
		cur_strs->strs._back_tab = NULL;
		reset_prog_mode();
	}
# endif
# ifdef DIOCSETT
	reset_prog_mode();
# endif
# ifdef LTILDE
	ioctl(cur_term -> Filedes, TIOCLGET, &n);
	if (n & LTILDE);
		reset_prog_mode();
# endif
#endif /* DUMPTI */

#ifdef _VR2_COMPAT_CODE
	memcpy(&cur_term->_b1, &cur_bools->_auto_left_margin,
		(char *)&cur_term->_c1 - (char *)&cur_term->_b1);
	memcpy(&cur_term->_c1, &cur_nums->_columns,
		(char *)&cur_term->_Vr2_Astrs._s1 - (char *)&cur_term->_c1);
	memcpy(&cur_term->_Vr2_Astrs._s1, &cur_strs->strs._back_tab,
		(char *)&cur_term->Filedes - (char *)&cur_term->_Vr2_Astrs._s1);
#endif /* _VR2_COMPAT_CODE */

	cur_term->rmul_rmso_neq = strcmp(exit_underline_mode,exit_standout_mode);
	cur_term->rmul_sgr0_neq = strcmp(exit_underline_mode,exit_attribute_mode);
	cur_term->rmacs_sgr0_neq = strcmp(exit_alt_charset_mode,exit_attribute_mode);
	cur_term->rmacs_rmso_neq = strcmp(exit_alt_charset_mode,exit_standout_mode);
	cur_term->rmso_sgr0_neq = strcmp(exit_standout_mode,exit_attribute_mode);

	return 0;
}

#ifndef DUMPTI

reset_prog_mode()
{
	if (BR(Nttyb))
		(void) ioctl(cur_term -> Filedes,
#ifdef SYSV
			TCSETAW,
#else
			TIOCSETN,
#endif
			&PROGTTY);
# ifdef LTILDE
	ioctl(cur_term -> Filedes, TIOCLGET, &cur_term -> oldlmode);
	cur_term -> newlmode = cur_term -> oldlmode & ~LTILDE;
	if (cur_term -> newlmode != cur_term -> oldlmode)
		ioctl(cur_term -> Filedes, TIOCLSET, &cur_term -> newlmode);
# endif
#ifdef DIOCSETT
	if (cur_term -> old.st_termt == 0)
		ioctl(cur_term->Filedes, DIOCGETT, &cur_term -> old);
	cur_term -> new = cur_term -> old;
	cur_term -> new.st_termt = 0;
	cur_term -> new.st_flgs |= TM_SET;
	ioctl(cur_term->Filedes, DIOCSETT, &cur_term -> new);
#endif
}

def_shell_mode()
{
#ifdef SYSV
	(void) ioctl(cur_term -> Filedes, TCGETA, &SHELLTTY);
#else
	(void) ioctl(cur_term -> Filedes, TIOCGETP, &SHELLTTY);
#endif
	/* This is a useful default for Nttyb, too */
	if (BR(Nttyb) == 0)
		PROGTTY = SHELLTTY;
}

#endif /* DUMPTI */
