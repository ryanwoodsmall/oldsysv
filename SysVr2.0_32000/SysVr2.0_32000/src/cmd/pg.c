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

static char sccsid[] = "@(#)pg.c	1.5";

#include <signal.h>
#include <setjmp.h>
#include <sys/types.h>
#include <sys/dir.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <curses.h>
#include <term.h>

/*
 *      pg -- paginator for crt terminals
 *
 *	Includes the ability to display pages that have
 *	already passed by. Also gives the user the ability
 *	to search forward and backwards for regular expressions.
 *	This works for piped input by copying to a temporary file,
 *	and resolving backreferences from there.
 *
 *	Note:	The reason that there are so many commands to do
 *		the same types of things is to try to accommodate
 *		users of other paginators.
 */

#define LINSIZ	1024
#define QUIT	'\034'
#define BOF	(EOF - 1)	/* Begining of File */
#define STOP    (EOF - 2)
#define PROMPTSIZE	256

struct line {			/* how line addresses are stored */
	long	l_addr;		/* file offset */
	int	l_no;		/* line number in file */
};

typedef	struct line	LINE;
LINE		*zero = NULL,		/* first line */
		*dot,		/* current line */
		*dol,		/* last line */
		*contig;	/* where contiguous (non-aged) lines start */
short           nlall;          /* room for how many LINEs in memory */

FILE		*in_file,	/* current input stream */
		*tmp_fin,	/* pipe temporary file in */
		*tmp_fou;	/* pipe temporary file out */
char		*tmp_name = "/tmp/pgXXXXXX";

long		ftell();
char		*malloc(),
		*realloc();

short		sign;		/* sign of command input */

int             fnum,           /* which file argument we're in */
		pipe_in,	/* set when stdin is a pipe */
		out_is_tty;	/* set if stdout is a tty */

int		on_brk(),
		end_it();
short		brk_hit;	/* interrupt handling is pending flag */

int             window = 0;	/* window size in lines */
short		eof_pause = 1;	/* pause w/ prompt at end of files */
short		soflag = 0;	/* output all messages in standout mode */
short           promptlen;      /* length of the current prompt */
short           firstf = 1;	/* set before first file has been processed */
short           inwait,		/* set while waiting for user input */
		errors;         /* set if error message has been printed.
				 * if so, need to erase it and prompt */

char		**fnames;
short		fflag = 0;	/* set if the f option is used */
short		nflag = 0;	/* set for "no newline" input option */
short		clropt = 0;	/* set if the clear option is used */
int		initopt = 0;	/* set if the line option is used */
int		srchopt = 0;	/* set if the search option is used */
int		initline;
char		initbuf[BUFSIZ];
char            leave_search = 't';     /* where on the page to leave a found string */
short           nfiles;
char		*shell;
char		*promptstr = ":";
char		*setprompt();
short		lenprompt;		/* length of prompt string */
int		nchars;			/* return from getline in find() */
jmp_buf		restore;
char		Line[LINSIZ+2];

struct screen_stat {
	int	first_line;
	int	last_line;
	short	is_eof;
	};
struct screen_stat old_ss = { 0, 0, 0 };
struct screen_stat new_ss;

short		eoflag;		/* set whenever at end of current file */
short		doliseof;	/* set when last line of file is known */
int		eofl_no;	/* what the last line of the file is */

#define USAGE() { fprintf(stderr,"Usage: pg [-number] [-p string] [-cefns] [+line] [+/pattern/] files\n"); exit(1); }

