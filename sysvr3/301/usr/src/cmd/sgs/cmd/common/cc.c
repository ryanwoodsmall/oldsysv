/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sgs-cmd:common/cc.c	1.89.1.1"

#define	PARGS	if(debug){printf("%scc: ", prefix);for(j=0;j<nlist[AV];j++)printf(" '%s'",list[AV][j]);printf("\n");}

/*===================================================================*/
/*                                                                   */
/*                 CC Command                                        */
/*                                                                   */
/*                                                                   */
/*-------------------------------------------------------------------*/
/*                                                                   */
/*     The cc command parses the command line to set up command      */
/*     lines for and exec whatever processes (preprocessor,          */
/*     compiler, optimizer, assembler, link editor) are required.    */
/*                                                                   */
/*===================================================================*/


#include	<stdio.h>
#include	<signal.h>
#include	"paths.h"
#include	"sgs.h"



/* performance statistics */

#ifdef PERF

#define	STATS( s )	if (stat == 1) {			\
				stats[ii].module = s;		\
				stats[ii].ttim = ttime;		\
				stats[ii++].perform = ptimes;	\
				if (ii > 25)			\
					pexit();		\
			}

FILE	*perfile;
long	ttime;
struct	tbuffer {
	long	proc_user_time;
	long	proc_system_time;
	long	child_user_time;
	long	child_system_time;
	} ptimes;
extern	long	times();
struct	perfstat {
	char	*module;
	long	ttim;
	struct	tbuffer	perform;
	} stats[30];
int	ii = 0;
int	stat = 0;

#endif

#define cunlink(x)	if (x)	unlink(x);

/* tool names */

#define CRT0	"crt0.o"
#define MCRT0	"mcrt0.o"
#define PCRT0	"pcrt0.o"
#if TV
#define TVCRT0	"tvcrt0.o"
#define MTVCRT0	"mtvcrt0.o"
#endif
#if defined(M32) || defined(M32MAU)
#define CRT1	"crt1.o"
#define FCRT1	"fcrt1.o"
#define FMCRT1	"fmcrt1.o"
#define MCRT1	"mcrt1.o"
#define CRTN	"crtn.o"
#endif

#define	N_CPP	"cpp"
#define	N_C0	"comp"

#ifdef IAPX
#define	N_XC0	"xcomp"
#endif

#if IAPX
#define N_OPTIM	"optim16"
#define N_XOPTIM "optim20"
#else
#define	N_OPTIM	"optim"
#endif /* IAPX	*/

#define N_PROF	"basicblk"
#define	N_AS	"as"
#define	N_LD	"ld"


/* number of args always passed to ld; used to determine whether
 * or not to run ld when no files are given */

#ifdef	IAPX
#define	MINLDARGS	2
#else
#define	MINLDARGS	3
#endif


/* list indexes */

#define	Xcp	0	/* index of list containing options to cpp */
#define	Xc0	1	/* index of list containing options to the compiler */
#define	Xc2	2	/* index of list containing options to the optimizer */
#define	Xas	3	/* index of list containing options to the assembler */
#define	Xld	4	/* index of list containing options to ld */
#define	AV	5	/* index of the argv list to be supplied to execvp */
#define	CF	6	/* index of list containing the names of files to be 
				compiled and assembled */
#define LL	7	/* index of list contianing all supplied -L options */

#define	NLIST	8	/* total number of lists */

/* option string for getopt() */

#ifdef N3B
#define OPTSTR	"cpq:fOSW:t:EPB:go:D:I:U:CHd:VQl:u:L:Y:#"
#endif

#ifdef U3B
#define	OPTSTR	"cpq:fOSW:t:EPB:go:D:I:U:CHd:VQl:u:L:Y:#"
#endif

#ifdef VAX
#define	OPTSTR	"cpq:fOSW:t:EPB:go:D:I:U:CHVl:u:L:Y:#"
#endif

#ifdef MC68
#define	OPTSTR	"cpq:fOSW:t:EPB:go:D:I:U:CHd:Vl:u:L:Y:#"
#endif

#ifdef M32
#define	OPTSTR	"cpq:fOSW:t:EPB:go:D:I:U:CHd:VQl:u:L:FK:Y:#"
#endif

#ifdef IAPX
#define	OPTSTR	"cpfOSW:t:EPB:gk:vo:D:I:U:CHd:Vl:u:L:Y:#"
#endif


/* file names */

char	*cpp_out,	/* output of preprocessor */
	*c_in,		/* compiler input */
	*c_out,		/* compiler output */
	*as_in,		/* assembler input */
	*as_out,	/* assembler output */
	*ld_out = NULL,	/* link editor output (object file) */
	*tmp1 = NULL,	/* temporaries */
	*tmp2 = NULL,
	*tmp4 = NULL,
	*tmp5 = NULL;


/* path names of the various tools and directories */

char	*passcp = NULL,
	*passc0 = NULL,
	*passc2 = NULL,
	*passprof = NULL,
	*passas = NULL,
	*passld = NULL,
	*libploc = NULL,
	*crtdir	= NULL;

#ifdef M32MAU
char	*fpdir	= NULL,
	*fplibdir = NULL;
#endif

#ifdef M32
	char	*profdir= LLIBDIR1;
#else
	char	*profdir= LIBDIR;
#endif


/* flags: ?flag corresponds to ? option */

