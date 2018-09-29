/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/getch.c	1.11.1.1"
# include	"curses.ext"
# include	<signal.h>
# include	<errno.h>
#ifdef DEBUG
# include	<ctype.h>
#endif /* DEBUG */

#if !defined(FIONREAD)
# define TRADITIONAL
static int sig_caught;
#else
# undef TRADITIONAL
#endif /* FIONREAD */

extern int _outch();

/*
 *	This routine reads in a character from the window.
 *
 *	wgetch MUST return an int, not a char, because it can return
 *	things like ERR, meta characters, and function keys > 256.
 */
int
wgetch(win)
register WINDOW	*win;
{
	register int inp;
	bool	weset = FALSE;
	register FILE	*inf;

# ifdef DEBUG
	if(outf) {
		fprintf(outf, "WGETCH: SP->fl_echoit = %c, SP->fl_rawmode = %c\n", SP->fl_echoit ? 'T' : 'F', SP->fl_rawmode ? 'T' : 'F');
		fprintf(outf, "_use_keypad %d, kp_state %d\n", win->_use_keypad, SP->kp_state);
		fprintf(outf, "file %x fd %d\n", SP->input_file, fileno(SP->input_file));
	}
# endif

#ifdef SYSV
	/* Register the fact that getch is being used so */
	/* that typeahead checking can be done. */
	/* This code should GO AWAY when a poll() or FIONREAD can */
	/* be done on the file descriptor as then the check */
	/* will be non-destructive. */
	SP->fl_typeahdok = TRUE;
#endif /* SYSV */

	if (SP->fl_echoit && !SP->fl_rawmode) {
		cbreak();
		weset++;
	}

	/* Make sure keypad on is in proper state */
	if (win->_use_keypad != SP->kp_state) {
		_kpmode(win->_use_keypad);
		__cflush();
	}

	/* Make sure we are in proper nodelay state */
	if (win->_nodelay != SP->fl_nodelay)
		_fixdelay((bool)SP->fl_nodelay, (bool)win->_nodelay);

	inf = SP->input_file;
	if (inf == stdout)	/* so output can be teed somewhere */
		inf = stdin;

	if ((win->_flags & (_WINCHANGED|_WINMOVED)) && !(win->_flags&_ISPAD))
		wrefresh(win);

	/* ask for input */
	if (SP->ungotten > 0) {
		SP->ungotten--;
	} else if (req_for_input) {
		tputs(req_for_input, 1, _outch);
		__cflush();
	}

	/* Only read a character if there is no typeahead. */
	if (SP->input_queue[0] != -1) {
		inp = SP->input_queue[0];
	} else {
		inp = readchar(win, inf);
		if (inp == ERR)
			goto goterr;
		SP->input_queue[0] = inp;
		SP->input_queue[1] = -1;
	}

# ifdef DEBUG
	if(outf) fprintf(outf,"WGETCH got '%s'\n",unctrl(inp));
# endif

	/* Check for arrow and function keys */
	inp = getkey (win, inp, inf);

	/* decode an ungetch()'d character */
	if (inp < 0 && inp != -1)
		inp = -(inp + 0100);

	/* echo the key out to the screen */
	if (SP->fl_echoit &&
	    (inp < 0200) && (inp >= 0) &&
	    !(win->_flags&_ISPAD)) {
		wechochar (win, (chtype) inp);
	}
#ifdef SYSV
	/*
	 * Do nl() mapping. nl() affects both input and output. Since
	 * we turn off input mapping of CR->NL to not affect input
	 * virtualization, we do the mapping here in software.
	 * Note that we can't do this with BSD because there's no way
	 * to separate out input and output CR->NL mapping, which loses
	 * big with input virtualization on terminals whose function
	 * keys transmit a CR as part of the sequence (most HP's).
	 */
	if (inp == '\r' && ((PROGTTY.c_oflag&ONLCR)!=0))
		inp = '\n';
#endif /* SYSV */

goterr:
	if (weset)
		nocbreak();

#ifdef DEBUG
	if (outf) fprintf(outf, "getch returns 0%o (%s), pushed 0%o (%s) 0%o (%s) 0%o (%s)\n", inp, isascii(inp) ? unctrl(inp) : " ", SP->input_queue[0], isascii(SP->input_queue[0]) ? unctrl(SP->input_queue[0]) : " ", SP->input_queue[1], isascii(SP->input_queue[1]) ? unctrl(SP->input_queue[1]) : " ", SP->input_queue[2], isascii(SP->input_queue[2]) ? unctrl(SP->input_queue[2]) : " ");
#endif
	return inp;
}