main(argc, argv)
int argc;
char *argv[];
{
	register char	*s;
	register char	*p;
	register char	ch;
	int		prnames = 0; 
	char		*getenv();
	FILE		*checkf();

	nfiles = argc;
	fnames = argv;
	while (--nfiles > 0) {
		if ((ch = (*++fnames)[0]) == '-') {
			if (fnames[0][1] == '\0' )
				break;
			for (s = fnames[0]+1; *s != '\0'; s++)
				if (isdigit(*s)) {
					window = 0;
					do {
						window = window*10+*s-'0';
					} while (isdigit(*++s));
					if (*s != '\0')
						USAGE();
					break;
				}
				else if (*s == 'c')
					clropt = 1;
				else if (*s == 'e')
					eof_pause = 0;
				else if (*s == 'f')
					fflag = 1;
				else if (*s == 'n')
					nflag = 1;
				else if (*s == 's')
					soflag = 1;	/* standout mode */
				else if (*s == 'p') {
					if (*++s != '\0')
						promptstr = setprompt(s);
					else if (nfiles > 1) {
						promptstr = setprompt(*++fnames);
						nfiles--;
					}
					else
						USAGE();
					break;
				}
				else
					USAGE();

		}
		else if (ch == '+') {
			s = *fnames;
			if (*++s == '/') {
				srchopt++;
				initopt = 0;
				for (++s, p=initbuf; *s!='\0' && *s!='/';)
					if (p < initbuf + sizeof(initbuf))
						*p++ = *s++;
					else {
						fprintf(stderr,"pg: pattern too long\n");
						exit(1);
					}
				*p = '\0';
			}
			else {
				initopt++;
				srchopt = 0;
				for (; isdigit(*s); s++)
					initline = initline*10 + *s -'0';
				if (*s != '\0')
					USAGE();
			}
		}
		else
			break;
	}
	signal(SIGQUIT,end_it);
	signal(SIGINT,end_it);
	out_is_tty = isatty(1);
	if (out_is_tty) {
		terminit();
		signal(SIGQUIT,on_brk);
		signal(SIGINT,on_brk);
	}
	if (window == 0)
		window = lines - 1;
	lenprompt = strlen(promptstr);
	if (window <= 1)
		window = 2;
	if (initline <= 0)
		initline = 1;
	if (nfiles > 1)
		prnames++;

	if (nfiles == 0) {
		fnames[0] = "-";
		nfiles++;
	}
	while (fnum < nfiles) {
		if (strcmp(fnames[fnum],"") == 0)
			fnames[fnum] = "-";
		if ((in_file = checkf(fnames[fnum])) == NULL)
			fnum++;
		else {
			if (out_is_tty)
				fnum += screen(fnames[fnum]);
			else {
				if (prnames) {
					pr("::::::::::::::\n");
					pr(fnames[fnum]);
					pr("\n::::::::::::::\n");
				}
				copy_file(in_file,stdout);
				fnum++;
			}
			fflush(stdout);
			if (pipe_in)
				save_pipe();
			else
			if (in_file != tmp_fin)
				fclose(in_file);
		}
	}
	end_it();
}

char *
setprompt(s)
register char *s;
{
	register int i = 0;
	register int pct_d = 0;
	static char pstr[PROMPTSIZE];

	while (i < PROMPTSIZE - 2)
		switch(pstr[i++] = *s++) {
		case '\0':
			return(pstr);
		case '%':
			if (*s == 'd' && !pct_d) {
				pct_d++;
			}
			else if (*s != '%')
				pstr[i++] = '%';
			if ((pstr[i++] = *s++) == '\0')
				return(pstr);
			break;
		default:
			break;
		}
	fprintf(stderr,"pg: prompt too long\n");
	exit(1);
}


/*
 * Print out the contents of the file f, one screenful at a time.
 */

screen(file_name)
char *file_name;
{
	int cmd_ret = 0;
	int start;
	short hadchance = 0;

	old_ss.is_eof = 0;
	old_ss.first_line = 0;
	old_ss.last_line = 0;
	new_ss = old_ss;

	if (!firstf)
		cmd_ret = command(file_name);
	else {
		firstf = 0;
		if (initopt) {
			initopt = 0;
			new_ss.first_line = initline;
			new_ss.last_line = initline + window - 1;
		}
		else
		if (srchopt) {
			srchopt = 0;
			if (!search(initbuf,1))
				cmd_ret = command(file_name);
		}
		else {
			new_ss.first_line = 1;
			new_ss.last_line = window;
		}
	}

	for (;;) {
		if (cmd_ret)
			return(cmd_ret);
		if (hadchance && new_ss.first_line >= eofl_no - 1)
			return(1);
		hadchance = 0;

		if (new_ss.last_line < window)
			new_ss.last_line = window;
		if (find(0,new_ss.last_line + 1) != EOF)
			new_ss.is_eof = 0;
		else {
			new_ss.is_eof = 1;
			new_ss.last_line = eofl_no - 1;
			new_ss.first_line = new_ss.last_line - window + 1;
		}

		if (new_ss.first_line < 1)
			new_ss.first_line = 1;
		if (clropt) {
			doclear();
			start = new_ss.first_line;
		}
		else {
			if (new_ss.first_line == old_ss.last_line)
				start = new_ss.first_line + 1;
			else
			if (new_ss.first_line > old_ss.last_line)
				start = new_ss.first_line;
			else
			if (old_ss.first_line < new_ss.first_line)
				start = old_ss.last_line + 1;
			else
				start = new_ss.first_line;

			if (start < old_ss.first_line)
				sopr("...skipping backward\n",0);
			else
			if (start > old_ss.last_line + 1)
				sopr("...skipping forward\n",0);
		}

		for(; start <= new_ss.last_line; start++) {
			find(0,start);
			pr(Line);
			if (brk_hit) {
				new_ss.last_line = find(1,0);
				new_ss.is_eof = 0;
				break;
			}
		}

		brk_hit = 0;
		fflush(stdout);
		if (new_ss.is_eof) {
			if (!eof_pause || eofl_no == 1)
				return(1);
			hadchance++;
			error("(EOF)");
		}
		old_ss = new_ss;
		cmd_ret = command((char *)NULL);
	}
}

