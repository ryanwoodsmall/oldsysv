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
/*	@(#)driver.c	1.7	*/

#include <stdio.h>
#include <ctype.h>
#include "defines"
#include "machdefs"
#include "drivedefs"
#include "ftypes"
#include <signal.h>

static FILEP diagfile	= {stderr} ;
static int sigivalue	= 0;
static int sigqvalue	= 0;
static int sighvalue	= 0;
static int sigtvalue	= 0;

static char *pass1name	= PASS1NAME ;
static char *pass2name	= PASS2NAME ;
static char *asmname	= ASMNAME ;
static char *sortname	= SORTNAME;
static char *pass2opt	= PASS2OPT;
static char *ldname	= LDNAME ;
static char *footname	= FOOTNAME;
static char *proffoot	= PROFFOOT;
static char *macroname	= MACRONAME;
static char *ccname	= CCNAME;
static char *eflname	= EFLNAME;
static char *ratname	= RATNAME;
static char *aoutname	= "a.out" ;
static char *temppref;
#ifdef NOFLFOOT
static char *noflfoot	= NOFLFOOT;
static char *noflprof	= NOFLPROF;
#endif

static char *infname;
static char textfname[40];
static char asmfname[40];
static char initfname[40];
static char sortfname[40];
static char prepfname[40];
static char objfdefault[40];
static char optzfname[40];
static char setfname[40];

static char fflags[50]	= "-";
static char cflags[20]	= "-c";
static char eflags[30]	= "system=unix ";
static char rflags[30]	= "";
static char lflag[3]	= "-x";
static char *fflagp	= fflags+1;
static char *cflagp	= cflags+2;
static char *eflagp	= eflags+12;
static char *rflagp	= rflags;
static char **loadargs;
static char **loadp;

static flag erred	= NO;
static flag loadflag	= YES;
static flag saveasmflag	= NO;
static flag profileflag	= NO;
static flag optimflag	= NO;
static flag debugflag	= NO;
static flag verbose	= NO;
static flag nofloating	= NO;
static flag fortonly	= NO;
static flag macroflag	= NO;



