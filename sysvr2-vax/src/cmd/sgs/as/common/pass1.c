static char *ID_pass1 = "@(#) pass1.c: 1.5 3/10/83";

#include <stdio.h>
#include <ctype.h>
#include <signal.h>
#include <paths.h>
#include "systems.h"
#include "symbols.h"
#include "gendefs.h"

/*
 *
 *	"pass1.c" is a file containing the main routine for the first
 *	pass of the assembler.  It is invoked with the command:
 *
 *		as1 [flags] ifile ofile t1 t2 t3 t4 t5 t6
 *
 *	where {flags] are optional flags passed from pass 0,
 *	"ifile" is the name of the assembly language input file,
 *	"t1" through "t6" are the names of temporary files to be used
 *	by the assembler, and "ofile" is the file where the object code
 *	is to be written.  Pass 1 of the assembler reads "ifile" and
 *	writes the temporary text section to "t1", the temporary data
 *	section to "t2", and the symbol table to "t3".
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
 *	   This is followed by a call to "cgsect". Normally this
 *	   function is used to change the section into which code
 *	   is generated. In this case, it is only called to make
 *	   sure that "dottxt", "dotdat", and "dotbss" contain the
 *	   proper values for the program counters for the respective
 *	   sections. The following symbols are then defined:
 *
 *		.text	This has a type of text and a value of zero.
 *			It is used to label the beginning of the text
 *			section, and later as a reference for relocation
 *			entries that are relative to the text section.
 *
 *		.data	This has a type of data and a value of zero.
 *			It is used to label the beginning of the data
 *			section, and later as a reference for relocation
 *			entries that are relative to the data section.
 *
 *		.bss	This has a type of bss and a value of zero.  It
 *			is used to label the beginning of the bss
 *			section, and later as a reference for relocation
 *			entries that are relative to the bss section.
 *
 *		(text)	This is a totally internal symbol used to
 *			remember the size of the text section.  It has
 *			characters in it that cannot legally be used in
 *			a symbol, and hence cannot be referenced or
 *			redefined by a user.
 *
 *		(data)	This is an internal symbol used to remember the
 *			size of the data section.
 *
 *		(bss)	This is an internal symbol used to remember the
 *			size of the bss section.
 *
 *		(sdicnt) This is the internal symbol used to remember
 *			the number of span dependent instructions
 *			on which optimizations were performed.
 *
 *	     3. The function "dmpstb" is called to dump the symbol
 *		table out to a temporary file to be used by pass 2 of
 *		the assembler.
 *
 *	4. The temporary files are closed and the next pass (if any)
 *	   is called.
 *
 */

#if ONEPROC
extern short passnbr;
#endif

extern char file[];

extern char *filenames[];

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
	fixsyms(),
	flags(),
	flushbuf(),
	onintr();

extern FILE
	*fderr;

#if !ONEPROC
extern FILE
	*fdstab;
#endif

extern upsymins
	*lookup();

extern long
	dottxt,
	dotdat,
	dotbss;

#if !ONEPROC
char	*xargp[15];
#endif

short	opt = YES,
	Oflag = NO;

#if M4ON
extern short rflag;
#endif

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

FILE	*fdin,
#if !ONEPROC
	*fdstring,
	*fdlong,
#endif
	*fdtext,
#if DEBUG
	*perfile,	/* performance data file descriptor */
#endif
	*fddata;

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
						opt = NO;
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
	strcpy(file,filenames[0]);
#endif
	if ((fdin = fopen(file, "r")) == NULL)
		aerror("Unable to open input file");
	if ((fdtext = fopen(filenames[2], "w")) == NULL)
		aerror("Unable to open temporary (text) file");
	if ((fddata = fopen(filenames[3], "w")) == NULL)
		aerror("Unable to open temporary (data) file");
#if !ONEPROC
	if ((fdstring = fopen(filenames[6], "w")) == NULL)
		aerror("Unable to open temporary (string) file");
#endif
#if FLEXNAMES
	strtabinit();
#endif
	creasyms();
	dot = (*lookup(".",INSTALL, USRNAME)).stp;
	dot->styp = TXT;
	dot->value = newdot = 0L;
	yyparse();	/* pass 1 */
	fclose(fdin);
#if !ONEPROC
	fflush(fdstring);
	if (ferror(fdstring))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdstring);
#endif
	cgsect(TXT);
	flushbuf();
	fflush(fdtext);
	if (ferror(fdtext))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdtext);
	fflush(fddata);
	if (ferror(fddata))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fddata);

#if !ONEPROC
	if ((fdlong = fopen(filenames[5], "w")) == NULL)
		aerror("Unable to open temporary (sdi) file");
#endif
	fixsyms();
	cgsect(TXT); /* update "dottxt" */
#if !ONEPROC
	fflush(fdlong);
	if (ferror(fdlong))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdlong);
#endif

	ptr = (*lookup(".text", INSTALL, USRNAME)).stp;
	ptr->styp = TXT;
	ptr->value = 0L;
	ptr = (*lookup("(text)", INSTALL, USRNAME)).stp;
	ptr->styp = TXT;
	ptr->value = dottxt;
	ptr = (*lookup(".data", INSTALL, USRNAME)).stp;
	ptr->styp = DAT;
	ptr->value = 0L;
	ptr = (*lookup("(data)", INSTALL, USRNAME)).stp;
	ptr->styp = DAT;
	ptr->value = dotdat;
	ptr = (*lookup(".bss", INSTALL, USRNAME)).stp;
	ptr->styp = BSS;
	ptr->value = 0L;
	ptr = (*lookup("(bss)", INSTALL, USRNAME)).stp;
	ptr->styp = BSS;
	ptr->value = dotbss;

#if !ONEPROC
	ptr = (*lookup("(sdicnt)",INSTALL,USRNAME)).stp;
	ptr->value = (long)sdicnt; /* has to be set after fixsyms is called */
	ptr->styp = ABS;

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
