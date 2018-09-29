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
/*	@(#)functs.h	1.2	*/
/* 
 *  Result type declarations 
 */

/* access.c */

unsigned int bchkget();
unsigned int chkget();
unsigned int get();
void         put();

/* command.c */

void command();

/* expr.c */

int  expr();
int  getnum();
int  varchk();

/* format.c */

STRING exform();
int    inkdot();
void   shell();
void   scanform();

/* input.c */

int  eocmd();
void getformat();
int  nextchar();
char quotchar();
char rdc();
char readchar();

/* main.c */

void chkerr();
void done();
void error();
int  round();

/* opcode.c */

void mkioptab();
void printins();

/* output.c */

void adbpr();
void endline();
int  eqstr();
void flushbuf();
void iclose();
void newline();
void oclose();
void printc();
void prints();

/* pcs.c */

void subpcs();

/* print.c */

struct grres getreg();
void         printpc();
void         sigprint();

/* runpcs.c */

void  delbp();
void  endpcs();
int   getsig();
int   runpcs();
BKPTR scanbkpt();
void  setup();

/* setup.c */

void setsym();
void setcor();

/* sym.c */

unsigned int findsym();
void         localsym();
void         psymoff();
SYMPTR       symget();
void         symset();
void         valpr();
