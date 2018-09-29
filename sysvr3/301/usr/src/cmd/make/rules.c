/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)make:rules.c	1.9.1.1"
#include "defs"
/* DEFAULT RULES FOR UNIX */

/*
 *	These are the internal rules that "make" trucks around with it at
 *	all times. One could completely delete this entire list and just
 *	conventionally define a global "include" makefile which had these
 *	rules in it. That would make the rules dynamically changeable
 *	without recompiling make. This file may be modified to local
 *	needs. 
 *	The makefile for make (make.mk) is parameterized for a different
 *	rules file. The macro $(RULES) defined in "make.mk" can be set
 *	to another file and when "make" is "made" the procedure will
 *	use the new file. The recommended way to do this is on the
 *	command line as follows:
 *		"make -f make.mk RULES=pwbrules"
 */

CHARSTAR builtin[] =
	{
	".SUFFIXES: .o .c .c~ .y .y~ .l .l~ .s .s~ .h .h~ .sh .sh~ .f .f~",

/* PRESET VARIABLES */
	"MAKE=make",
	"AR=ar",	"ARFLAGS=rv",
	"AS=as",	"ASFLAGS=",
	"CC=cc",	"CFLAGS=-O",
	"F77=f77",	"F77FLAGS=",
	"GET=get",	"GFLAGS=",
	"LEX=lex",	"LFLAGS=",
	"LD=ld",	"LDFLAGS=",
	"YACC=yacc",	"YFLAGS=",


/* SINGLE SUFFIX RULES */
	".c:",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@",
	".c~:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) $*.c -o $*",
		"\t-rm -f $*.c",
	".f:",
		"\t$(F77) $(F77FLAGS) $(LDFLAGS) $< -o $@",
	".f~:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(F77) $(F77FLAGS) $(LDFLAGS) $< -o $*",
		"\t-rm -f $*.f",
	".sh:",
		"\tcp $< $@; chmod 0777 $@",
	".sh~:",
		"\t$(GET) $(GFLAGS) $<",
		"\tcp $*.sh $*; chmod 0777 $@",
		"\t-rm -f $*.sh",

/* DOUBLE SUFFIX RULES */
	".c.o:",
		"\t$(CC) $(CFLAGS) -c $<",
	".c~.c:",
		"\t$(GET) $(GFLAGS) $<",
	".c~.o:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(CC) $(CFLAGS) -c $*.c",
		"\t-rm -f $*.c",
	".f.a:",
		"\t$(F77) $(F77FLAGS) $(LDFLAGS) -c $*.f",
		"\t$(AR) $(ARFLAGS) $@ $*.o",
		"\t-rm -f $*.o",
	".f.o:",
		"\t$(F77) $(F77FLAGS) $(LDFLAGS) -c $*.f",
	".f~.a:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(F77) $(F77FLAGS) $(LDFLAGS) -c $*.f",
		"\t$(AR) $(ARFLAGS) $@ $*.o",
		"\t-rm -f $*.[fo]",
	".f~.f:",
		"\t$(GET) $(GFLAGS) $<",
	".f~.o:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(F77) $(F77FLAGS) $(LDFLAGS) -c $*.f",
		"\t-rm -f $*.f",
	".s.o:",
		"\t$(AS) $(ASFLAGS) -o $@ $<",
	".s~.o:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(AS) $(ASFLAGS) -o $*.o $*.s",
		"\t-rm -f $*.s",
	".s~.s:",
		"\t$(GET) $(GFLAGS) $<",
	".sh~.sh:",
		"\t$(GET) $(GFLAGS) $<",
	".y.o:",
		"\t$(YACC) $(YFLAGS) $<",
		"\t$(CC) $(CFLAGS) -c y.tab.c",
		"\trm y.tab.c",
		"\tmv y.tab.o $@",
	".y~.o:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(YACC) $(YFLAGS) $*.y",
		"\t$(CC) $(CFLAGS) -c y.tab.c",
		"\trm -f y.tab.c $*.y",
		"\tmv y.tab.o $*.o",
	".l.o:",
		"\t$(LEX) $(LFLAGS) $<",
		"\t$(CC) $(CFLAGS) -c lex.yy.c",
		"\trm lex.yy.c",
		"\tmv lex.yy.o $@",
	".l~.c:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(LEX) $(LFLAGS) $*.l",
		"\tmv lex.yy.c $@",
		"\t-rm -f $*.l",
	".l~.l:",
		"\t$(GET) $(GFLAGS) $<",
	".l~.o:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(LEX) $(LFLAGS) $*.l",
		"\t$(CC) $(CFLAGS) -c lex.yy.c",
		"\trm -f lex.yy.c $*.l",
		"\tmv lex.yy.o $*.o",
	".y.c :",
		"\t$(YACC) $(YFLAGS) $<",
		"\tmv y.tab.c $@",
	".y~.c :",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(YACC) $(YFLAGS) $*.y",
		"\tmv y.tab.c $*.c",
		"\t-rm -f $*.y",
	".y~.y:",
		"\t$(GET) $(GFLAGS) $<",
	".l.c :",
		"\t$(LEX) $(LFLAGS) $<",
		"\tmv lex.yy.c $@",
	".c.a:",
		"\t$(CC) -c $(CFLAGS) $<",
		"\t$(AR) $(ARFLAGS) $@ $*.o",
		"\trm -f $*.o",
	".c~.a:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(CC) -c $(CFLAGS) $*.c",
		"\t$(AR) $(ARFLAGS) $@ $*.o",
		"\trm -f $*.[co]",
	".s~.a:",
		"\t$(GET) $(GFLAGS) $<",
		"\t$(AS) $(ASFLAGS) -o $*.o $*.s",
		"\t$(AR) $(ARFLAGS) $@ $*.o",
		"\t-rm -f $*.[so]",
	".h~.h:",
		"\t$(GET) $(GFLAGS) $<",
	"markfile.o:	markfile",
		"\tA=@;echo \"static char _sccsid[] = \\042`grep $$A'(#)' markfile`\\042;\" > markfile.c",
		"\t$(CC) -c markfile.c",
		"\trm -f markfile.c",
	0 };
