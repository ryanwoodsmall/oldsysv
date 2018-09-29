/*	@(#)extern.c	1.3	*/
#include "defs.h"

SCCSID(@(#)extern.c	1.3);

int trace = FALSE;             /* TRUE for debugging printout */

MSG ADWRAP   = "address wrap around";
MSG BADCOM   = "bad command";
MSG BADDAT   = "data address not found";
MSG BADEQ    = "unexpected `='";
MSG BADFIL   = "bad file format";
MSG BADKET   = "unexpected ')'";
MSG BADLOC   = "automatic variable not found";
MSG BADMAGIC = "bad core magic number";
MSG BADMOD   = "bad modifier";
MSG BADNAM   = "not enough space for symbols";
MSG BADRAD   = "must have 2<= radix <= 16";
MSG BADSYM   = "symbol not found";
MSG BADSYN   = "syntax error";
MSG BADTXT   = "text address not found";
MSG BADVAR   = "bad variable";
MSG BADWAIT  = "wait error: process disappeared!";
MSG ENDPCS   = "process terminated";
MSG EXBKPT   = "too many breakpoints";
MSG LONGFIL  = "filename too long";
MSG NOADR    = "address expected";
MSG NOBKPT   = "no breakpoint set";
MSG NOCFN    = "c routine not found";
MSG NOEOR    = "newline expected";
MSG NOFORK   = "try again";
MSG NOMATCH  = "cannot locate value";
MSG NOPCS    = "no process";
MSG NOTOPEN  = "cannot open";
MSG SZBKPT   = "bkpt: command too long";

int      adrflg;              /* User specified an address */
int      adrval;              /* TRUE if user specified an address */
int      argcount;            /* Number of supplied files */
BKPTR    bkpthead;            /* Boints to start of bkpt linked list */
int      callpc;              /* PC from one of the stack frames */
int      cntflg;              /* User specified an count */
int      cntval;              /* TRUE if user specified an count */
STRING   corfil = "core";     /* Name of the core file being used */
int      datsiz;              /* Size of data segment; default from core */
int      ditto;               /* Dot when command was entered */
int      dot;                 /* Current address */
int      dotinc;              /* Value by which dot is incremented */
int      entrypt;             /* Entry point for object file */
int      eof;                 /* Result of last read from < input */
STRING   errflg;              /* Error message */
int      executing;           /* TRUE when command is ":" */
int      expv;                /* Result of evaluating an expression */
int      fcor;                /* Core file file descriptor */
int      fsym;                /* a.out file descriptor */
int      infile;              /* File descriptor for redirected input */
char     lastc = EOR;         /* Result of last read */
int      lastcom = '=';       /* Type of last command */
int      localval;            /* Address of a local variable */
int	locadr;
int	numlocs;
int      locval;
int      loopcnt;             /* Process execution counter */
STRING   lp;                  /* Points at current char in input line */
short    magic;               /* Magic number for object file */
int      maxfile = 1<<24;     /* Maximum length of object file */
int      maxoff;              /* Max offset for symbolic addresses */
int      maxpos;              /* Maximum print width on output */
unsigned maxstor = 1<<31;     /* Maximum storage address */
int      mkfault;             /* Counts interrupts */
int      outfile = 1;         /* File descriptor for user output file */
char     peekc;
int      pid;                 /* Processid of user program */
MAP      qstmap;              /* ? map */
int      radix = DEFRADIX;    /* Current output radix */
REGLIST  reglist[] = {        /* Register description information */
	"ps",  PS,
	"pc",  PC,
	"sp",  SP,
	"fp",  FP,
	"ap",  AP,
	"r11", R11,
	"r10", R10,
	"r9",  R9,
	"r8",  R8,
	"r7",  R7,
	"r6",  R6,
	"r5",  R5,
	"r4",  R4,
	"r3",  R3,
	"r2",  R2,
	"r1",  R1,
	"r0",  R0,
};
SIGunion sigint;              /* Current interrupt handling routine */
int      signo;               /* Signal that caused process halt */
SIGunion sigqit;              /* Current quit handling routine */
MAP      slshmap;             /* / map */
int      stksiz;              /* Size of stack from core file */
int      txtsiz;              /* Size of text segment; default from core */
int      symbas;              /* A.out file address of symbol table */
SYMTAB   symbol;              /* I/O buffer for a.out symbols */
STRING   symfil = "a.out";    /* Name of the object file being used */
int      symnum;              /* Number of symbols in symbol table */
SYMSLAVE *symvec;             /* Beginning of slave array */
int      var[VARNO];          /* Holds values of adb variables */
int      wtflag;              /* TRUE if user gave -w flag */

union Uuniontag Uunion;       /* User block in core file */