char	cmdbuf[LINSIZ], *cmdptr;
#define BEEP()		if (bell) { putp(bell); fflush(stdout); }
#define	BLANKS(p)	while (*p == ' ' || *p == '\t') p++
#define CHECKEND()	BLANKS(cmdptr); if (*cmdptr) { BEEP(); break; }

/*
 * Read a command and do it. A command consists of an optional integer
 * argument followed by the command character.  Return the number of files
 * to skip, 0 if we're still talking about the same file.
 */

command (filename)
char *filename;
{
	register int nlines;
	register int retval;
	register char c;
	FILE *sf;
	char *cmdend;
	short checkit = 0;
	int id;
	int skip;

	for (;;) {
		/* Wait for output to drain before going on.     */
		/* This is done so that the user will not hit    */
		/* break and quit before he has seen the prompt. */
		ioctl(1,TCSBRK,1);
		if (setjmp(restore) != 0)
			end_it();
		inwait = 1;
		brk_hit = 0;
		if (errors)
			errors = 0;
		else {
			kill_line();
			prompt(filename);
		}
		fflush(stdout);
		if (ttyin())
			continue;
		cmdptr = cmdbuf;
		nlines = number();
		BLANKS(cmdptr);
		switch (*cmdptr++) {
		case 'h':
			CHECKEND();
			help();
			break;
		case '\014': /* ^L */
		case '.':       /* redisplay current window */
			CHECKEND();
			new_ss.first_line = old_ss.first_line;
			new_ss.last_line = new_ss.first_line + window - 1;
			inwait = 0;
			return(0);
		case 'w':       /* set window size */
		case 'z':
			if (sign == -1) {
				BEEP();
				break;
			}
			CHECKEND();
			if (nlines == 0)
				nlines = window;
			else 
			if (nlines > 1)
				window = nlines;
			else {
				BEEP();
				break;
			}
			new_ss.first_line = old_ss.last_line;
			new_ss.last_line = new_ss.first_line + window - 1;
			inwait = 0;
			return(0);
		case '\004': /* ^D */
		case 'd':
			CHECKEND();
			if (sign == 0)
				sign = 1;
			new_ss.last_line = old_ss.last_line + sign*window/2;
			new_ss.first_line = new_ss.last_line - window + 1;
			inwait = 0;
			return(0);
		case 's':
			/*
			* save input in filename.
			* Check for filename, access, etc.
			*/
			BLANKS(cmdptr);
			if (!*cmdptr) {
				BEEP();
				break;
			}
			if (setjmp(restore) != 0) {
				BEEP();
			}
			else {
				if ((sf=fopen(cmdptr,"w")) == NULL) {
					error("cannot open save file");
					break;
				}
				kill_line();
				sopr("saving file ",1);
				sopr(cmdptr,1);
				fflush(stdout);
				save_input(sf);
				error("saved");
			}
			fclose(sf);
			break;
		case 'q':
		case 'Q':
			CHECKEND();
			inwait = 0;
			end_it();
		case 'f':       /* skip forward screenfuls */
			CHECKEND();
			if (sign == 0)
				sign++;	/* skips are always relative */
			if (nlines == 0)
				nlines++;
			nlines = nlines * (window - 1);
			if (sign == 1)
				new_ss.first_line = old_ss.last_line + nlines;
			else
				new_ss.first_line = old_ss.first_line - nlines;
			new_ss.last_line = new_ss.first_line + window - 1;
			inwait = 0;
			return(0);
		case 'l':       /* get a line */
			CHECKEND();
			if (nlines == 0) {
				nlines++;
				if (sign == 0)
					sign = 1;
			}
			switch(sign){
			case 1:
				new_ss.last_line = old_ss.last_line + nlines;
				new_ss.first_line = new_ss.last_line - window + 1;
				break;
			case 0:  /* leave addressed line at top */
				new_ss.first_line = nlines;
				new_ss.last_line = nlines + window - 1;
				break;
			case -1:
				new_ss.first_line = old_ss.first_line - nlines;
				new_ss.last_line = new_ss.first_line + window - 1;
				break;
			}
			inwait = 0;
			return(0);
		case '\0': /* \n or blank */
			if (nlines == 0){
				nlines++;
				if (sign == 0)
					sign = 1;
			}
			nlines = (nlines - 1) * (window - 1);
			switch(sign) {
			case 1:
				new_ss.first_line = old_ss.last_line + nlines;
				new_ss.last_line = new_ss.first_line + window - 1;
				break;
			case 0:
				new_ss.first_line = nlines + 1;
				new_ss.last_line = nlines + window;
				break;
			case -1:
				new_ss.last_line = old_ss.first_line - nlines;
				new_ss.first_line = new_ss.last_line - window + 1;
				break;
			}
			inwait = 0;
			return(0);
		case 'n':       /* switch to next file in arglist */
			CHECKEND();
			if (sign == 0)
				sign = 1;
			if (nlines == 0)
				nlines++;
			if ((skip = skipf(sign *nlines)) == 0) {
				BEEP();
				break;
			}
			inwait = 0;
			return(skip);
		case 'p':       /* switch to previous file in arglist */
			CHECKEND();
			if (sign == 0)
				sign = 1;
			if (nlines == 0)
				nlines++;
			if ((skip = skipf(-sign * nlines)) == 0) {
				BEEP();
				break;
			}
			inwait = 0;
			return(skip);
		case '$':       /* go to end of file */
			CHECKEND();
			sign = 1;
			while(find(1,10000) != EOF)
				/* any large number will do */;
			new_ss.last_line = eofl_no - 1;
			new_ss.first_line = eofl_no - window;
			inwait = 0;
			return(0);
		case '/':       /* search forward for r.e. */
		case '?':       /*   "  backwards */
		case '^':	/* this ones a ? for regent100s */
			if(sign < 0) {
				BEEP();
				break;
			}
			if (nlines == 0)
				nlines++;
			cmdptr--;
			cmdend = cmdptr + (strlen(cmdptr) - 1);
			if ( (cmdend > cmdptr + 1)
				&& (*cmdptr ==  *(cmdend - 1))
				&& ( ((c = *cmdend) == 't')
					|| (c == 'm')
					|| (c == 'b') ) ) {
				leave_search = c;
				cmdend--;
			}
			if ((cmdptr < cmdend) && (*cmdptr == *cmdend))
				*cmdend = '\0';
			if (*cmdptr != '/')  /* signify back search by - */
				nlines = -nlines;
			if (!search(++cmdptr, nlines))
				break;
			else {
				inwait = 0;
				return(0);
			}
		case '!':       /* shell escape */
			if (!hard_copy) { /* redisplay the command */
				pr(cmdbuf);
				pr("\n");
			}
			if ((id = fork ()) < 0) {
				error("cannot fork, try again later");
				break;
			}
			if (id == 0) {
				/*
				* if stdin is a pipe, need to close it so
				* that the terminal is really stdin for
				* the command
				*/
				fclose(stdin);
				dup(fileno(stdout));
				execl(shell, shell, "-c", cmdptr, 0);
				fprintf(stderr,"exec failed\n");
				exit(1);
			}
			signal (SIGINT, SIG_IGN);
			signal (SIGQUIT, SIG_IGN);
			wait ((int *) 0);
			pr("!\n");
			fflush(stdout);
			signal(SIGINT,on_brk);
			signal(SIGQUIT,on_brk);
			break;
		default:
			BEEP();
			break;
		}
	}
}