main(argc, argv)
int argc;
char **argv;
{
int i, c, status, gflg = 0;
char *setdoto(), *lastchar(), *lastfield(), *copys();
ptr ckalloc();
register char *s;
char fortfile[20], *t;
char buff[100];
int intrupt();

sigivalue = (int) signal(SIGINT, SIG_IGN) & 01;
sigqvalue = (int) signal(SIGQUIT,SIG_IGN) & 01;
sighvalue = (int) signal(SIGHUP, SIG_IGN) & 01;
sigtvalue = (int) signal(SIGTERM,SIG_IGN) & 01;
enbint(intrupt);

temppref = tempnam(NULL,"F77");		/* create temporary file name prefix */
crfnames();				/* create file names */

loadargs = (char **) ckalloc( (argc+20) * sizeof(*loadargs) );

#if HERE==PDP11
loadargs[0] = LDNAME;
loadargs[1] = "-X";
loadargs[2] = "-u";
loadargs[3] = "_MAIN__";
loadp = loadargs + 4;
#endif

#if HERE==X3B
loadargs[0] = LDNAME;
loadargs[1] = "-u";
loadargs[2] = "MAIN__";
loadargs[3] = "-t";
loadp = loadargs + 4;
#endif

#if HERE==VAX || HERE==NSC
loadargs[0] = LDNAME;
loadargs[1] = "-u";
loadargs[2] = "_MAIN__";
loadargs[3] = "-t";
loadp = loadargs + 4;
#endif

--argc;
++argv;

while(argc>0 && argv[0][0]=='-' && argv[0][1]!='\0')
	{
	for(s = argv[0]+1 ; *s ; ++s) switch(*s)
		{
		case 'T':  /* use special passes */
			switch(*++s)
				{
				case '1':
					pass1name = s+1; goto endfor;
				case '2':
					pass2name = s+1; goto endfor;
				case 'a':
					asmname = s+1; goto endfor;
				case 'i':
					pass2opt = s+1; goto endfor;
				case 'l':
					ldname = s+1; goto endfor;
				case 'f':
					footname = proffoot = s+1;
#					ifdef NOFLFOOT
					noflfoot = noflprof = s+1;
#					endif
					goto endfor;
				case 'm':
					macroname = s+1; goto endfor;
				case 's':
					sortname = s+1; goto endfor;
				case 'c':
					ccname = s+1; goto endfor;
				case 'e':
					eflname = s+1; goto endfor;
				case 'r':
					ratname = s+1; goto endfor;
				default:
					fatali("bad option -T%c", *s);
				}
			break;

		case '6':
			if(s[1]=='6')
				{
				*fflagp++ = *s++;
				goto copyfflag;
				}
			else	{
				fprintf(diagfile, "invalid flag 6%c\n", s[1]);
				done(1);
				}

		case 'w':
			if(s[1]=='6' && s[2]=='6')
				{
				*fflagp++ = *s++;
				*fflagp++ = *s++;
				}

		copyfflag:
		case 'u':
		case 'U':
		case 'M':
		case '1':
		case 'C':
			*fflagp++ = *s;
			break;
		case 'g':
			gflg = 1;
			goto copyfflag;

		case 'O':
			optimflag = YES;
			*fflagp++ = 'O';
			if( isdigit(s[1]) )
				*fflagp++ = *++s;
			break;

		case 'N':
			*fflagp++ = 'N';
			if( oneof(*++s, "qxscn") )
				*fflagp++ = *s++;
			else	{
				fprintf(diagfile, "invalid flag -N%c\n", *s);
				done(1);
				}
			while( isdigit(*s) )
				*fflagp++ = *s++;
			*fflagp++ = 'X';
			goto endfor;

		case 'm':
			if(s[1] == '4')
				++s;
			macroflag = YES;
			break;

		case 'S':
			saveasmflag = YES;

		case 'c':
			loadflag = NO;
			break;

		case 'v':
			verbose = YES;
			break;

		case 'd':
			debugflag = YES;
			verbose = YES;
			goto copyfflag;

		case 'p':
			profileflag = YES;
			*cflagp++ = 'p';
			goto copyfflag;

		case 'o':
			if( ! strcmp(s, "onetrip") )
				*fflagp++ = '1';
			else if (*++s)
				{
				aoutname = s;
				s += strlen(s)-1;
				}
			else
				{
				s--;
				aoutname = *++argv;
				--argc;
				}
			goto endfor;

		case 'f':
			nofloating = YES;
			break;

		case 'F':
			fortonly = YES;
			loadflag = NO;
			break;

		case 'I':
			fprintf(diagfile,"-I flag no longer supported\n");
			if(s[1]=='2' || s[1]=='4' || s[1]=='s') s++;
/*				{
 *				*fflagp++ = *s++;
 *				goto copyfflag;
 *				}
 *			fprintf(diagfile, "invalid flag -I%c\n", s[1]);
 *			done(1);
 */
			break;

		case 'l':	/* letter ell--library */
			s[-1] = '-';
			*loadp++ = s-1;
			goto endfor;

		case 'E':	/* EFL flag argument */
			while( *eflagp++ = *++s)
				;
			*eflagp++ = ' ';
			goto endfor;
		case 'R':
			while( *rflagp++ = *++s )
				;
			*rflagp++ = ' ';
			goto endfor;
		default:
			lflag[1] = *s;
			*loadp++ = copys(lflag);
			break;
		}
endfor:
	--argc;
	++argv;
	}

*fflagp = '\0';

loadargs[0] = ldname;
#ifdef NOFLFOOT
	if(nofloating)
		*loadp++ = (profileflag ? noflprof : noflfoot);
	else
#endif
*loadp++ = (profileflag ? proffoot : footname);
if (gflg)
	*loadp++ = "/usr/lib/libg.a";

for(i = 0 ; i<argc ; ++i)
	switch(c =  dotchar(infname = argv[i]) ) {
	case 'r':	/* Ratfor file */
	case 'e':	/* EFL file */
		if( unreadable(argv[i]) )
			{
			erred = YES;
			break;
			}
		s = fortfile;
		t = lastfield(argv[i]);
		while( *s++ = *t++)
			;
		s[-2] = 'f';

		if(macroflag)
			{
			sprintf(buff, "%s %s >%s", macroname, infname, prepfname);
			if( sys(buff) )
				{
				rmf(prepfname);
				erred = YES;
				break;
				}
			infname = prepfname;
			}

		if(c == 'e')
			sprintf(buff,"%s %s %s >%s", eflname, eflags,
				infname, fortfile);
		else
			sprintf(buff,"%s %s %s >%s",ratname,rflags,infname,fortfile);
		status = sys(buff);
		if(macroflag)
			rmf(infname);
		if(status)
			{
			erred = YES;
			rmf(fortfile);
			break;
			}

		if( ! fortonly )
			{
			infname = argv[i] = lastfield(argv[i]);
			*lastchar(infname) = 'f';

			if( dofort(argv[i]) )
				erred = YES;
			else	{
				if( nodup(t = setdoto(argv[i])) )
					*loadp++ = t;
				rmf(fortfile);
				}
			}
		break;

	case 'f':	/* Fortran file */
	case 'F':
		if( unreadable(argv[i]) )
			erred = YES;
		else if( dofort(argv[i]) )
			erred = YES;
		else if( nodup(t=setdoto(argv[i])) )
			*loadp++ = t;
		break;

	case 'c':	/* C file */
	case 's':	/* Assembler file */
		if( unreadable(argv[i]) )
			{
			erred = YES;
			break;
			}
		sprintf(buff,"%s -c %s",ccname,argv[i]);
		if( sys(buff) )
			erred = YES;
		else
			if( nodup(t = setdoto(argv[i])) )
				*loadp++ = t;
		break;

	case 'o':
		if( nodup(argv[i]) )
			*loadp++ = argv[i];
		break;

	default:
		if( ! strcmp(argv[i], "-o") )
			aoutname = argv[++i];
		else if (*argv[i] == '-' && *(argv[i]+1) == 'o')
			aoutname = argv[i]+2;
		else
			*loadp++ = argv[i];
		break;
		}

if(loadflag && !erred)
	doload(loadargs, loadp);
done(erred);
}

