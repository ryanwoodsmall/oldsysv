/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/pass2.c	1.18"

#include <stdio.h>
#include <signal.h>
#include <ctype.h>
#include "scnhdr.h"
#include "filehdr.h"
#include "systems.h"
#include "gendefs.h"
#include "symbols.h"
#include "codeout.h"
#include "section.h"

/*
 *
 *	"pass2.c" is a file that contains the main routine for the
 *	final pass of the assembler.  This includes the primary control
 *	(function "main.c") and "getargs", the flag handling routine.
 *	This pass of the assembler is invoked with the command:
 *
 *		as2 [flags] ifile ofile t2 t3 t4 t5 t6 ... t[NSECTIONS+5]
 *
 *	where [flags] are optional flags passed from pass 1,
 *	"ifile" is the name of the assembly language source
 *	program, "t2" ... are the names of temporary files,
 *	and "ofile" is the name of the file where the object program
 *	is to be written.  Pass2 uses the files as follows:
 *
 *	name	internal	mode	use
 *
 *	ifile	filenames[0]		name used in error messages
 *	ofile	filenames[1]	write	assembly output
 *	t2	filenames[2]	read*	section table
 *	t3	filenames[3]	read*	flexnames string table
 *	t4	filenames[4]	read*	symbol table
 *	t5	filenames[5]	read*	span dependent instruction table
 *	t6	filenames[6]	read	temp for section 1
 *	...
 *	tk	filenames[k]	read	temp for section k-5
 *	
 *  '*' after mode indicates files used only for multiple process implementations
 *
 *	Since all the information about the
 *	program is contained in temporary files at the time that the
 *	second pass is executed, the name "ifile" is only used in
 *	printing error messages.  The intermediate form of the 
 *	sections appears in the temporary file t6 through t[5+n], where n is
 *	the number of sections. t2 contains the section information, including
 *	the total number of sections, as determined by pass 1.
 *	See the commentary in the function "main" for the exact sequence of
 *	events. "ofile" is used as the output from this pass.
 *
 */

/*
 *
 *	The following are declarations for the major global variables
 *	used in the second pass of the assembler.  The variables and
 *	their meainings are as follows:
 *
 *	fdout	This is used to contain the file descriptor for the
 *		object file.
 *
 *	fdsect	This is the file descriptor where temporary section
 *		information is to be written.  This will refer to
 *		several different temporary files throughout execution
 *		of this pass.
 *
 *	fdline	This is the file descriptor where the completed line
 *		number entries are always written. Both the line number
 *		entries for all sections  are placed in this file. 
 *		t4 is used to store this information.
 *
 *	fdrel	This is the file descriptor where preliminary relocation
 *		entries are to be written.  As it happens, this will
 *		always refer to the temporary file "t5" during execution
 *		of this program.
 *
 *	fdsymb	This is the file descriptor where symbol table entries
 *		will be written.  These are symbol table entries from
 *		".def" pseudo operations, not to be confused with the
 *		internal symbol table entries that are read from "t4"
 *		when this pass begins execution.  This file descriptor
 *		will refer to the temporary file "t3" during execution
 *		of this program.
 *
 *	fdgsymb	This is the file descriptor where symbol table entries
 *		for global symbols whose declarations appear in
 *		the intermediate file for the "DAT" sections will be
 *		written.  These are written to a file other than that
 *		referred to by "fdsymb" so that declarations for
 *		external static symbols can find their way ahead of the
 *		global symbol declarations. t4 contains this info.
 *
 *	filenames[0]
 *		This is a pointer to the string which is the input file
 *		name ("ifile", as described at the beginning of this
 *		file).  This will be used if necessary to report the
 *		file name in an error message (see "errors.c").
 *
 *	filhead	This is a structure that is used to build the header for
 *		the object file.  It is of type "FILHDR", defined in
 *		"filehdr.h".
 *
 *
 *	symhead	This is a structure that is used to build the dummy
 *		section header whose offset points to the symbol table.
 *		It is of type "SCNHDR", defined in "scnhdr.h".
 *
 *	symbent	A count of the number of entries in the symbol table
 *		whose file is referred to be "fdsymb".
 *
 *	gsymbent A count of the number of entries in the symbol table
 *		whose file is referred to by "fdgsymb".
 *
 *	seccnt	A count of the number of sections. The first
 *		section is numbered 1.
 *
 *	sectab	The array of section headers that will be written 
 *		after the file header in the output file.  Indexed
 *		from 1 through seccnt.
 *
 *	secdat	An internal array of section attributes including
 *		the type of the section, the input/output file
 *		descriptor and the output buffer.
 *
 */