static int readchar(win, inf)
register WINDOW *win;
register FILE *inf;
{
	register int i;
	char c;

#ifdef FIONREAD
	int arg;

	if (win->_nodelay) {
		(void) ioctl(fileno(inf), FIONREAD, &arg);
#ifdef DEBUG
		if (outf) fprintf(outf, "FIONREAD returns %d\n", arg);
#endif
		if (arg < 1)
			return -1;
	}
#endif

#ifdef FIONREAD
	if (cur_term->timeout > 0) {
		char c;
		int infd;
		infd = 1 << fileno(inf);
		t.tv_sec = 0;
		t.tv_usec = 100 * cur_term->timeout;
		i = select(20, &infd, (int *)NULL, (int *)NULL, &t);
		if (i < 0)
			return ERR;
		i = read(fileno(inf), &c, 1);
	} else
#endif /* !FIONREAD */
	/*
	 * There used to be a useless loop here looping on interrupts.
	 * This prevented the use of signal handlers and was inconsistent
	 * with the way stdio and the original curses worked.
	 */
	i = read(fileno(inf), &c, 1);

#ifdef DEBUG
	if (outf) fprintf(outf, "read from %d returns %d chars, first %o\n", fileno(inf), i, c);
#endif

	if (i > 0) {
		if (!win->_use_meta)
			return ((int) c) & 0177;
		else
			return ((int) c) & 0377;
	} else
		return ERR;
}

#ifdef DEBUG
extern char *_asciify();
#endif /* DEBUG */

/*
    This algorithm is a "learning" algorithm. The premise is
    that keys used once are like to be used again and again.
    Since the time for a linear search of the table is so 
    expensive, we move keys that are found up to the top of 
    the list. The keys so moved are looked up in a binary search 
    the next time, making the access to a repeated key very fast. 
    A second premise is that if one function key is going to be used,
    a number will probably be used. Hence the choice of a binary
    search.
*/

