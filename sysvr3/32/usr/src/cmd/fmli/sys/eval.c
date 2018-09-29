/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm.sys:src/sys/eval.c	1.7"

#include	<stdio.h>
#include	<ctype.h>
#include	"wish.h"
#include	"eval.h"
#include	"terror.h"

#define MAXARGS		64

#define IN_DQ		1
#define IN_SQ		2
#define IN_BQ		4
#define IN_SQUIG	8
#define FROM_BQ		16

/*
 * NOTE: increment should match value of INCREMENT in oh/io.c
 * (should eventually be placed in a header file or removed).
 */
#define INCREMENT	1024

/*
 * list of "special" characters, and flags in which they are not
 * treated as special
 */
static char	spchars[] = "\"'\\`$\n \t{}|&;<>";
static int	nflags[] = {
	FROM_BQ | IN_SQ,		/* double quote */
	FROM_BQ | IN_DQ,		/* single quote */
	FROM_BQ,			/* backslash    */
	FROM_BQ | IN_SQ,		/* back quote   */
	FROM_BQ | IN_SQ,		/* dollar sign  */
	IN_SQUIG | IN_SQ | IN_DQ,	/* new line     */
	FROM_BQ | IN_SQ | IN_DQ | EV_GROUP,		/* space        */
	FROM_BQ | IN_SQ | IN_DQ | EV_GROUP,		/* tab          */
	FROM_BQ | IN_SQUIG | IN_SQ | IN_DQ | IN_BQ,	/* open squig   */
	FROM_BQ | IN_SQ | IN_DQ | IN_BQ,		/* close squig  */
	FROM_BQ | IN_SQ | IN_DQ | EV_GROUP,	/* pipe symbol  */
	FROM_BQ | IN_SQ | IN_DQ | EV_GROUP,	/* ampersand    */
	FROM_BQ | IN_SQ | IN_DQ | EV_GROUP,	/* semicolon    */
	FROM_BQ | IN_SQ | IN_DQ | EV_GROUP,	/* less than    */
	FROM_BQ | IN_SQ | IN_DQ | EV_GROUP,	/* greater than */
};

/* return code from most recently executed command */
int	EV_retcode;
int	EV_backquotes;
int	Lasttok;