#if ONEPROC
extern short passnbr;
extern unsigned short sdicnt;
#endif

extern int seccnt;
extern struct scnhdr sectab[];
extern struct scninfo secdat[];

extern char
	*filenames[];

extern char
	*malloc(),
	*strcpy();

extern short
	anyerrs;

extern unsigned short
	lineent;

extern int
	aerror(),
	delexit(),
	deltemps(),
	flags(),
	onintr();

extern upsymins
	*lookup();

#if FLEXNAMES
extern long	currindex;
extern char	*strtab;
#endif

BYTE	*longsdi;

#if ONEPROC
extern char islongsdi[];
#endif

unsigned short
	relent;

#if ONEPROC
extern short transvec;
#else
short transvec = NO; /* indicates transfer vector program addressing */
#endif

short
	uflag = NO,
	dlflag = NO;

#if M4ON
extern short rflag;
#endif

FILE
	*fdout,
	*fdsect,
	*fdline,
	*fdrel,
	*fdsymb,
	*fdgsymb
#if !ONEPROC
	,
	*fdstring
#endif
	;
#if DEBUG
extern FILE
	*perfile;	/* performance data file descriptor */
#endif

long	symbent = 0L,
	gsymbent = 0L;
long hdrptr;

#if !ONEPROC
static char
	teststr[4] = {'-','t','\0','\0'};
#endif

static short
#if !ONEPROC
	filecnt,
#endif
	testas = TESTVAL;

#if DEBUG
/*
 *	Performance data variables
 */