static int
getkey (win, inp, inf)
register WINDOW *win;
register int inp;
FILE *inf;
{
	register struct map *kp;
	register short *inputQ;
	register int i, j, middle, top, bottom, key;

	/* Can it be a function key? */
	if (!win->_use_keypad ||
	    (inp >= 0400) || (inp < 0) ||
	    !SP->funckeystarter[inp]) {
		collapse(1);
		return inp;
	}

	/*
	 * Look the specified escape sequence up in the sorted portion of
	 * the key table. If the key is not found, bottom will be pointing
	 * to the element in the binary list that would be directly above 
	 * the desired escape sequence. This is used to index into the table
	 * to add the key when it is found by the linear search.
	 */

#ifdef DEBUG
	if (outf) fprintf(outf, "getch(): looking in binary table, lastsortedkey=%d.\n", SP->lastsortedkey);
#endif /* DEBUG */

	top = 0;
	bottom = SP->lastsortedkey;
	inputQ = SP->input_queue;
	kp = SP->kp;

#ifdef DEBUG
	if (outf) fprintf(outf, "	top=%d, bottom=%d\n", top, bottom);
#endif /* DEBUG */

	while (top <= bottom) {
		middle = (top + bottom) / 2;

#ifdef DEBUG
	if (outf) fprintf(outf, "	top=%d, middle=%d, bottom=%d\n", top, middle, bottom);
#endif /* DEBUG */

		for (i = 0, j = 1;
		     (i < INP_QSIZE) &&
		     (kp[middle].sends[i] == inputQ[i]);
		     i++, j++) {
#ifdef DEBUG
	if (outf) {
		fprintf(outf, "	kp[middle=%d].sends[i=%d] = SP->input_queue[i=%d] = 0%o(%s)\n", middle, i, i, inputQ[i], unctrl(inputQ[i]));
		fprintf(outf, "	kp[middle=%d].sends[j=%d] = 0%o(%s), SP->input_queue[j=%d] = 0%o\n", middle, j, kp[middle].sends[j], unctrl(kp[middle].sends[j]), j, inputQ[j]);
	}
#endif /* DEBUG */
			/* peek ahead */
			if ((j < INP_QSIZE) &&
			    (inputQ[j] == -1) &&
			    (kp[middle].sends[j] != '\0')) {
				inputQ[j] =
				    win->_notimeout ? _pk(inf) : _fpk(inf);
				if (j < INP_QSIZE-1)
					inputQ[j+1] = -1;
			}
		}

		/* we found it! match only first INP_QSIZE chars */
		if (i >= INP_QSIZE)
			for ( ; kp[middle].sends[i]; i++)
				(void) (win->_notimeout ?
					_pk(inf): _fpk(inf));

		if ((i >= INP_QSIZE) || (kp[middle].sends[i] == '\0')) {
			collapse(i);
			if (SP->ungotten > 0)
				if (SP->ungotten > i-1)
					SP->ungotten -= i-1;
				else
					SP->ungotten = 0;
			return kp[middle].keynum;
		}

		/* kp[middle] < inpQ */
		else if (kp[middle].sends[i] < inputQ[i])
			top = middle + 1;

		/* kp[middle] > inpQ */
		else
			bottom = middle - 1;
	}

	/*
	 * Look in the non-sorted portion of the table for the given
	 * escape sequence. If it is found, add the key to the binary
	 * table. Otherwise, return the first key in the input Q.
	 */

#ifdef DEBUG
	if (outf) fprintf(outf, "getch(): looking in linear table, inp=%d (%c), lastsortedkey=%d.\n", inputQ[0], isascii(inputQ[0]) ? inputQ[0] : ' ', SP->lastsortedkey);
#endif /* DEBUG */

	for (key = SP->lastsortedkey + 1; kp[key].keynum >= 0; key++) {
#ifdef DEBUG
	if (outf) fprintf(outf, "	looking at key %d. sends '%s', value 0%o\n", key, _asciify(kp[key].sends), kp[key].keynum);
#endif /* DEBUG */
		if (kp[key].sends[0] == inp) {
#ifdef DEBUG
	if(outf) fprintf(outf,"matchstart %d '%s'\n", key, _asciify(kp[key].sends));
#endif /* DEBUG */
			for (i = 1; i < (INP_QSIZE-1); i++) {
#ifdef DEBUG
	if (outf) fprintf(outf, "	kp[%d].sends[%d] == 0%o(%s), inputQ[%d] == 0%o\n", key, i, kp[key].sends[i], unctrl(kp[key].sends[i]), i, inputQ[i]);
#endif /* DEBUG */
				/* found it? */
				if (kp[key].sends[i] == '\0')
					break;
				/* partial match? peek ahead. */
				if (inputQ[i] == -1) {
					inputQ[i] =
						win->_notimeout ?
						_pk(inf) : _fpk(inf);
# ifdef DEBUG
	if(outf) fprintf(outf,"notimeout %d, pk returns %o\n", win->_notimeout, SP->input_queue[i]);
# endif
					inputQ[i+1] = -1;
				}
				/* not this one? */
				if (kp[key].sends[i] != inputQ[i])
					goto contouter;
			}
			/* we found it! match only first INP_QSIZE chars */
			if (i >= INP_QSIZE)
				for ( ; kp[key].sends[i]; i++)
					(void) (win->_notimeout ?
						_pk(inf): _fpk(inf));
# ifdef DEBUG
	if(outf) fprintf(outf,"got it! key %o\n", kp[bottom+1].keynum);
# endif
			addtobinary(bottom, key);
			collapse (i);
			if (SP->ungotten > 0)
				if (SP->ungotten > i-1)
					SP->ungotten -= i-1;
				else
					SP->ungotten = 0;
			return kp[bottom+1].keynum;
		}
	contouter: ;
	}

	/* key not found */
# ifdef DEBUG
	if(outf) fprintf(outf,"did not match anything\n");
# endif
	collapse (1);
	return inp;
}


/*
    Collapse the input queue to remove the
    escape sequence from the stack. Also
    guarantee that the Q ends in -1.
*/
collapse (j)
register int j;
{
	register int i = 0, inpQ_j;
	register short *inputQ = SP->input_queue;

	for ( ; (j < INP_QSIZE) && ((inpQ_j = inputQ[j]) != -1) &&
	     (inpQ_j != -2); i++, j++)
		inputQ[i] = inpQ_j;
	inputQ[i] = -1;
}

/*
    addtobinary adds the specified key from the unsorted part of the
    key list to the sorted portion of the list. bkey points to
    the key above where the key is supposed to go. lkey points to the
    key to be moved. We move the key right below the sorted list into the
    vacated space and slip the saved key into the middle of the sorted list.
*/