int	cflag	= 0,	/* compile and assemble only to .o; no load */
	Oflag	= 0,	/* optimizer request */
	Sflag	= 0,	/* leave assembly output in .s file */
	Eflag	= 0,	/* preprocess only, output to stdout */
	Pflag	= 0,	/* preprocess only, output to file.i */
	eflag	= 0,	/* error count */
	lflag	= 0,	/* turned on if -l option is supplied */
#ifndef VAX
	dsflag	= 1,	/* turn off symbol attribute output in compiler */
	dlflag	= 1,	/* turn of line number output in compiler */
#endif
#if IAPX
	vflag	= 0,	/* if on, user wants extended addressing (20-bit) */
#endif
	pflag	= 0,	/* profile request for standard profiler */
	qarg	= 0,	/* profile request for xprof or lprof */
	gflag	= 0,	/* include libg.a on load line */
#ifdef M32
	fflag	= 0,	/* request for floating point emulation code */
#endif
#ifdef TV
	tvflag	= 0,	/* transfer vector addressing */
#endif
	debug	= 0;	/* cc command debug flag (prints command lines) */
#ifdef M32
char	*Karg	= NULL;	/* will hold the -K flag and argument (if -K is given) */
#endif
	char	*nopt;		/* current -W flag option */


/* lists */

char	**list[NLIST];	/* list of lists */
int	nlist[NLIST],	/* current index for each list */
	limit[NLIST],	/* length of each list */
	nxo	= 0;	/* number of .o files in list[Xld] */
	nxn	= 0;	/* number of non-.o files in list[Xld] */
int	argcount;	/* initial length of each list */


#ifdef IAPX
char    *crt    = 0;            /* name of run-time startoff- IAPX will not have
                                   a startoff routine unless profiling is on */
#else
#if defined(M32) || defined(M32MAU)
char	*crt	= CRT1;
#else
char    *crt    = CRT0;         /* name of run-time startoff */
#endif
#ifdef TV
char	*tvcrt	= TVCRT0;	/* name of run-time startoff */
#endif
#endif /* IAPX */
char	*prefix;


/* interface for getopt() */

extern	int	optind,		/* arg list index */
		opterr,		/* turn off error message */
		optopt;		/* current option char */
extern	char	*optarg;	/* current option argument */


/* functions */

char	getsuf(),
	*setsuf(),
	*stralloc(),
	*strchr(),
	*strtok(),
	*strcpy(),
	*copy(),
	*getpref(),
	*makename(),
	*passname(),
	*tmpnam();

int	callsys(),
	nodup(),
	idexit(),
	move();

int	sfile;			/* indicates current file in list[CF] is .s */
int	as_escapes;		/* indicates whether the current .c file */
				/* has assembler escapes in it		 */


