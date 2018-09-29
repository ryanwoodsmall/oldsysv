/* Copyright (c) 1979 Regents of the University of California */
#include "ex.h"
#include "ex_re.h"
#include "ex_tty.h"
#include "ex_vis.h"

/*
 * Entry points to open and visual from command mode processor.
 * The open/visual code breaks down roughly as follows:
 *
 * ex_v.c	entry points, checking of terminal characteristics
 *
 * ex_vadj.c	logical screen control, use of intelligent operations
 *		insert/delete line and coordination with screen image;
 *		updating of screen after changes.
 *
 * ex_vget.c	input of single keys and reading of input lines
 *		from the echo area, handling of \ escapes on input for
 *		uppercase only terminals, handling of memory for repeated
 *		commands and small saved texts from inserts and partline
 *		deletes, notification of multi line changes in the echo
 *		area.
 *
 * ex_vmain.c	main command decoding, some command processing.
 *
 * ex_voperate.c   decoding of operator/operand sequences and
 *		contextual scans, implementation of word motions.
 *
 * ex_vops.c	major operator interfaces, undos, motions, deletes,
 *		changes, opening new lines, shifts, replacements and yanks
 *		coordinating logical and physical changes.
 *
 * ex_vops2.c	subroutines for operator interfaces in ex_vops.c,
 *		insert mode, read input line processing at lowest level.
 *
 * ex_vops3.c	structured motion definitions of ( ) { } and [ ] operators,
 *		indent for lisp routines, () and {} balancing. 
 *
 * ex_vput.c	output routines, clearing, physical mapping of logical cursor
 *		positioning, cursor motions, handling of insert character
 *		and delete character functions of intelligent and unintelligent
 *		terminals, visual mode tracing routines (for debugging),
 *		control of screen image and its updating.
 *
 * ex_vwind.c	window level control of display, forward and backward rolls,
 *		absolute motions, contextual displays, line depth determination
 */

/*
 * Enter open mode
 */
oop()
{
	register char *ic;
	char atube[TUBESIZE + LBSIZE];
	register ttymode f;

#ifndef OPENCODE
	error("Open mode not available@Use visual or command mode");
#else
	ovbeg();
	if (peekchar() == '/') {
		ignore(compile(getchar(), 1));
		savere(scanre);
		if (execute(0, dot) == 0)
			error("Fail|Pattern not found on addressed line");
		ic = loc1;
		if (ic > linebuf && *ic == 0)
			ic--;
	} else {
		getDOT();
		ic = vskipwh(linebuf);
	}
	newline();

	/*
	 * If overstrike then have to HARDOPEN
	 * else if can move cursor up off current line can use CRTOPEN (~~vi1)
	 * otherwise (ugh) have to use ONEOPEN (like adm3)
	 */
	if (OS && !EO)
		bastate = HARDOPEN;
	else if (CA || UP)
		bastate = CRTOPEN;
	else
		bastate = ONEOPEN;
	setwind();

	/*
	 * To avoid bombing on glass-crt's when the line is too long
	 * pretend that such terminals are 160 columns wide.
	 * If a line is too wide for display, we will dynamically
	 * switch to hardcopy open mode.
	 */
	if (state != CRTOPEN)
		WCOLS = TUBECOLS;
	if (!inglobal)
		savevis();
	vok(atube);
	if (state != CRTOPEN)
		COLUMNS = WCOLS;
	Outchar = vputchar;
	f = ostart();
	if (state == CRTOPEN) {
		if (outcol == UKCOL)
			outcol = 0;
		vmoveitup(1, 1);
	} else
		outline = destline = WBOT;
	vshow(dot, NOLINE);
	vnline(ic);
	vmain();
	if (state != CRTOPEN)
		vclean();
	Command = "open";
	ovend(f);
#endif
}

ovbeg()
{

	if (!value(OPEN))
		error("Can't use open/visual unless open option is set");
	if (inopen)
		error("Recursive open/visual not allowed");
	Vlines = lineDOL();
	fixzero();
	setdot();
	pastwh();
	dot = addr2;
}

ovend(f)
	ttymode f;
{

	splitw++;
	vgoto(WECHO, 0);
	vclreol();
	vgoto(WECHO, 0);
	holdcm = 0;
	splitw = 0;
	ostop(f);
	setoutt();
	undvis();
	COLUMNS = OCOLUMNS;
	inopen = 0;
	flusho();
	netchHAD(Vlines);
}

/*
 * Enter visual mode
 */
