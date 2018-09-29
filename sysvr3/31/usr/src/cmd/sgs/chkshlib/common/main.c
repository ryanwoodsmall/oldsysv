/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)chkshlib:common/main.c	1.6"

#include <stdio.h>
#include <signal.h>
#include <memory.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "filehdr.h"
#include "ldfcn.h"
#include "string.h"
#include "reloc.h"
#include "sgs.h"
#include "syms.h"
#include "scnhdr.h"
#include "chkshlib.h"
#include "aouthdr.h"


/* GLOBAL VARIABLES */
int	vflag;	/* verbose option for incompatibilities */
int	iflag;	/* remove identical targetnames restriction - warn only*/
int	bflag;	/* report out of bounds warnings */
int	nflag;	/* nested shared libraries */
int 	exit_status;		/* return from main */
int 	file_count;		/* number of input files */
int 	incompatible_flag;	/* an incompatibility is found */
int 	bounds_flag;		/* a symbol out of bounds is found - used for	*/
				/* case of two targets or two hosts only 	*/
int 	reverse_flag;		/* 2 HOST or 2 TARG - check both ways */
symrec 	*hashtab[TABSIZ]; 	/* hash table to store file symbols */
file_data *head;		/* first input file1 */
file_data *tail;		/* last input file */
file_data *current;		/* current input file */

/* main routine will call cmdline to process the command line, i.e. validate	*/
/* input and build linked list of struct "file_data" for each input file	*/
/* Each file is checked against the first; return from main if no fatal errors	*/

main(argc, argv)
int	argc;
char	*argv[];
{
	int i;

	cmdline(argc, argv);	/* process the command line */
	
	/* two hosts or two targets, reverse_flag is set and for loop	*/
	/* will be executed twice, (once to check file1 vs. file2 and	*/
	/* then visa versa) else for loop is executed only once		*/
	for (i=0; i < (reverse_flag + 1); i++)	{

		for (current=head->next; current!=NULL; current=current->next)	{

			/* not iflag, current's targ(s) must be same as file1*/
			if (!(iflag))
				if (!(check_lib() )) {
					print1_1("Target pathnames not identical", current->f_name);
					continue;
				}

			/* reads and stores "current's" symbols into hash table */
			read_sym_tbl();

			/* check symbols from the firstfile against "current's"*/
			do_check();

			/* null out hash table and flag */
			null_out();

		}
		if (reverse_flag) 
			/* if comparing two hosts or two targets, 	*/
			/* reverse linked list and check other way 	*/
			reverse_files();

	} /* end for */

	return(exit_status);
}

/* For each "other file" (which is to be checked against the first file) */
/* null out the hash table and incompatible flag */

void
null_out()
{
	int j;

	for (j=0; j<TABSIZ; j++) {
		if (hashtab[j] == NULL) continue;
		free( (char *)hashtab[j] );
		hashtab[j] = NULL;
	}
	incompatible_flag = FALSE;
}

/* when you check two hosts or two targets, you must check both ways	*/
/* reverse_flag was set by "cmdline" and the "nodes" in the linked list of  */
/* file_data structures associated with the two files must be reversed	*/
/* Also used when input is  TARG HOST; */
/* do not set reverse_flag, but reverse to HOST TARG and proceed */

void
reverse_files()
{
	head->next = NULL;
	tail ->next = head;
	head = tail;
	tail = head->next;
}

/* Cmdline processes the command line using getopt(3) 			*/
/* The valid types of input are: (EXEC for executable, TARG for target	*/
/*	shared library and HOST for host shared library)		*/
/*	1) EXEC	[TARG1 ... TARGn]					*/
/*	2) EXEC [HOST1 ... HOSTn]					*/
/*	3) HOST TARG							*/
/*	4) HOST HOST							*/
/*	5) TARG HOST							*/
/*	6) TARG TARG							*/
/* As per requirements, all other forms of input will be rejected	*/

