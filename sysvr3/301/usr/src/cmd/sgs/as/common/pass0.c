/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/pass0.c	1.28"

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include <paths.h>
#include "systems.h"
#include "sgs.h"
#include "gendefs.h"

/*
 *
 *	"pass0.c" is a file containing the source for the setup program
 *	for the SGS Assemblers.  Routines in this file parse the
 *	arguments, determining the source and object file names, invent
 *	names for temporary files, and execute the first pass of the
 *	assembler.
 *
 *	This program can be invoked with the command:
 *
 *			as [flags] ifile [-o ofile]
 *
 *	where [flags] are optional user-specified flags,
 *	"ifile" is the name of the assembly languge source file
 *	and "ofile" is the name of a file where the object program is
 *	to be written.  If "ofile" is not specified on the command line,
 *	it is created from "ifile" by the following algorithm:
 *
 *	     1.	If the name "ifile" ends with the two characters ".s",
 *		these are replaced with the two characters ".o".
 *
 *	     2.	If the name "ifile" does not end with the two
 *		characters ".s" and is no more than 12 characters in
 *		length, the name "ofile" is created by appending the
 *		characters ".o" to the name "ifile".
 *
 *	     3.	If the name "ifile" does not end with the two
 *		characters ".s" and is greater than 12 characters in
 *		length, the name "ofile" is created by appending the
 *		characters ".o" to the first 12 characters of "ifile".
 *
 *	The global array "filenames" is used to store all of the file names:
 *	filenames[0]		ifile
 *	filenames[1]		ofile
 *	filenames[2]		temp file 1
 *	filenames[NFILES - 1]	temp files NFILES-2
 *
 */

#define NO		0
#define YES		1
#define	MAXFLAGS	16 - 9	/* sizeof(xargp)-num of args required for pass1 */

#if ONEPROC
short passnbr = 0;

extern short opt;
extern short workaround;
#if	M32
extern int warlevel;
extern int need_mau;
#endif
extern short
	tstlookup,
	Oflag,
	uflag,
	dlflag;

extern char *filenames[];

#if M4ON
extern short rflag;
#endif

#if M32RSTFIX
extern short rstflag;	/* Don't generate workaround for RESTORE chip bug */
#endif

#endif

char	*tempnam();

static char prefix[]="asX";
short	transvec = NO,
	argindex = 1,
	flagindex = 0;

#if !ONEPROC
static char
	nextpass[15];

static char
	teststr[4] = {'-','t','\0','\0'};

static char
	*xargp[NFILES + 9];
#endif

static char
	flagstr[8][3];

#if M4ON
static short
	macro = MACRO;
static char *m4name = M4;
#if !NODEFS
static char
	*regdef = CM4DEFS
#if TRANVEC
	,
	*tvdef  = CM4TVDEFS
#endif
	;
#endif
#endif

#if !ONEPROC
static short
	testas = -2;
#endif

extern char
	*strcpy(),
	*strcat(),
	*strncat();

#define max(A,B) (((A)<(B))?(B):(A))

extern char *malloc(), *getenv(), *mktemp();
extern int access();

static char *pcopy(), *seed="AAA";

/*
 *
 *	"tempnam" is the routine that allocates the temporary files needed
 *	by the assembler.  It searches directories for enough space for the
 *	files, then assigns a (guaranteed) unique filename using the prefix
 *	in the function call.
 *
 */

/*
 *
 *	"getargs" is a general purpose argument parsing routine.
 *	It locates flags (identified by a preceding minus sign ('-'))
 *	and initializes any associated flags for the assembler.
 *	If there are any file names in the argument list, then a
 *	pointer to the name is stored in a global variable for
 *	later use.
 *
 */

