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
	/* @(#) head.h: 1.8 5/17/83 */

/*  
 *  Prefixed all definitions with 'extern'
 *  and put actual space allocations in udef.c
 */
#include <setjmp.h>
#include "bio.h"
#include "mode.h"
#include "machdep.h"
#include <stdio.h>
#include <termio.h>

#define FDTTY	1	/*  file desc for saving/restoring tty state */
#define FDIN	0	/*  for fcntl save/restore */
#define FPRT1	stdout		/*  sdb fprint error output */
#define FPRT2	stderr		/*  debugging fprint output */
extern FILE	*inputfile;	/*  file for sdb commands - stdin default */
/* access modes */
#define RD	0	/* read */
#define WT	1	/* write */

/* access spaces */
#define NSP	0	/*  no space ?? */
#define	ISP	1	/*  instruction (text) space */
#define	DSP	2	/*  data space  */

/* breakpoint flags;  any non-zero value means breakpoint in code */
#define BKPTSET	1	/*  breakpoint set (code overwritten) */
#define BKPTEXEC 2	/*  must execute breakpoint		*/
			/*    (single step through instruction	*/
			/*    at which breakpoint was set)	*/

/* input line decoding */
extern char	proc[];		/* procedure name */
extern long	integ;		/* count or number in input */
extern int	integ2;		/* count or number in input following comma */
extern char 	cmd;		/* command letter */
extern char	re[];		/* regular expression */
extern char	args[];		/* arguments */
extern char	*argsp;		/* pointer to args */
extern char	var[];		/* variable name */
extern int	scallf;		/* set to 1 iff procedure call */
extern int	reflag;		/* set to 1 iff re */
extern int	redir;		/* set to 1 iff forward search */
extern int	colonflag;	/* set to 1 iff colon typed */
extern int	ncolonflag;	/* set to 1 iff colon typed after number */
extern int	percentflag;	/* set to 1 iff percent symbol typed */

/* types of values (INTEGS can be extended chars or shorts)
 * the numbering is crucial for the macro.
 */
enum valtype {INTEG=0, FLOAT=1, DBL=2};
#define TYPETYPE(x,y)	(int)(x)*3 + (int)(y)  /* pair of types -> unique no. */

/* value to be assigned ('!' command) decoding */
struct rhs_struct {
	union dbl val;		/* actual value */
	enum valtype tag;	/* tag identifying type of value */
};

extern struct rhs_struct rhs_value;

/* source file i/o */
extern char	curfile[];	/* name of file being edited */
struct stmt {
	int lnno;		/* line number */
	int stno;		/* statement number in line */
};
extern struct stmt curstmt;	/* current statement in file */
extern int	maxfline;	/* maximum line number in file, 0 if unknown */
extern struct brbuf  fiobuf;	/* file descriptor */
extern char	fbuf[];		/* current line from file */
extern char 	filework[];	/* place to put filename */
extern	char	sdbpath[];	/* Directory lookup path array for files. */
extern	int	sdbpsz;		/* Sizeof of the sdb path lookup char array. */
extern char 	*fwp;		/* pointer to it (or within it -- see main) */
extern int	nolines;	/* set to 1 iff no lines in file */

/* returned by slookup */
extern char	sl_name[];
extern char	sl_class;
extern ushort	sl_type;
extern ushort	sl_stype;	/* simple type, i.e. type of dereferenced var */
extern int	sl_size, sl_addr;
extern short	sl_scnum;
extern struct proct *sl_procp;
extern ushort	sl_dimen[];
extern int	subflag;

/* procedure call information */
extern int scallx;			/* procedure call in progress */
extern ADDR fps, aps, pcs;		/* old stack frame */
extern BKPTR bkpts;			/* old breakpoint */
extern int flagss;			/*  and its flags */
extern char dschar;			/* '/' if value should be displayed */
extern ADDR adsubn, adsubc;		/* addresses of __dbsubn, __dbsubc */
extern int adargs;			/* address of __dbargs */

/* symbol table info */
extern long	ststart;		/* offset of symbol table in a.out */
extern struct brbuf sbuf;		/* buffer for symbol table entries */
					/*    also for line number entries */
extern long	extstart;		/* offset of first external in a.out */
extern	char	*strtab;		/* String table array. */

/* address info */
extern ADDR	dot;			/* current address */

/* setup information */
extern STRING	symfil, corfil;		/* file names */
extern INT	fsym, fcor;		/* file descriptors */
extern INT	fakecor;		/* 1 iff user "core" not really core */
extern MAP	txtmap, datmap;		/* maps */
extern INT	argcount;		/* number of arguments to sdb */

/* process info */
extern L_INT	cntval;			/* count for c and s instructions */
extern INT	signo;			/* returned signal */
extern INT	adrflg;			/* new address for doexec */
extern ADDR	userpc;			/* program counter in user program */
extern INT	pid;			/* process id of user program */
extern BKPTR	bkpthead;		/* first breakpoint */