main (argc, argv)
	int argc;
	char *argv[];
{
	int	c;		/* current option char from getopt() */
	char	*t, *s, *p;	/* char ptr temporary */
	int	done;		/* used to indicate end of arguments passed by -W */
	int	i;		/* loop counter, array index */
	char	*chpass = NULL,	/* list of substitute pass names for -B */
		*newpath = NULL,	/* path prefix for substitute passes from -B */
		*chpiece = NULL,	/* list of pieces whose default location has
					   been changed by the -Y option */
		*npath = NULL;	/* new location for pieces changed by -Y option */
	char	*optstr = OPTSTR;
	int	cur_arg;

	opterr = 0;
#ifdef PERF
	if ((perfile = fopen("cc.perf.info", "r")) != NULL) {
		fclose(perfile);
		stat = 1;
	}
#endif

	prefix = getpref( argv[0] );

	/* initialize the lists */
	argcount= argc + 6;
	c = sizeof(char *) * argcount;
	for (i = 0; i < NLIST; i++) {
		nlist[i] = 0;
		list[i] = (char **) stralloc(c);
		limit[i]= argcount;
	}

	setbuf(stdout, (char *) NULL);	/* unbuffered output */

	while (optind < argc) {
		cur_arg = optind;
		c = getopt(argc, argv, optstr);
		switch (c) {

		case 'c':	/* produce .o files only */
			cflag++;
			break;

		case 'p':	/* standard profiler */
			pflag++;
#if defined(M32) || defined(M32MAU)
			crt = MCRT1;
#else
			crt = MCRT0;
#endif
			qarg= 0; /* can only have one type profiling on */
			break;

		case 'q':	/* xprof, lprof or standard profiler */
			if (strcmp(optarg,"p") == 0) {
				pflag++;
#if defined(M32) || defined(M32MAU)
				crt = MCRT1;
#else
				crt = MCRT0;
#endif
				qarg= 0; /* can only have one type profiling on */
			} else if (strcmp(optarg,"l")==0 || strcmp(optarg,"x")==0) {
				qarg = optarg[0];
				pflag= 0;
			} else {
				fprintf(stderr,"No such profiler %sprof\n",optarg);
				exit(1);
			}
			break;

		case 'f':	/* floating point interpreter */
#ifdef M32
			fflag++;
			fprintf(stderr, "Note: -f option loads floating point emulator; not needed for new objects\n");
#else
			fprintf(stderr, "-f option ignored on this processor\n");
#endif
			break;

		case 'F':
			addopt(Xc0,"-F");
			break;

#ifdef M32
		case 'K':	/* optimize for size or speed */
			if (strcmp(optarg,"sd")==0)
				Karg = "-K sd";
			else if (strcmp(optarg,"sz")==0)
				Karg = "-K sz";
			else {
				fprintf(stderr,"Illegal argument to -K flag, '-K %s'\n",optarg);
				exit(1);
			}
			break;
#endif

		case 'O':	/* invoke optimizer */
			Oflag++;
			break;

		case 'S':	/* produce assembly code only */
			Sflag++;
			cflag++;
			break;

		case 'W':
			if (optarg[1] != ',' || optarg[2] == '\0') {
				fprintf(stderr,"Invalid subargument: -W%s\n", optarg);
				exit(1);
			}
			t = optarg;
			if ((i = getXpass((c = *t), "-W")) == -1) {
				fprintf(stderr,"Invalid subargument: -W%s\n", optarg);
				exit(1);
			}
			t+=2;
			done=0;
			while (!done) {
				p=t;
				while (((s = strchr(p,',')) != NULL) &&
								(*(s-1) == '\\')) {
					p=s;
					s--;
					while (*s != '\0') {
						*s = *(s+1);
						s++;
					}
				}
				if (s != NULL)
					*s = '\0';
				else
					done=1;
				nopt = stralloc(strlen(t) + 1);
				strcpy(nopt, t);
				addopt(i,nopt);
				t+= strlen(t) + 1;
			}
			break;

		case 'Y':
			if (((chpiece=strtok(optarg,",")) != optarg) ||
				((npath=strtok(NULL,",")) == NULL)) {
				fprintf(stderr,"Invalid argument: -Y %s\n",optarg);
				exit(1);
			} else if ((t=strtok(NULL," ")) != NULL) {
				fprintf(stderr,"Too many arguments: -Y %s,%s,%s\n",chpiece, npath,t);
				exit(1);
			}
			chg_pathnames(prefix, chpiece, npath);
			break;

		case 't':
#ifdef TV
			if (optarg[0] == 'v')
			{
				tvflag++;
				addopt(Xc0,"-tv");
				addopt(Xas,"-tv");
				addopt(Xld,"-tv");
			}
			else
#endif
			{
				fprintf(stderr,"-t option will be eliminated in SVR4. Use the -Y option instead.\n");
				if (chpass != NULL) {
					fprintf(stderr,"-t overwrites earlier option\n");
					exit(1);
				}
				chpass = optarg;
				if (chpass[0] == 0)
					chpass = "012p";
			} /* end if */
			break;

		case 'E':	/* run only cpp, output to stdout */
			Eflag++;
			addopt(Xcp,"-E");
			cflag++;
			break;

		case 'P':	/* run only cpp, output to file.i */
			Pflag++;
			addopt(Xcp,"-P");
			cflag++;
			break;

		case 'C':	/* tell cpp to leave comments in (lint) */
			addopt(Xcp,"-C");
			break;

		case 'H':	/* cpp- print pathnames of included files on stderr */
			addopt(Xcp,"-H");
			break;

		case 'B':	/* substitute path name */
			fprintf(stderr,"-B option will be eliminated in SVR4. Use the -Y option instead.\n");
			if (newpath != NULL) {
				fprintf(stderr,"-B overwrites earlier option\n");
				exit(1);
			}
			newpath = optarg;
			if (newpath[0] == '\0')
				newpath = "/lib/o";
			break;

		case 'g':	/* turn on symbols and line numbers */
#ifndef VAX
			dsflag = dlflag = 0;
#endif
			gflag++;
			break;

#ifdef IAPX
		case 'k':	/* what Kind of code?  8086? or 80186?	*/
				/* default is 8086 code. 80186 is the   *
				 * currently the only accepted argument.*/
			if (strcmp(optarg,"80186") == 0) {
				addopt(Xc0,"-k");
				addopt(Xc0,"80186");
				addopt(Xc2,"-k");
			} else {
				fprintf(stderr,"%s is not a valid argument for the -k option - only \"80186\" is valid\n", optarg);
				exit(1);
			}
			break;

		case 'v':	/* produce Very big pointers (20-bit addressing) */
			addopt(Xas,"-x");
			vflag++;
			break;
#endif

		case 'o':	/* object file name */
			if (optarg[0]) {
				ld_out = optarg;
				if (((c = getsuf(ld_out)) == 'c') ||
					(c == 'i') || (c == 's')){
					fprintf(stderr,"Illegal suffix for object file %s\n", ld_out);
					exit(17);
				}
			}
			break;

		case 'D':
		case 'I':
		case 'U':
			t = stralloc(strlen(optarg)+2);
			sprintf(t,"-%c%s",c,optarg);
			addopt(Xcp,t);
			break;

#ifndef VAX
		case 'd':
			for (t = optarg; *t; t++)
				if (*t == 's')	/* -ds: no sym tab output */
					dsflag++;
				else if (*t == 'l') /* -dl: no line num */
					dlflag++;
				else {
					fprintf(stderr,"bad option -d%c\n", *t);
					exit(1);
				}
			break;
#endif

		case 'V':	/* version flag or ld VS flag */
			addopt(Xc0,"-V");
			addopt(Xc2,"-V");
			addopt(Xas,"-V");
			addopt(Xld,"-V");
			fprintf(stderr, "%scc: command -%s\n",
				prefix, RELEASE);
			break;

#ifdef TV
		case 'Q':	/* warn for non-call use of tv symbols */
			addopt(Xas,"-Q");
			break;
#endif

		case 'l':	/* ld: include library */
			lflag++;
		case 'L':	/* ld: alternate lib path prefix */
			t = stralloc(strlen(optarg) + 3);
			sprintf(t,"-%c%s",optopt,optarg);
			if (lflag)
				addopt(Xld,t);
			else
				addopt(LL,t);
			break;

		case 'u':	/* ld: enter sym arg as undef in sym tab */
			t = stralloc(strlen(optarg) + 4);
			addopt(Xld,"-u");
			addopt(Xld,optarg);
			break;

		case '?':	/* opt char not in optstr; pass to ld */
			t = stralloc( 3 );
			sprintf(t,"-%c",optopt);
			addopt(Xld,t);
			break;

		case '#':	/* cc command debug flag */
			debug++;
			break;

		case EOF:	/* file argument */
			t = argv[optind++]; /* -> file name */
			if (((c = getsuf(t)) == 'c') ||
				(c == 'i') || (c == 's') || Eflag) {
				addopt(CF,t);
				t = setsuf(t, 'o');
			}
			if (nodup(list[Xld], t)) {
				addopt(Xld,t);
				if (getsuf(t) == 'o')
					nxo++;
				else
					nxn++;
			}
			break;
		} /* end case */
	} /* end while */

	/* if -q option is given, make sure basicblk exists */
	if (qarg) {
		passprof= makename(profdir,prefix,N_PROF);
		if (access(passprof,00)==-1) {
			fprintf(stderr,"%cprof is not available\n",qarg);
			exit(1);
		}
		crt = PCRT0;
#ifndef VAX
		dsflag = dlflag = 0;
#endif
		gflag++;
		addopt(Xld,"-lprof");
		addopt(Xld,"-lld");
		addopt(Xld,"-lm");
	}

	/* if -o flag was given, the output file shouldn't overwrite an input file */
	if (ld_out != NULL) {
		if (!nodup(list[Xld], ld_out)) {
			fprintf(stderr,"-o would overwrite %s\n",ld_out);
			exit(1);
		}
	}

	if (nlist[CF] && !(Pflag || Eflag)) /* more than just the preprocessor is
			        	     * running, so temp files are required */
		mktemps();

	if (eflag)
		dexit();

	if (signal(SIGHUP, SIG_IGN) == SIG_DFL)
		signal(SIGHUP, idexit);
	if (signal(SIGINT, SIG_IGN) == SIG_DFL)
		signal(SIGINT, idexit);
	if (signal(SIGTERM, SIG_IGN) == SIG_DFL)
		signal(SIGTERM, idexit);

	if (chpass != NULL)
		mk_pathnames(chpass, newpath);

	mk_defpaths( prefix, chpass, newpath );

#ifdef M32
	if (Oflag && (Karg != NULL)) {
		addopt(Xc0, Karg);
		addopt(Xc2, Karg);
	}
#endif


	/* process each file (name) in list[CF] */

	for (i = 0; i < nlist[CF]; i++) {
		if (nlist[CF] > 1)
			printf("%s:\n", list[CF][i]);
		sfile = (getsuf(list[CF][i]) == 's');
		as_escapes = 0;
#ifndef IAPX
		if (sfile && !Eflag && !Pflag) {
			as_in = list[CF][i];
			assemble(i);
			continue;
		}
#endif
		if (getsuf(list[CF][i]) == 'i')
			cpp_out = list[CF][i];
		else if (!preprocess(i))
			continue;
		if (!compile(i))
			continue;
		if (Oflag)
			optimize(i);
		if (passprof)
			profile(i);
		if (!Sflag)
			assemble(i);
	} /* end loop */
	if (!eflag && !cflag)
		link();

	dexit();
}