getargs(xargc,xargv)
	register int xargc;
	register char **xargv;
{
	register char ch;

	while (xargc-- > 0) {
		if (**xargv == '-') {
			while ((ch = *++*xargv) != '\0')
				switch (ch) {
#if M4ON
				case 'm':
					macro = ! macro;
					break;
				case 'Y': /* specify directory for m4 and/or cm4defs */
					{ char *dirname, *keys;

					  keys = dirname = (*++xargv);
					  xargc--;
					  while (*dirname)
						if (*dirname == ',') {
							dirname++;
							break;
						} else dirname++;
					  while (*keys != ',' && *keys != '\0'){
						switch (*keys) {
						case 'm' :
							m4name=malloc(strlen(dirname)+4);
							strcpy(m4name,dirname);
							strcat(m4name,"/m4");
							break;
#if !NODEFS
						case 'd' :
#if TRANVEC
							if (transvec) {
								regdef=malloc(strlen(dirname)+11);
								strcpy(tvdef,dirname);
								strcat(tvdef,"/cm4tvdefs");
								break;
							}
#endif
							regdef=malloc(strlen(dirname)+9);
							strcpy(regdef,dirname);
							strcat(regdef,"/cm4defs");
							break;
#endif
						default :
							werror(
							"Unknown -Y suboption");
							break;
						}
						keys++;
					  }
					  *xargv = dirname + strlen(dirname) - 1;
					}
					break;
#endif
				case 'o':
					filenames[1]=malloc(strlen(*(++xargv))+1);
					strcpy(filenames[1],*(xargv));
					--xargc;
					while (*++*xargv != '\0') ;
					--*xargv;
					break;
				case 'd':
					if (*++*xargv == 'l')
#if !ONEPROC
						xargp[argindex++] = "-dl";
#else
						dlflag = YES;
#endif
					break;

				case 'u':
#if ONEPROC
					uflag = YES;
#else
					xargp[argindex++] = "-u";
#endif
					break;

#if TRANVEC || !ONEPROC
				case 't':
					++*xargv;
#endif
#if TRANVEC
					if (**xargv == 'v'){
						transvec = YES;
#if !ONEPROC
						xargp[argindex++] = "-tv";
#endif
						break;
					}
#endif
#if !ONEPROC
					if (isdigit(**xargv)) {
						testas = **xargv - '0' - 1;
						if (testas > -1) {
							teststr[2] = (char)(testas + '0');
						}
					}
					else {
						--*xargv;
						testas += 2;
					}
					xargp[argindex++] = teststr;
					break;
#endif
				case 'V':
					/* version flag */
					fprintf(stderr,"%sas: assembler - %s\n",
						SGS,RELEASE);
					break;
#if ONEPROC
				case 'n':
					switch (*++*xargv) {
					case 'f': /* -nf option */
					/* no frills; disable work-arounds */
						workaround = NO;
#if M32RSTFIX
						rstflag = NO;
#endif	/* M32RSTFIX */
						break;
#if	M32
					case 'a': /* -na option */
						/* disable 32001 work-arounds */
						warlevel = NO_AWAR;
						break;
					case 'm': /* -nm option */
						/* set mau required flag */
						need_mau = YES;
						break;
#endif
					default:	/* -n option */
						opt = NO;
						*--*xargv;
					}
					break;
#if DEBUG
				case 'O':
					Oflag = YES;
					break;
				case 'T': {
					switch (*++*xargv) {
						case 'L' : {
							tstlookup = YES;
							break;
						}
					}
					break;
				}
#endif
#if M4ON
				case 'R':
					rflag = YES;
					break;
#endif
#if M32RSTFIX
				case 'r': /* Don't generate the workaround
					   * for the RESTORE chip bug. 
                                           */
					rstflag = NO;
					break;
#endif	/* M32RSTFIX */
#endif
#if VAX
				case 'j': /* invoke ljas instead of as */
					if (strcmp(xargv[0],"ljas")) {
						execvp("ljas",xargv);
						aerror("cannot exec ljas");
						};
					break;
#endif
				default:
#if ONEPROC
					flags(ch);
#else
					flagstr[flagindex][0] = '-';
					flagstr[flagindex][1] = ch;
					xargp[argindex++] = &(flagstr[flagindex++][0]);
#endif
					break;
				}
			xargv++;
		}
		else {
			filenames[0] = *xargv++;
		}
	}
}

/*
 *
 *	"main" is the main driver for the assembler. It calls "getargs"
 *	to parse the argument list, set flags, and stores pointers
 *	to any file names in the argument list .
 *	If the output file was not specified in the argument list
 *	then the output file name is generated. Next the temporary
 *	file names are generated and the first pass of the assembler
 *	is invoked.
 *
 */