number()
{
	register int i;
	register char *p;

	i = 0;
	sign = 0;
	p = cmdptr;
	BLANKS(p);
	if (*p == '+') {
		p++;
		sign = 1;
	}
	else
	if (*p == '-') {
		p++;
		sign = -1;
	}
	while (isdigit(*p))
		i = i * 10 + *p++ - '0';
	cmdptr = p;
	return(i);
}

ttyin ()
{
	register char *sptr;
	register char ch;
	register int slash = 0;
	int state = 0;

	fixterm();
	set_state(&state,' ');	/* initialize state processing */
	sptr = cmdbuf;
	while(state != 10) {
		ch = readch();
		if (ch == '\n' && !slash)
			break;
		if (ch == erasechar() && !slash) {
			if (sptr > cmdbuf) {
				--promptlen;
				pr("\b \b");
				--sptr;
				if (*sptr < ' ') {
					--promptlen;
					pr("\b \b");
				}
			}
			set_state(&state,ch,sptr);
			fflush(stdout);
			continue;
		}
		else
		if (ch == killchar() && !slash) {
			if (hard_copy)
				putchar(ch);
			resetterm();
			return(1);
		}
		if (slash) {
			slash = 0;
			pr("\b \b");
			sptr--;
			promptlen--;
		}
		else /* is there room to keep this character? */
		if (sptr>=cmdbuf + sizeof(cmdbuf) || promptlen + 2 >= columns) {
			BEEP();
			continue;
		}
		else
		if (ch == '\\')
			slash++;
		if (set_state(&state,ch,sptr) == 0) {
			BEEP();
			continue;
		}
		*sptr++ = ch;
		if (ch < ' ') {
			ch += 0100;
			putchar('^');
			promptlen++;
		}
		putchar(ch);
		promptlen++;
		fflush(stdout);
	}

	*sptr = '\0';
	kill_line();
	fflush(stdout);
	resetterm();
	return(0);
}