static addtobinary(bkey, lkey)
register int bkey, lkey;
{
	register struct map *kp = SP->kp;
	register int i;
	register int lastbinkey;
	struct map saveunsortedkey;

	saveunsortedkey = kp[lkey];
	lastbinkey = SP->lastsortedkey++;
	kp[lkey] = kp[lastbinkey+1];

#ifdef DEBUG
	if (outf) fprintf(outf, "getch(): adding key (lkey=%d) to binary table after (bkey=%d), SP->lastsortedkey=%d.\n", lkey, bkey, SP->lastsortedkey);
	if (outf) fprintf(outf, "	saving SP->kp[lkey=%d].\n", lkey);
	if (outf) fprintf(outf, "	moving SP->kp[lastbinkey+1=%d] to SP->kp[lkey=%d].\n", lastbinkey+1, lkey);
#endif /* DEBUG */

	for (i = lastbinkey; i > bkey; i--) {
#ifdef DEBUG
		if (outf) fprintf(outf, "	moving SP->kp[i=%d] to SP->kp[i+1=%d].\n", i, i+1);
#endif /* DEBUG */
		kp[i+1] = kp[i];
	}
#ifdef DEBUG
	if (outf) fprintf(outf, "	placing saved key in SP->bkey[bkey+1=%d].\n", bkey+1);
#endif /* DEBUG */

	kp[bkey+1] = saveunsortedkey;
}

/*
 * Fast peek key.  Like getchar but if the right flags are set, times out
 * quickly if there is nothing waiting, returning -1.
 * f is an output stdio descriptor, we read from the fileno.  win is the
 * window this is supposed to have something to do with.
 * We wait for long enough for a terminal to send another character
 * (at 15cps repeat rate, this is 67 ms, I'm using 100ms to allow
 * a bit of a fudge factor) and time out more quickly.
 * -2 is returned if we time out, -1 is returned if interrupted, and the
 * character is returned otherwise.
 */
#ifndef FIONREAD

/*
 * Traditional implementation.  The best resolution we have is 1 second,
 * so we set a 1 second alarm and try to read.  If we fail for 1 second,
 * we assume there is no key waiting.  Problem here is that 1 second is
 * too long; people can type faster than this.
 */

_catch_alarm()
{
	sig_caught = 1;
}

static
_fpk(f)
FILE *f;
{
	char c;
	int rc;
#ifdef SIGPOLL	/* Vr3 and beyond */
	void (*oldsig)();
#else
	int (*oldsig)();
#endif /* SIGPOLL */
	unsigned int oldalarm, alarm();

	/* turn off any user alarms and set our own */
	oldalarm = alarm(0);
	sig_caught = 0;
	oldsig = signal(SIGALRM, _catch_alarm);
	(void) alarm(1);
	rc = read(fileno(f), &c, 1);
	(void) alarm(0);
#ifdef SYSV
	/* This code is to take care of the possibility of */
	/* the process getting swapped out in the middle of */
	/* read() call above. The interrupt will cause the */
	/* read() call to return, even if a character is really */
	/* on the clist. So we do a non-blocking read() to make */
	/* sure that there really isn't a character there. */
	if (sig_caught && rc != 1)
		if (SP->check_fd != -1)
			rc = read(SP->check_fd, &c, 1);
		else {
#			include <fcntl.h>
			register int fcflags = fcntl(fileno(f), F_GETFL);
			(void) fcntl(fileno(f), F_SETFL, fcflags | O_NDELAY);
			rc = read(fileno(f), &c, 1);
			(void) fcntl(fileno(f), F_SETFL, fcflags);
		}
#endif /* SYSV */
	/* restore the user alarms */
	(void) signal(SIGALRM, oldsig);
	if (sig_caught && oldalarm > 1)
		oldalarm--;
	(void) alarm(oldalarm);
	if (rc == 1)			/* got a character */
		return c;
	else if (sig_caught)		/* timed out */
		return -2;
	else				/* EOF or got interrupted */
		return -1;
}
#else /* FIONREAD */
/*
 * If we have the select system call, we can do much better than the
 * traditional method. Even if we don't have the real 4.2BSD select, we
 * can emulate it with napms and FIONREAD.  napms might be done with only
 * 1 second resolution, but this is no worse than what we have in the
 * traditional implementation.
 */
static
_fpk(f)
FILE *f;
{
	int infd, rc;
	int *outfd, *exfd;
	char c;
	struct _timeval {
		long tv_sec;
		long tv_usec;
	} t;

	infd = 1 << fileno(f);
	outfd = exfd = (int *) NULL;
	t.tv_sec = 0;
	t.tv_usec = 100000;
	rc = select(20, &infd, outfd, exfd, &t);
	if (rc < 0)
		return -2;
	rc = read(fileno(f), &c, 1);
	return rc == 1 ? c : -1;
}
#endif /* FIONREAD */

/*
 * Plain peekchar function.  Nothing fancy.  This is just like _fpk
 * but will wait forever rather than time out.
 */
static
_pk(f)
FILE *f;
{
	char c;
	int rc;

	rc = read(fileno(f), &c, 1);
	return rc == 1 ? c : -1;
}