/*===================================================================*/
/*								     */
/*                   PREPROCESSOR                                    */
/*                                                                   */
/*===================================================================*/

preprocess (i)
	int i; /* list[CF] index of filename */
{
	int j;
	
	nlist[AV]= 0;
	/* build argv argument to callsys */
	addopt(AV,passname(prefix, N_CPP));
	addopt(AV,list[CF][i]);
	addopt(AV,cpp_out=Eflag ? "-" : 
			(Pflag ? setsuf(list[CF][i], 'i') : tmp1));
	for (j = 0; j < nlist[Xcp]; j++)
		addopt(AV,list[Xcp][j]);
#ifdef IAPX
	if ( sfile ) {
		if ( vflag )
			if (tvflag)
				addopt(AV,"-DTYPE20TV");
			else
				addopt(AV,"-DTYPE20");
		else
			if (tvflag)
				addopt(AV,"-DTYPE16TV");
			else
				addopt(AV,"-DTYPE16");
		addopt(AV,"-P");
	}
#endif
#ifdef MC68
#ifdef vax
	addopt(AV,"-Uvax");
#endif
#ifdef u3b	
	addopt(AV,"-Uu3b");
#endif
	addopt(AV,"-Uunix");
	addopt(AV,"-Dmc68000");
#ifdef INT16BIT
	addopt(AV,"-Dmc68k16");
#else
	addopt(AV,"-Dmc68k32");
#endif
#endif

	list[AV][nlist[AV]] = 0;	/* terminate arg list */

	PARGS;

	if (callsys(passcp, list[AV])) {
		cflag++;
		eflag++;
		return(0);
	}
	if (Pflag || Eflag)
		return(0);
#ifdef IAPX
	if (sfile) {
		as_in = cpp_out;
		assemble(i);
		return(0);
	}
#endif
	return(1);
}