/* process	input file		output file(s)
 *
 * f77pass1	(FILE.f)		asmfname  (*.s)
 *				  	initfname (*.d)
 *					textfname (*.x)
 * sort		initfname (*.d)		sortfname (*.S)
 * dodata	sortfname (*.S)		asmfname  (*.s)
 * pass2	textfname (*.x)		asmfname  (*.s)
 * optim	asmfname  (*.s)		optzfname (*.z)
 * asm		asmfname/optzfname      (FILE.o)
 */

dofort(s)
char *s;
{
FILEP fp;
int retcode;
char buff[200];

infname = s;

if( (fp = fopen(asmfname, "w")) == NULL)	/* this is to get the */
	badfile(asmfname);			/* Lflt statics on the */
else						/* first line.    */
	{					/* and .file for vax */
	prjunk(fp, infname);
	fclose(fp);
	}


/* PASS 1: FORTRAN front-end */
sprintf(buff, "%s %s %s %s %s %s",
	pass1name, fflags, s, asmfname, initfname, textfname);
switch( sys(buff) )
	{
	case 1:
		goto error;
	case 0:
		break;
	default:
		goto comperror;
	}


/* If initialized data, sort and translate into assembler pseudo-ops */
if (content(initfname) > 0)
	if (dodata())
		goto error;


/* PASS 2: code generator */
sprintf(buff,"%s <%s >>%s", pass2name, textfname, asmfname);
if (sys(buff))
	goto comperror;


/* Optimizer and assembler */
doasm(s);
retcode = 0;

ret:
	rmf(asmfname);
	rmf(initfname);
	rmf(textfname);
	rmf(optzfname);
	return(retcode);

error:
	fprintf(diagfile, "\nError.  No assembly.\n");
	retcode = 1;
	goto ret;

comperror:
	fprintf(diagfile, "\ncompiler error.\n");
	retcode = 2;
	goto ret;
}