void
cmdline(namec, namev)
int 	namec;
char	**namev;
{
	extern char	*optarg;
	extern int	optind;
	int	c;
	file_data	*temptr;

	/* v => verbose reporting of all incompatibilities		*/
	/* b => symbols in file1 out of bounds of "other file(s) 	*/
	/* i => do NOT stop if target pathnames different - warn only   */
	/* n => nested target shared libraries follow			*/

	while ((c = getopt(namec, namev, OPTARGS)) != EOF)
		switch (c) {
			case 'v':
				vflag=TRUE;
				break;
			case 'i':
				iflag=TRUE;
				break;
			case 'b':
				bflag=TRUE;
				break;
			case 'n':
				nflag=TRUE;
				break;
			default:
				usagerr();
		}

	if ( namec <= optind ) /* there are no input files */
		usagerr();

	/* process each of the input files: store in linked list of stuct	*/
	/* "file_data", name, type, target names, bounds, and next ptr		*/
	/* also check each file can be read and IS associated with shared librs */

	process_files(optind, namec, namev);

	/* compare nested targets - treat first as EXEC and return */
	if (nflag)	{
		if ((file_count != 2) || (head->f_type != TARG) || (head->next->f_type != TARG))
			usagerr();
		head->f_type = EXEC;	/* treat as though first is EXEC */
		return;
	}

	if ( file_count == 1 ) /* only one input file, read .lib and return */

		if (ISTARGET (head->f_type)) 
			/* cannot only have one target input file */
			usagerr();
		else 	{  
			/* read .lib for target(s) to check against */
			read_lib (head->f_name); 
			return;
		}

	/* more than two input files, first must be EXEC		*/

	if ((file_count > 2) && (!ISEXECUTABLE(head->f_type)))
		usagerr();

	/* no file other than first can be an EXEC 			*/

	for (temptr=head->next; temptr!=NULL; temptr=temptr->next)
		if (ISEXECUTABLE(temptr->f_type))
			usagerr();

	/* others must all be the same type of file			*/

	switch (head->next->f_type) {
		case HOST:
			/* if two files, first is target and second is host */
			/* reverse input files and process */
			if ((file_count == 2) && (ISTARGET(head->f_type)))  {
				reverse_files();
				break;
			}
			/* two hosts, set reverse_flag			*/
			if ((file_count == 2) &&(ISHOST(head->f_type)))  {
				reverse_flag = 1;
				break;
			}
			allsame(head->next->f_type);
			break;
		case TARG:
			/* two targets, set reverse_flag		*/
			if ((file_count == 2) &&(ISTARGET(head->f_type)))  {
				reverse_flag = 1;
				break;
			}
			allsame(head->next->f_type);
			break;
			
		default:
			usagerr();
	}

}

/* process each of the input files:  call "process" to store in linked list of	*/
/* stuct "file_data", name, type, target names, bounds, and next ptr		*/

void
process_files(index, namec, namev)
int index, namec;
char **namev;
{
	/* run through all input files */
	for (; index < namec; index++, file_count++)
		process(namev[index]);
}

/* This routine reads .lib section of "file" (EXEC or HOST), for targets to check*/
/* It calls process to store the targets in linked list of struct "file_data"	 */

void
read_lib()
{
	targ_name	*tptr;

	/* run through list of target names off first file's "f_targ" member */
	for (tptr=head->f_targ; tptr != NULL; tptr= tptr->next)
		process(tptr->t_name);
}

/* process is called by read_lib and process_files */
/* It checks that file can be open for reading, allocates a "file_data"	*/
/* structure for each file, and fills that stucture.  It also checks no	*/
/* file has been truncated accidently.  It also calls "store_lib" to	*/
/* be sure each file is a shared lib file (has .lib) and store targs 	*/

void 
process(name)
char *name;
{
	file_data	*fdptr;
	struct stat 		buf;
	long	sects_size;

	/* allocate a file_data structure for each input file */
	fdptr = filedata_alloc();
	/* store file name */
	fdptr->f_name = stralloc(name);

	/* get the stat information for file */
	if (stat(name, &buf) != 0)
		printerr("Stat failed", name);

	/* type fills sects_size with sum of sizes of the sections of the file */
	fdptr->f_type = type(name, &sects_size);

	/* if the total size of the file is less than the sum of sections */
	if (buf.st_size < sects_size)
		printerr("Possible file truncation",name);

	/* read the .lib section of file and store target(s) */
	fdptr->f_targ = store_lib(name);

	/* build the linked list of file_data for each input file */
	if (head == NULL) {
		head = tail = fdptr;
		fdptr->next = NULL;
	}
	else	{
		fdptr->next = NULL;
		tail = tail->next = fdptr;
	}

}

