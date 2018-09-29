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
/*	@(#)error.c	1.3	*/
#include "defs"


warn1(s,t)
char *s, *t;
{
char buff[100];
sprintf(buff, s, t);
warn(buff);
}


warn(s)
char *s;
{
if(nowarnflag)
	return;
fprintf(diagfile, "Warning on line %d of %s: %s\n", f_lineno, infname, s);
++nwarn;
}


errstr(s, t)
char *s, *t;
{
char buff[100];
sprintf(buff, s, t);
err(buff);
}



erri(s,t)
char *s;
int t;
{
char buff[100];
sprintf(buff, s, t);
err(buff);
}


err(s)
char *s;
{
fprintf(diagfile, "Error on line %d of %s: %s\n", f_lineno, infname, s);
++nerr;
}


yyerror(s)
char *s;
{ err(s); }



dclerr(s, v)
char *s;
Namep v;
{
char buff[100];

if(v)
	{
	sprintf(buff, "Declaration error for %s: %s", varstr(VL, v->varname), s);
	err(buff);
	}
else
	errstr("Declaration error %s", s);
}



execerr(s, n)
char *s, *n;
{
char buf1[100], buf2[100];

sprintf(buf1, "Execution error %s", s);
sprintf(buf2, buf1, n);
err(buf2);
}


fatal(t)
char *t;
{
void exit();

fprintf(diagfile, "Compiler error line %d of %s: %s\n", f_lineno, infname, t);
if(debugflag)
	abort();
done(3);
exit(3);
}




fatalstr(t,s)
char *t, *s;
{
char buff[100];
sprintf(buff, t, s);
fatal(buff);
}



fatali(t,d)
char *t;
int d;
{
char buff[100];
sprintf(buff, t, d);
fatal(buff);
}



badthing(thing, r, t)
char *thing, *r;
int t;
{
char buff[50];
sprintf(buff, "Impossible %s %d in routine %s", thing, t, r);
fatal(buff);
}



badop(r, t)
char *r;
int t;
{
badthing("opcode", r, t);
}



badtag(r, t)
char *r;
int t;
{
badthing("tag", r, t);
}





badstg(r, t)
char *r;
int t;
{
badthing("storage class", r, t);
}




badtype(r, t)
char *r;
int t;
{
badthing("type", r, t);
}


many(s,c,v)
char *s, *c;
int v;
{
char buff[256];

sprintf(buff, "Too many %s.\nTry the '-N%s#' option (# currently = %d)", s, c, v);
fatal(buff);
}


err66(s)
char *s;
{
errstr("Fortran 77 feature used: %s", s);
}



errext(s)
char *s;
{
errstr("F77 compiler extension used: %s", s);
}