doasm(s)
char *s;
{
register char *lastc;
char *obj;
char buff[200];
char *lastchar(), *setdoto();
char *afile;

afile = asmfname;
if(*s == '\0')
	s = objfdefault;
lastc = lastchar(s);
obj = setdoto(s);

if (optimflag) {
#	if TARGET==X3B
	sprintf(buff, "%s -I %s -O %s", pass2opt, asmfname, optzfname);
#	else
	sprintf(buff, "%s %s %s", pass2opt, asmfname, optzfname);
#	endif
	if (!sys(buff))
		afile = optzfname;
	}

if (saveasmflag) {
	*lastc = 's';
	sprintf(buff,"/bin/mv %s %s",afile,obj);
	sys(buff);
	*lastc = 'o';
	}
else {
	sprintf(buff, "%s -o %s %s", asmname, obj, afile);
	if (sys(buff))
		fatal("assembler error");
	}
}



doload(v0, v)
register char *v0[], *v[];
{
char **p;
int waitpid;

for (p = (profileflag ? pliblist : liblist); *p; *v++ = *p++);

*v++ = "-o";
*v++ = aoutname;
*v = NULL;

if (verbose)
	{
	fprintf(diagfile,"\n");
	for(p = v0 ; p<v ; ++p)
		fprintf(diagfile, "%s ", *p);
	fprintf(diagfile, "\n");
	}

if( (waitpid = fork()) == 0)
	{
	enbint(SIG_DFL);
	execv(ldname, v0);
	fatalstr("couldn't load %s", ldname);
	}
await(waitpid);
}

/* Process control and Shell-simulating routines */

sys(str)
char *str;
{
register char *t;
char *argv[100];
char *inname, *outname;
int append;
int waitpid;
int argc;


if (verbose)
	fprintf(diagfile,"\n%s\n", str);
inname  = NULL;
outname = NULL;
argc = 0;

t = str;
while( isspace(*t) )
	++t;
while(*t)
	{
	if(*t == '<')
		inname = t+1;
	else if(*t == '>')
		{
		if(t[1] == '>')
			{
			append = YES;
			outname = t+2;
			}
		else	{
			append = NO;
			outname = t+1;
			}
		}
	else
		argv[argc++] = t;
	while( !isspace(*t) && *t!='\0' )
		++t;
	if(*t)
		{
		*t++ = '\0';
		while( isspace(*t) )
			++t;
		}
	}

if(argc == 0)   /* no command */
	return(-1);
argv[argc] = 0;

if((waitpid = fork()) == 0)
	{
	if(inname)
		if (!freopen(inname, "r", stdin))
			fatalstr("Cannot reopen stdin to %s", inname);
	if(outname)
		if (!freopen(outname, (append ? "a" : "w"), stdout))
			fatalstr("Cannot reopen stdout to %s", outname);
	enbint(SIG_DFL);

	execv(argv[0],argv);

	fatalstr("Cannot load %s",argv[0]);
	}

return( await(waitpid) );
}



done(k)
int k;
{
static int recurs	= NO;

if(recurs == NO)
	{
	recurs = YES;
	rmfiles();
	}
exit(k);
}






enbint(k)
int (*k)();
{
if(sigivalue == 0)
	signal(SIGINT,k);
if(sigqvalue == 0)
	signal(SIGQUIT,k);
if(sighvalue == 0)
	signal(SIGHUP,k);
if(sigtvalue == 0)
	signal(SIGTERM,k);
}




intrupt()
{
done(2);
}



await(waitpid)
int waitpid;
{
int w, status, rc;

enbint(SIG_IGN);
while ( (w = wait(&status)) != waitpid)
	if(w == -1)
		fatal("bad wait code");
enbint(intrupt);
if(status & 0377)
	{
	if(status != SIGINT)
		fprintf(diagfile, "Termination code %d\n", status);
	done(3);
	}
rc = (status >> 8);
if (verbose)
	fprintf(diagfile,"Process complete. Return code = %d\n",rc);
return(rc);
}

/* File Name and File Manipulation Routines */

unreadable(s)
register char *s;
{
register FILE *fp;

if(fp = fopen(s, "r"))
	{
	fclose(fp);
	return(NO);
	}

else
	{
	fprintf(diagfile, "Error: Cannot read file %s\n", s);
	return(YES);
	}
}