/*===================================================================*/
/*                                                                   */
/*                  COMPILER 					     */
/*                                                                   */
/*===================================================================*/

compile (i)
	int i;
{
	int j;
	int front_ret;
	
	nlist[AV]= 0;
#ifdef IAPX
	if (vflag)
		addopt(AV,passname(prefix, N_XC0));
	else
#endif
	addopt(AV,passname(prefix, N_C0));
#ifdef VAX
        addopt(AV,cpp_out);
        addopt(AV,c_out = as_in
                 = (Sflag && !Oflag && !qarg) ? setsuf(list[CF][i], 's') : tmp2);
        if (pflag)
                addopt(AV,"-XP");
        if (gflag)
                addopt(AV,"-Xg");
#else
	addopt(AV,"-i");
	addopt(AV,cpp_out);
	addopt(AV,"-o");
	addopt(AV,c_out = as_in
		 = (Sflag && !Oflag && !qarg) ? setsuf(list[CF][i], 's') : tmp2);
	addopt(AV,"-f");
	addopt(AV,list[CF][i]);
	if (dsflag)
		addopt(AV,"-ds");
	if (dlflag)
		addopt(AV,"-dl");
	if (pflag)
		addopt(AV,"-p");
#endif
	for (j = 0; j < nlist[Xc0]; j++)
		addopt(AV,list[Xc0][j]);
	list[AV][nlist[AV]] = 0;	/* terminate arg list */

	PARGS;

	front_ret = callsys( passc0, list[AV] );
	if (front_ret == 1) 	/* COMP or FRONT returns 1 for assembler escapes */
		as_escapes++;
	else if (front_ret > 1) {
		cflag++;
		eflag++;
		return(0);
	}

#ifdef PERF
	STATS("compiler ");
#endif
	return(1);
}


/*===================================================================*/
/*                                                                   */
/*                      OPTIMIZER                                    */
/*                                                                   */
/*===================================================================*/

optimize (i)
	int i;
{
	int j;
	
	nlist[AV]= 0;
#ifdef IAPX
	if (vflag)
		addopt(AV,passname(prefix, N_XOPTIM));
	else
#endif
	addopt(AV,passname(prefix, N_OPTIM));
	addopt(AV,"-I");
	addopt(AV,c_out);
	addopt(AV,"-O");
	addopt(AV,as_in
		 = (Sflag && !qarg) ? setsuf(list[CF][i], 's') : tmp4);
	for (j = 0; j < nlist[Xc2]; j++)
		addopt(AV,list[Xc2][j]);
	list[AV][nlist[AV]] = 0;	/* terminate arg list */

	PARGS;

	if (callsys(passc2, list[AV])) {
		if (Sflag) {
			if (move(c_out, as_in)) { /* move failed */
				cunlink(c_out);
				return(0);
			}
		}
		else {
			cunlink(as_in);
			as_in = c_out;
		}
		printf("Optimizer failed, -O ignored for %s\n", list[CF][i]);
	} else
		c_out= as_in;

#ifdef PERF
	STATS("optimizer");
#endif

	return(1);
}

/*===================================================================*/
/*                                                                   */
/*                      PROFILER                                     */
/*                                                                   */
/*===================================================================*/

profile(i)
	int i;
{
	int j;
	
	nlist[AV]= 0;
	addopt(AV,passname(prefix, N_PROF));
	if (qarg == 'l')
		addopt(AV,"-l");
	else
		addopt(AV,"-x");
	addopt(AV,c_out);
	addopt(AV,as_in
		 = Sflag ? setsuf(list[CF][i], 's') : tmp5);
	list[AV][nlist[AV]] = 0;	/* terminate arg list */

	PARGS;

	if (callsys(passprof, list[AV])) {
		if (Sflag) {
			if (move(c_out, as_in)) { /* move failed */
				cunlink(c_out);
				return(0);
			}
		}
		else {
			cunlink(as_in);
			as_in = c_out;
		}
		printf("Profiler failed, '-q %c' ignored for %s\n", qarg, list[CF][i]);
	}

#ifdef PERF
	STATS("profiler");
#endif

	return(1);
}
	
/*===================================================================*/
/*                                                                   */
/*                    ASSEMBLER                                      */
/*                                                                   */
/*===================================================================*/

assemble (i)
	int i;
{
	int j;
	
	nlist[AV]= 0;
	addopt(AV,passname(prefix, N_AS));
#ifndef VAX
	if (dlflag)
		addopt(AV,"-dl");
#endif
#if N3B || U3B
	addopt(AV,"-A"); /* alignment flag */
#endif
#ifdef M32MAU
	addopt(AV,"-na");
	addopt(AV,"-nm");
#endif
	addopt(AV,"-o");
	addopt(AV,as_out = setsuf(list[CF][i], 'o'));
	for (j = 0; j < nlist[Xas]; j++)
		addopt(AV,list[Xas][j]);
	addopt(AV,as_in);
	list[AV][nlist[AV]] = 0;	/* terminate arg list */

	PARGS;

	if (callsys(passas, list[AV])) {
		cflag++;
		eflag++;
		cunlink(as_out);
		return(0);
	}

#ifdef PERF
	STATS("assembler");
#endif

	return(1);
}