set_state(pstate,c,pc)
register int *pstate;
register int c;
register char *pc;
{
	static char *psign;
	static char *pnumber;
	static char *pcommand;
	static int slash;

	if (*pstate == 0) {
		psign = NULL;
		pnumber = NULL;
		pcommand = NULL;
		*pstate = 1;
		slash = 0;
		return(1);
	}
	if (c == '\\' && !slash) {
		slash++;
		return(1);
	}
	if (c == erasechar() && !slash)
		switch(*pstate) {
		case 4:
			if (pc > pcommand)
				return(1);
			pcommand = NULL;
		case 3:
			if (pnumber && pc > pnumber) {
				*pstate = 3;
				return(1);
			}
			pnumber = NULL;
		case 2:
			if (psign && pc > psign) {
				*pstate = 2;
				return(1);
			}
			psign = NULL;
		case 1:
			*pstate = 1;
			return(1);
		}

	slash = 0;
	switch(*pstate) {
	case 1: /* before recieving anything interesting */
		if (c == '\t' || (!nflag && c == ' '))
			return(1);
		if (c == '+' || c == '-') {
			psign = pc;
			*pstate = 2;
			return(1);
		}
	case 2: /* recieved sign, waiting for digit */
		if (isdigit(c)) {
			pnumber = pc;
			*pstate = 3;
			return(1);
		}
	case 3: /* recieved digit, waiting for the rest of the number */
		if (isdigit(c))
			return(1);
		if (strchr("h\014.wz\004dqQfl np$",c)) {
			pcommand = pc;
			if (nflag)
				*pstate = 10;
			else
				*pstate = 4;
			return(1);
		}
		if (strchr("s/^?!",c)) {
			pcommand = pc;
			*pstate = 4;
			return(1);
		}
		return(0);
	case 4:
		return(1);
	}
}

readch ()
{
	char ch;

	read (fileno(stdout), &ch, 1);
	return (ch);
}

help()
{
	if (clropt)
		doclear();

	pr("-------------------------------------------------------\n");
	pr("   h               help\n");
	pr("   q or Q          quit\n");
	pr("   <blank> or \\n   next page\n");
	pr("   l               next line\n");
	pr("   d or ^D         display half a page more\n");
	pr("   . or ^L         redisplay current page\n");
	pr("   f               skip the next page forward\n");
	pr("   n               next file\n");
	pr("   p               previous file\n");
	pr("   $               last page\n");
	pr("   w or z          set window size and display next page\n");
	pr("   s savefile      save current file in savefile\n");
	pr("   /pattern/       search forward for pattern\n");
	pr("   ?pattern? or\n");
	pr("   ^pattern^       search backward for pattern\n");
	pr("   !command         execute command\n");
	pr("\n");
	pr("Most commands can be preceeded by a number, as in:\n");
	pr("+1\\n (next page); -1\\n (previous page); 1\\n (page 1).\n");
	pr("\n");
	pr("See the manual page for more detail.\n");
	pr("-------------------------------------------------------\n");
}

/*
 * Skip nskip files in the file list (from the command line). Nskip may be
 * negative.
 */

skipf (nskip)
register int nskip;
{
	if (fnum + nskip < 0) {
		nskip = -fnum;
		if (nskip == 0)
			error("No previous file");
	}
	else
	if (fnum + nskip > nfiles - 1) {
		nskip = (nfiles - 1) - fnum;
		if (nskip == 0)
			error("No next file");
	}
	return(nskip);
}

/*
 * Check whether the file named by fs is a file which the user may
 * access.  If it is, return the opened file. Otherwise return NULL.
 */