/* other */
extern int	sflag;			/* option -s specified */
extern int	Wflag;			/* option -W (no warnings) specified */
#if DEBUG > 1
extern int	debugflag;		/* option -d specified */
#endif
extern STRING	errflg;			/* 0 or pointer to error message */
extern char	odesc[];		/* descriptor of last displayed var */
extern ADDR	oaddr;			/* address of last displayed variable */
extern char	otype;			/* type of last displayed variable */
extern char	oclass;			/* class of last displayed variable */
extern short	oincr;			/* size of last displayed variable */
extern struct termio sdbttym, usrttym;	/* tty structure(s) */
extern int sdbttyf, usrttyf;			/* fcntl flags */
enum lastcom {NOCOM, PRCOM, DSCOM, DSICOM};
/* last command: no repeat, print source, display variable, display instr */
extern enum lastcom lastcom;

extern char	oldargs[];
extern char	prname[];		/* print name used by outvar */
extern jmp_buf	env;			/* environment for setjmp, longjmp */
extern int	debug;			/* toggled by Y command */
extern int	vmode;			/*  verbose; toggled by 'v' cmnd */
extern time_t	symtime;		/* modification time of symfil */
extern ADDR	exactaddr, lnfaddr;	/* set by adrtolineno() */
extern ADDR	firstdata;		/* lowest address of data */

#define	WINDOW	10		/* window size for display commands */
#define	COMMANDS	"\004\"+-=!</?BCDIMQRSTVXYabcdegiklmpqrstvwxz"
				/* each sdb command must appear here */
#define SUBCMDS		"\004\"+-=</?BCDIMQRSTVXYabcdegiklmpqrstvwxz"
				/* all commands which cannot have ':' in arg */
#define NUMARGS 16		/* number of args allowed in sub call */
#define SUBSTSP 512		/* length of space for sub args and strings */

#define F77		0	/* value for file type flag in struct filet */
#define OTHERTYPE	1	/* default value for file type flag */

struct filet {
	char	sfilename[32];	/* source file name */
	char	lineflag;	/* set iff this is a '#line' file */
	ADDR	faddr;		/* address in core */
	long	stf_offset;	/* offset in a.out */
	long	f_statics;	/* offset in a.out for statics; for coff */
	int	f_type;		/* type of file (F77 or OTHER) */
};
extern struct filet *files, *badfile;

struct proct {
	char	*pname;		/* procedure name */
	ADDR	paddress;	/* address in core */
	long	st_offset;	/* offset in a.out */
	struct filet  *sfptr;	/* source file name pointer */
	ushort	lineno;		/* line number in source file: for '{' on 3B */
	ushort	ef_line;	/* line number in source file: for '}' on 3B */
	char	notstab;	/* 1 if not a stab entry */
	char	entrypt;	/* 1 iff a F77 entry */
};
extern struct proct *procs, *badproc;

extern INT		mkfault;
extern INT		executing;
extern ADDR		sigint;
extern ADDR		sigqit;
extern CHAR		lastc;

#define PROCINCR 20
#define FILEINCR 10

#define ERROR	(-1)		/*  an illegal address (don't change) */
#define TRUE	(-1)		/*  taken from mac.h in adb		  */
#define FALSE	  0

#define smplchar(x)	((x>='A' && x<='Z') || \
			(x>='a' &&  x<='z') || \
			x == '_' || \
			x == '.' || \
			x == '-' || \
			x == '>' || \
			x == '*' || x == '?')
#define varchar(x)	(smplchar(x) || x == ']')
#define indxchar(x)	(number(x) || \
			(x>='A' && x<= 'F') || \
			(x>='a' &&x<='f') || \
			(x == 'x' /* 0x */) || \
			(x == 'X' /* 0X */) || \
			eqany(x, ",;*"))
#define number(x)	(x >= '0' && x <= '9')

/* masks for metaflag (which indicates metacharacters present)
 * note that any non-zero value means that symbolic name is to be printed
 */
#define METACHAR	 01	/* "true" metachars ? or *	*/
#define ARRAYEXP	 02	/* expand array to all elements */
#define STREXP		 04	/* expand structure to all members */
#define SCRIPTEXP	010	/* print names because input is script */
extern int	metabase;	/* initial value for metaflag 
				 * (0 or SCRIPTEXP)
				 */

/*	All possible function names	*/
char	*cpname();
char	*cpstr();
char	*cpall();
char	*findfl();
char	*readline();
char	readchar();
char	rdc();
char	*sbrk();
char	*typetodesc();
enum	valtype	argvalue();
int	octdigit();
int	decdigit();
int	hexdigit();
int	octconv();
int	decconv();
int	hexconv();
long	atol();
long	adtostoffset();
long	extlookup();
long	readint();
long	rint();
long	sglookup();
long	slooknext();
long	strlookup();
long	getval();
ADDR	dispvar();
ADDR	extaddr();
ADDR	findvar();
ADDR	formaddr();
ADDR	getindir();
ADDR	inkdot();
ADDR	setdot();
ADDR	stackreg();
ADDR	varaddr();
BKPTR	scanbkpt();
L_INT	round();
POS	bchkget(), chkget();
POS	get();
STRING	exform();
VOID	fault();
struct	proct	*curproc();
struct	proct	*findproc();
struct	proct	*adrtoprocp();
struct	proct	*initframe();
struct	proct	*nextframe();
struct	filet	*findfile();
struct	filet	*adrtofilep();
struct	stmt	adtostmt();