/*===================================================================*/
/*                                                                   */
/*                LINKAGE EDITOR                                     */
/*                                                                   */
/*===================================================================*/

link ()
{
	int j;
	char *str;
	
	nlist[AV]= 0;
	addopt(AV,passname(prefix, N_LD));
	if (ld_out != NULL)
	{
		addopt(AV,"-o");
		addopt(AV,ld_out);
	}

#if defined(M32) || defined(M32MAU)
	/* determine correct crt to use depending on flags */
	if (fflag) {
		if (pflag)
			crt = FMCRT1;
		else
			crt = FCRT1;
	}
#endif

#ifndef IAPX
#if TV
	if(tvflag)
		if(pflag)
			addopt(AV,makename(crtdir,prefix,MTVCRT0));
		else
			addopt(AV,makename(crtdir,prefix,tvcrt));
	else
#endif
		addopt(AV,makename(crtdir,prefix,crt));
#else /* IAPX */
	if (crt)
		addopt(AV,makename(crtdir,prefix,crt));
#endif /* IAPX */

	for (j = 0; j < nlist[LL]; j++) /* app files, opts, and libs */
		addopt(AV,list[LL][j]);

	if (pflag) {
#ifdef M32MAU
		str= stralloc(strlen(fplibdir) + 7);
		sprintf(str,"-L%s/libp",fplibdir);
		addopt(AV,str); /* use profiled libraries */
#endif
		addopt(AV,libploc); /* use profiled libraries */
	}
#ifdef M32MAU
	str= stralloc(strlen(fplibdir) + 2);
	sprintf(str,"-L%s",fplibdir);
	addopt(AV,str);
#endif
	for (j = 0; j < nlist[Xld]; j++) /* app files, opts, and libs */
		addopt(AV,list[Xld][j]);
	addopt(AV,"-lc");

#if !IAPX && !MC68
	if (gflag)
		addopt(AV,"-lg");
#endif
#ifdef M32
	addopt(AV,makename(crtdir,prefix,CRTN));
#endif
	list[AV][nlist[AV]] = 0;	/* terminate arg list */

	if (nlist[AV] > MINLDARGS) /* if file given or flag set by user */
	{
		PARGS;
		if (nxo || nxn)
			eflag |= callsys(passld, list[AV]);
	}

	if ((nlist[CF] == 1) && (nxo == 1) && (eflag == 0))
		/* delete .o file if single file compiled and loaded */
		cunlink(setsuf(list[CF][0], 'o'));

#ifdef PERF
	STATS("link edit");
#endif

}

/*===================================================================*/
/*								     */
/*	Make Path Names						     */
/*								     */
/*===================================================================*/

mk_pathnames( chpass, newpath )
	char	*chpass;
	char	*newpath;
{
	char	*t;	/* char ptr temporary */

	/* set up alternate passes defined by -B and -t options */
	if (newpath == NULL)
		newpath= "/lib/n";
	for (t = chpass; *t; t++)
		switch (*t) {
		case 'p':
			if (passcp)
				break;
			passcp = stralloc(strlen(newpath) + 4);
			strcpy(passcp, newpath);
			strcat(passcp, N_CPP);
			break;

		case '0': /* compiler, pass 1 */
			if (passc0)
				break;
			passc0 = stralloc(strlen(newpath) + 6);
			strcpy(passc0, newpath);
#ifdef IAPX
			if (vflag)
				strcat(passc0, N_XC0);
			else
#endif
			strcat(passc0, N_C0);
			break;

		case '2': /* optimizer */
			if (passc2)
				break;
			passc2 = stralloc(strlen(newpath) + 6);
			strcpy(passc2, newpath);
#ifdef IAPX
			if (vflag)
				strcat(passc2, N_XOPTIM);
			else
#endif
			strcat(passc2, N_OPTIM);
			break;

		case 'a': /* assembler */
			if (passas)
				break;
			passas = stralloc(strlen(newpath) + 6);
			strcpy(passas, newpath);
			strcat(passas, N_AS);
			break;

		case 'l': /* linkage editor */
			if (passld)
				break;
			passld = stralloc(strlen(newpath) + 6);
			strcpy(passld, newpath);
			strcat(passld, N_LD);
			break;

		default: /* error */
			fprintf(stderr, "Bad option -t%c\n", *t);
			exit(1);

		} /* end switch */
}


/* 
   chg_pathnames() overrides the default pathnames as specified by the -Y option
*/
chg_pathnames(prefix, chpiece, npath)
char *prefix;
char *chpiece;
char *npath;
{
	char	*t;
	char	*topt;

	for (t = chpiece; *t; t++)
		switch (*t) {
		case 'p':
			passcp = makename( npath, prefix, N_CPP );
			break;
		case '0':
#ifdef IAPX
			if (vflag)
				passc0 = makename( npath, prefix, N_XC0 );
			else
#endif
			passc0 = makename( npath, prefix, N_C0 );
			break;
		case '2':
#ifdef IAPX
			if (vflag)
				passc2 = makename( npath, prefix, N_XOPTIM );
			else
#endif
			passc2 = makename( npath, prefix, N_OPTIM );
			break;
		case 'b':
			profdir= stralloc(strlen(npath));
			strcpy(profdir,npath);
			break;
		case 'a':
			passas = makename( npath, prefix, N_AS );
			break;
		case 'l':
			passld = makename( npath, prefix, N_LD );
			break;
		case 'S':
			crtdir= stralloc(strlen(npath));
			strcpy(crtdir,npath);
			break;
		case 'I':
			topt = stralloc(strlen(npath)+4);
			sprintf(topt,"-Y%s",npath);
			addopt(Xcp,topt);
			break;
		case 'L':
			libploc= stralloc(strlen(npath) + 7);
			sprintf(libploc,"-L%s/libp",npath);
		case 'U':
			addopt(Xld,"-Y");
			topt = stralloc(strlen(npath)+2);
			sprintf(topt,"%c,%s",*t,npath);
			addopt(Xld,topt);
			break;
#ifdef M32MAU
		case 'F':
			fplibdir= stralloc(strlen(npath));
			strcpy(fplibdir,npath);
			break;
#endif

			
		default: /* error */
			fprintf(stderr,"Bad option -Y %s,%s\n",chpiece, npath);
			exit(1);
		} /* end switch */
}