FILE *
checkf (fs)
register char *fs;
{
	struct stat stbuf;
	register FILE *f;
	int fd;
	int c;

	pipe_in = 0;
	if (strcmp(fs,"-") == 0) {
		if (tmp_fin == NULL)
			f = stdin;
		else {
			rewind(tmp_fin);
			f = tmp_fin;
		}
	}
	else {
		if ((f=fopen(fs, "r")) == NULL) {
			fflush(stdout);
			perror(fs);
			return (NULL);
		}
	}
	if (fstat(fileno(f), &stbuf) == -1) {
		fflush(stdout);
		perror(fs);
		return (NULL);
	}
	if (stbuf.st_mode & S_IFDIR) {
		fprintf(stderr,"pg: %s is a directory\n",fs);
		return (NULL);
	}
	if (stbuf.st_mode & S_IFREG) {
		if (f == stdin)		/* It may have been read from */
			rewind(f);	/* already, and not reopened  */
	}
	else {
		if (f != stdin) {
			fprintf(stderr,"pg: special files only handled as standard input\n");
			return(NULL);
		}
		else {
			mktemp(tmp_name);
			if ((fd=creat(tmp_name,0600)) < 0) {
			    fprintf(stderr,"pg: Can't create temp file\n");
			    return(NULL);
			}			
			close(fd);
			if ((tmp_fou = fopen(tmp_name, "w")) == NULL) {
				fprintf(stderr,"pg: Can't get temp file for writing\n");
				return(NULL);
			}
			if ((tmp_fin = fopen(tmp_name, "r")) == NULL) {
				fprintf(stderr,"pg: Can't get temp file for reading\n");
				return(NULL);
			}
			pipe_in = 1;
		}
	}
	lineset(BOF);
	return(f);
}

copy_file(f, out)
register FILE *f, *out;
{
	register int c;

	while ((c = getc(f)) != EOF)
		putc(c,out);
}

#define INIT		register char *sp = instring;
#define GETC()		(*sp++)
#define PEEKC()		(*sp)
#define UNGETC()	(--sp)
#define RETURN(c)	return
#define ERROR(c)	re_error(c)

#include <regexp.h>
re_error(i)
int i;
{
	int j;
	static struct messages {
		char *message;
		int number;
		} re_errmsg[] = {
		"Pattern not found",				1,
		"Range endpoint too large",			11,
		"Bad number",					16,
		"`\\digit' out of range",			25,
		"Illegal or missing delimeter",			36,
		"No remembered search string",  		41,
		"\\( \\) imbalance",				42,
		"Too many \\(",					43,
		"More than two numbers given in \\{ \\}",	44,
		"} expected after \\",				45,
		"First number exceeds second in \\{ \\}",	46,
		"[] imbalance",					49,
		"Regular expression overflow",			50,
		"Bad regular expression",		 	0
		};

	for (j = 0; re_errmsg[j].number != 0; j++ )
		if (re_errmsg[j].number == i )
			break;
	error(re_errmsg[j].message);
	longjmp(restore,1);  /* restore to search() */
}

/*
 * Search for nth ocurrence of regular expression contained in buf in the file
 *	negative n implies backward search
 *	n 'guaranteed' non-zero
 */

char expbuf[BUFSIZ];

search (buf, n)
char buf[];
register int n;
{
	register int direction;
	char *endbuf;
	int END_COND;

	endbuf = buf + strlen(buf)-2;
	if(*endbuf++ != '\\' && *endbuf == '$') {
		*endbuf++ = '\\';
		*endbuf++ = 'n';
		*endbuf = '\0';
		}
	if (setjmp(restore) == 0) {
		compile(buf,expbuf,expbuf+BUFSIZ,NULL);

		if (n < 0) {	/* search back */
			direction = -1;
			find(0,old_ss.first_line);
			END_COND = BOF;
		}
		else {
			direction = 1;
			find(0,old_ss.last_line);
			END_COND = EOF;
		}

		while (find(1,direction) != END_COND){
			if (brk_hit)
				break;
			if (step(Line,expbuf))
				if ((n -= direction) == 0) {
					switch(leave_search) {
					case 't':
						new_ss.first_line = find(1,0);
						new_ss.last_line = new_ss.first_line + window - 1;
						break;
					case 'b':
						new_ss.last_line = find(1,0);
						new_ss.first_line = new_ss.last_line - window + 1;
						break;
					case 'm':
						new_ss.first_line =
find(1,0) - (window - 1)/2;
						new_ss.last_line = new_ss.first_line + window - 1;
						break;
					}
					return(1);
				}
		}
		re_error(1); /* Pattern not found */
	}
	BEEP();
	return(0);
}

/*
 *	find -- find line in file f, subject to certain constraints.
 *
 *	This is the reason for all the funny stuff with sign and nlines.
 *	We need to be able to differentiate between relative and abosolute
 *	address specifications. 
 *
 *	So...there are basically three cases that this routine
 *	handles. Either line is zero, which  means there is to be
 *	no motion (because line numbers start at one), or
 *	how and line specify a number, or line itself is negative,
 *	which is the same as having how == -1 and line == abs(line).
 *
 *	Then, figure where exactly it is that we are going (an absolute
 *	line number). Find out if it is within what we have read,
 *	if so, go there without further ado. Otherwise, do some
 *	magic to get there, saving all the intervening lines,
 *	in case the user wants to see them some time later.
 *
 *	In any case, return the line number that we end up at. 
 *	(This is used by search() and screen()). If we go past EOF,
 *	return EOF.
 *	This EOF will go away eventually, as pg is expanded to
 *	handle multiple files as one huge one. Then EOF will
 *	mean we have run off the file list.
 *	If the requested line number is too far back, return BOF.
 */