/* store_lib checks that the first file is associated with shared libr 	 */
/* (has a .lib) and then stores the target pathname(s) found in the .lib */

targ_name *
store_lib(file)
char *file;
{
	LDFILE 		*ldptr;
	int 		i,numfiles, found;
	SCNHDR 		secthead;
	long 		total_length, offset, file_length, file_byte_length;
	char 		*sptr;
	targ_name 	*tptr, *top = NULL;
		
	ldptr = NULL;
	found = FALSE;

	do  
	{
		if ((ldptr = ldopen(file, ldptr)) != NULL)
	    	{
	    	    /* read .lib section header */
		    if (ldnshread(ldptr,".lib", &secthead) != SUCCESS)
			continue;

		    /* seek to .lib section */
		    if (ldnsseek(ldptr, ".lib") != SUCCESS) continue;

		    if (found == TRUE) 
			printwarn2("Found another .lib section", file);
		    found = TRUE;	/* found .lib section! */

		    /* s_paddr field hold the number of target files */
		    numfiles = (int) secthead.s_paddr;

		    /* if the size in the section header is 0, the .lib is empty */
		    /* note that the size can be 0 and s_paddr be >0 */
		    if (secthead.s_size <= 0)
				printerr("Empty .lib section", file);

		    /* the .lib section contains:			*/
		    /* long, length of targ filename plus other info.	*/
		    /* long, offset to start of target filename		*/
		    /* null padded target filename			*/
		    for (i=0; i<numfiles; i++) {

			if (FREAD((char *)&total_length,(sizeof(long)),1,ldptr) != 1)
				printerr("Trouble reading .lib section", file);
			if (FREAD((char *)&offset, (sizeof (long)), 1, ldptr) != 1)
				printerr("Trouble reading .lib section", file);
			file_length = total_length - offset;
			/* compute length of targ filename in bytes	*/
			file_byte_length = (sizeof (long))*file_length;

			/* get space for targ_name structure to hold pointer 	*/
			/* to linked list of target pathnames from .lib	 	*/
			tptr = targname_alloc();
			tptr->t_name = chk2_malloc ((unsigned)file_byte_length);

			/* store in t_name member the target pathname */
			if (FREAD(tptr->t_name, file_byte_length, 1, ldptr) != 1)
				printerr("Trouble reading .lib section", file);

			/* form the linked list of target pathnames */
			tptr->next = top;
			top = tptr;

		    } /*end for*/

	    	}
		else	printerr("Cannot open file", file);

	} while (ldclose (ldptr) == FAILURE);

	if (found == FALSE) printerr("Cannot find .lib section (header)", file);

	return(top);
}

/* check_lib checks target pathname(s) in the file are identical to file1 */
/* "current" points to current "other file" 's file_data stucture */
/* check it's target pathname(s) and to match with file1 */

int
check_lib()
{
	int 	compares;
	targ_name	*tptr, *tptr2;  /* tptr for file1's targ_name struct ptr */
					/* tptr2 for current's targ_name struct ptr*/

	/* loop through "current" files target names */
	for (tptr2=current->f_targ; tptr2 != NULL; tptr2=tptr2->next) {

		compares = FALSE;
		/* loop through "first" files target names */
		for (tptr=head->f_targ; tptr!=NULL; tptr=tptr->next) 
		{
			/* if found set compares */
			if (strcmp(tptr->t_name, tptr2->t_name) == 0) 
				compares = TRUE;
		}

	}
	return(compares);
}

/* usagerr: invalid use of the command and/or the input - exit status 2, exit	*/

void
usagerr()
{
	(void)fprintf(stderr,"usage: chkshlib [-b] [-i] [-n] [-v] file1 [file2 file3 ...]\n");
	exit(2);
}


/* printerr: print to stderr string and filename - exit status 2, exit	*/
void
printerr(string, filename)
char *string;
char *filename;
{
	(void)fprintf(stderr,"Error: %s\tfile =%s\n", string, filename);
	exit(2);
}


/* printerr2: print to stderr string only - exit status 2, exit	*/
void
printerr2(string)
char *string;
{
	(void)fprintf(stderr,"Error: %s\n", string);
	exit(2);
}


/* printwarn: print warning to stderr string symbolname and value	*/
void
printwarn(string, name, value)
char *string;
char *name;
long value;
{
	(void)fprintf(stderr,"Warning: %s\t%s\tvalue=%lx\n", string, name, value);
	bounds_flag = 1;
}