clf(p)
FILEP *p;
{
if(p!=NULL && *p!=NULL && *p!=stdout)
	{
	if(ferror(*p))
		fatal("writing error");
	fclose(*p);
	}
*p = NULL;
}

rmfiles()
{
rmf(textfname);
rmf(asmfname);
rmf(initfname);
rmf(optzfname);
}



/* Return -1 if file cannot be accessed, 0 if it is of zero length,
 * and 1 if of positive length.
 */

content(filename)
char *filename;
{
#include <sys/types.h>
#include <sys/stat.h>
struct stat buf;

if(stat(filename,&buf) < 0)
	return(-1);
return( buf.st_size > 0 );
}




crfnames()
{
fname(textfname, "x");
fname(asmfname, "s");
fname(initfname, "d");
fname(sortfname, "S");
fname(objfdefault, "o");
fname(prepfname, "p");
fname(optzfname, "z");
fname(setfname, "A");
}




rmf(fn)
register char *fn;
{
if(!debugflag && fn!=NULL && *fn!='\0')
	unlink(fn);
}





LOCAL fname(name, suff)
char *name, *suff;
{
sprintf(name, "%s.%s", temppref, suff);
}




dotchar(s)
register char *s;
{
for( ; *s ; ++s)
	if(s[0]=='.' && s[1]!='\0' && s[2]=='\0')
		return( s[1] );
return(NO);
}



char *lastfield(s)
register char *s;
{
register char *t;
for(t = s; *s ; ++s)
	if(*s == '/')
		t = s+1;
return(t);
}



char *lastchar(s)
register char *s;
{
while(*s)
	++s;
return(s-1);
}

char *setdoto(s)
register char *s;
{
*lastchar(s) = 'o';
return( lastfield(s) );
}



badfile(s)
char *s;
{
fatalstr("cannot open intermediate file %s", s);
}



ptr ckalloc(n)
int n;
{
ptr p, calloc();

if( p = calloc(1, (unsigned) n) )
	return(p);

fatal("out of memory");
/* NOTREACHED */
}





char *copyn(n, s)
register int n;
register char *s;
{
register char *p, *q;

p = q = (char *) ckalloc(n);
while(n-- > 0)
	*q++ = *s++;
return(p);
}



char *copys(s)
char *s;
{
return( copyn( strlen(s)+1 , s) );
}





oneof(c,s)
register c;
register char *s;
{
while( *s )
	if(*s++ == c)
		return(YES);
return(NO);
}



nodup(s)
char *s;
{
register char **p;

for(p = loadargs ; p < loadp ; ++p)
	if( !strcmp(*p, s) )
		return(NO);

return(YES);
}



static fatal(t)
char *t;
{
fprintf(diagfile, "Compiler error in file %s: %s\n", infname, t);
if(debugflag)
	abort();
done(1);
exit(1);
}




static fatali(t,d)
char *t;
int d;
{
char buff[100];
sprintf(buff, t, d);
fatal(buff);
}




static fatalstr(t, s)
char *t, *s;
{
char buff[100];
sprintf(buff, t, s);
fatal(buff);
}
err(s)
char *s;
{
fprintf(diagfile, "Error in file %s: %s\n", infname, s);
}

/* Code to generate initializations for DATA statements */

LOCAL int nch	= 0;
LOCAL FILEP asmfile;
LOCAL FILEP sortfile;

#include "ftypes"

static ftnint typesize[NTYPES]
	= { 1, SZADDR, SZSHORT, SZLONG, SZLONG, 2*SZLONG,
	    2*SZLONG, 4*SZLONG, SZLONG, 1, 1, 1};
static int typealign[NTYPES]
	= { 1, ALIADDR, ALISHORT, ALILONG, ALILONG, ALIDOUBLE,
	    ALILONG, ALIDOUBLE, ALILONG, 1, 1, 1};