find(how,line)	/* find the line and seek there */
short how;
short line;
{
	/* no compacted memory yet */
	register FILE *f = in_file;
	register short where;
	int c;			/* used to check for end of file */

	if (how == 0)
		where = line;
	else
		if (dot == zero - 1)
			where = how * line;
		else
			where = how * line + dot->l_no;

	/* now, where is either at, before, or after dol */
	/* most likely case is after, so do it first */

	eoflag = 0;
	if (where >= dol->l_no) {
		if (doliseof) {
			dot = dol;
			eoflag++;
			return(EOF);
		}
		if (pipe_in)
			in_file = f = stdin;
		else
			fseek(f, dol->l_addr, 0);
		dot = dol - 1;
		while ((nchars = getline(f)) != EOF) {
			dot++;
			newdol(f);
			if ( where == dot->l_no || brk_hit)
				break;
		}
		if (nchars != EOF)
			return(dot->l_no);
		else { /* EOF */
			dot = dol;
			eoflag++;
			doliseof++;
			eofl_no = dol->l_no;
			return(EOF);
		}
	}
	else { /* where < dol->l_no */
		if (pipe_in) {
			fflush(tmp_fou);
			in_file = f = tmp_fin;
		}
		if (where < zero->l_no){
			fseek(f, zero->l_addr, 0);
			dot = zero - 1;
			return(BOF);
		}
		else {
			dot = zero + where - 1;
			fseek(f, dot->l_addr, 0);
			nchars = getline(f);
			return(dot->l_no);
		}
	}
}

/*
 * Get a logical line
 */

getline(f)
register FILE *f;
{
	register int	c;
	register char	*p;
	register int	column;
	register int	i;
	register int (*rdchar)();
	int fgetc();
	int fgetputc();
	static int	colflg;	/* true if received \f or a long line */

	if (pipe_in && f == stdin)
		rdchar = fgetputc;
	else
		rdchar = fgetc;

	for (i = 1, column = 0, p = Line; i < LINSIZ - 1; i++, p++) {
		*p = c = (*rdchar)(f);
		switch(c) {
		case EOF:
			clearerr(f);
			if (p > Line) {	/* last line doesn't have '\n', */
				*p++ = '\n';
				*p = '\0';	/* print it any way */
				return(column);
			}
			return(EOF);
		case '\n':
			break;
		case '\t': /* just a guess */
			column = 1 + (column | 7);
			break;
		case '\b':
			if (column > 0)
				column--;
			break;
		case '\r':
			column = 0;
			break;
		default:
			if (c >= ' ')
				column++;
			break;
		}
		if (c == '\n') {
			p++;
			break;
		}
		if (column >= columns && !fflag) {
			p++;
			break;
		}
	}
	if (c != '\n') { /* We're stopping in the middle of the line */
		if (column != columns || !auto_right_margin)
			*p++ = '\n';	/* for the display */
		/* peek at the next character */
		c = fgetc(f);
		if (c == '\n') {
			ungetc(c,f);
			c = (*rdchar)(f); /* gobble and copy it */
		}
		else
		if (c == EOF) /* get it next time */
			clearerr(f);
		else
			ungetc(c,f);
	}
	*p = 0;
	return(column);
}

save_input(f)
register FILE f;
{
	if (pipe_in) {
		save_pipe();
		in_file = tmp_fin;
		pipe_in = 0;
	}
	fseek(in_file,0L,0);
	copy_file(in_file,f);
}

save_pipe()
{
	if (!doliseof)
		while (fgetputc(stdin) != EOF)
			if (brk_hit) {
				brk_hit = 0;
				error("Piped input only partially saved");
				break;
			}
	fclose(tmp_fou);
}

fgetputc(f)	/* copy anything read from a pipe to tmp_fou */
register FILE *f;
{
	register int c;
	if ((c = getc(f)) != EOF)
		putc(c,tmp_fou);
	return(c);
}

lineset(how)	/* initialize line memory */
int how;
{
	if (zero == NULL) {
		nlall = 128;
		zero = (LINE *) malloc(nlall * sizeof (LINE));
	}
	dol = contig = zero;
	zero->l_no = 1;
	zero->l_addr = 0l;
	if (how == BOF) {
		dot = zero - 1;
		eoflag = 0;
		doliseof = 0;
		eofl_no = -1;
	}
	else {
		dot = dol;
		eoflag = 1;
		doliseof = 1;
		eofl_no = 1;
	}
	return;
}

newdol(f)	/* add address of new 'dol' */
		/* assumes that f is currently at beginning of said line */
		/* updates dol */