eval(instr, outstr, flags)
IOSTRUCT	*instr;
IOSTRUCT	*outstr;
int	flags;
{
	register int	c;
	register int	tok;
	bool	done;
	bool	first;

	if (!(flags & IN_BQ))
		EV_retcode = 0;
	EV_backquotes = FALSE;
	c = getac(instr);
	/* skip leading white space */
	if (flags & (EV_TOKEN | EV_GROUP)) {
		while (isspace(c))
			c = getac(instr);
		if (c == '#') {
			/*
			 * skip everything until end of line
			 */
			while ((c = getac(instr)) && c != '\n' && c != EOF)
				;
		}
	}
	/* handler `` at beginning of line if in GROUP mode */
	if ((flags & EV_GROUP) && c == '`') {
		eval_backquote(instr, outstr, flags);
		io_flags(instr, io_flags(instr, 0) & ~FROM_BQ);
		c = getac(instr);
	}
#ifdef _DEBUG2
	if ((flags & EV_TOKEN) && (instr->flags & EV_USE_STRING)) {
		_debug2(stderr, "input is '%.*s'\n", instr->mu.str.count - instr->mu.str.pos, instr->mu.str.val +  instr->mu.str.pos);
	}
#endif
	Lasttok = tok = ET_EOF;
	for (done = FALSE; c; c = getac(instr)) {
		register char	*p;
		char	*strchr();

		while (!(p = strchr(spchars, c))) {
			Lasttok = ET_WORD;
			putac(c, outstr);
			if (!(c = getac(instr))) {
				done = TRUE;
				break;
			}
		}
		if (done)
			break;
		/* single | and & are correct here */
		if ((instr->flags | flags) & nflags[tok = p - spchars])
			tok = !!c;
		else {
			tok += ET_DQUOTE;
#ifdef _DEBUG2
			_debug2(stderr, "eval: got special char 0x%x\n", tok);
#endif
		}
		switch (tok) {
		case ET_EOF:
			done = TRUE;
			break;
		case ET_WORD:
			Lasttok = tok;
			putac(c, outstr);
			break;
		case ET_DQUOTE:
			flags ^= IN_DQ;
			if (!(flags & EV_TOKEN))
				putac(c, outstr);
			break;
		case ET_SQUOTE:
			flags ^= IN_SQ;
			if (!(flags & EV_TOKEN))
				putac(c, outstr);
			break;
		case ET_BSLASH:
			c = getac(instr);
			/*
			 * if (not tokenizing or if we're in quotes and the
			 *	next character is not special, leave backslash
			 *	there
			 * else
			 *	remove it (don't copy to output)
			 */
			if (!(flags & EV_TOKEN) || (flags & (IN_SQ | IN_DQ)) && (!(p = strchr(spchars, c)) || (instr->flags | flags) & nflags[p - spchars]))
				putac('\\', outstr);
			putac(c, outstr);
			break;
		case ET_BQUOTE:
			if (flags & EV_TOKEN) {
				if (flags & IN_BQ) {
					if (Lasttok == ET_EOF) {
						putac(c, outstr);
						c = getac(instr);
						Lasttok = tok;
					}
					done = TRUE;
				}
				else
					eval_backquote(instr, outstr, flags);
			}
			else {
				flags ^= IN_BQ;
				putac(c, outstr);
			}
			EV_backquotes = TRUE;
			break;
		case ET_DOLLAR:
			if (flags & EV_TOKEN)
				eval_dollar(instr, outstr, flags);
			else
				putac(c, outstr);
			break;
		case ET_NEWLINE:
		case ET_SPACE:
		case ET_TAB:
			Lasttok = ET_WORD;
			if ((flags & EV_GROUP) && (flags & IN_BQ))
				putac(c, outstr);
			else
				done = TRUE;
			break;
		case ET_OSQUIG:
		case ET_CSQUIG:
			putac(c, outstr);
			if (flags & EV_GROUP)
				flags ^= IN_SQUIG;
			else if (flags & EV_TOKEN) {
				if (Lasttok == ET_EOF) {
					c = getac(instr);
					Lasttok = tok;
				}
				done = TRUE;
			}
			break;
		case ET_PIPE:
		case ET_AMPERSAND:
		case ET_SEMI:
		case ET_LTHAN:
		case ET_GTHAN:
			if (flags & IN_BQ) {
				if (Lasttok == ET_EOF) {
					register int	oldc;

					putac(c, outstr);
					oldc = c;
					if ((c = getac(instr)) == oldc) {
						putac(c, outstr);
						c = getac(instr);
						tok += DOUBLE;
					}
					Lasttok = tok;
				}
				done = TRUE;
			}
			else
				putac(c, outstr);
			break;
		}
		if (done)
			break;
		Lasttok = ET_WORD;
	}
	if (c)
		ungetac(c, instr);
#ifdef _DEBUG2
	if (flags & EV_TOKEN) {
		_debug2(stderr, "eval -> '%s'\n", io_ret_string(outstr));
		_debug2(stderr, "eval returning 0x%x\n", Lasttok);
	}
#endif
	return Lasttok;
}

/*ARGSUSED*/
static
eval_dollar(instr, outstr, flags)
IOSTRUCT	*instr;
IOSTRUCT	*outstr;
int	flags;
{
	register char	*p;
	register int	c;
	register IOSTRUCT	*iop;
	char	*expand();

	iop = io_open(EV_USE_STRING, NULL);
	putac('$', iop);
	if ((c = getac(instr)) == '{') {
		while (c != '}') {
			putac(c, iop);
			c = getac(instr);
		}
		putac(c, iop);
	}
	else {
		while (isalpha(c) || isdigit(c) || c == '_') {
			putac(c, iop);
			c = getac(instr);
		}
		if (c)
			ungetac(c, instr);
	}
	if (p = expand(io_ret_string(iop))) {
		/*
		 * This check should not be necessary but putastr()
		 * does not seem to like "reused" io buffers with
		 * large amounts of text (see oh/io.c).
		 */
		if (strlen(p) >= INCREMENT) {
			io_close(iop);
			iop = io_open(EV_USE_STRING, NULL);
		} 
		else
			io_clear(iop);		/* clear (reuse) iop */
		putastr(p, iop);
		free(p);
		io_seek(iop, 0);
		io_push(instr, iop);
		return SUCCESS;
	}
	return FAIL;
}

static char *
eval_token(instr, flags)
IOSTRUCT	*instr;
int	flags;
{
	register char	*p;
	static IOSTRUCT	*tmp;

	if (instr == NULL) {
		io_close(tmp);
		return NULL;
	}
	if (tmp == NULL)
		tmp = io_open(EV_USE_STRING, NULL);
	(void) eval(instr, tmp, flags);
	p = io_string(tmp);
	io_seek(tmp, 0);
	return p;
}