extern long	ttime;
extern struct	tbuffer {
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
#if M4ON
					case 'R': {
					/* set flag to remove input file */
						rflag = YES;
						break;
					}
#endif
					case 'd':
						if (*++*xargv == 'l')
							dlflag = YES;
						break;

					case 'u':
						uflag = YES;
						break;

					case 't': {
						++*xargv;
#if TRANVEC
						if (**xargv == 'v') {
							transvec = YES;
							break;
						}
#endif
						if (isdigit(**xargv)) {
							testas = **xargv - '0' -1;
							if (testas > TESTVAL + 1)
								teststr[2] = (char)(testas + '0');
						}
						else {
							--*xargv;
							testas += 2;
						}
						break;
					}
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

/*
 *
 *	"main" is the main function for the second pass of the assembler.
 *	It is invoked as described at the beginning of this file.  It
 *	performs initialization by setting the variable "file" to point
 *	to the source file name, setting "arglist" to point to the
 *	argument list (this is so temporary files can be removed easily
 *	in case of error), checking the argument count, and calling
 *	"getstab" to get the symbol table from temporary file t4.  It
 *	then performs the following sequence of steps:
 *
 *	     1.	The section lengths are determined from sectab[].
 *		If necessary, the section sizes are rounded up to
 *		even sizes and the "s_up" flag is set in the 
 *		corresponding secdat[] entry. "siz" keeps a
 *		running total of the section sizes and is used
 *		as an argument to fixstab() to convert the offsets
 *		in each section to addresses.
 *
 *	     2. The following file descriptors are opened:
 *
 *		fdsect	This is opened for writing on the output file
 *			ofile.  The section information for the each
 *			section will be written to this file.
 *
 *		fdsymb	This is opened for writing on the temporary file
 *			t3.  The symbol table entries will be written
 *			to this file.
 *
 *		fdline	This is opened for writing on the temporary file
 *			t4.  The preliminary line number entries for
 *			the text section will be written to this file.
 *
 *		fdrel	This is opened for writing on the temporary file
 *			t5.  The preliminary relocation information for
 *			the text section will be written to this file.
 *
 *		The following occurs for each section S in sectab[]:
 *
 *		lineent	This is used to count the number of line number
 *			entries written to "fdline".  The value of this
 *			variable will be copied to sectab[S].s_nlnno.
 *
 *		relent	This is used to count the number of relocation
 *			entries written to "fdrel".  The value of this
 *			variable will be copied to sectab[S].s_nreloc.
 *
 *		symbent	This is used to keep count of the number of
 *			symbol table entries written to "fdsymb".
 *
 *		gsymbent This is used to keep count of the number of
 *			symbol table entries written to "fdgsymb".  This
 *			will not be used until the data section is
 *			processed.
 *
 *		The function "codout" (from codout.c) is called to
 *		process the "TXT" and "DAT" sections.  If the flag 
 *		"s_up" is set, an extra byte having the value hex "f4" is 
 *		written to the end of "fdsect".  This rounds the section out
 *		to an even number of bytes by adding a "hlt"
 *		instruction.
 *
 *	     3. The file descriptor "fdout" is initialized and is
 *		set to point to the beginning of the object file to
 *		be created.  The function "headers" is called to
 *		create the file headers and all section headers for
 *		the object file.
 *
 *	     4. The file descriptor "fdrel" is opened to read from t5,
 *		and "reloc" is called to process the text section
 *		and data section relocation entries and write them
 *		to the object file.
 *
 *	     5. The file t4 containing the line number entries is
 *		appended to the object file by calling "copysect".
 *
 *	     6. The file t3 containing the complete symbol table is
 *		copied to the object file using "copysect".  Following
 *		this, the file descriptor "fdout" can be closed, since
 *		the object file is complete.
 *
 *	     7. If the assembler has not been called for testing
 *		the temporary file are removed by calling "deltemps".
 *
 */

main(argc,argv)
	int argc;
	char **argv;
#else

aspass2()

#endif
{
	register int i;
	register struct scnhdr *sect;
	register struct scninfo *seci;
	register symbol *ptr;
	long home,rel,siz;
	FILE *fd;
#if !ONEPROC
	unsigned int sdicnt = 0;
	char *savptr;
#endif

#if ONEPROC
	passnbr = 2;
#else
	if (signal(SIGHUP,SIG_IGN) == SIG_DFL)
		signal(SIGHUP,onintr);
	if (signal(SIGINT,SIG_IGN) == SIG_DFL)
		signal(SIGINT,onintr);
	if (signal(SIGTERM,SIG_IGN) == SIG_DFL)
		signal(SIGTERM,onintr);
	fderr = stderr;
#endif

#if DEBUG
/*	Performance data collected here	*/
	ttime = times(&ptimes);
#endif

#if !ONEPROC
	argv++;
	argc--;
	getargs(argc,argv);
	strcpy(file,filenames[0]);
	if (filecnt < NFILES)
		aerror("Illegal Argument Count");
	getstab(filenames[4]);
	ptr = (*lookup("(sdicnt)",N_INSTALL,USRNAME)).stp;
	if (ptr != NULLSYM && (sdicnt = (unsigned)(ptr->value)) != 0) {
		if ((fdsect = fopen(filenames[5],"r")) == NULL)
			aerror("Cannot open temporary (sdi) file");
		longsdi = (BYTE *)malloc(sdicnt);
		savptr = longsdi;
		fread(longsdi,sizeof(*longsdi),(int)sdicnt,fdsect);
		fclose(fdsect);
		}
	/*
	 * get section table
	 */

	if (!(fd = fopen(filenames[2], "r")))
		aerror("Cannot read temporary (section table) file");
	for (seccnt = 0, sect = &sectab[1];
		fread((char*)sect, sizeof(*sect), 1, fd) == 1;
		seccnt++, sect++) ;
        fclose(fd);
#else
	longsdi = (BYTE *)islongsdi;
#endif
	/*
	 * compute section sizes, aligning to SCTALIGN
	 */

	siz = 0;
	for (i = 1, sect = &sectab[1], seci = &secdat[1]; i <= seccnt; i++, sect++, seci++) {
		ptr = (*lookup(sect->s_name,N_INSTALL,USRNAME)).stp;
		ptr->value = 0L;	/* has s_flags, needed for pass1; clear
						before fixstab() below */
		if (seci->s_up = sect->s_size % SCTALIGN) {
			seci->s_up = SCTALIGN - seci->s_up;
			sect->s_size += seci->s_up;
			}
		if (i > 1) fixstab(0L, siz, i);
		siz += sect->s_size;
		}

	if ((( fdsect = fopen( filenames[1],"w+" )) == NULL)
		&& ((unlink(filenames[1]) == -1)
		|| (fdsect = fopen(filenames[1], "w+" )) == NULL))
	{
		errmsg("", "Cannot Open Output File");
		deltemps();
		exit(127);
	}
	fdout = fdsect;
	hdrptr = ftell(fdout);
	/*
	 * position pointer past area for headers
	 */

	fseek(fdout, (long)(hdrptr + FILHSZ + seccnt * SCNHSZ), 0);
#if !ONEPROC
	if((fdstring = fopen(filenames[3],"r"))==NULL)
		aerror("Cannot Open Temporary (string) File");
#endif
	if((fdgsymb = fopen(filenames[2],"w"))==NULL)
		aerror("Cannot Open Temporary (global symbol) File");
	if((fdsymb = fopen(filenames[3],"w"))==NULL)
		aerror("Cannot Open Temporary (symbol) File");
	if((fdline = fopen(filenames[4],"w"))==NULL)
		aerror("Cannot Open Temporary (line no) File");
	if((fdrel = fopen(filenames[5],"w"))==NULL)
		aerror("Cannot Open Temporary (rel) File");
#if !ONEPROC
	if ( sdicnt != 0 )
		free(savptr);	/* free space no longer needed */
#endif
        rel = 0;
        siz = 0;
        for (i = 1, sect = &sectab[1], seci = &secdat[1]; i <= seccnt; i++, sect++, seci++)
        {
        	lineent = 0;
                relent = 0;
                codout(filenames[i+5], siz, i);
                if (seci->s_typ == TXT)
                {
#if MC68
                        if (seci->s_up & 01)
                        {
                                seci->s_up--;
                                codgen(BITSPBY, FILL);
                        }
                        while (seci->s_up > 0)
                        {
                                codgen(2 * BITSPBY, TXTFILL);
                                seci->s_up -= 2;
                        }
#else
                        while (seci->s_up--) codgen(BITSPBY, TXTFILL);
#endif
                }
                else if (seci->s_typ != BSS)
			while (seci->s_up--) codgen(BITSPBY, FILL);
                sect->s_nlnno = lineent;
                sect->s_nreloc = relent;
                rel += sect->s_nreloc;
                siz += sect->s_size;
        }

        /*
         * raw code for all sections now complete
         */

        fixst(fdsymb);
#if !ONEPROC
	fclose(fdstring);	/* file no longer needed */
#endif
	fflush(fdline);
	if (ferror(fdline))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdline);	/* line numbers are complete */
	fflush(fdrel);
	if (ferror(fdrel))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdrel);	/* flush the buffer */
	fflush(fdgsymb);
	if (ferror(fdgsymb))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdgsymb);	/* flush the buffer */

	if ((fdgsymb = fopen(filenames[2],"r")) == NULL)
		aerror("Cannot Open Temporary (gbl sym) File");
	symout();
	fclose(fdgsymb);	/* preliminary global symbols are processed */
	fflush(fdsymb);
	if (ferror(fdsymb))
		aerror("trouble writing; probably out of temp-file space");
	fclose(fdsymb);	/* symbol table is complete */
	home = ftell(fdout);	/* remember where we are */
	fseek(fdout,0L,0);
	if (dlflag) {
		for (i = 1; i <= seccnt; i++)
			sectab[i].s_nlnno = 0;
	}
	headers();
	fseek(fdout,home,0);	/* return for more work */

	if ((fdrel = fopen(filenames[5],"r")) == NULL)
		aerror("Cannot Open Temporary (rel) File");
	reloc(rel);
	fclose(fdrel);	/* relocation info is complete and appended */

