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
/*	@(#)err.c	1.3	*/
/*	3.0 SID #	1.3	*/
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include "fio.h"
#define STR(x) (x==NULL?"":x)

/*global definitions*/
unit units[MXUNIT];	/*unit table*/
flag init;	/*0 on entry, 1 after initializations*/
cilist *elist;	/*active external io list*/
flag reading;	/*1 if reading, 0 if writing*/
flag cplus,cblank;
char *fmtbuf;
flag external;	/*1 if external io, 0 if internal */
int (*doed)(),(*doned)();
int (*doend)(),(*donewrec)(),(*dorevert)();
flag sequential;	/*1 if sequential io, 0 if direct*/
flag formatted;	/*1 if formatted io, 0 if unformatted*/
int (*getn)(),(*putn)();	/*for formatted io*/
FILE *cf;	/*current file*/
unit *curunit;	/*current unit*/
int recpos;	/*place in current record*/
int cursor,scale;

/*error messages*/
char *F_err[] =
{
	"error in format",				/* 100 */
	"illegal unit number",				/* 101 */
	"formatted io not allowed",			/* 102 */
	"unformatted io not allowed",			/* 103 */
	"direct io not allowed",			/* 104 */
	"sequential io not allowed",			/* 105 */
	"can't backspace file",				/* 106 */
	"null file name",				/* 107 */
	"can't stat file",				/* 108 */
	"unit not connected",				/* 109 */
	"off end of record",				/* 110 */
	"truncation failed in endfile",			/* 111 */
	"incomprehensible list input",			/* 112 */
	"out of free space",				/* 113 */
	"unit not connected",				/* 114 */
	"read unexpected character",			/* 115 */
	"blank logical input field",			/* 116 */
	"bad variable type",				/* 117 */
	"bad namelist name",				/* 118 */
	"variable not in namelist",			/* 119 */
	"no end record",				/* 120 */
	"variable count incorrect"			/* 121 */
};
#define MAXERR (sizeof(F_err)/sizeof(char *)+100)
fatal(n,s) char *s;
{
	if(n<100 && n>=0) perror(s); /*SYSDEP*/
	else if(n>=(int)MAXERR)
	{	fprintf(stderr,"%s: illegal error number %d\n",s,n);
	}
	else if(n<0) fprintf(stderr,"%s: end of file %d\n",s,n);
	else
		fprintf(stderr,"%s: %s\n",s,F_err[n-100]);
	fprintf(stderr,"apparent state: unit %d named %s\n",curunit-units,
		STR(curunit->ufnm));
	fprintf(stderr,"last format: %s\n",STR(fmtbuf));
	fprintf(stderr,"lately %s %s %s %s IO\n",reading?"reading":"writing",
		sequential?"sequential":"direct",formatted?"formatted":"unformatted",
		external?"external":"internal");
	_cleanup();
	abort();
}
/*initialization routine*/
f_init()
{	unit *p;
	init=1;
	p= &units[0];
	p->ufd=stderr;
	p->useek=canseek(stderr);
	p->ufmt=1;
	p->uwrt=1;
	p = &units[5];
	p->ufd=stdin;
	p->useek=canseek(stdin);
	p->ufmt=1;
	p->uwrt=0;
	p= &units[6];
	p->ufd=stdout;
	p->useek=canseek(stdout);
	p->ufmt=1;
	p->uwrt=1;
}
canseek(f) FILE *f; /*SYSDEP*/
{	struct stat x;
	(void) fstat(fileno(f),&x);
	if(x.st_nlink > 0 /*pipe*/ && !isatty(fileno(f)))
	{
		return(1);
	}
	return(0);
}
nowreading(x) unit *x;
{
	long loc;
	x->uwrt=0;
	loc=ftell(x->ufd);
	if(freopen(x->ufnm,"r",x->ufd) == NULL)
		return(1);
	(void) fseek(x->ufd,loc,0);
	return(0);
}
nowwriting(x) unit *x;
{
	long loc;
	loc=ftell(x->ufd);
	x->uwrt=1;
	if(!freopen(x->ufnm,"r+",x->ufd) && !freopen(x->ufnm, "w+", x->ufd))
		return(1);
	(void) fseek(x->ufd,loc,0);
	return(0);
}