register FILE *f;
{
	register int diff;

	if ((dol - zero) + 1 >= nlall){
		LINE *ozero = zero;

		nlall += 512;
		free ((char *) zero);
		if ((zero = (LINE *) realloc ((char *) zero,
		     (unsigned)(nlall * sizeof(LINE)))) == NULL){
			zero = ozero;
			compact();
		}
		diff = (char *)zero - (char *)ozero;
		dot = (LINE *)((char *)dot + diff);
		dol = (LINE *)((char *)dol + diff);
		contig = (LINE *)((char *)contig + diff);
	}
	dol++;
	if (!pipe_in)
		dol->l_addr = ftell(f);
	else {
		fflush(tmp_fou);
		dol->l_addr = ftell(tmp_fou);
	}
	dol->l_no = (dol-1)->l_no + 1;
}

compact()
{
	fprintf(stderr, "pg: no more memory - line %d\n",dol->l_no);
	end_it();
}

terminit()	/* set up terminal dependencies from termlib */
{
	int err_ret;
	struct termio ntty;

	if ((freopen("/dev/tty","r+",stdout)) == NULL) {
		fprintf(stderr,"pg: cannot reopen stdout\n");
		exit(1);
	}
	setupterm(0,fileno(stdout),&err_ret);
	if (err_ret != 1)
		setupterm("dumb",fileno(stdout),&err_ret);
	if (err_ret != 1) {
		fprintf(stderr,"pg: cannot find terminal type\n");
		exit(1);
	}

	/* there must be a better way using "curses" */
	ioctl(fileno(stdout),TCGETA,&ntty);
	ntty.c_lflag &= ~(ECHONL | ECHO | ICANON);
	ntty.c_cc[VMIN] = 1;
	ntty.c_cc[VTIME] = 1;
	ioctl(fileno(stdout),TCSETA,&ntty);
	saveterm();
	resetterm();
	if (lines <= 0 || hard_copy) {
		hard_copy = 1;
		lines = 24;
	}
	if (columns <= 0)
		columns = 80;
	if (clropt && !clear_screen)
		clropt = 0;
	if ((shell = getenv("SHELL")) == NULL)
			shell = "/bin/sh";
}

error(mess)
char *mess;
{
	kill_line();
	sopr(mess,1);
	prompt((char *) NULL);
	errors++;
}

prompt(filename)
char *filename;
{
	char outstr[PROMPTSIZE+6];
	int pagenum;
	if (filename != NULL) {
		sopr("(Next file: ",1);
		sopr(filename,1);
		sopr(")",1);
	}
	else {
		if ((pagenum=(int)((new_ss.last_line-2)/(window-1)+1))
						> 999999)
			pagenum = 999999;
		sprintf(outstr,promptstr,pagenum);
	}
	sopr(outstr,1);
	fflush(stdout);
}

/*
 *  sopr puts out the message (please no \n's) surrounded by standout
 *  begins and ends
 */

sopr(m,count)
	char *m;
	short count;
{
	if (count)
		promptlen += strlen(m);
	if (soflag && enter_standout_mode && exit_standout_mode) {
		pr(enter_standout_mode);
		pr(m);
		pr(exit_standout_mode);
	}
	else
		pr(m);
}

pr(s)
char	*s;
{
	fputs(s,stdout);
}

doclear()
{
	if (clear_screen)
		putp(clear_screen);
	putchar('\r');  /* this resets the terminal drivers character */
			/* count in case it is trying to expand tabs  */
}

kill_line()
{
	erase_line(0);
	if (!clr_eol) putchar ('\r');
}

/* erase from after col to end of prompt */
erase_line(col)
register int col;
{

	if (promptlen == 0)
		return;
	if (hard_copy)
		putchar('\n');
	else {
		if (col == 0)
			putchar('\r');
		if (clr_eol) {
			putp(clr_eol);
			putchar('\r');  /* for the terminal driver again */
		}
		else
			for (col = promptlen - col; col > 0; col--)
				putchar (' ');
	}
	promptlen = 0;
}

/*
 * Come here if a quit or interrupt signal is received
 */

on_brk(sno)
	int sno;	/* signal number generated */
{
	signal(sno, on_brk);
	if (!inwait) {
		BEEP();
		brk_hit = 1;
	}
	else {
		brk_hit = 0;
		longjmp(restore,1);
	}
}

/*
 * Clean up terminal state and exit.
 */

end_it ()
{

	if (out_is_tty) {
		kill_line();
		resetterm();
	}
	if (tmp_fin)
		fclose(tmp_fin);
	if (tmp_fou)
		fclose(tmp_fou);
	if (tmp_fou || tmp_fin)
		unlink(tmp_name);
	exit(0);
}
