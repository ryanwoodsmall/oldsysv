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
/*	@(#)rules.c	1.2 of 3/28/83	*/
#include "defs"
/* DEFAULT RULES FOR UNIX */

/*
 *	These are the internal rules that "make" trucks around with it at
 *	all times. One could completely delete this entire list and just
 *	conventionally define a global "include" makefile which had these
 *	rules in it. That would make the rules dynamically changeable
 *	without recompiling make. This file may be modified to local
 *	needs. There are currently two versions of this file with the
 *	source; namely, rules.c (which is the version running in Columbus)
 *	and pwbrules.c which is my attempt at satisfying the requirements
 *	of PWB systems.
 *	The makefile for make (make.mk) is parameterized for a different
 *	rules file. The macro $(RULES) defined in "make.mk" can be set
 *	to another file and when "make" is "made" the procedure will
 *	use the new file. The recommended way to do this is on the
 *	command line as follows:
 *		"make -f make.mk RULES=pwbrules"
 */

CHARSTAR builtin[] =
	{
	".SUFFIXES: .o .c .c~ .y .y~ .l .l~ .s .s~ .sh .sh~ .h .h~",

/* PRESET VARIABLES */
	"MAKE=make",
	"YACC=yacc",
	"YFLAGS=",
	"LEX=lex",
	"LFLAGS=",
	"LD=ld",
	"LDFLAGS=",
	"CC=cc",
	"CFLAGS=-O",
	"AS=as",
	"ASFLAGS=",
	"GET=get",
	"GFLAGS=",

/* SINGLE SUFFIX RULES */
	".c:",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) $< -o $@",
	".c~:",
		"\t$(GET) $(GFLAGS) -p $< > $*.c",
		"\t$(CC) $(CFLAGS) $(LDFLAGS) $*.c -o $*",
		"\t-rm -f $*.c",
	".sh:",
		"\tcp $< $@; chmod 0777 $@",
	".sh~:",
		"\t$(GET) $(GFLAGS) -p $< > $*.sh",
		"\tcp $*.sh $*; chmod 0777 $@",
		"\t-rm -f $*.sh",

/* DOUBLE SUFFIX RULES */
	".c.o:",
		"\t$(CC) $(CFLAGS) -c $<",
	".c~.o:",
		"\t$(GET) $(GFLAGS) -p $< > $*.c",
		"\t$(CC) $(CFLAGS) -c $*.c",
		"\t-rm -f $*.c",
	".c~.c:",
		"\t$(GET) $(GFLAGS) -p $< > $*.c",
	".s.o:",
		"\t$(AS) $(ASFLAGS) -o $@ $<",
	".s~.o:",
		"\t$(GET) $(GFLAGS) -p $< > $*.s",
		"\t$(AS) $(ASFLAGS) -o $*.o $*.s",
		"\t-rm -f $*.s",
	".y.o:",
		"\t$(YACC) $(YFLAGS) $<",
		"\t$(CC) $(CFLAGS) -c y.tab.c",
		"\trm y.tab.c",
		"\tmv y.tab.o $@",
	".y~.o:",
		"\t$(GET) $(GFLAGS) -p $< > $*.y",
		"\t$(YACC) $(YFLAGS) $*.y",
		"\t$(CC) $(CFLAGS) -c y.tab.c",
		"\trm -f y.tab.c $*.y",
		"\tmv y.tab.o $*.o",
	".l.o:",
		"\t$(LEX) $(LFLAGS) $<",
		"\t$(CC) $(CFLAGS) -c lex.yy.c",
		"\trm lex.yy.c",
		"\tmv lex.yy.o $@",
	".l~.o:",
		"\t$(GET) $(GFLAGS) -p $< > $*.l",
		"\t$(LEX) $(LFLAGS) $*.l",
		"\t$(CC) $(CFLAGS) -c lex.yy.c",
		"\trm -f lex.yy.c $*.l",
		"\tmv lex.yy.o $*.o",
	".y.c :",
		"\t$(YACC) $(YFLAGS) $<",
		"\tmv y.tab.c $@",
	".y~.c :",
		"\t$(GET) $(GFLAGS) -p $< > $*.y",
		"\t$(YACC) $(YFLAGS) $*.y",
		"\tmv y.tab.c $*.c",
		"\t-rm -f $*.y",
	".l.c :",
		"\t$(LEX) $<",
		"\tmv lex.yy.c $@",
	".c.a:",
		"\t$(CC) -c $(CFLAGS) $<",
		"\tar rv $@ $*.o",
		"\trm -f $*.o",
	".c~.a:",
		"\t$(GET) $(GFLAGS) -p $< > $*.c",
		"\t$(CC) -c $(CFLAGS) $*.c",
		"\tar rv $@ $*.o",
		"\trm -f $*.[co]",
	".s~.a:",
		"\t$(GET) $(GFLAGS) -p $< > $*.s",
		"\t$(AS) $(ASFLAGS) -o $*.o $*.s",
		"\tar rv $@ $*.o",
		"\t-rm -f $*.[so]",
	".h~.h:",
		"\t$(GET) $(GFLAGS) -p $< > $*.h",
	"markfile.o:	markfile",
		"\tA=@;echo \"static char _sccsid[] = \\042`grep $$A'(#)' markfile`\\042;\" > markfile.c",
		"\tcc -c markfile.c",
		"\trm -f markfile.c",
	0 };