vop()
{
	register int c;
/*
 * atube should be automatic to save space for users of ex.  However,
 * some versions of Unix have a bug that causes a core dump eventually
 * if a stackframe is too big.  Such systems can define SGROWBUG to
 * get around this problem.
 */
#ifdef SGROWBUG
	static
#endif
	char atube[TUBESIZE + LBSIZE];
	register ttymode f;

	if (!CA && UP == NOSTR) {
#ifdef OPENCODE
		if (initev) {
toopen:
			merror("[Using open mode]");
			putNFL();
			oop();
			return;
		}
#endif
		error("Visual needs addressible cursor or upline capability");
	}
	if (OS && !EO) {
#ifdef OPENCODE
		if (initev)
			goto toopen;
#endif
		error("Can't use visual on a terminal which overstrikes");
	}
	if (!CL) {
#ifdef OPENCODE
		if (initev)
			goto toopen;
#endif
		error("Visual requires clear screen capability");
#ifdef CRNL
		if (NS && !SF) {
#ifdef OPENCODE
			if (initev)
				goto toopen;
#endif
			error("Visual requires scrolling");
		}
#endif
	}
	ovbeg();
#ifdef OPENCODE
	bastate = VISUAL;
#endif
	c = 0;
	if (any(peekchar(), "+-^."))
		c = getchar();
	pastwh();
	vsetsiz(isdigit(peekchar()) ? getnum() : value(WINDOW));
	setwind();
	newline();
	vok(atube);
	if (!inglobal)
		savevis();
	Outchar = vputchar;
	vmoving = 0;
	f = ostart();
	if (initev == 0) {
		vcontext(dot, c);
		vnline(NOSTR);
	}
	vmain();
	Command = "visual";
	ovend(f);
}

/*
 * Hack to allow entry to visual with
 * empty buffer since routines internally
 * demand at least one line.
 */
fixzero()
{

	if (dol == zero) {
		register bool ochng = chng;

		vdoappend("");
		if (!ochng)
			sync();
		addr1 = addr2 = one;
	} else if (addr2 == zero)
		addr2 = one;
}

/*
 * Save lines before visual between unddol and truedol.
 * Accomplish this by throwing away current [unddol,truedol]
 * and then saving all the lines in the buffer and moving
 * unddol back to dol.  Don't do this if in a global.
 *
 * If you do
 *	g/xxx/vi.
 * and then do a
 *	:e xxxx
 * at some point, and then quit from the visual and undo
 * you get the old file back.  Somewhat weird.
 */
savevis()
{

	if (inglobal)
		return;
	truedol = unddol;
	saveall();
	unddol = dol;
	undkind = UNDNONE;
}

/*
 * Restore a sensible state after a visual/open, moving the saved
 * stuff back to [unddol,dol], and killing the partial line kill indicators.
 */
undvis()
{

	if (ruptible)
		signal(SIGINT, onintr);
	squish();
	pkill[0] = pkill[1] = 0;
	unddol = truedol;
	unddel = zero;
	undap1 = one;
	undap2 = dol + 1;
	undkind = UNDALL;
}

/*
 * Set the window parameters based on the base state bastate
 * and the available buffer space.
 */
setwind()
{

	WCOLS = COLUMNS;
#ifdef OPENCODE
	switch (bastate) {

	case ONEOPEN:
		if (AM)
			WCOLS--;
		/* fall into ... */

	case HARDOPEN:
		basWTOP = WTOP = WBOT = WECHO = 0;
		ZERO = 0;
		holdcm++;
		break;

	case CRTOPEN:
		basWTOP = LINES - 2;
		/* fall into */

	case VISUAL:
#endif
		ZERO = LINES - TUBESIZE / WCOLS;
		if (ZERO < 0)
			ZERO = 0;
		if (ZERO > basWTOP)
			error("Screen too large for internal buffer");
		WTOP = basWTOP; WBOT = LINES - 2; WECHO = LINES - 1;
#ifdef OPENCODE
		break;
	}
	state = bastate;
#endif
	basWLINES = WLINES = WBOT - WTOP + 1;
}

/*
 * Can we hack an open/visual on this terminal?
 * If so, then divide the screen buffer up into lines,
 * and initialize a bunch of state variables before we start.
 */
vok(atube)
	register char *atube;
{
	register int i;

	if (WCOLS == 1000)
		serror("Don't know enough about your terminal to use %s", Command);
	if (WCOLS > TUBECOLS)
		error("Terminal too wide");
	if (WLINES >= TUBELINES || WCOLS * (WECHO - ZERO + 1) > TUBESIZE)
		error("Screen too large");

	vtube0 = atube;
	vclrbyte(atube, WCOLS * (WECHO - ZERO + 1));
	for (i = 0; i < ZERO; i++)
		vtube[i] = (char *) 0;
	for (; i <= WECHO; i++)
		vtube[i] = atube, atube += WCOLS;
	for (; i < TUBELINES; i++)
		vtube[i] = (char *) 0;
	vutmp = atube;
	vundkind = VNONE;
	vUNDdot = 0;
	OCOLUMNS = COLUMNS;
	inopen = 1;
#ifdef CBREAK
	signal(SIGINT, vintr);
#endif
	vmoving = 0;
	splitw = 0;
	doomed = 0;
	holdupd = 0;
	Peekkey = 0;
	vcnt = vcline = 0;
	if (vSCROLL == 0)
		vSCROLL = (value(WINDOW)+1)/2;	/* round up so dft=6,11 */
}

#ifdef CBREAK
vintr()
{

	signal(SIGINT, vintr);
	if (vcatch)
		onintr();
	ungetkey(ATTN);
	draino();
}
#endif

/*
 * Set the size of the screen to size lines, to take effect the
 * next time the screen is redrawn.
 */
vsetsiz(size)
	int size;
{
	register int b;

#ifdef OPENCODE
	if (bastate != VISUAL)
		return;
#endif
	b = LINES - 1 - size;
	if (b >= LINES - 1)
		b = LINES - 2;
	if (b < 0)
		b = 0;
	basWTOP = b;
	basWLINES = WBOT - b + 1;
}