/* printwarn2: print warning to stderr string only	*/
void
printwarn2(string, filename)
char *string;
char *filename;
{
	(void)fprintf(stderr,"Warning: %s\tfile=%s\n", string, filename);
}

/* print1: to stdout string, symbolname + value - exit status and incompat flag 1*/
void
print1(string, name, value)
char *string;
char *name;
long value;
{
	(void)printf("%s:\t%s\tvalue= %lx\n", string,name,value);
	exit_status = 1;
	incompatible_flag = 1;
}

/* print1_1: to stdout string + filename - exit status 1*/
void
print1_1(string, filename)
char *string;
char *filename;
{
	(void)printf("%s: file =%s\n", string, filename);
	exit_status = 1;
}

/* print1_2: to stdout string, symbolname + value in file1 and in other file	*/
/* exit status and incompat flag 1*/
void
print1_2(string, name, value1, value2)
char *string;
char *name;
long value1;
long value2;
{
	(void)printf("%s:\t%s\tin file1= %lx\tin other= %lx\n", string,name,value1, value2);
	exit_status = 1;
	incompatible_flag = 1;
}

/* depending upon the input, SUCCESS or FAILURE, a message will be printed 	*/
/* determining whether file1 and file2 "check."  We use the phrase "may not"	*/
/* for FAILURE and "can" or "may" for SUCCESS					*/

void
print_result(status)
int status;
{
	char *string = "may not";	/* if FAILURE */
	char *firstfile, *otherfile;

	firstfile = head->f_name;
	otherfile = current->f_name;

	if (status == SUCCESS) 	string = "can";		/* if SUCCESS */

	switch (head->f_type) {

		case EXEC:
	
			if (nflag) {
				(void) printf("\t%s %s include %s\n", firstfile, string, otherfile);
				break;
			}
			if (current->f_type == TARG)
				(void)printf("\t%s %s execute using %s\n", firstfile, string, otherfile);
			else	/*other is HOST*/
			{
				if (status == SUCCESS) 	string = "may";
				(void)printf("\t%s %s have been produced using %s\n", firstfile, string, otherfile);
			}
			break;
	
		case  TARG:
	
			/* both are Targets */
			if (bounds_flag) string = "may not";
			(void)printf("\t%s %s replace %s\n", otherfile, string, firstfile);
			break;
	
		case  HOST:
	
			if (current->f_type == TARG)
				(void)printf("\t%s %s produce executables which will run with %s\n", firstfile, string, otherfile);
			else	/* both are Hosts */
			{
				if (bounds_flag) string = "may not";
				(void)printf("\t%s %s replace %s\n", otherfile, string, firstfile);
			}
			break;
	
		default:
			break;
	}
}

void
allsame(type)
int type;
{
	file_data	*fdptr=head->next->next;

	/* go through the rest of the files to see that all 	   */
	/* the same as the second: either all hosts or all targets */
	for (; fdptr!=NULL; fdptr=fdptr->next)	
		if (fdptr->f_type != type)
			usagerr();
}

type(fname, sects_size)
char *fname;
long *sects_size;
{
	LDFILE	*ldptr=NULL;
	int	ftype=HOST;
	AOUTHDR	ohead;

	*sects_size = 0;
	do	{
		if ( (ldptr = ldopen(fname, ldptr)) != NULL)
		{
			if (ISCOFF(HEADER(ldptr).f_magic))
			{
				FSEEK(ldptr, FILHSZ, 0);
				if (HEADER(ldptr).f_opthdr != 0 &&
					FREAD(&ohead,HEADER(ldptr).f_opthdr,1,ldptr) == 1)
				/* compute the sum of text and data sizes */
				*sects_size += ohead.dsize + ohead.tsize;
	
				if (!ISARCHIVE(TYPE(ldptr)))
				{
					if (ohead.magic == LIBMAGIC)
						ftype= TARG;
					else
						ftype= EXEC;
				}
				/* else it's a HOST - assignment in declaration */
			}
			else  	{
				ldclose (ldptr);
				printerr("Bad magic number in file", fname);
			}

		}
		else	printerr("Cannot open file", fname);

	} while (ldclose(ldptr) == FAILURE);

	return (ftype);
}
