/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)curses:screen/curshdr.h	1.3.1.1"

/*LINTLIBRARY*/

/*
 * IC and IL overheads and costs should be set to this
 * value if the corresponding feature is missing
 */
#define INFINITY 500

struct line
{
	int     hash;		/* hash value for this line, 0 if not known */
	struct line *next;	/* pointer to the next line in list of lines */
	short   bodylen;	/* the cost of redrawing this line */
	short   length;		/* the number of valid characters in line */
	chtype  *body;		/* the actual text of the line */
};

struct costs {
	unsigned ilvar;		/* Insert line varying part * 32 */
	int   ilfixed;		/* Insert line fixed overhead */
	unsigned dlvar;		/* Delete line varying part * 32 */
	int   dlfixed;		/* Delete line fixed overhead */
	unsigned icvar;		/* Insert char varying part * 32 */
	int   icfixed;		/* Insert char fixed overhead */
	unsigned dcvar;		/* Delete char varying part * 32 */
	int   dcfixed;		/* Delete char fixed overhead */
	short Cursor_address;
	short Cursor_home;
	short Carriage_return;
	short Tab;
	short Back_tab;
	short Cursor_left;
	short Cursor_right;
	short Cursor_down;
	short Cursor_up;
	short Parm_left_cursor;
	short Parm_right_cursor;
	short Parm_up_cursor;
	short Parm_down_cursor;
	short Column_address;
	short Row_address;
	short Clr_eol;
	short Clr_bol;
	short Erase_chars;
};
#define _cost(field) (SP->term_costs.field)

struct map {
#ifdef DEBUG
	/* the label is not used for now. */
	char *label;		/* Text the key is labelled with */
#endif /* DEBUG */
	char *sends;		/* Characters sent when key is pressed */
	short keynum;		/* "char" we pass back to program */
};

struct slkdata {
	WINDOW *window;		/* The screen labels window */
	char label[8][9];	/* The soft key-labels */
	char scrlabel[8][9];	/* Labels on the terminal */
	char nblabel[8][9];	/* The values of the labels w/o blanks */
	bool changed[8];	/* Whether label has changed since doupdate */
	short offset[8];	/* Where the label starts on the screen */
	char len;		/* How long labels are */
	bool fl_changed;	/* Whether ANY label has changed */
};

struct screen {
	unsigned fl_nonl	: 1;	/* we shouldn't output \n */
	unsigned fl_uppercase	: 1;	/* upper case terminal */
	unsigned fl_normtty	: 1;	/* currently in normal tty mode */
	unsigned fl_my_term	: 1;	/* user names his terminal type */
	unsigned fl_echoit	: 1;	/* in software echo mode */
	unsigned fl_rawmode	: 1;	/* in raw or cbreak mode */
	unsigned fl_endwin	: 1;	/* has called endwin */
	unsigned phys_irm	: 1;	/* physically in insert char mode */
	unsigned virt_irm	: 1;	/* want to be in insert char mode */
	unsigned fl_nodelay	: 1;	/* tty is in nodelay mode */
	unsigned fl_changed	: 1;	/* if any line has changed */
	unsigned fl_meta	: 1;	/* in meta mode */
	unsigned fl_typeahdok	: 1;	/* ok to use typeahead */
	struct line **cur_body;	/* physical screen image */
	struct line **std_body;	/* desired screen image */
	struct line *freelist;	/* free space list */
	short phys_x;		/* physical cursor X position */
	short phys_y;		/* physical cursor Y position */
	short virt_x;		/* virtual cursor X position */
	short virt_y;		/* virtual cursor Y position */
	short phys_top_mgn;	/* physical top margin of scrolling region */
	short phys_bot_mgn;	/* physical bottom margin of scr. region */
	short des_top_mgn;	/* desired top margin of scr. region */
	short des_bot_mgn;	/* desired bottom margin of scr. region */
	chtype *curptr;		/* pointer to cursor */
	chtype virt_gr;		/* desired highlight state */
	chtype phys_gr;		/* current highlight state */
	short winsize;		/* no. of lines on which ID operations done */
	short doclear;		/* flag to say whether screen garbaged */
	short baud;		/* baud rate of this tty */
	short check_input;	/* flag for input check */
	short check_fd;		/* file descriptor for input check */
	struct _win_st *std_scr;/* primary output screen */
	struct _win_st *cur_scr;/* what's physically on the screen */
	TERMINAL *tcap;		/* TERMINFO info */
	struct costs term_costs;/* costs of various capabilities */
	short kp_state;		/* 1 iff keypad is on, else 0 */
	short ml_above;		/* # memory lines above top of screen */
#ifndef 	NONSTANDARD
	SGTTY	save_tty_buf;	/* current state of this tty */
	FILE *term_file;	/* File to write on for output. */
#endif		/* NONSTANDARD */
	struct map *kp;		/* keypad map */
	bool funckeystarter[0400];/* a map of which keys start fn keys */
#define INP_QSIZE	20
	short *input_queue;	/* a place to put stuff ungetc'ed */
	FILE *input_file;	/* Where to get keyboard input */
	WINDOW *leave_lwin;	/* where to go after doupdate() */
	bool leave_leave;	/* ...	leaveok() */
	bool leave_use_idl;	/* ...	okay to use idl */
	short leave_x;		/* ...	X coord */
	short leave_y;		/* ...	Y coord */
	chtype acsmap[0400];	/* map of alternate char set chars */
	short Yabove;		/* How many lines are above stdscr */
	short Ybelow;		/* How many lines are below stdscr */
	struct slkdata *slk;	/* Soft label information */
	int lastsortedkey;	/* where sorted key list ends */
	short ungotten;		/* how many chars ungotten by ungetch() */
	bool cursorstate;	/* cursor: 0 = invis, 1 = norm, 2 = vvis */
};

#ifndef 	NONSTANDARD
extern struct screen *SP;
#endif

#ifdef DEBUG
# ifndef outf
extern	FILE	*outf;
# endif /* outf */
#endif

extern int _use_slk;
struct _ripdef
    {
    int line;
    int (*initfunction)();
    };
extern struct _ripdef _ripstruct[5];
extern int _ripcounter;

/* terminfo magic number */
#define MAGNUM 0432

/* curses screen dump magic number */
#define DUMP_MAGIC_NUMBER	(MAGNUM+1)

extern char *malloc();
void memSset();
void free();
#ifndef memcpy
char *memcpy();
#endif
void perror();
void exit();
char *strcpy();
char *strcat();
unsigned sleep();

/*
 * Getting the baud rate is different on the two systems.
 * In either case, a baud rate of 0 hangs up the phone.
 * Since things are often initialized to 0, getting the phone
 * hung up on you is a common result of a bug in your program.
 * This is not very friendly, so if the baud rate is 0, we
 * assume we're doing a reset_xx_mode with no def_xx_mode, and
 * just don't do anything.
 */
#ifdef SYSV
# define BR(x) (cur_term->x.c_cflag&CBAUD)
#else
# define BR(x) (cur_term->x.sg_ispeed)
#endif