dodata()
{
char buff[120];
char varname[XL+1], ovarname[XL+1];
int status;
flag erred;
ftnint offset, vlen, type, size;
register ftnint ooffset, ovlen;
ftnint nblank, vchar;
int align;
int vargroup;
ftnint totlen, doeven();

erred = NO;
ovarname[0] = '\0';
ooffset = ovlen = totlen = nch = 0;

sprintf(buff,"%s %s >%s",sortname,initfname,sortfname);
if(status = sys(buff))
	fatali("call sort status = %d", status);
if (verbose)
	fprintf(diagfile,"\ndodata <%s >%s\n", sortfname, asmfname);
if( (sortfile = fopen(sortfname, "r")) == NULL)
	badfile(sortfname);
if( (asmfile = fopen(asmfname, "a")) == NULL)
	badfile(asmfname);
pruse(asmfile, USEINIT);

while( rdname(&vargroup, varname) && rdlong(&offset) && rdlong(&vlen) 
  && rdlong(&size) && rdlong(&type) )
	{
	if( strcmp(varname, ovarname) )
		{
		prspace(ovlen-ooffset);
		strcpy(ovarname, varname);
		ooffset = 0;
		totlen += ovlen;
		ovlen = vlen;
		if(vargroup == 0)
			align = (type==TYCHAR || type==TYBLANK ?
					SZLONG : typealign[type]);
		else	align = ALIDOUBLE;
		totlen = doeven(totlen, align);
		if(vargroup == 2)
			prcomblock(asmfile, varname);
		else
			fprintf(asmfile, LABELFMT, varname);
		}
	if(offset < ooffset)
		{
		erred = YES;
		err("overlapping initializations");
		ooffset = offset;
		}
	if(offset > ooffset)
		{
		prspace(offset-ooffset);
		ooffset = offset;
		}
	if(type == TYCHAR)
		{
		if( rdlong(&vchar) )
			prch( (int) vchar );
		else
			fatal("bad intermediate file format");
		}
	else if(type == TYBLANK)
		{
		if( rdlong(&nblank) )
			{
			size = nblank;
			while( --nblank >= 0)
				prch( ' ' );
			}
		else
			fatal("bad intermediate file format");
		}
	else
		{
		putc('\t', asmfile);
		while	( putc( getc(sortfile), asmfile)  != '\n')
			;
		}
	if( (ooffset += size) > ovlen)
		{
		erred = YES;
		err("initialization out of bounds");
		}
	}

prspace(ovlen-ooffset);
totlen = doeven(totlen+ovlen, (ALIDOUBLE>SZLONG ? ALIDOUBLE : SZLONG) );
clf(&sortfile);
clf(&asmfile);
clf(&sortfile);
rmf(sortfname);
if (verbose)
	fprintf(diagfile,"Process complete. Return code = %d\n", erred);
return(erred);
}




prspace(n)
register ftnint n;
{
register ftnint m;

while(nch>0 && n>0)
	{
	--n;
	prch(0);
	}
m = SZSHORT * (n/SZSHORT);
if(m > 0)
	prskip(asmfile, m);
for(n -= m ; n>0 ; --n)
	prch(0);
}




ftnint doeven(tot, align)
register ftnint tot;
int align;
{
ftnint new;
new = roundup(tot, align);
prspace(new - tot);
return(new);
}



rdname(vargroupp, name)
int *vargroupp;
register char *name;
{
register int i, c;

if( (c = getc(sortfile)) == EOF)
	return(NO);
*vargroupp = c - '0';

for(i = 0 ; i<XL ; ++i)
	{
	if( (c = getc(sortfile)) == EOF)
		return(NO);
	if(c != ' ')
		*name++ = c;
	}
*name = '\0';
return(YES);
}



rdlong(n)
register ftnint *n;
{
register int c;

for(c = getc(sortfile) ; c!=EOF && isspace(c) ; c = getc(sortfile) );
	;
if(c == EOF)
	return(NO);

for(*n = 0 ; isdigit(c) ; c = getc(sortfile) )
	*n = 10* (*n) + c - '0';
return(YES);
}




prch(c)
register int c;
{
static int buff[SZSHORT];

buff[nch++] = c;
if(nch == SZSHORT)
	{
	prchars(asmfile, buff);
	nch = 0;
	}
}