mk_defpaths(prefix)
char *prefix;
{
	/* make defaults */
#ifdef M32MAU
	if (!fpdir)
		fpdir= makename(LLIBDIR1,"","fp");
	if (!fplibdir)
		fplibdir= makename(LLIBDIR1,"","fp");
#endif

	if (!passcp)
		passcp = makename( LIBDIR, prefix, N_CPP );
	if (!passc0)
#ifdef IAPX
		if (vflag)
			passc0 = makename( LIBDIR, prefix, N_XC0 );
		else
#endif
#ifdef M32MAU
		passc0 = makename( fpdir, prefix, N_C0 );
#else
		passc0 = makename( LIBDIR, prefix, N_C0 );
#endif
	if (!passc2)
#ifdef IAPX
		if (vflag)
			passc2 = makename( LIBDIR, prefix, N_XOPTIM );
		else
#endif
		passc2 = makename( LIBDIR, prefix, N_OPTIM );
	if (!passas)
		passas = makename( BINDIR, prefix, N_AS );
	if (!passld)
		passld = makename( BINDIR, prefix, N_LD );
	if (!crtdir)
#ifdef M32MAU
		crtdir = fpdir;
#else
		crtdir = LIBDIR;
#endif
	if (pflag && !libploc) {
#ifdef N3B
		libploc= stralloc(strlen(LLIBDIR) + 7);
		sprintf(libploc,"-L%s/libp",LLIBDIR);
#else
		libploc= stralloc(strlen(LIBDIR) + 7);
		sprintf(libploc,"-L%s/libp",LIBDIR);
#ifdef M32
		libploc= stralloc(strlen(LLIBDIR1) + 7);
		sprintf(libploc,"-L%s/libp",LLIBDIR1);
#else
		libploc= stralloc(strlen(LIBDIR) + 7);
		sprintf(libploc,"-L%s/libp",LIBDIR);
#endif
#endif
	}
}


/* return the prefix of "cc" */

char *
getpref( cp )
	char *cp;	/* how cc was called */
{
	extern char	*strrchr();
	static char	prefix[14];	/* enough room for prefix and \0 */
	int		cmdlen,
			preflen;
	char		*prefptr,
			*ccname;

#ifdef M32MAU
	ccname= "fpcc";
#else
	ccname= "cc";
#endif
	if ((prefptr= strrchr(cp,'/')) == NULL)
		prefptr=cp;
	else
		prefptr++;
	cmdlen= strlen(prefptr);
	preflen= cmdlen - strlen(ccname);
	if ( (preflen < 0 )		/* if invoked with a name shorter
					   than ccname */
	    || (strcmp(prefptr + preflen, ccname) != 0)) {
		fprintf(stderr, "command name must end in \"%s\"\n", ccname);
		exit(1);
	} else {
		strncpy(prefix,prefptr,preflen);
		prefix[preflen]='\0';
		return(prefix);
	}
}

/* Add the string pointed to by opt to the list given by list[lidx]. */
addopt(lidx, opt)
int	lidx;	/* index of list */
char	*opt;  /* new argument to be added to the list */
{
	/* check to see if the list is full */
	if ( nlist[lidx] == limit[lidx] - 1 ) {
		limit[lidx] += argcount;
		if ((list[lidx]= (char **)realloc(list[lidx],
					limit[lidx]*sizeof(char *))) == NULL) {
			fprintf(stderr, "Out of space\n");
			dexit();
		}
	}

	list[lidx][nlist[lidx]++]= opt;
}

/* make absolute path names of called programs */
char *
makename( path, prefix, name )
	char *path;
	char *prefix;
	char *name;
{
	extern char	*malloc();
	char	*p;

	if ((p = malloc(strlen(path)+strlen(prefix)+strlen(name)+2)) == NULL) {
		fprintf(stderr, "Out of space\n");
		dexit();
	}
	(void) strcpy( p, path );
	(void) strcat( p, "/" );
	(void) strcat( p, prefix );
	(void) strcat( p, name );

	return( p );
}

/* make the name of the pass */

char *
passname(prefix, name)
	char *prefix;
	char *name;
{
	extern char	*malloc();
	char	*p;

	if ((p = malloc( strlen( prefix ) + strlen( name ) + 1 )) == NULL) {
		fprintf(stderr, "Out of space\n");
		dexit();
	}
	(void) strcpy( p, prefix );
	(void) strcat( p, name );
	return( p );
}

idexit()
{
	signal(SIGINT, idexit);
	signal(SIGTERM, idexit);
	eflag = 100;
	dexit();
}

