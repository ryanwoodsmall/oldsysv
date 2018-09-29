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
/*	@(#)main.c	1.3	*/
char *xxxvers[] = "\n@(#) FORTRAN 77 PASS 1, VERSION 2.14,  4 MARCH 1981\n";

#include "defs"
#include <signal.h>

#ifdef HOG			/* plock compiler */
#include <sys/lock.h>
extern int errno;
#endif

#include <a.out.h>


main(argc, argv)
int argc;
char **argv;
{
char *s;
int k, retcode, *ip;
FILEP opf();
int flovflo();

#define DONE(c)	{ retcode = c; goto finis; }

signal(SIGFPE, flovflo);  /* catch overflows */

#if HERE == PDP11
	ldfps(01200);	/* trap on overflow */
#endif


#ifdef HOG
	if (plock(PROCLOCK) == -1)
		fatali("plock fails, errno = %d\n", errno);
#endif


--argc;
++argv;

while(argc>0 && argv[0][0]=='-')
	{
	for(s = argv[0]+1 ; *s ; ++s) switch(*s)
		{
		case 'w':
			if(s[1]=='6' && s[2]=='6')
				{
				ftn66flag = YES;
				s += 2;
				}
			else
				nowarnflag = YES;
			break;

		case 'U':
			shiftcase = NO;
			break;

		case 'u':
			undeftype = YES;
			break;

		case 'O':
			optimflag = YES;
			if( isdigit(s[1]) )
				{
				k = *++s - '0';
				if(k > MAXREGVAR)
					{
					warn1("-O%d: too many register variables", (char *)k);
					maxregvar = MAXREGVAR;
					}
				else
					maxregvar = k;
				}
			break;

		case 'd':
			debugflag = YES;
			break;

		case 'p':
			profileflag = YES;
			break;

		case 'C':
			checksubs = YES;
			break;

		case '6':
			no66flag = YES;
			noextflag = YES;
			break;

		case '1':
			onetripflag = YES;
			break;

#ifdef SDB
		case 'g':
			sdbflag = YES;
			break;
#endif

		case 'N':
			switch(*++s)
				{
				case 'q':
					ip = &maxequiv; goto getnum;
				case 'x':
					ip = &maxext; goto getnum;
				case 's':
					ip = &maxstno; goto getnum;
				case 'c':
					ip = &maxctl; goto getnum;
				case 'n':
					ip = &maxhash; goto getnum;

				default:
					fatali("invalid flag -N%c", *s);
				}
		getnum:
			k = 0;
			while( isdigit(*++s) )
				k = 10*k + (*s - '0');
			if(k <= 0)
				fatal("Table size too small");
			*ip = k;
			break;

		case 'I':
			if(*++s == '2')
				tyint = TYSHORT;
			else if(*s == '4')
				{
				shortsubs = NO;
				tyint = TYLONG;
				}
			else if(*s == 's')
				shortsubs = YES;
			else
				fatali("invalid flag -I%c\n", *s);
			tylogical = tyint;
			break;

		default:
			fatali("invalid flag %c\n", *s);
		}
	--argc;
	++argv;
	}

if(argc != 4)
	fatali("arg count %d", argc);
asmfile  = opf(argv[1]);
initfile = opf(argv[2]);

#ifdef FONEPASS
	if (!freopen(argv[3], "w", stdout))
	{
		perror(argv[3]);
		fatal("cannot reopen stdout");
		/* NOTREACHED */
	}
#else
textfile = opf(argv[3]);
#endif /*FONEPASS*/

initkey();
if(inilex( copys(argv[0]) ))
	DONE(1);
fprintf(diagfile, "%s:\n", argv[0]);


#ifdef SDB
#	if TARGET == PDP11
	for(s = argv[0] ; ; s += 8)
	{
	prstab(s,N_SO,0,NULL);
	if( strlen(s) < 8 )
		break;
	}
#	else
	{
	if (sdbflag)		/* put out Complex and Dcomplex */
		{		/* structure definitions */
		p2pass("\t.text\n\t.def\tComplex;\t.scl\t10;\t.type\t010;\t.size\t8;\t.endef");
		p2pass("\t.def\treal;\t.val\t0;\t.scl\t8;\t.type\t6;\t.endef");
		p2pass("\t.def\timag;\t.val\t4;\t.scl\t8;\t.type\t6;\t.endef");
		p2pass("\t.def\t.eos;\t.val\t8;\t.scl\t102;\t.tag\tComplex;\t.endef");
		p2pass("\t.def\tDcomplex;\t.scl\t10;\t.type\t010;\t.size\t16;\t.endef");
		p2pass("\t.def\treal;\t.val\t0;\t.scl\t8;\t.type\t7;\t.endef");
		p2pass("\t.def\timag;\t.val\t8;\t.scl\t8;\t.type\t7;\t.endef");
		p2pass("\t.def\t.eos;\t.val\t16;\t.scl\t102;\t.tag\tDcomplex;\t.endef");
		}
	}
#	endif
#endif

fileinit(debugflag);
procinit();
if(k = yyparse())
	{
	fprintf(diagfile, "Bad parse, return code %d\n", k);
	DONE(1);
	}
if(nerr > 0)
	DONE(1);
if(parstate != OUTSIDE)
	{
	warn("missing END statement");
	endproc();
	}
doext();
preven(ALIDOUBLE);
prtail();
#if FAMILY==PCC
	puteof();
#endif

if(nerr > 0)
	DONE(1);
DONE(0);


finis:
	done(retcode);
	return(retcode);
}



done(k)
int k;
{
void exit();
static int recurs	= NO;

if(recurs == NO)
	{
	recurs = YES;
	clfiles();
	}
exit(k);
}


LOCAL FILEP opf(fn)
char *fn;
{
FILEP fp;
if( fp = fopen(fn, "a") )
	return(fp);

fatalstr("cannot open intermediate file %s", fn);
/* NOTREACHED */
}



LOCAL clfiles()
{
clf(&textfile);
clf(&asmfile);
clf(&initfile);
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




flovflo()
{
err("floating exception during constant evaluation");
#if HERE == VAX || HERE == NSC
	fatal("vax cannot recover from floating exception");
	/* vax returns a reserved operand that generates
	   an illegal operand fault on next instruction,
	   which if ignored causes an infinite loop.
	*/
#endif
signal(SIGFPE, flovflo);
}