static
eval_backquote(instr, outstr, flags)
IOSTRUCT	*instr;
IOSTRUCT	*outstr;
int	flags;
{
	int	argc;
	char	*argv[MAXARGS];
	bool	doit;
	bool	skip;
	bool	piped;
	bool	special;
	IOSTRUCT	*mystdin;
	IOSTRUCT	*mystdout;
	IOSTRUCT	*altstdout;

#ifdef _DEBUG2
	_debug2(stderr, "eval_backquote\n");
#endif
	mystdin = io_open(EV_USE_STRING, NULL);
	mystdout = io_open(EV_USE_STRING, NULL);
	altstdout = NULL;
	doit = skip = piped = special = FALSE;
	for (argc = 0; ; ) {
		argv[argc++] = eval_token(instr, EV_TOKEN | IN_BQ);
		switch (Lasttok) {
		case ET_EOF:
		case ET_BQUOTE:
			special = doit = TRUE;
			break;
		case ET_PIPE:
			{
				register FILE	*fp;
				FILE	*tempfile();

				if (altstdout) {
#ifdef _DEBUG2
					_debug2(stderr, "PIPE and > in same eval command\n");
#endif
					io_close(altstdout);
					altstdout = NULL;
				}
				if (fp = tempfile(NULL, "w+"))
					altstdout = io_open(EV_USE_FP, fp);
				special = doit = piped = TRUE;
			}
			break;
		case ET_AMPERSAND:
			break;
		case ET_SEMI:
			special = doit = TRUE;
			break;
		case ET_LTHAN:
			{
				register FILE	*fp;
				register char	*p;

				special = TRUE;
				p = eval_token(instr, EV_TOKEN | IN_BQ);
				if (fp = fopen(p, "r")) {
					io_close(mystdin);
					mystdin = io_open(EV_USE_FP, fp);
				}
				else
					warn(NOPEN, p);
				free(p);
			}
			break;
		case ET_GTHAN:
			{
				register FILE	*fp;
				register char	*p;

				special = TRUE;
				if (altstdout) {
#ifdef _DEBUG2
					_debug2(stderr, "2 >'s in eval command\n");
#endif
					io_close(altstdout);
				}
				p = eval_token(instr, EV_TOKEN | IN_BQ);
				if (fp = fopen(p, "w"))
					altstdout = io_open(EV_USE_FP, fp);
				else
					warn(NOPEN, p);
				free(p);
			}
			break;
		/* OR symbol */
		case ET_PIPE + DOUBLE:
			special = doit = TRUE;
			break;
		/* AND symbol */
		case ET_AMPERSAND + DOUBLE:
			special = doit = TRUE;
			break;
		/* semicolon (twice in a row) */
		case ET_SEMI + DOUBLE:
			special = doit = TRUE;
			break;
		/* here document */
		case ET_LTHAN + DOUBLE:
			break;
		/* append symbol */
		case ET_GTHAN + DOUBLE:
			break;
		}
		if (special) {
			free(argv[--argc]);
			special = FALSE;
		}
		if (doit || argc >= MAXARGS - 1) {
			register int	n;

			doit = FALSE;
			if (!skip) {
				argv[argc] = NULL;
				EV_retcode = evalargv(argc, argv, mystdin, altstdout ? altstdout : mystdout);
			}
			skip = (EV_retcode && Lasttok == ET_AMPERSAND + DOUBLE)
				|| (!EV_retcode && Lasttok == ET_PIPE + DOUBLE);
			for (n = 0; n < argc; n++)
				free(argv[n]);
			argc = 0;
			io_close(mystdin);
			if (piped) {
				mystdin = altstdout;
				io_seek(mystdin, 0);
				piped = FALSE;
			}
			else {
				mystdin = io_open(EV_USE_STRING, NULL);
				if (altstdout)
					io_close(altstdout);
			}
			altstdout = NULL;
			if (Lasttok == ET_EOF || Lasttok == ET_BQUOTE)
				break;
		}
	}
	if ((argc = unputac(mystdout)) && argc != '\n')
		putac(argc, mystdout);
	if (flags & EV_GROUP)
		putac('\n', mystdout);
	io_seek(mystdout, 0);
	io_flags(mystdout, io_flags(mystdout, 0) | FROM_BQ);
	io_push(instr, mystdout);
}