#if DEBUG
/*
 *	Performance data collected and written out here
 */

	ttime = times(&ptimes) - ttime;
	if ((perfile = fopen("as.info", "r")) != NULL ) {
		fclose(perfile);
		if ((perfile = fopen("as.info", "a")) != NULL ) {
			fprintf(perfile,
			   "as2\t%07ld\t%07ld\t%07ld\t%07ld\t%07ld\tpass 2\n",
			    ttime, ptimes);
			fclose(perfile);
		}
	}

#endif

	if (!anyerrs) {
		int i;

		if (!dlflag)
			copysect(filenames[4]);	/* append line number info */
#if AR32W || AR32WR
		/*
		 * The following code removes all unused symbol
		 * table entries.  It also will detect errors in
		 * COFF.
		*/
		fflush(fdout);	/* make sure everything is written out */

		if (uflag)
		{
			if((i = unused( filenames[3], filenames[1] )) <= -1000)
				aerror("Internal object file error");
			else if(i < 0)
				werror("Cannot reduce symbol table - unused symbols remain");
			else if(i == 0) /* no symbols to remove */
				copysect(filenames[3]);/* append symbol table */
		}
		else
			copysect(filenames[3]);
		/*
		 * End of removal code
		*/
#else
		copysect(filenames[7]);	/* append symbol table */
#endif
#if FLEXNAMES
		if (currindex > 4)	/* Does a string table exist?	*/
		{
			fseek( fdout, 0L, 2 );	/* write at the end of the symbol table */
			fwrite((char *)&currindex,sizeof(long),sizeof(char),fdout);
			fwrite(&strtab[4],sizeof(char),currindex-4,fdout);
		}
#endif
		fflush(fdout);
		if (ferror(fdout))
			aerror("trouble writing; probably out of temp-file space");
		fclose(fdout);

#if DEBUG
		if (testas != TESTVAL + 1)
#endif
			deltemps();
#if ONEPROC
		return(0);
#else
		exit(0);
#endif
	}
	else
		delexit();
}
