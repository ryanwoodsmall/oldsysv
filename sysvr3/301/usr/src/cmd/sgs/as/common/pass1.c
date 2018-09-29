/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/pass1.c	1.16"

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <paths.h>
#include "systems.h"
#include "symbols.h"
#include "gendefs.h"
#include "codeout.h"
#include "section.h"

/*
 *
 *	"pass1.c" is a file containing the main routine for the first
 *	pass of the assembler.  It is invoked with the command:
 *
 *		as1 [flags] ifile ofile t2 t3 t4 t5 t6 ... t[NSECTIONS+5]
 *
 *	where [flags] are optional flags passed from pass 0,
 *	"ifile" is the name of the assembly language input file,
 *	"t2" ... are the names of temporary files to be used
 *	by the assembler, and "ofile" is the file where the object code
 *	is to be written.  Pass1 uses the files as follows:
 *	
 *	name	internal	mode	use
 *
 *	ifile	filenames[0]	read	input source file
 *	ofile	filenames[1]		not used in this pass
 *	t2	filenames[2]	write*	section table
 *	t3	filenames[3]	write*	flexname string table
 *	t4	filenames[4]	write*	symbol table
 *	t5	filenames[5]	write*	span dependent instruction table
 *	t6	filenames[6]	write	temp for section 1
 *	...
 *	tk	filenames[k]	write	temp for section k-5
 *
 *	'*' after mode indicates files used only for multiple process as
 *
 *	The following things are done by this function:
 *
 *	1. Initialization. This consists of calling signal to catch
 *	   interrupt signals for hang-up, break, and terminate. Then
 *	   the argument list is processed by "getargs" followed by the
 *	   initialization of the symbol table with mnemonics for
 *	   instructions and pseudos-ops.
 *
 *	2. "yyparse" is called to do the actual first pass processing.
 *	   This is followed by a call to "cgsect(1)". Normally this
 *	   function is used to change the section into which code
 *	   is generated. In this case, it is only called to make
 *	   sure that the section program counters are up to date.
 *
 *	3. The section name symbols are then defined. Each section
 *	   name symbol has the type of the section and a value of zero.
 *	   The symbol is used to label the beginning of the section
 *	   and later as a reference for relocation entries that are
 *	   relative to the section.
 *
 *	4. Next the symbol (sdicnt) is defined.  This is the internal
 *	   symbol used to remember the number of span dependent
 *	   instructions on which optimizations were performed.
 *
 *	5. The function "dmpstb" is called to dump the symbol
 *	   table out to a temporary file to be used by pass 2 of
 *	   the assembler.
 *
 *	6. The temporary files are closed and the next pass (if any)
 *	   is called.
 *
 */

#if ONEPROC
extern short passnbr;
#endif

extern char *filenames[];
extern struct scnhdr sectab[];
extern struct scninfo secdat[];
extern int seccnt;

extern unsigned short
	line,
	sdicnt;


#if DEBUG
extern unsigned
	numcalls,
	numids,
	numcoll;
#endif

extern short
	anyerrs;

extern int
	aerror(),
	delexit(),
#if !ONEPROC
	dmpstb(),
#endif
#if MC68
	ckalign(),	/* force even section alignment */
#endif
	fixsyms(),
	flags(),
	flushbuf(),
	cgsect(),
	onintr();

extern FILE
	*fderr;

#if !ONEPROC
extern FILE
	*fdstab;
#endif

extern upsymins
	*lookup();

#if !ONEPROC
char	*xargp[15];
#endif

short	opt = YES,
	workaround = YES,
	Oflag = NO;

#if M4ON
extern short rflag;
#endif

#if	M32
extern int warlevel;
extern int need_mau;
#endif
#if M32RSTFIX
short rstflag = YES;
#endif	/* M32RSTFIX */
#if ONEPROC
extern short
	transvec;
#else
short
	transvec = NO,
	argindex = 1;
#endif

#if ONEPROC
extern long newdot;
extern symbol *dot;
#else
long	newdot;
symbol	*dot;
#endif

FILE	*fdin;
#if !ONEPROC
FILE	*fdstring,
	*fdlong;
#endif

#if !ONEPROC
static char
	nextpass[80];

static char
	teststr[4] = {'-','t','\0'};
#endif

short tstlookup = NO;

static short
#if !ONEPROC
	filecnt = 0,
#endif
	testas = TESTVAL;

#if DEBUG
/*
 *	Performance data structure
 */
	long	ttime;
	struct	tbuffer {
		long	proc_user_time;
		long	proc_system_time;
		long	child_user_time;
		long	child_system_time;
		} ptimes;
	extern	long	times();