dexit()
{
	if (!Pflag) {
		if (qarg)
			cunlink(tmp5);
		if (Oflag)
			cunlink(tmp4);
		cunlink(tmp2);
		cunlink(tmp1);
	}
#ifdef PERF
	if (eflag == 0)
		pwrap();
#endif
	exit(eflag);
}

error(s, x, y)
	char *s, *x, *y;
{
	fprintf(Eflag ? stderr : stdout , s, x, y);
	putc('\n', Eflag ? stderr : stdout);
	cflag++;
	eflag++;
}




char
getsuf(as)
	char as[];
{
	register int c;
	register char *s;
	register int t;

	s = as;
	c = 0;
	while (t = *s++)
		if (t == '/')
			c = 0;
		else
			c++;
	s -= 3;
	if ((c <= 14) && (c > 2) && (*s++ == '.'))
		return(*s);
	return(0);
}

char *
setsuf(as, ch)
	char as[];
{
	register char *s, *s1;

	s = s1 = copy(as);
	while (*s)
		if (*s++ == '/')
			s1 = s;
	s[-1] = ch;
	return(s1);
}

int
callsys(f, v)
	char f[], *v[];
{
	register int pid, w;
	int status;

	fflush(stdout);
	fflush(stderr);

	if (debug >= 2) {	/* user specified at least two #'s */
		printf("%scc: process: %s\n", prefix, f);
		if (debug >= 3)	/* 3 or more #'s:  don't exec anything */
			return(0);
	}

#ifdef PERF
	ttime = times(&ptimes);
#endif

	if ((pid = fork()) == 0) {
		execvp(f, v);
		fprintf(stderr,"Can't exec %s\n", f);
		exit(100);
	}
	else
		if (pid == -1) {
			fprintf(stderr,"Process table full - try again later\n");
			eflag = 100;
			dexit();
		}
	while ((w = wait(&status)) != pid && w != -1) ;

#ifdef PERF
	ttime = times(&ptimes) - ttime;
#endif

	if (w == -1) {
		fprintf(stderr,"Lost %s - No child process!\n", f);
		eflag = w;
		dexit();
	}
	else {
		if (((w = status & 0xff) != 0) && (w != SIGALRM)) {
			if (w != SIGINT) {
				fprintf(stderr, "Fatal error in %s\n", f);
				fprintf( stderr, "Status 0%o\n", status );
			}
			eflag = status;
			dexit();
		}
	}
	return((status >> 8) & 0xff);
}

int
nodup(l, os)
	char **l, *os;
{
	register char *t;

	if (getsuf(os) != 'o')
		return(1);
	while(t = *l++) {
		if (strcmp(t,os) == 0)
			return(0);
	}
	return(1);
}

int
move(from, to)
	char *from, *to;
{
	list[AV][0] = "mv";
	list[AV][1] = from;
	list[AV][2] = to;
	list[AV][3] = 0;
	if (callsys("mv", list[AV])) {
		error("Can't move %s to %s", from, to);
		return(1);
	}
	return(0);
}

char *
copy(s)
	register char *s;
{
	register char *ns;

	ns = stralloc(strlen(s));
	return(strcpy(ns, s));
}

char *
stralloc(n)
	int n;
{
	extern char *malloc();
	register char *s;

	if ((s = malloc(n+1)) == NULL) {
		error("out of space", (char *) NULL, (char *)NULL);
		dexit();
	}
	return(s);
}

mktemps()
{
	tmp1 = tempnam(TMPDIR, "ctm1");
	tmp2 = tempnam(TMPDIR, "ctm2");
	tmp4 = tempnam(TMPDIR, "ctm4");
	tmp5 = tempnam(TMPDIR, "ctm5");

	if (!(tmp1 && tmp2 && tmp4 && tmp5) || creat(tmp1, 0666) < 0)
		error( "%scc: cannot create temporaries: %s", prefix, tmp1);
}


getXpass(c, opt)
	char	c,
		*opt;
{
	switch (c) {
	case '0':
		return(Xc0);
	case '2':
		return(Xc2);
	case 'p':
		return(Xcp);
	case 'a':
		return(Xas);
	case 'l':
		return(Xld);
	default:
		error("Unrecognized pass name: '%s%c'", opt, c);
		return(-1);
	}
}

#ifdef PERF
pexit()
{
	fprintf(stderr, "Too many files for performance stats\n");
	dexit();
}
#endif

#ifdef PERF
pwrap()
{
	int	i;

	if ((perfile = fopen("cc.perf.info", "r")) == NULL)
		dexit();
	fclose(perfile);
	if ((perfile = fopen("cc.perf.info", "a")) == NULL)
		dexit();
	for (i = ii-1; i > 0; i--) {
		stats[i].perform.proc_user_time -= stats[i-1].perform.proc_user_time;
		stats[i].perform.proc_system_time -= stats[i-1].perform.proc_system_time;
		stats[i].perform.child_user_time -= stats[i-1].perform.child_user_time;
		stats[i].perform.child_system_time -= stats[i-1].perform.child_system_time;
	}
	for (i = 0; i < ii; i++)
		fprintf(perfile, "%s\t%07ld\t%07ld\t%07ld\t%07ld\t%07ld\n",
			stats[i].module,stats[i].ttim,stats[i].perform);
	fclose(perfile);
}
#endif
