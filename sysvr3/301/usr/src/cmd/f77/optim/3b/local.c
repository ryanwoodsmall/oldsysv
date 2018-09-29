/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*	static char  ID[] = "@(#)optim:3b/local.c	1.24";	*/

# include <ctype.h>
# include "optim.h"
# include "sgs.h"
# include "paths.h"

# define LINELEN 400

# define FATAL(S)	fatal((S), (char *) 0)

char fixstr[256];
char line[LINELEN];
char * linptr;		/* pointer to current line */
int fixlen;
char * deflab;
int opn;
int m;
int numauto;		/* number of bytes of automatic vars. */
boolean indata;
int lineno = IDVAL;
int nusel = 0;		/* semantically useless instructions */
int nspinc = 0;		/* useless sp increments */
int nmc = 0;		/* move followed by compare */
int nmal = 0;		/* move followed by arithmetic or logical */
int nredcmp = 0;	/* redundant compares */
int nadpsh = 0;		/* addw3 or subw3 followed by push */
int nadmsh = 0;		/* addw3 or subw3 followed by mov */
int nadmv = 0;		/* replace addw3 or subw3 by mov */
int indef = false;	/* flag to tell if in .def */
extern int ndisc;

# define numops 362

struct opent optab[numops] = {
	{ "acjl", (SCCODE|SIDE), ACJL, 0x0, 0x0, 0xf, 0x0, 0x1, 4, TYWORD },
	{ "acjle", (SCCODE|SIDE), ACJLE, 0x0, 0x0, 0xf, 0x0, 0x1, 4, TYWORD },
	{ "acjleu", (SCCODE|SIDE|UNSIGN), ACJLEU, 
			0x0, 0x0, 0xf, 0x0, 0x1, 4, TYWORD },
	{ "acjlu", (SCCODE|SIDE|UNSIGN), ACJLU, 
			0x0, 0x0, 0xf, 0x0, 0x1, 4, TYWORD },
	{ "addb2", (SCCODE|ARITH|ALLMATH), ADDB2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYBYTE },
	{ "addb3", (SCCODE|ARITH|ALLMATH), ADDB3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYBYTE },
	{ "addh2", (SCCODE|ARITH|ALLMATH), ADDH2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYHALF },
	{ "addh3", (SCCODE|ARITH|ALLMATH), ADDH3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYHALF },
	{ "addw2", (SCCODE|ARITH|ALLMATH), ADDW2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "addw3", (SCCODE|ARITH|ALLMATH), ADDW3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "alsw2", (SCCODE|ARITH|ALLMATH), ALSW2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "alsw3", (SCCODE|ARITH|ALLMATH), ALSW3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "andb2", (SCCODE|ALLMATH), ANDB2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYBYTE },
	{ "andb3", (SCCODE|ALLMATH), ANDB3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYBYTE },
	{ "andh2", (SCCODE|ALLMATH), ANDH2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYHALF },
	{ "andh3", (SCCODE|ALLMATH), ANDH3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYHALF },
	{ "andw2", (SCCODE|ALLMATH), ANDW2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "andw3", (SCCODE|ALLMATH), ANDW3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "arsw2", (SCCODE|ARITH|ALLMATH), ARSW2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "arsw3", (SCCODE|ARITH|ALLMATH), ARSW3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "atjnzb", (SCCODE|SIDE), ATJNZB, 0x0, 0x0, 0x7, 0x0, 0x1, 3, TYWORD },
	{ "atjnzh", (SCCODE|SIDE), ATJNZH, 0x0, 0x0, 0x7, 0x0, 0x1, 3, TYWORD },
	{ "atjnzw", (SCCODE|SIDE), ATJNZW, 0x0, 0x0, 0x7, 0x0, 0x1, 3, TYWORD },
	{ "atoi", (SCCODE), ATOI, 0x0, 0x0, 0x3, 0x0, 0x5, 3, TYWORD },
	{ "bitb", (SCCODE), BITB, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "bith", (SCCODE), BITH, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "bitw", (SCCODE), BITW, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "bpt", (SIDE), BPT, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "cachi", (SCCODE), CACHI, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "cale", (SCCODE|SIDE), CALE, 0x4, RETREG, 0x7, 0x0, 0x0, 3, TYNONE },
	{ "call", (SCCODE|SIDE), CALL, 0x4, RETREG, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "chrh", (NOFLAG), CHRH, 0x0, 0x0, 0x0, 0x0, 0x1, 1, TYWORD },
	{ "chrm", (NOFLAG), CHRM, 0x0, 0x0, 0x0, 0x0, 0x1, 1 , TYWORD },
	{ "cinov", (SCCODE), CINOV, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "clrmd0", (SCCODE|SIDE), CLRMD0, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "clrmd1", (SCCODE|SIDE), CLRMD1, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "clrod0", (SIDE), CLROD0, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "clrod1", (SCCODE|SIDE), CLROD1, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "cmpb", (CMP|SCCODE), CMPB, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYBYTE },
	{ "cmpcc", (SCCODE), CMPCC, 0x0, 0x0, 0x7, 0x0, 0x7, 3, TYNONE },
	{ "cmpcce", (SCCODE), CMPCCE, 0x0, 0x0, 0xf, 0x0, 0x7, 4, TYNONE },
	{ "cmpce", (SCCODE), CMPCE, 0x0, 0x0, 0x7, 0x0, 0x3, 3, TYNONE },
	{ "cmph", (CMP|SCCODE), CMPH, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYHALF },
	{ "cmpnw", (CMP|SCCODE), CMPNW, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYWORD },
	{ "cmpw", (CMP|SCCODE), CMPW, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYWORD },
	{ "cmpzv", (SCCODE), CMPZV, 0x0, 0x0, 0x7, 0x0, 0x0, 3, TYNONE },
	{ "cpoff", (SCCODE|SIDE), CPOFF, 0x0, 0x0, 0x0, 0x0, 0x3, 2, TYWORD },
	{ "cpon", (SCCODE|SIDE), CPON, 0x0, 0x0, 0x0, 0x0, 0x3, 2, TYWORD },
	{ "cpsema", (SCCODE|SIDE), CPSEMA, 0x1, 0x1, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "cs", (SIDE), CS, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "cvsema", (SCCODE|SIDE), CVSEMA, 0x5, 0x1, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "decpth", (SCCODE), DECPTH, 0x0, 0x0, 0x3, 0x0, 0x1, 2, TYWORD },
	{ "decptw", (SCCODE), DECPTW, 0x0, 0x0, 0x3, 0x0, 0x1, 2, TYWORD },
	{ "deque", (SIDE), DEQUE, 0x0, 0x0, 0xf, 0x0, 0x0, 4, TYNONE },
	{ "diag", (SCCODE|SIDE), DIAG, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "divw2", (SCCODE|ARITH|ALLMATH), DIVW2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "divw3", (SCCODE|ARITH|ALLMATH), DIVW3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "dmioh", (SCCODE|SIDE), DMIOH, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYHALF },
	{ "dmiow", (SCCODE|SIDE), DMIOW, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "doioh", (SCCODE|SIDE), DOIOH, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYHALF },
	{ "doiow", (SCCODE|SIDE), DOIOW, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "enmd", (SCCODE|SIDE), ENMD, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "enod", (SCCODE|SIDE), ENOD, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "extzv", (SCCODE), EXTZV, 0x0, 0x0, 0x7, 0x0, 0x8, 4, TYWORD },
	{ "faddd2", (SCCODE), FADDD2, 0x0, 0x0, 0x3, 0x3, 0x2, 2, TYDOUBLE },
	{ "faddd3", (SCCODE), FADDD3, 0x0, 0x0, 0x3, 0x7, 0x4, 3, TYDOUBLE },
	{ "fadds2", (SCCODE), FADDS2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "fadds3", (SCCODE), FADDS3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "fcmpd", (SCCODE), FCMPD, 0x0, 0x0, 0x3, 0x3, 0x0, 2, TYDOUBLE },
	{ "fcmps", (SCCODE), FCMPS, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYWORD },
	{ "fdivd2", (SCCODE), FDIVD2, 0x0, 0x0, 0x3, 0x3, 0x2, 2, TYDOUBLE },
	{ "fdivd3", (SCCODE), FDIVD3, 0x0, 0x0, 0x3, 0x7, 0x4, 3, TYDOUBLE },
	{ "fdivs2", (SCCODE), FDIVS2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "fdivs3", (SCCODE), FDIVS3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "fidl", (SCCODE|SIDE), FIDL, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "fmuld2", (SCCODE), FMULD2, 0x0, 0x0, 0x3, 0x3, 0x2, 2, TYDOUBLE },
	{ "fmuld3", (SCCODE), FMULD3, 0x0, 0x0, 0x3, 0x7, 0x4, 3, TYDOUBLE },
	{ "fmuls2", (SCCODE), FMULS2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "fmuls3", (SCCODE), FMULS3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "frz", (SCCODE), FRZ, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "fsubd2", (SCCODE), FSUBD2, 0x0, 0x0, 0x3, 0x3, 0x2, 2, TYDOUBLE },
	{ "fsubd3", (SCCODE), FSUBD3, 0x0, 0x0, 0x3, 0x7, 0x4, 3, TYDOUBLE },
	{ "fsubs2", (SCCODE), FSUBS2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "fsubs3", (SCCODE), FSUBS3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "gcc", (UCCODE), GCC, 0x0, 0x0, 0x0, 0x0, 0x1, 1, TYWORD },
	{ "getdct", (NOFLAG), GETDCT, 0x0, 0x0, 0x1, 0x0, 0x1, 2, TYWORD },
	{ "haltx", (SIDE),  HALTX, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYWORD },
	{ "hlabel", (HLAB|LAB), HLABEL, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "incpth", (SCCODE), INCPTH, 0x0, 0x0, 0x3, 0x0, 0x1, 2, TYWORD },
	{ "incptw", (SCCODE), INCPTW, 0x0, 0x0, 0x3, 0x0, 0x1, 2, TYWORD },
	{ "inctst", (SCCODE), INCTST, 0x0, 0x0, 0x7, 0x0, 0x1, 3, TYWORD },
	{ "inhmd", (SCCODE|SIDE), INHMD, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "inhod", (SCCODE|SIDE), INHOD, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "initgp", (SIDE), INITGP, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "insv", (SCCODE), INSV, 0x0, 0x0, 0x7, 0x0, 0x8, 4, TYWORD },
	{ "iocler", (SCCODE|SIDE), IOCLER, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "ioeack", (SCCODE|SIDE), IOEACK, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "ioiack", (SCCODE|SIDE), IOIACK, 0x0, 0x0, 0x1, 0x0, 0x, 2, TYWORD },
	{ "ioidl", (SCCODE|SIDE), IOIDL, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "iord", (SCCODE|SIDE), IORD, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "ioread", (SCCODE|SIDE), IOREAD, 0x0, 0x0, 0x3, 0x0, 0xc, 4, TYWORD },
	{ "iorint", (SCCODE|SIDE), IORINT, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "iorsr", (SCCODE|SIDE), IORSR, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "iorst", (SCCODE|SIDE), IORST, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "iosrack", (SCCODE|SIDE), IOSRACK, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "iowca", (SCCODE|SIDE), IOWCA, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "iowd", (SCCODE|SIDE), IOWD, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "iowdp", (SCCODE|SIDE), IOWDP, 0x0, 0x0, 0x7, 0x0, 0x0, 3, TYNONE },
	{ "iowt", (SCCODE|SIDE), IOWT, 0x0, 0x0, 0x7, 0x0, 0x8, 4, TYWORD },
	{ "itoa", (SCCODE|SIDE), ITOA, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "jbc", (BR|CBR|REV), JBC, 0x0, 0x0, 0x7, 0x0, 0x0, 3, TYNONE },
	{ "jbs", (BR|CBR|REV), JBS, 0x0, 0x0, 0x7, 0x0, 0x0, 3, TYNONE },
	{ "jcc", (BR|CBR|UCCODE|REV), JCC, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jcs", (BR|CBR|UCCODE|REV), JCS, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "je", (BR|CBR|UCCODE|REV), JE, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jfle", (BR|CBR|UCCODE), JFLE, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jg", (BR|CBR|UCCODE|REV), JG, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jge", (BR|CBR|UCCODE|REV), JGE, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jgeu", (BR|CBR|UCCODE|UNSIGN|REV), 
			JGEU, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jgu", (BR|CBR|UCCODE|UNSIGN|REV), 
			JGU, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jioe", (BR|CBR|UCCODE), JIOE, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jiom", (BR|CBR|UCCODE), JIOM, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jion", (BR|CBR|UCCODE), JION, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jiot", (BR|CBR|UCCODE), JIOT, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jl", (BR|CBR|UCCODE|REV), JL, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jle", (BR|CBR|UCCODE|REV), JLE, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jleu", (BR|CBR|UCCODE|UNSIGN|REV), 
			JLEU, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jlu", (BR|CBR|UCCODE|UNSIGN|REV), 
			JLU, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jmp", (BR|UNCBR), JMP, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jne", (BR|CBR|UCCODE|REV), JNE, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jneg", (BR|CBR|UCCODE|REV), JNEG, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jnneg", (BR|CBR|UCCODE|REV), JNNEG, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jnpos", (BR|CBR|UCCODE|REV), JNPOS, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jnz", (BR|CBR|UCCODE|REV), JNZ, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jpos", (BR|CBR|UCCODE|REV), JPOS, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jsb", (SCCODE|SIDE), JSB, 0x1, RETREG, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jvc", (BR|CBR|REV|UCCODE|REV), JVC, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jvs", (BR|CBR|REV|UCCODE|REV), JVS, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "jz", (BR|CBR|REV|UCCODE|REV), JZ, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "label", (LAB), LABEL, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE }, 
	{ "llsw2", (SCCODE|ALLMATH), LLSW2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "llsw3", (SCCODE|ALLMATH), LLSW3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "lmchb", (NOFLAG), LMCHB, 0x0, 0x0, 0x0, 0x0, 0x1, 1, TYWORD },
	{ "lmchs", (NOFLAG), LMCHS, 0x0, 0x0, 0x0, 0x0, 0x1, 1, TYWORD },
	{ "loccc", (SCCODE), LOCCC, 0x0, 0x0, 0x7, 0x0, 0x3, 3, TYWORD },
	{ "loccce", (SCCODE), LOCCCE, 0x0, 0x0, 0xf, 0x0, 0x3, 4, TYWORD },
	{ "locce", (SCCODE), LOCCE, 0x0, 0x0, 0x7, 0x0, 0x1, 3, TYWORD },
	{ "lockh", (SIDE), LOCKH, 0x0, 0x0, 0x1, 0x0, 0x1, 1, TYWORD },
	{ "lrsw2", (SCCODE|ALLMATH), LRSW2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "lrsw3", (SCCODE|ALLMATH), LRSW3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "lsm", (NOFLAG), LSM, REGS, REGS, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "mchex", (SIDE), MCHEX, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE }, 
	{ "mchin", (SIDE), MCHIN, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "mcomb", (SCCODE|ALLMATH), MCOMB, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYBYTE },
	{ "mcomh", (SCCODE|ALLMATH), MCOMH, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYHALF },
	{ "mcomw", (SCCODE|ALLMATH), MCOMW,  0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "mnegb", (SCCODE|ARITH|ALLMATH), MNEGB, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYBYTE }, 
	{ "mnegh", (SCCODE|ARITH|ALLMATH), MNEGH, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYHALF },
	{ "mnegw", (SCCODE|ARITH|ALLMATH), MNEGW, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "modw2", (SCCODE|ARITH|ALLMATH), MODW2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "modw3", (SCCODE|ARITH|ALLMATH), MODW3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "movab", (SCCODE|ALLMATH), MOVAB, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "movah", (SCCODE|ALLMATH), MOVAH, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "movaw", (SCCODE|ALLMATH), MOVAW, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "movb", (SCCODE|ARITH|ALLMATH), MOVB, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYBYTE },
	{ "movbbh", (SCCODE|ARITH|ALLMATH), MOVBBH, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYHALF },
	{ "movbbw", (SCCODE|ARITH|ALLMATH), MOVBBW, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "movbhw", (SCCODE|ARITH|ALLMATH), MOVBHW, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "movblb", (SCCODE|SIDE), MOVBLB, 0x7, 0x7, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "movblh", (SCCODE|SIDE), MOVBLH, 0x7, 0x7, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "movblkb", (SCCODE|SIDE), MOVBLKB, 0x0, 0x0, 0x7, 0x0, 0x7, 3, TYWORD },
	{ "movblkh", (SCCODE|SIDE), MOVBLKH, 0x0, 0x0, 0x7, 0x0, 0x7, 3, TYWORD },
	{ "movblkw", (SCCODE|SIDE), MOVBLKW, 0x0, 0x0, 0x7, 0x0, 0x7, 3, TYWORD },
	{ "movblw", (SCCODE|SIDE), MOVBLW, 0x7, 0x7, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "movcce", (SIDE), MOVCCE, 0x0, 0x0, 0xf, 0x0, 0x7, 4, TYWORD },
	{ "movccep", (SIDE), MOVCCEP, 0x0, 0x0, 0x1f, 0x0, 0x7, 5, TYWORD },
	{ "movce", (SIDE), MOVCE, 0x0, 0x0, 0x7, 0x0, 0x3, 3, TYWORD },
	{ "movd", (SCCODE), MOVD, 0x0, 0x0, 0x1, 0x3, 0x2, 2, TYDOUBLE },
	{ "movdd", (SCCODE), MOVDD, 0x0, 0x0, 0x1, 0x3, 0x2, 2, TYDOUBLE },
	{ "movdh", (SCCODE), MOVDH, 0x0, 0x0, 0x1, 0x1, 0x2, 2, TYHALF },
	{ "movds", (SCCODE), MOVDS,  0x0, 0x0, 0x1, 0x1, 0x2, 2, TYWORD },
	{ "movdw", (SCCODE), MOVDW,  0x0, 0x0, 0x1, 0x1, 0x2, 2, TYWORD },
	{ "movh", (SCCODE|ARITH|ALLMATH), MOVH, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYHALF },
	{ "movhd", (SCCODE), MOVHD, 0x0, 0x0, 0x1, 0x2, 0x2, 2, TYDOUBLE },
	{ "movhs", (SCCODE), MOVHS, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "movsd", (SCCODE), MOVSD, 0x0, 0x0, 0x1, 0x2, 0x2, 2, TYDOUBLE },
	{ "movsh", (SCCODE), MOVSH, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYHALF },
	{ "movsw", (SCCODE), MOVSW, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "movtdh", (SCCODE), MOVTDH, 0x0, 0x0, 0x1, 0x1, 0x2, 2, TYHALF },
	{ "movtdw", (SCCODE), MOVTDW,  0x0, 0x0, 0x1, 0x1, 0x2, 2, TYWORD },
	{ "movthb", (SCCODE|ARITH|ALLMATH), MOVTHB,  0x0, 0x0, 0x1, 0x0, 0x2, 2, TYBYTE },
	{ "movtsh", (SCCODE), MOVTSH, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYHALF },
	{ "movtsw", (SCCODE), MOVTSW, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "movtwb", (SCCODE|ARITH|ALLMATH), MOVTWB,  0x0, 0x0, 0x1, 0x0, 0x2, 2, TYBYTE },
	{ "movtwh", (SCCODE|ARITH|ALLMATH), MOVTWH, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYHALF },
	{ "movw", (SCCODE|ARITH|ALLMATH), MOVW, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "movwd", (SCCODE), MOVWD, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYDOUBLE },
	{ "movws", (SCCODE), MOVWS, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "movzbh", (SCCODE|ALLMATH), MOVZBH, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYHALF },
	{ "movzbw", (SCCODE|ALLMATH), MOVZBW, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "movzhw", (SCCODE|ALLMATH), MOVZHW, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "mrf", (SCCODE|SIDE), MRF, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "msrf", (SCCODE), MSRF, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "msrn", (NOFLAG), MSRN, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "mssrf", (SCCODE), MSSRF, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "mssrn", (SCCODE), MSSRN, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "mswf", (SCCODE|SIDE), MSWF, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "mulw2", (SCCODE|ARITH|ALLMATH), MULW2,  0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "mulw3", (SCCODE|ARITH|ALLMATH), MULW3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "nop", (SIDE), NOP, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "orb2", (SCCODE|ALLMATH), ORB2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYBYTE },
	{ "orb3", (SCCODE|ALLMATH), ORB3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYBYTE },
	{ "orh2", (SCCODE|ALLMATH), ORH2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYHALF },
	{ "orh3", (SCCODE|ALLMATH), ORH3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYHALF },
	{ "orw2", (SCCODE|ALLMATH), ORW2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "orw3", (SCCODE|ALLMATH), ORW3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "ost", (SCCODE|SIDE), OST, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "other", (SCCODE|UCCODE), OTHER,  0x0, 0x0, 0x1f, 0x1f, 0x0, 5, TYNONE },
	{ "patb", (SIDE), PATB, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "pioe", (SCCODE|SIDE), PIOE, 0x0, 0x0, 0x0, 0x0, 0x1, 1, TYWORD },
	{ "pioi", (SCCODE), PIOI, 0x0, 0x0, 0x0, 0x0, 0x1, 1, TYWORD },
	{ "pior", (SCCODE|SIDE), PIOR, 0x0, 0x0, 0x0, 0x1, 1, TYWORD },
	{ "popw", (SCCODE), POPW, 0x800, 0x800, 0x0, 0x0, 0x1, 1, TYWORD },
	{ "psema", (SCCODE|SIDE), PSEMA, 0x5, 0x1, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "psiplse", (SCCODE|SIDE), PSIPLSE, 0x0, 0x0, 0x3, 0x0, 0x0, 0, TYNONE },
	{ "psipo", (SCCODE|SIDE), PSIPO, 0x0, 0x0, 0x7, 0x0, 0x8, 4, TYWORD },
	{ "psird", (SCCODE|SIDE), PSIRD, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "psiwt", (SCCODE|SIDE), PSIWT, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "pushaw", (SCCODE), PUSHAW, 0x800, 0x800, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "pushbb", (SCCODE), PUSHBB, 0x800, 0x800, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "pushbh", (SCCODE), PUSHBH, 0x800, 0x800, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "pushd", (SCCODE), PUSHD, 0x800, 0x800, 0x1, 0x1, 0x0, 1, TYNONE },
	{ "pushw", (SCCODE), PUSHW, 0x800, 0x800, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "pushzb", (SCCODE), PUSHZB, 0x800, 0x800, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "pushzh", (SCCODE), PUSHZH, 0x800, 0x800, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "queue", (SIDE), QUEUE, 0x0, 0x0, 0xf, 0x0, 0x4, 0, TYNONE },
	{ "qtest", (SCCODE), QTEST, 0x0, 0x0, 0xf, 0x0, 0x0, 4, TYNONE },
	{ "rcb", (SCCODE), RCB, 0x0, 0x0, 0x1, 0x0, 0x3, 2, TYBYTE },
	{ "rch", (SCCODE), RCH, 0x0, 0x0, 0x1, 0x0, 0x3, 2, TYHALF },
	{ "rcrefe", (SCCODE|SIDE), RCREFE, 0x0, 0x0, 0x0, 0x0, 0x1, 1, TYWORD },
	{ "rcw", (SCCODE), RCW, 0x0, 0x0, 0x1, 0x0, 0x3, 2, TYWORD },
	{ "rdblk", (SCCODE|SIDE), RDBLK, 0x0, 0x0, 0x1, 0x0, 0x6, 3, TYWORD },
	{ "rdblki", (SCCODE|SIDE), RDBLKI, 0x0, 0x0, 0x1, 0x0, 0x6, 3, TYWORD },
	{ "rdblkr", (SCCODE|SIDE), RDBLKR, 0x0, 0x0, 0x1, 0x0, 0x6, 3, TYWORD },
	{ "rdcstk", (SCCODE|SIDE), RDCSTK, 0x0, 0x0, 0x0, 0x0, 0x1, 1, TYWORD },
	{ "rdinhp", (SCCODE), RDINHP, 0x0, 0x0, 0x1, 0x0, 0x6, 3, TYWORD },
	{ "rdinhpi", (SCCODE|SIDE), RDINHPI, 0x0, 0x0, 0x1, 0x0, 0x6, 3, TYWORD },
	{ "rdinhpr", (SCCODE|SIDE), RDINHPR, 0x0, 0x0, 0x1, 0x0, 0x6, 3, TYWORD },
	{ "rdistk", (SCCODE), RDISTK, 0x0, 0x0, 0x0, 0x0, 0x1, 1, TYWORD },
	{ "rdoser", (SCCODE), RDOSER, 0x0, 0x0, 0x2, 0x0, 0x1, 2, TYWORD },
	{ "rdphy", (SCCODE), RDPHY, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "rdser", (SCCODE|SIDE), RDSER, 0x0, 0x0, 0x2, 0x0, 0x1, 2, TYWORD },
	{ "rdsr", (SCCODE), RDSR, 0x0, 0x0, 0x2, 0x0, 0x1, 2, TYWORD },
	{ "rdtim", (SCCODE), RDTIM, 0x0, 0x0, 0x0, 0x0, 0x1, 1, TYWORD },
	{ "rdwrd", (SCCODE), RDWRD, 0x0, 0x0, 0x1, 0x0, 0x6, 3, TYWORD },
	{ "rdwrdi", (SCCODE|SIDE), RDWRDI, 0x0, 0x0, 0x1, 0x0, 0x6, 3, TYWORD },
	{ "rdwrdr", (SCCODE|SIDE), RDWRDR, 0x0, 0x0, 0x1, 0x0, 0x6, 3, TYWORD },
	{ "ret", (FRET|HBR|UNCBR|BR|UCCODE), RET, RETREG, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "rete", (FRET|HBR|UNCBR|BR|UCCODE), RETE, RETREG, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "rmchb", (SCCODE), RMCHB, 0x0, 0x0, 0x0, 0x0, 0x1, 1, TYWORD },
	{ "rmchr", (SCCODE), RMCHR, 0x0, 0x0, 0x2, 0x0, 0x1, 2, TYWORD },
	{ "rmchs", (SCCODE), RMCHS, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYWORD },
	{ "rotlw", (NOFLAG), ROTLW, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "rotrw", (NOFLAG), ROTRW, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "rp", (SIDE), RP, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "rrblk", (SCCODE|SIDE), RRBLK, 0x0, 0x0, 0x1, 0x0, 0x6, 3, TYWORD },
	{ "rrwrd", (SCCODE), RRWRD, 0x0, 0x0, 0x1, 0x0, 0x6, 3, TYWORD },
	{ "rsb", (FRET|BR|UNCBR|HBR|UCCODE), RSB, RETREG, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "rsblk", (SCCODE|SIDE), RSBLK, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "rstat", (SCCODE), RSTAT, 0x0, 0x0, 0x1, 0x0, 0x6, 3, TYWORD },
	{ "rstati", (SCCODE|SIDE), RSTATI, 0x0, 0x0, 0x1, 0x0, 0x6, 3, TYWORD },
	{ "rstatr", (SCCODE|SIDE), RSTATR, 0x0, 0x0, 0x1, 0x0, 0x6, 3, TYWORD },
	{ "rswrd", (SCCODE|SIDE), RSWRD, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "rtb", (SCCODE|SIDE), RTB, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "rti", (SCCODE), RTI, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "rto", (SCCODE|SIDE), RTO, 0x0, 0x1, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "rtt", (SCCODE|SIDE), RTT, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "save", (SCCODE|SIDE), SAVE, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "scc", (SCCODE), SCC, 0x0, 0x0, 0x1, 0x0, 0x0, 0, TYNONE },
	{ "sds", (SIDE), SDS, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "sendint", (SCCODE|SIDE), SENDINT, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "setmpr", (SCCODE|SIDE), SETMPR, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "sinov", (SCCODE|SIDE), SINOV, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "sioh", (SCCODE|SIDE), SIOH, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "siow", (SCCODE|SIDE), SIOW, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "smccp", (SIDE), SMCCP, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "smcdp", (SIDE), SMCDP, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "smchc", (SCCODE|SIDE), SMCHC, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "smchd", (SCCODE|SIDE), SMCHD, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "sminst", (SCCODE|SIDE), SMINST, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "smioh", (SCCODE|SIDE), SMIOH, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "smiow", (SCCODE|SIDE), SMIOW, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "ss", (SCCODE|SIDE), SS, 0x0, 0x0, 0x2, 0x0, 0x5, 3, TYWORD },
	{ "sss", (SIDE), SSS,  0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "stat", (SCCODE), STAT, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "stmcbp", (SIDE), STMCBP, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "stmchb", (SIDE), STMCHB, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "stmchc", (SIDE), STMCHC, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "stsm", (SIDE), STSM, REGS, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "subb2", (SCCODE|ARITH|ALLMATH), SUBB2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYBYTE },
	{ "subb3", (SCCODE|ARITH|ALLMATH), SUBB3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYBYTE },
	{ "subh2", (SCCODE|ARITH|ALLMATH), SUBH2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYHALF },
	{ "subh3", (SCCODE|ARITH|ALLMATH), SUBH3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYHALF },
	{ "subw2", (SCCODE|ARITH|ALLMATH), SUBW2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "subw3", (SCCODE|ARITH|ALLMATH), SUBW3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "svsxl", (SIDE), SVSXL, 0x0, 0x0, 0x2, 0x0, 0x1, TYWORD },
	{ "switch", (BR|UNCBR|SCCODE|SIDE|SWIT), SWITCH, 0x0, 0x0, 0x7, 0x0, 0x0, 3, TYNONE },
	{ "switcht", (BR|UNCBR|SCCODE|SIDE|SWIT), SWITCHT, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "swks", (SIDE), SWKS, 0x0, 0x800, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "swps", (SIDE), SWPS, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "sxl", (SIDE), SXL, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "tarbb", (SCCODE), TARBB, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYBYTE },
	{ "tarbh", (SCCODE), TARBH, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYHALF },
	{ "tarbw", (SCCODE), TARBW, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "tasbb", (SCCODE), TASBB, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYBYTE },
	{ "tasbh", (SCCODE), TASBH, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYHALF },
	{ "tasbw", (SCCODE), TASBW, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "task", (SCCODE|SIDE), TASK, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "tio", (SCCODE|SIDE), TIO, 0x0, 0x0, 0x1, 0x0, 0x3, 2, TYWORD },
	{ "ucrd", (NOFLAG), UCRD, 0x0, 0x0, 0x0, 0x0, 0x1, 1, TYWORD },
	{ "ucwt", (SIDE), UCWT, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "udivw2", (SCCODE|UNSIGN|ALLMATH), UDIVW2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "udivw3", (SCCODE|UNSIGN|ALLMATH), UDIVW3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "umodw2", (SCCODE|UNSIGN|ALLMATH), UMODW2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "umodw3", (SCCODE|UNSIGN|ALLMATH), UMODW3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "umulw2", (SCCODE|UNSIGN|ALLMATH), UMULW2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "umulw3", (SCCODE|UNSIGN|ALLMATH), UMULW3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "utnop", (SIDE), UTNOP, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "vsema", (SCCODE|SIDE), VSEMA, 0x5, 0x1, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "vtop", (SCCODE), VTOP,  0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "wait", (SIDE), WAIT, 0x0, 0x0, 0x0, 0x0, 0x0, 0, TYNONE },
	{ "waitii", (SIDE), WAITII, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "wcsdump", (SIDE), WCSDUMP, 0x0, 0x0, 0x7, 0x0, 0x0, 3, TYNONE },
	{ "wcspump", (SIDE), WCSPUMP, 0x0, 0x0, 0x7, 0x0, 0x0, 3, TYNONE },
	{ "wtblk", (SCCODE|SIDE), WTBLK, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "wtblki", (SCCODE|SIDE), WTBLKI, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "wtblkr", (SCCODE|SIDE), WTBLKR, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "wtcmd", (SCCODE|SIDE), WTCMD, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "wtcmdi", (SCCODE|SIDE), WTCMDI, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "wtcmdr", (SCCODE|SIDE), WTCMDR, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "wtcstk", (SCCODE|SIDE), WTCSTK, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "wtistk", (SCCODE|SIDE), WTISTK, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
	{ "wtmsk", (SCCODE|SIDE), WTMSK, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "wtmskb", (SCCODE|SIDE), WTMSKB, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "wtmski", (SCCODE|SIDE), WTMSKI, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "wtmskr", (SCCODE|SIDE), WTMSKR, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "wtpar", (SCCODE|SIDE), WTPAR, 0x0, 0x0, 0x7, 0x0, 0x8, 4, TYWORD },
	{ "wtpari", (SCCODE|SIDE), WTPARI, 0x0, 0x0, 0x7, 0x0, 0x8, 4, TYWORD },
	{ "wtparr", (SCCODE|SIDE), WTPARR, 0x0, 0x0, 0x7, 0x0, 0x8, 4, TYWORD },
	{ "wtphy", (NOFLAG), WTPHY, 0x0, 0x0, 0x3, 0x0, 0x0, 2, TYNONE },
	{ "wtrtn", (SCCODE|SIDE), WTRTN, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "wtsr", (NOFLAG), WTSR, 0x0, 0x0, 0x1, 0x0, 0x2, 2, TYWORD },
	{ "wtwrd", (SCCODE|SIDE), WTWRD, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "wtwrdi", (SCCODE|SIDE), WTWRDI, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "wtwrdr", (SCCODE|SIDE), WTWRDR, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "xorb2", (SCCODE|ALLMATH), XORB2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYBYTE },
	{ "xorb3", (SCCODE|ALLMATH), XORB3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYBYTE },
	{ "xorh2", (SCCODE|ALLMATH), XORH2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYHALF },
	{ "xorh3", (SCCODE|ALLMATH), XORH3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYHALF },
	{ "xorw2", (SCCODE|ALLMATH), XORW2, 0x0, 0x0, 0x3, 0x0, 0x2, 2, TYWORD },
	{ "xorw3", (SCCODE|ALLMATH), XORW3, 0x0, 0x0, 0x3, 0x0, 0x4, 3, TYWORD },
	{ "zeroblk", (SIDE), ZEROBLK, 0x0, 0x0, 0x1, 0x0, 0x0, 1, TYNONE },
};

FILE *stmpfile;	/* Temporary storage for strings which are in the text
		 * section. The strings are collected and printed at
		 * the end of a function.  Requirement for field update */
extern char *mktemp();
FILE *tmpopen(),
      *outfile;
char tmpname[50];	/* name of file for storing string */
boolean instring = false;
boolean inswitch = false;

yylex()
{
    extern char * strchr();

    char * s;				/* temporary string ptr */
    char * start;			/* another temporary string ptr */
    int i;				/* temporary int */

    linptr = NULL;			/* start off with no line */

    while (linptr != NULL || (linptr = fgets(line,LINELEN,stdin)) != NULL)
    {
	switch (*linptr)		/* dispatch on first char in line */
	{
	case '\n':			/* ignore new line */
	    break;
	    
	case '#':			/* check for special comments */
	    s = linptr + 1;		/* look at comment only, not # */

	    if (strncmp(s,"STRBEG",6) == 0)
		instring = true;	/* declare we're in a string */
	    else if (strncmp(s,"STREND",6) == 0)
		instring = false;	/* now out of string-ness */
	    else if (strncmp(s,"SWBEG",5) == 0)
		inswitch = true;	/* now in a switch table */
	    else if (strncmp(s,"SWEND",5) == 0)
		inswitch = false;	/* out of switch table */
		
	    break;			/* in any case, ignore line further */
	    
	case '.':			/* compiler label */
	default:			/* ordinary label */
	    if (indata)			/* in data section, just print */
	    {
		printf("%s",linptr);
		break;
	    }

	    /* reach here on labels in text space */

	    s = strchr(linptr,':');	/* find : */
	    if (s == NULL)		/* error if not found */
	    { FATAL("Bad input format\n"); }

	    *s++ = '\0';		/* change : to null, skip past */

/* for N3B (DMERT), switch code and jump tables are in text space,
** but SWBEG, SWEND bracket just the table.  For m32 and u3b, jump tables are
** always in data space.
** Write labels found when "inswitch" to special file for N3B only.
*/

#ifdef N3B
	    if (instring || inswitch)	/* inside special string or switch... */
#else
	    if (instring)		/* inside special string?... */
#endif	/* def N3B */
	    {
		putstr(linptr);		/* write label to special file */
		putstr(":\n");		/* put : back in */
	    }
	    else			/* normal text space label */
	    {
		applbl(linptr,s-linptr); /* append label node */
		lastnode->userdata = ilookup(LABEL);
		lastnode->uniqid = IDVAL;
	    }
	    linptr = s;			/* continue processing past label */
	    continue;			/* next iteration */
/* handle more usual case of line beginning with space/tab */
	case ' ':
	case '\t':
	    for (s = linptr; isspace(*s); s++) /* skip white space */
		;
		
	    switch(*s)			/* dispatch on next character */
	    {
	    case '\0':			/* line of white space */
	    case '#':			/* comment */
	    case '\n':			/* new line */
		break;			/* ignore */
		
	    case '.':			/* start of pseudo-op */
		pseudo(s);		/* do pseudo-ops */
		break;

	    default:			/* normal instruction */
		if (indata)		/* in data section this is weird case */
		    printf("%s",linptr); /* just write line */
		else			/* normal instruction in text */
		{
		    struct opent * tempop;
		    for (start = s; ! isspace(*s); s++)
			;		/* skip over instruction mnemonic */

		    *s++ = '\0';	/* demarcate with null */

		    tempop = lookup(start);
		    if (tempop->oopcode == OTHER)
			saveop(0,start,s-start,OTHER);
					/* save funny instruction */
		    else
			saveop(0,tempop->oname,0,tempop->oopcode);
					/* save normal inst. (m set by
					** lookup)
					*/
		    lastnode->userdata = tempop;
		    for ( ; isspace(*s); s++ )
			;		/* span white space */
		    opn = 1;		/* now save operands */
		    op(s);		/* skip intervening spaces/tabs */
		}
		break;
	    }   /* end space/tab case */
	break;
	}	/* end first character switch */

	linptr = NULL;			/* indicate we're done with line */
    }   /* end while */
    return;				/* just exit */
}
op (s)
	register char *s;
{
	char *t;

	while (*s)
	{
		/* scan over leading space/tabs for this operand */
		while (*s == ' ' || *s == '\t')
			s++;

		/* remember effective operand start */
		t = s;

		/* find end of operand */
		while (*s != ',' && *s != '\n')
		{
			if (*s == ' ' || *s == '\t')
				*s = '\0';
			s++;
		}

		/* 'null'-terminate this operand string */
		*s++ = '\0';

		/* save away this operand string */
		saveop(opn++, t, s-t, 0);

		/* operand is a label, add reference */
		if (*t == '.')
			if (inswitch)
				addref(t, s-t);
	}

	lastnode->uniqid = lineno;
	lineno = IDVAL;
}


pseudo (s)
	register char *s;		/* points at pseudo-op */
{
	void peep();			/* peephole improver */
	extern int cflag;		/* independent part -c flag:
					** enable/disable common tail
					*/
	int pop;			/* pseudo-op code */
	char savechar;			/* saved char that follows pseudo */
	char * word = s;		/* pointer to beginning of pseudo */

/* The idea here is to stick in a NUL after the pseudo-op, then replace
** the character we clobbered, rather than copy the pseudo-op somewhere else.
*/

	for ( ; ! isspace(*s); s++ )
	    ;				/* scan to white space char */
	savechar = *s;			/* mark end of pseudo-op */
	*s = '\0';

	/* if we're in a data section, we write all pseudo-ops to output,
	** except for .word's, which we must examine, and .text .
	*/

	pop = plookup(word);		/* identify pseudo-op */
	*s = savechar;			/* restore saved character */
	if (indata) {			/* check pseudo-op possibilities */
		if (pop == TEXT) {
			indata = false;	/* not in data section anymore */
			return;
		}
		else if (pop != WORD) {
			printf("%s",linptr); /* output the line */
			return;
		}
	}

	switch (pop) {			/* dispatch on pseudo-op in text */
		case HALF:
		case BYTE:
			if (!(inswitch || instring ))
				FATAL("`.half' or `.byte' in text; unable to optimize\n");
			else
				putstr(linptr);
			break;
		case GLOBL:
		case TV:
			line[(m=length(line))-2] = '\0'; /* remove newline */
			appfl(line, m-1);
			break;
		case TEXT:
			indata = false;
			break;
		case FIL:
		case BSS:
		case COMM:
			printf("%s", line);
			break;
		case LN:
			getln(line);
			break;
		case ALIGN:
			line[(m=length(line))-2] = '\0'; /* remove NL */
			if (inswitch) {
#ifdef N3B			/* for DMERT, put aligns in switch to special
				** output file with switch table
				*/
				line[m-2] = '\n';	/* restore new line */
				putstr(linptr);
#else
				saveop(0, line+1, m-2, OTHER);
				opn = 1;
				lastnode->userdata = ilookup(OTHER);
				lastnode->uniqid = lineno;
				lineno = IDVAL;
#endif	/* def N3B */
			}
			else
				appfl(line, m-1);
			break;
		case SET:
			if (*++s == '.' && *++s == 'F') {
				printf("\t.text\n");
				setauto(line);
				printf("%s", line);
			}
			else {
				line[(m=length(line))-2] = '\0';
				appfl(line, m-1);
			}
			break;
		case DATA:
			printf("%s", line);
			indata = true;
			break;
		case ENDEF:
		case SCL:
		case VAL:
		case SIZE:
			if(!indef)
				FATAL("Invalid Pseudo-op outside def-endef\n");
		case DEF: {
			int found;
			int loop = false;
			s = word;

			line[(m=length(line))-2] = '\0'; /* remove newline */
			found = 0;
			while (true) {
				if(!indef || loop)
					while (*s++ != ';');
				loop = true;
				while(*s != '.' && *s)
					s++;
				if(!*s) {
					indef = true;
					break;
				}
				if (*(s += 1) == 'e') { /* endef */
					indef = false;
					break;
				}
/* There are two special cases for ".val".  The first is
** ".val ." .  Since we're going to move the .def line, we need
** to replace the '.' with a real label (which we make a hard label)
** first.  The second special case is ".val .Lxxx" (a local label).
** This arises in the compiler from a user-defined label.  The
** problem here is that if the optimizer discards the local label,
** there will be a reference to a non-existent label which will only
** show up at link time.  To avoid this, we delete (ignore) the whole
** line.
*/
				else if (*s++ == 'v') { /* .val */
					if (s[3] == '.')
					{
					    if (s[4] == ';')	/* first case */
						found = 1;
					    else if (s[4] == 'L')
						found = 3;	/* second */
					}
				}
				else if (*s == 'c') /* .scl */
					if (*(s+3) == '-' && *(s+4) == '1') {
						found = 2;
						break;
					}
			} switch(found)
			{
			case 1:			/* .val . */
				deflab = (char *) newlab();
				deffix(line, deflab);
				appfl(fixstr, fixlen+1);
				fixlbl(deflab);
				applbl(deflab, length(deflab));
				lastnode->userdata = ilookup(HLABEL);
				lastnode->op = HLABEL;
				lastnode-> uniqid = IDVAL;
				break;
			case 2:			/* end of function */
				cflag = -1;	/* turn off common tail
						** merging:  improve speed,
						** not space.
						*/
				filter();
				optim();
				peep();		/* do peephole improvements */
				chkauto();
				prtext();
				prstr();
				printf("%s", line);
				printf("\n");
				init();
				break;
			case 3:			/* .val .Lxxx */
				break;		/* ignore line */
			default:		/* other .defs */
				appfl(line, m-1);
				break;
			}
		}
			break;
		case WORD:
			if (!(inswitch || indata  || instring))
				FATAL("`.word' in text; unable to optimize\n");
			s++;			/* point past tab/nl */

/* we have to deal with whether .word is within a switch (SWBEG/SWEND) or
** not, and whether or not it appears in a data section.
*/

			if (inswitch) {		/* always add reference */
				register char * t = strchr(s,'\n');

				*t = '\0';	/* change NL to null */
				addref(s,t+1-s); /* add the reference */
				*t = '\n';	/* restore NL */
			}

			if (indata) 		/* in data, flush to output */
				printf("%s",linptr); /* print line */
			else if (inswitch)  	/* text (implicit) and switch */
				putstr(linptr);	/* flush to special file */
			else {			/* text, not in switch */
				saveop(0,".word",6,OTHER);
				lastnode->userdata = ilookup(OTHER);
				opn = 1;	/* doing first operand */
				op(s);
			}
			break;

		default:			/* all unrecognized text
						** pseudo-ops
						*/
			if (! (instring || inswitch))
				printf("%s",linptr); /* flush to output */
			else
			    putstr(linptr);	/* in(switch|string):  to
						** special file
						*/
			break;
	}
}

int
plookup(s)	/* look up pseudo op code */
	char *s;

{
/* Note:  to improve the linear search speed, these pseudo-ops
** are ordered by frequency of occurrence in a sample of C programs.
*/

	static char *pops[numpops] =
		{
		    ".word", ".byte", ".set", ".def", ".align",
		    ".data", ".text", ".globl", ".ln", ".comm",
		    ".file", ".tv", ".bss", ".half", ".val", ".zero",
		    ".line", ".scl", ".size", ".tag", ".type",
		    ".dim", ".strbeg", ".jmpbeg", ".double", ".float", ".il",
		    ".ident", ".endef"
		};

	static int popcode[numpops] =
		{
		    WORD, BYTE, SET, DEF, ALIGN,
		    DATA, TEXT, GLOBL, LN, COMM,
		    FIL, TV, BSS, HALF, VAL, ZERO,
		    LINE, SCL, SIZE, TAG, TYPE,
		    DIM, STRBEG, JMPBEG, DOUBLE, FLOAT, IL, IDENT, ENDEF
		};

	register int i;

	for (i = 0; i < numpops; i++)
		if (!strcmp(s, pops[i]))
			return(popcode[i]);
	fprintf(stderr,"Optimizer: unrecognized pseudo-op %s\n",s);
	return(POTHER);
}


yyinit(flags) char * flags; {

	indata = false;
	for (; *flags != '\0'; flags++)
		if (*flags == 'V')
			fprintf(stderr,"%s: optimizer - %s %s\n",
				SGSNAME,RELEASE);
		    else
			fprintf(stderr,"Optimizer: invalid flag '%c'\n",*flags);

	}
struct opent *
ilookup(op)
int op;
{
	if(op < LOWOP || op > HIGHOP) {
		fprintf(stderr,"SCREWED UP OP\n");
		return(&optab[OTHER-LOWOP]);
	}
	return(&optab[op-LOWOP]);
}

struct opent *
lookup(op) char *op; { /* look up op code ordinal */

	int f,l,om,x;

	f = 0;
	l = numops;
	om = 0;
	m = (f+l)/2;
	while (m != om) {
		x = strcmp(op,optab[m].oname);
		if (x == 0)
			return(&optab[m]);
		else if (x < 0)
			l = m-1;
		    else
			f = m+1;
		om = m;
		m = (f+l)/2;
		}
#ifndef M32
	fprintf(stderr,"Optimizer: unrecognized op %s\n",op);
#endif
	return(ilookup(OTHER));
	}

deffix(str,lbl) char * str, * lbl; { /* change "val .;" to "val ..n;" */

	char * nc;
	boolean scanning;

	scanning = true;
	nc = fixstr;
	fixlen = 0;
	while (scanning)
		if (*str == '.' && *(str+1) == ';') {
			str++;
			scanning = false;
			}
		    else {
			*nc++ = *str++;
			fixlen++;
			}
	while (*nc++ = *lbl++)
		fixlen++;
	nc--;
	while (*nc++ = *str++)
		fixlen++;
	}

fixlbl(str) char * str; { /* add colon to end of label */

	while (*str++);
	*--str = ':';
	*++str = '\0';
	}

int
length(str) char * str; { /* return length of string (including \0) */

	int l;

	l = 1;
	while (*str++)
		l++;
	return (l);
	}

getln(str) char * str; { /* extract line number */

	while (!isdigit(*str))
		str++;
	lineno = 0;
	while (isdigit(*str)) {
		lineno = lineno * 10 + *str - '0';
		str++;
		}
	}

prinst(p) NODE *p; { /* print instruction */
	register int i;
	if (p->uniqid != IDVAL)
		printf("	.ln	%d\n", p->uniqid);
	switch (p->op) {
		case LABEL:
			printf("%s:\n", p->opcode);
			break;
		case HLABEL:
			printf("%s\n", p->opcode);
			break;
		case MISC:
			printf("	%s\n", p->opcode);
			break;
		default:
			printf("	%s	", p->opcode);
			for(i = 1; i <= MAXOPS; i++ ) {
				if (p->ops[i] != NULL)
					{
						if(i != 1)
							putchar(',');
						printf("%s", p->ops[i]);
					}
			}
			printf("\n");
			break;
	}
}

boolean
ishlp(p) NODE *p; { /* return true if a fixed label present */

	for (; islabel(p); p=p->forw)
		if (ishl(p))
			return(true);
	return(false);
	}

FILE *
tmpopen() {
	strcpy( tmpname, TMPDIR );
	strcat( tmpname, "/25ccXXXXXX" );
	return( fopen( mktemp( tmpname ), "w" ) );
	}

putstr(string)   char *string; {
	/* Place string from the text section into a temporary file
	 * to be output at the end of the function */

	if( stmpfile == NULL )
		stmpfile = tmpopen();
	fprintf(stmpfile,"%s",string);
	}

prstr() {
/* print the strings stored in stmpfile at the end of the function */

	if( stmpfile != NULL ) {
		register int c;

		stmpfile = freopen( tmpname, "r", stmpfile );
		if( stmpfile != NULL )
			while( (c=getc(stmpfile)) != EOF )
				putchar( c );
		else
			{
			fprintf( stderr, "optimizer error: ");
			fprintf( stderr, "lost temp file\n");
			}
		(void) fclose( stmpfile );	/* close and delete file */
		unlink( tmpname );
		stmpfile = NULL;
		}
}
putp(p,c) NODE *p; char *c; {  /* insert pointer into jump node */

	if (p->op == CALL)
		p->op2 = c;
	else if (p->op == JBC || p->op == JBS)
		p->op3 = c;
	else if (p->op == RET) {
		p->op1 = NULL;
		p->op2 = c;
	}
	else
		p->op1 = c;
	}

revbr(p) NODE *p; { /* reverse jump in node p */

	switch (p->op) {
	    case JZ: p->op = JNZ; p->opcode = "jnz"; break;
	    case JNZ: p->op = JZ; p->opcode = "jz"; break;
	    case JPOS: p->op = JNPOS; p->opcode = "jnpos"; break;
	    case JNPOS: p->op = JPOS; p->opcode = "jpos"; break;
	    case JNEG: p->op = JNNEG; p->opcode = "jnneg"; break;
	    case JNNEG: p->op = JNEG; p->opcode = "jneg"; break;
	    case JE: p->op = JNE; p->opcode = "jne"; break;
	    case JCC: p->op = JCS; p->opcode = "jcs"; break;
	    case JCS: p->op = JCC; p->opcode = "jcc"; break;
	    case JVC: p->op = JVS; p->opcode = "jvs"; break;
	    case JVS: p->op = JVC; p->opcode = "jvc"; break;
	    case JNE: p->op = JE; p->opcode = "je"; break;
	    case JL: p->op = JGE; p->opcode = "jge"; break;
	    case JLE: p->op = JG; p->opcode = "jg"; break;
	    case JG: p->op = JLE; p->opcode = "jle"; break;
	    case JGE: p->op = JL; p->opcode = "jl"; break;
	    case JLU: p->op = JGEU; p->opcode = "jgeu"; break;
	    case JLEU: p->op = JGU; p->opcode = "jgu"; break;
	    case JGU: p->op = JLEU; p->opcode = "jleu"; break;
	    case JGEU: p->op = JLU; p->opcode = "jlu"; break;
	    case JBC: p->op = JBS; p->opcode = "jbs"; break;
	    case JBS: p->op = JBC; p->opcode = "jbc"; break;
	    }
	    p->userdata = ilookup(p->op);
	}

int
dst(p,ptr,ind)	/* Does a given instruction overwrite an operand during the
		execution of the instruction? */
NODE *p;
char **ptr;
int *ind;
	{
	int x;
	int i;
	*ind = p->userdata->odstops;
	i = *ind;
	if(i & (i-1)) {
		*ptr = "";
		return(false);
	}
	if(!i) {
		*ptr = "";
		return(true);
	}
	for(x =1; i != 1; i = i >> 1, x++) ;
	*ptr = p->ops[x];
	return(true);
	}

/*
 *	The function 'stype' is currently only called from window
 *	optimizations
 */
int
stype(n)  int n; { /* return implied type of op code */

	struct opent * p;
	p = ilookup(n);
	return(p->otype);
	}

boolean
samereg(cp1,cp2) char *cp1, *cp2; { /* return true if same register */

	if (*cp1 == '%' && *cp1 == *cp2 && *++cp1 == *++cp2 && *++cp1 == *++cp2)
		return(true);
	    else
		return(false);
	}

boolean
usesreg(cp1,cp2) char *cp1, *cp2; { /* return true if cp2 used in cp1 */

	while(*cp1 != '\0') {
		if (*cp1 == *cp2 && *(cp1+1) == *(cp2+1) &&
		    *(cp1+2) == *(cp2+2))
			return(true);
		cp1++;
		}
	return(false);
	}

dstats() { /* print stats on machine dependent optimizations */

	fprintf(stderr,"%d semantically useless instructions(s)\n", nusel);
	fprintf(stderr,"%d useless move(s) before compare(s)\n", nmc);
	fprintf(stderr,"%d merged move-arithmetic/logical(s)\n", nmal);
	fprintf(stderr,"%d useless sp increment(s)\n", nspinc);
	fprintf(stderr,"%d redundant compare(s)\n", nredcmp);
	}

wrapup() { /* print unprocessed text and update statistics file */

	FILE *fopen(), *sp;
	int mc,mal,usel,spinc,redcmp,disc,inst;

	if (n0.forw != NULL) {
		printf("	.text\n");
		filter();
		prtext();
		prstr();
		}
	}

setauto(str) char *str; { /* set indicator for number of autos */

	while (*str++ != ',')
	    ;
	numauto += atoi(str);		/* remember # of bytes */
	return;
	}

chkauto()	 /* delete sp increment if no autos, or fewer than 15 words */
{
    NODE *p;

/* 5.0 UNIX microcode on the 3B20 supports a "save" instruction that
** can also allocate temporary space on the stack (up to 15 words).
** Standard IS25 "save" does not do the stack allocation.
*/

#ifdef UCODE50
    if (numauto <= 4*15)		/* save handles up to 15 words */
#else
    if (numauto == 0)			/* if no auto's allocated */
#endif	/* def UCODE50 */
    {
	p = n0.forw;
	while (islabel( p ))
	    p = p->forw;
	if (    p->op == SAVE
	    &&  p->forw->op == ADDW2
	    )				/* save followed by addw2 */
	{
#ifdef UCODE50				/* must add second operand to save */
	    p->op2 = getspace(1+2+1);	/* for "&dd\0" */
	    (void) sprintf(p->op2,"&%d",numauto/4);
					/* append operand with # of autos */
#endif	/* def UCODE50 */
	    p = p->forw;		/* point at ADDW2 */
	    DELNODE( p );		/* delete it */
	    nspinc++;			/* discarded sp increment */
	    ndisc++;			/* discarded instruction */
	}
    }
    numauto = 0;			/* reset for next routine */
    return;
}

uses(p) NODE *p; { /* set register use bits */

	int i; 
	int src;
	int dbl;
	int us =0;
	int t;

	src = p->userdata->osrcops;
	dbl = p->userdata->odblops;
	for(i =1; i <= p->userdata->onumrand; i ++) {
		if((t = (1 << (i-1))) & src) {
			if(t & dbl)
				us |= dscanreg(p->ops[i],false);
			else
				us |= scanreg(p->ops[i],false);
		}
		else {
			if(t & dbl)
				us |= dscanreg(p->ops[i],true);
			else
				us |= scanreg(p->ops[i],true);
		}
	}

	us |= p->userdata->ousedef;
	if(p->userdata->oflags & UCCODE)
		us |= CCODES;
	return(us);
	}

scanreg(cp,flag) char *cp; int flag; { /* determine registers referenced in
					  operand */

	int reg,set;

	if (cp == NULL)
		return (0);
	if (flag && *cp == '%')
		return (0);
	set = 0;
	while (*cp != '\0') {
		if (*cp == '%') {
			cp++;
			switch (*cp) {
			    case 'r':
				cp++;
				reg = *cp - '0';
				break;
			    case 'a':
				cp++;
				reg = 9;
				break;
			    case 'f':
				cp++;
				reg = 10;
				break;
			    case 's':
				cp++;
				reg = 11;
				break;
			    }
			set = set | 1 << reg;
			}
		cp++;
		}
	return(set);
	}

/* determine registers used for double operand */

int
dscanreg(cp,flag)
char * cp;			/* operand string */
boolean flag;			/* true if register is destination */
{
    int temp = scanreg(cp,flag);	/* do normal scan */

    if (*cp == '%')		/* if direct register reference... */
	temp = 3*temp;		/* set bit and its left neighbor (reg+1) */
    
    return(temp);
}

int
sets(p)		/* set register destination bits */
NODE *p;
	{
	char *ptr;
	int dst(), num;
	int st;
	int dbl;
	int i;
	int t;
	if(dst(p,&ptr,&num)) {
		return(((p->userdata->oflags & SCCODE) ? CCODES : 0) |
			p->userdata->osetdef |
			(p->userdata->otype == TYDOUBLE ?
				3*setreg(ptr) : setreg(ptr)));
	}
	dbl = p->userdata->odblops;
	for(i =1; i <= p->userdata->onumrand; i ++) {
		if((t = (1 << (i-1))) & num) {
			if(t & dbl)
				st |= dscanreg(p->ops[i],false);
			else
				st |= scanreg(p->ops[i],false);
		}
	}
	return(st | p->userdata->osetdef | (p->userdata->oflags & SCCODE ? CCODES : 0));
	}


boolean
side(p)	/* Does a given instruction have a 'side effect' */
NODE *p;	/* other than setting the condition code? */
	{
	return(p->userdata->oflags & SIDE);
	}


boolean
cond(pn)	/* Does a given instruction set the condition codes? */
NODE * pn;
{
	return(pn->userdata->oflags & SCCODE);
}


setreg(cp) char *cp; { /* set index of register destination (if one) */

	if (*cp == '%')
	    switch (*++cp) {
		    case 'r':
			cp++;
			return (1 << (*cp - '0'));
		    case 'a':
			return (1 << 9);
		    case 'f':
			return (1 << 10);
		    case 's':
			return (1 << 11);
		    default:			/* unrecognized name */
			break;
		    }
	return(0);			/* for non-register or unrecognized */
	}

boolean
isdead(cp,p) char *cp; NODE *p; { /* true iff *cp is dead after p */

	int reg;

	if (cp == NULL)
		return(false);
	if (*cp != '%')
		return(false);
	switch(*++cp) {
	    case 'r':
		reg = *++cp - '0';
		break;
	    case 'a':
		reg = 9;
		break;
	    case 'f':
		reg = 10;
		break;
	    case 's':
		reg = 11;
		break;
	    default:			/* unrecognized name */
		return(false);
	    }
	if ((p->nlive & (1 << reg)) == 0)
		return(true);
	    else
		return(false);
	}
char *
getp(p) NODE *p; { /* return pointer to jump destination operand */

	switch (p->op) {
	    case JMP:  case JSB:   case JZ:    case JNZ:
	    case JCC:  case JCS:   case JVC:   case JVS:
	    case JIOE: case JION:  case JIOM:  case JIOT:
	    case JPOS: case JNPOS: case JNEG: case JNNEG: case JE:
	    case JNE:  case JL:    case JLE:  case JG:    case JGE:
	    case JLU:  case JLEU:  case JGU:  case JGEU:
	    case MOVAW:
		return(p->op1);
	    case RET:
	    case RSB:
	    case SWITCH:
	    case SWITCHT:
	        return(NULL);
	    case JBC: case JBS:
		return(p->op3);
	    }
	}

char *
newlab() { /* generate a new label */

	static int lbn = 0;
	char *c;

	c = (char *)getspace(lbn < 100 ? 6 : (lbn < 1000 ? 7 : 8));
	sprintf(c, "..%d\0", lbn);
	lbn++;
	return(c);
	}