#endif
#if !ONEPROC
/*
 *
 *	"getargs" is a general purpose argument parsing routine.
 *	It locates flags (identified by a preceding minus sign ('-'))
 *	and initializes any associated flags for the assembler.
 *	If there are any file names in the argument list, then a
 *	pointer to the name is stored in the array "filenames" for
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
#endif
#if M4ON
					case 'R':
						rflag = YES;
						xargp[argindex++] = "-R";
						break;
#endif
					case 'd':
						if (*++*xargv == 'l')
						      xargp[argindex++] = "-dl";
						break;
					case 't': {
						++*xargv;
#if TRANVEC
						if (**xargv == 'v'){
							transvec = YES;
							xargp[argindex++]="-tv";
							break;
						}
#endif
						if (isdigit(**xargv)) {
							testas = **xargv - '0' -1;
							if (testas > TESTVAL + 1) {
								teststr[2] = (char)(testas + '0');
							}
						}
						else {
							--*xargv;
							testas += 2;
						}
						xargp[argindex++] = teststr;
						break;
					}
#if DEBUG
					case 'T': {
						switch (*++*xargv) {
							case 'L': {
								tstlookup = YES;
								break;
							}
						}
						break;
					}
#endif
#if M32RSTFIX
					case 'r':
						rstflag = NO;
						break;
#endif	/* M32RSTFIX */
					default: {
						/* installation dependent flag? */
						flags(ch);
						break;
					}
				}
			xargv++;
		}
		else {
			filenames[filecnt++] = *xargv++;
		}
	}
}

main(argc,argv)
	int argc;
	char **argv;
#else

aspass1()

#endif
{
	register short i;
	register symbol *ptr;
	register FILE *fd;
	
#if ONEPROC
	passnbr = 1;
#endif
	if (signal(SIGHUP,SIG_IGN) == SIG_DFL)
		signal(SIGHUP,onintr);
	if (signal(SIGINT,SIG_IGN) == SIG_DFL)
		signal(SIGINT,onintr);
	if (signal(SIGTERM,SIG_IGN) == SIG_DFL)
		signal(SIGTERM,onintr);
	fderr = stderr;

#if DEBUG
/*	Performance data collected	*/
	ttime = times(&ptimes);
#endif

#if !ONEPROC
	strcpy(nextpass,argv[0]);
	argv++;
	argc--;
	getargs(argc,argv);
	if (filecnt < NFILES)
		aerror("Illegal number of temporary files");
#endif
	if ((fdin = fopen(filenames[0], "r")) == NULL)
		aerror("Unable to open input file");
#if !ONEPROC
	if ((fdstring = fopen(filenames[3], "w")) == NULL)
		aerror("Unable to open temporary (string) file");
#endif
#if FLEXNAMES
	strtabinit();
#endif
	creasyms();
	dot = (*lookup(".",INSTALL, USRNAME)).stp;
	dot->value = newdot = 0L;
	dot->sectnum = 0;
	yyparse();	/* pass 1 */
	fclose(fdin);
#if !ONEPROC
	fflush(fdstring);
	if (ferror(fdstring))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdstring);
#endif
	cgsect(1);
	flushbuf();

#if !ONEPROC
	if ((fdlong = fopen(filenames[5], "w")) == NULL)
		aerror("Unable to open temporary (sdi) file");
#endif
	fixsyms();
	cgsect(1);
#if !ONEPROC
	fflush(fdlong);
	if (ferror(fdlong))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdlong);
#endif
	flushbuf();

#if !ONEPROC
	if ((fd = fopen(filenames[2], "w")) == NULL)
		aerror("Cannot create temporary (section table) file");
	fwrite((char*)sectab, sizeof(*sectab), seccnt, fd);
	fflush(fd);
	if (ferror(fd))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fd);
	ptr = (*lookup("(sdicnt)",INSTALL,USRNAME)).stp;
	ptr->value = (long)sdicnt; /* has to be set after fixsyms is called */
	ptr->sectnum = N_ABS;

	if ((fdstab = fopen(filenames[4], "w")) == NULL)
		aerror("Unable to open temporary (symtab) file");
	dmpstb();	/* dump the symbol table for the next pass */
	fflush(fdstab);
	if (ferror(fdstab))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdstab);
#endif

#if DEBUG
	if (tstlookup) {
		printf("Number of calls to lookup: %u\n",numcalls);
		printf("Number of identifiers: %u\n",numids);
		printf("Number of identifier collisions: %u\n",numcoll);
		fflush(stdout);
	}
/*
 *	Performance data collected and written out here
 */

	ttime = times(&ptimes) - ttime;
	if ((perfile = fopen("as.info", "r")) != NULL ) {
		fclose(perfile);
		if ((perfile = fopen("as.info", "a")) != NULL ) {
			fprintf(perfile,
			   "as1\t%07ld\t%07ld\t%07ld\t%07ld\t%07ld\tpass 1\n",
			    ttime, ptimes);
			fclose(perfile);
		}
	}

#endif

	if (!anyerrs) {
#if ONEPROC
		return(aspass2());
#else
		nextpass[strlen(nextpass) - 1] = '2';
		xargp[0] = nextpass;
		for (i=0; i < filecnt; ++i)
			xargp[argindex++] = filenames[i];
		if (testas != TESTVAL + 1) {
			if (testas > TESTVAL + 1){
				execv(NAS2,xargp);
			}
			else
				execv(AS2,xargp);
			aerror("Unable to exec pass 2");
		}
#endif
	}
	else {
		delexit();
	}
}