main(argc,argv)
	int argc;
	char **argv;
{
	register short index, count;
#if !ONEPROC
	register char *temp;
#endif
	FILE	*fd;

	argc--;
	if (argc <= 0) {
		fprintf(stderr,"Usage: %sas [options] file\n",SGS);
		exit(1);
	}
#if !ONEPROC
	temp = *argv;
	index = -1;
	while (temp[++index] != '\0') ;
	while (--index >= 0 && temp[index] != '/') ;
	strcpy(nextpass,&temp[++index]);
	nextpass[strlen(nextpass)] = '1';
#endif
	argv++;
	getargs(argc, argv);
/*	Check to see if input file exits */
	if ((fd = fopen(filenames[0],"r")) != NULL)
		fclose(fd);
	else {
		fprintf(stderr,"Nonexistent file\n");
		exit(1);
	}
	if (filenames[1] == 0) {
		for(index=0,count=0;filenames[0][index]!='\0';index++,count++)
			if(filenames[0][index]=='/')
				count = -1;
		filenames[1] = malloc(strlen(filenames[0]+index-count)+1);
		strcpy(filenames[1],filenames[0]+index-count);
		if(filenames[1][count-2]=='.' &&
		   filenames[1][count-1]=='s')
			filenames[1][count-1]='o';
		else
		{
			if(count>12)
				count = 12;
			strcpy(filenames[1]+count,".o");
		}
	}

	for (index = 2; index < NFILES; index++) {
		prefix[2] = 'A' + index -1;
		filenames[index] = tempnam(TMPDIR,prefix);
		}

#if !ONEPROC
	xargp[0] = nextpass;
#endif
#if M4ON
	if (macro) {
		/* tell pass1 to unlink its input when through */
#if ONEPROC
		rflag = YES;
#else	
		xargp[argindex++] = "-R";
#endif
		callm4();
	}
#endif

#if ONEPROC
	exit(aspass1());
#else
	if (argindex > MAXFLAGS) {
		fprintf(stderr,"Too many flags\n");
		exit (1);
	}

	for (index=0; index < NFILES; index++)
		xargp[argindex++] = filenames[index];
	xargp[argindex] = 0;
	if (testas != -1) {
		if (testas > -1) {
			execv(NAS1,xargp);
		}
		else
			execv(AS1,xargp);
	fprintf(stderr,"Assembler Error - Cannot Exec Pass 1\n");
	exit(1);
	}
#endif
} /* main */

#if M4ON
callm4()
{
	static char
		*av[] = { "m4", 0, 0, 0};

	/*
	*	The following code had to be added with a 'u370' ifdef due
	*	to a MAXI bug concerning static pointers. We think it's fixed
	*	that is why the code is commented out. It can be deleted
	*	when this fact is verified.
	*
	*	char *regdef, *tvdef;
	*	regdef = CM4DEFS;
	*	tvdef = CM4TVDEFS;
	*/

	av[0] = m4name;
#if !NODEFS
#if TRANVEC
	av[1] = (transvec) ? tvdef : regdef;
	av[2] = filenames[0];
#else
	av[1] = regdef;
	av[2] = filenames[0];
#endif
#else
	av[1] = filenames[0];
#endif
	prefix[2] = 'A';
	filenames[0] = tempnam(TMPDIR,prefix); 		/* m4 output file */
	if (callsys(m4name, av, filenames[0]) != 0) {
		unlink(filenames[0]);
		fprintf(stderr,"Assembly inhibited\n");
		exit(100);
	}
	return;
} /* callm4 */

callsys(f,v,o)
	char f[], *v[];
	char *o;	/* file name, if any, for redirecting stdout */
{
	int t, status;

	if ((t=fork())==0) {
		if (o != NULL) {	/* redirect stdout */
			if (freopen(o, "w", stdout) == NULL) {
				fprintf(stderr,"Can't open %s\n", o);
				exit(100);
			}
		}
		execv(f, v);
		fprintf(stderr,"Can't find %s\n", f);
		exit(100);
	} else
		if (t == -1) {
			fprintf(stderr,"Try again\n");
			return(100);
		}
	while(t!=wait(&status));
	if ((t=(status&0377)) != 0) {
		if (t!=SIGINT)		/* interrupt */
			{
			fprintf(stderr,"status %o\n",status);
			fprintf(stderr,"Fatal error in %s\n", f);
			}
		exit(100);
	}
	return((status>>8) & 0377);
} /* callsys */
#endif
