/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/rdspec.c	1.11"

#include <stdio.h>
#include <ctype.h>
#include "filehdr.h"
#include "ldfcn.h"
#include "reloc.h"
#include "scnhdr.h"
#include "syms.h"
#include "shlib.h"
#include "hst.h"
#include "trg.h"

#define	SEGBOUND	0x20000

#define	SKIPWHITE(x)	while (isspace(*x)) x++;

/* structure containing information about specification file directives */
#define	ADDRESS 1
#define	TARGET	2
#define	BRANCH	3
#define	OBJECTS 4
#define	INIT	5
#define	IDENT	6
#define EXPORT	7
#define HIDE	8


static struct {
	char	*dname;	/* name of directive */
	int	didx;	/* value given for use in switch statement */
} dirs[]=
{
	"address",	ADDRESS,
	"target",	TARGET,
	"branch",	BRANCH,
	"objects",	OBJECTS,
	"init",		INIT,
	"ident",	IDENT,
	"export",	EXPORT,
	"hide",		HIDE
};

#define	NUMDIRS	(sizeof(dirs)/sizeof(dirs[0]))

/* parameters specifying the current type of specification line */
#define	BAD	0
#define	BRANCHL	1
#define	OBJECTL	2
#define	INITL	3
#define SLIBS 	4		/* #objects noload entries */
#define	EXPL	5		/* Export linker directive */
#define	EXPR	6		/* Export runtime */
#define	HIDL	7		/* Hide linker */
#define	HIDR	8		/* Hide runtime */
#define	BR	9		/* Branch table entry must be exported */
#define	IN	10		/* Init entry must be exported */

/* Static variables needed for reading the shared library specification file. */
static FILE	*ifil;		/* pointer to ifile */
static FILE	*assemf;	/* pointer to ainit */
static int	aflag=FALSE,	/* indicates if #address directive was given */
		tflag=FALSE,	/* indicates if #target directive was given */
		bflag=FALSE,	/* indicates if #branch directive was given */
		oflag=FALSE,	/* indicates if #objects directive was given */
		iflag=FALSE,	/* indicates if #init directive was given */
		txtflag=FALSE,	/* indicates if start address of .text was given */
		dataflag=FALSE,	/* indicates if start address of .data was given */
		bssflag=FALSE;	/* indicates if start address of .bss was given */
		someexp=FALSE;	/* Some symbols exported by #export-error handling */
		somehide=FALSE;	/* Some symbols hidden by #hide-error handling */
static initinfo *pinit=NULL;	/* hold init info. concerning current object file */
static int	lineno,
		curtype=BAD;	/* describes current type line being processed */

/* Structure used to hold already input export and hide symbols */
typedef struct ehstruct{
	char	*symname;	/* Symbol name */
	int	type;		/* Way symbol was read in - for error handling */
	int	found;		/* indicates if symbol found */
	struct	ehstruct *next;	
} ehstruct;


ehstruct *explst[EXPSIZ];	/* Hash table containing symbols for export */
				/* entries - hashed on a function of symbol name */

ehstruct *hidelst[HIDSIZ];	/* Hash table containing symbols for hide */
				/* entries - hashed on a function of symbol name */

btrec  **btorder;	/* array of ptrs to entries in the branch table; this
/* Read the shared library specification file and set the appropriate 
 * global variables.
 */
void
rdspec()
{
	FILE	*sfil;		/* pointer to shared lib. specification file */
	char	line[BUFSIZ];	/* input buffer- holds the current line */
	char	*tptr;		/* pointer into the current line */
	int	i;


	/* open specname */
	if ((sfil= fopen(specname,"r")) == NULL)
		fatal("Cannot open %s", specname);

        /* allocate and initialize trgobjects */
        if ((trgobjects= (char **)calloc(OBJCHUNK,sizeof(char *))) == NULL)
                fatal("Out of space");

	/* allocate and initialize objnold - other shared libraries */
        if ((objnold= (char **)calloc(OBJCHUNK,sizeof(char *))) == NULL)
                fatal("Out of space");

	/* allocate and initialize expsyms - exported symbols */
        if ((expsyms= (char **)calloc(OBJCHUNK,sizeof(char *))) == NULL)
                fatal("Out of space");

	/* allocate and initialize hidesyms - hidden symbols */
        if ((hidesyms= (char **)calloc(OBJCHUNK,sizeof(char *))) == NULL)
                fatal("Out of space");

	/* if the target shared library is to be regenerated, then set up ifil2name
	 * and initialize target dependent structures */
	if (maketarg == TRUE) {
		/* open ifil2name */
		if ((ifil= fopen(ifil2name,"w")) == NULL) 
			fatal("Cannot open temp file");
	
		/* print leading lines of ifil */
		(void)fprintf(ifil,"SECTIONS\n{\n");

		/* allocate and initialize btorder */
        	if ((btorder= (btrec **)calloc(MAXBT,sizeof(btrec *))) == NULL) 
			fatal("Out of space");
	}

	if (makehost == TRUE) {
		/* open moddir as a directory to hold the modified objects 
		 * and initialization code */
		if (execute("mkdir", "mkdir", moddir, (char *)0))
			fatal("Internal mkdir invocation failed");
	}

	lineno=0;
	while (fgets(line,BUFSIZ,sfil) != NULL) {
		lineno++;
		if (strlen(line) >= BUFSIZ-1)
			fatal("%s, line %d: line too long\n",specname,lineno);
		tptr= line;
		SKIPWHITE(tptr);

		/* blank line */
		if (*tptr == '\0')
			continue;

		if (*tptr == '#')
		{
			directive(tptr);
		}
		else
			spec(tptr);
	}

	if (maketarg == TRUE) {
		if (!bssflag)
			(void)fprintf(ifil,".bss BIND (ADDR(.data)+SIZEOF(.data)): {}\n");
		(void)fprintf(ifil,"}\n");
		(void)fclose(ifil);

		/* make sure the branch table has no empty slots */
		for (i=0; i<tabsiz; i++) {
			if (btorder[i] == NULL)
				fatal("Branch table slot number %d is not specified\n", i+1);
		}

		if (!tflag)
                        fatal("Must specify the target pathname in %s- use the #target directive\n", specname);
		if (!bflag)
                        fatal("Must specify the branch table in %s- use the #branch directive\n", specname);
		if (!txtflag)
			warn("start address of the .text section of the target is not specified\n");
		if (!dataflag)
			warn("start address of the .data section of the target is not specified\n");
		if (!aflag)
			fatal("Must specify the start address of loaded sections in %s- use the #address directive\n", specname);
	}

	if (makehost == TRUE) {
		if (pinit != NULL) {
			/* finish with last .init section */
			(void)fclose(assemf);
			if (execute(asname,asname,"-dl","-o",pinit->initname,
								ainit,(char *)0))
				fatal("Internal %s invocation failed",asname);
		}
	}

	/* make sure the shared library is properly specified */
	if (!oflag)
		fatal("Must specify input objects in %s- use the #objects directive.\n", specname);

	(void)fclose(sfil);
}


void
spec(lptr)
char	*lptr;		/* pointer into current line */
{
	char	*str,
		*rest,
		*err,		/* used to tell if strtol() returns an error */
		*tname;		/* holds name of current branch table symbol */
	long	pos1,		/* lowest branch table slot for current branch 
				 * table symbol */
		pos2,		/* highest branch table slot for current branch 
				 * table symbol */
		tpos;
	btrec	*pbtrec;	/* pointer to current branch table entry */
	ehstruct *pehstruct;	/* Pointer to current export/hide hash entry */
	ehstruct *newehstruct();	/* Function declared later */
	long	hval;		/* hash value of tname in btlst */
	char	*import;	/* name of imported symbol */
	char	*pimport;	/* name of pointer to imported symbol */
	char	*hid;		/* Temporarily hold hidden symbol */
	char	*exp;		/* Temporarily hold exported symbol */
	char	*iname;		/* Temp name of exported symbol from init section */

	static int 	btordsiz=MAXBT;	/* current number of slots allocated in
					 * btorder[] */
	static int	maxnumobjs=OBJCHUNK;	/* current number of slots 
						 * allocated in trgobjects[] */

	static	int	maxnumlibs=OBJCHUNK;	/* current number of slots for objnold[] */
	static	int	maxnexpsyms=OBJCHUNK;	/* current number of slots for expsyms[] */
	static	int	maxnhidesyms=OBJCHUNK;	/* current number of slots for objnold[] */

	switch (curtype) 
	{
	case  BRANCHL:
		/* branch table specification line */
		if (maketarg == FALSE)
			break;
		bflag= TRUE;

		/* first get branch table symbol */
		if ((rest=gettok(lptr)) == NULL)
			fatal("%s, line %d: bad branch table specification",specname,lineno);
		tname= stralloc(lptr);

		/* now get branch table slot(s) */
		lptr= rest;
		if ((rest=gettok(lptr)) != NULL && *rest != '-')
			fatal("%s, line %d: bad branch table specification",specname,lineno);

		if (rest || (str= strchr(lptr,'-')) != NULL) {
			/* range of branch table slots specified */
			if (rest) {
				rest++;
				SKIPWHITE(rest);
				if (gettok(rest) != NULL)
					fatal("%s, line %d: bad branch table specifiation",specname,lineno);
				str=rest;
			} else
				*str++= '\0';

			pos1= strtol(lptr,&err,0);
			if (*err != '\0')
				fatal("%s, line %d: bad branch table specification",specname,lineno);
			pos2= strtol(str,&err,0);
			if (*err != '\0')
				fatal("%s, line %d: bad branch table specification",specname,lineno);
		} else {
			/* single slot specified */
			pos1=strtol(lptr,&err,0);
			if (*err != '\0')
				fatal("%s, line %d: bad branch table specification",specname,lineno);
			pos2= pos1;
		}

		hval=hash(tname, BTSIZ);
		pbtrec= btlst[hval];
		/* see if tname is already in branch table */
		while (pbtrec != NULL) {
			if (strcmp(pbtrec->name, tname) == 0) {
				/* tname found in branch table- adjust the slot 
				 * which will get the definition of tname */
				if (pbtrec->pos < pos2)
					pbtrec->pos = pos2;
				break;
			}
			pbtrec= pbtrec->next;
		}
		if (pbtrec == NULL) {
			/* tname not found in branch table- 
			 * allocate and set pbtrec */
			pbtrec=newbtrec(tname, pos2, btlst[hval]);
			btlst[hval]=pbtrec; /* add pbtrec to b.t. */
		}

		/* make sure that btorder is large enough to hold
		 * the new branch table entries */
		if (btordsiz < pos2) {
			btordsiz+= MAXBT* ((pos2-btordsiz)/MAXBT + 1);
			if ((btorder=(btrec **)realloc(btorder,sizeof(*btorder) * btordsiz)) == NULL)
				fatal("Out of space");
		}

		/* put pbtrec in appropriate order */
		tabsiz+= (pos2-pos1) + 1;
		for (tpos=pos1; tpos <= pos2; tpos++) {
			if (btorder[tpos-1] != NULL)
				fatal("%s, line %d: branch table slot number %d is multiply specified",specname,lineno,tpos);
			btorder[tpos-1]= pbtrec;
		}

		/* All branch table entries must be exported.  Look up name */
		/* in hash table explst.   If not there, add tname to hash  */
		/* table, and expsyms list.				    */

		hval = hash(tname, EXPSIZ);	/* Hash value of branch name */
		pehstruct = explst[hval];	/* Pointer to entry in explst */
		while (pehstruct != NULL) {
			if (strcmp(pehstruct->symname, tname) == 0) 
				break;
			pehstruct = pehstruct->next;
		}
		if (pehstruct == NULL) {
			/* tname not found in export/hide table- 
			 * allocate and set pehstruct */
			pehstruct=newehstruct(tname, BR, explst[hval]);
			explst[hval]=pehstruct; /* add pehstruct to e.h. table  */
				
			/* Add name to list of exported symbols */	
			/* This list is printed in loader ifile */
			nexpsyms++;	

			/* make sure that expsyms[] is large 
			 * enough to hold new exported name */
			if (nexpsyms > maxnexpsyms) {
				maxnexpsyms += OBJCHUNK;
				if ((expsyms=(char **)realloc((char *)expsyms,
					sizeof(char *) * maxnexpsyms)) == NULL)
					fatal("Out of space");
			}

			expsyms[nexpsyms-1]= stralloc(tname);
			if (hasmeta (expsyms [nexpsyms-1]) != 0)
				fatal ("%s, line %d:  regular expressions not allowed in branch table\n",specname,lineno);
			
		}
		break;

	case OBJECTL:
		/* object file specification line */

		/* scan line file object file names */
		rest= gettok(lptr);
		while (*lptr != '\0') {
			numobjs++;

			/* make sure that trgobjects[] is large 
			 * enough to hold new object name */
			if (numobjs > maxnumobjs) {
				maxnumobjs+= OBJCHUNK;
				if ((trgobjects=(char **)realloc((char *)trgobjects,
					sizeof(char *) * maxnumobjs)) == NULL)
					fatal("Out of space");
			}

			trgobjects[numobjs-1]= stralloc(lptr);
			if (rest == NULL)
				break;
			lptr=rest;
			rest= gettok(lptr);
		}
		break;
	case SLIBS:
		rest= gettok(lptr);
		while (*lptr != '\0') {
			numnold++;

			/* make sure that objnold[] is large 
			 * enough to hold new library name */
			if (numnold > maxnumlibs) {
				maxnumlibs+= OBJCHUNK;
				if ((objnold=(char **)realloc((char *)objnold,
					sizeof(char *) * maxnumlibs)) == NULL)
					fatal("Out of space");
			}

			objnold[numnold-1]= stralloc(lptr);
			if (rest == NULL)
				break;
			lptr=rest;
			rest= gettok(lptr);
		}
		break;

	case EXPL:
		someexp = FALSE;	/* Turn off error handling flag */
		if (allexp == TRUE)
			fatal ("%s, line %d:  can't list symbols explicitly if all symbols are exported\n",specname,lineno);
		rest= gettok(lptr);
		while (*lptr != '\0') {

			exp = stralloc(lptr);		/* Save exported symbol */
			hval = hash(exp, EXPSIZ);	/* Hash value of exported symbol */
			pehstruct = explst[hval];	/* Pointer to entry in explst */
			while (pehstruct != NULL) {
				if (strcmp(pehstruct->symname, exp) == 0) 
					break;
				pehstruct = pehstruct->next;
			}
			if (pehstruct == NULL) {
				/* exp not found in export/hide table- 
			 	* allocate and set pehstruct */
				pehstruct=newehstruct(exp, EXPL, explst[hval]);
				explst[hval]=pehstruct; /* add pehstruct to e.h. table  */


				/* Add name to list of exported symbols */	
				/* This list is printed in loader ifile */
				nexpsyms++;	

				/* make sure that expsyms[] is large 
			 	* enough to hold new exported name */
				if (nexpsyms > maxnexpsyms) {
					maxnexpsyms += OBJCHUNK;
					if ((expsyms=(char **)realloc((char *)expsyms,
						sizeof(char *) * maxnexpsyms)) == NULL)
						fatal("Out of space");
				}

				expsyms[nexpsyms-1]= stralloc(exp);
				if (hasmeta (expsyms [nexpsyms-1]) < 0)
					fatal ("%s, line %d:  syntax error in regular expression\n",specname,lineno);
			
			}
			else		/* Found in hash table */
			{
				if (pehstruct->type == EXPL)
					fatal ("%s, line %d:  %s exported twice\n",specname,lineno,exp);
			}

			if (rest == NULL)
				break;
			lptr=rest;
			rest= gettok(lptr);
		}
		break;

	case HIDL:
		somehide = FALSE;	/* Turn off error handling flag */
		if (allhide == TRUE)
			fatal ("%s, line %d:  can't list symbols explicitly if all symbols are hidden",specname,lineno);
		rest= gettok(lptr);
		while (*lptr != '\0') {
			nhidesyms++;

			/* make sure that hidesyms[] is large 
			 * enough to hold new hidden name */
			if (nhidesyms > maxnhidesyms) {
				maxnhidesyms += OBJCHUNK;
				if ((hidesyms=(char **)realloc((char *)hidesyms,
					sizeof(char *) * maxnhidesyms)) == NULL)
					fatal("Out of space");
			}

			hidesyms[nhidesyms-1]= stralloc(lptr);
			hid = hidesyms[nhidesyms-1];
			if (hasmeta (hidesyms[nhidesyms-1]) < 0)
				fatal ("%s, line %d:  syntax error in regular expression\n",specname,lineno);

			hval = hash(hid, HIDSIZ);	/* Hash value of hidden symbol */
			pehstruct = hidelst[hval];	/* Pointer to entry in hidelst */
			while (pehstruct != NULL) {
				if (strcmp(pehstruct->symname, hid) == 0) 
					break;
				pehstruct = pehstruct->next;
			}
			if (pehstruct == NULL) {
				/* hid not found in export/hide table- 
			 	* allocate and set pehstruct */
				pehstruct=newehstruct(hid, HIDL, hidelst[hval]);
				hidelst[hval]=pehstruct; /* add pehstruct to e.h. table  */
			}
			else		/* Found in hash table */
			{
				if (pehstruct->type == HIDL)
					fatal ("%s, line %d:  %s hidden twice\n",specname,lineno,hid);
			}

			if (rest == NULL)
				break;
			lptr=rest;
			rest= gettok(lptr);
		}
		break;

	case INITL:
		/* initialization specification line */
		/* get name of pointer to imported symbol */
		if ((rest=gettok(lptr)) == NULL)
			fatal("%s, line %d: bad init. spec. file line",specname,lineno);
		pimport=lptr;
		lptr=rest;
	
		/* get name of imported symbol */
		if (gettok(lptr) != NULL)
			fatal("%s, line %d: bad init. spec. file line",specname,lineno);
		import= lptr;
	
		/* generate initialization code */
		if (makehost == TRUE)
		{
			initpr(assemf, import, pimport);
		}
				
		/* All init entries must be exported.  Look up name */
		/* in hash table explst.   If not there, add name to hash  */
		/* table, and expsyms list.				    */

		iname = stralloc(pimport);

		hval = hash(iname, EXPSIZ);	/* Hash value of branch name */
		pehstruct = explst[hval];	/* Pointer to entry in explst */
		while (pehstruct != NULL) {
			if (strcmp(pehstruct->symname, iname) == 0) 
				break;
			pehstruct = pehstruct->next;
		}
		if (pehstruct == NULL) {
			/* symbol not found in export/hide table- 
			 * allocate and set pehstruct */
			pehstruct=newehstruct(iname, IN, explst[hval]);
			explst[hval]=pehstruct; /* add pehstruct to e.h. table  */
				
			/* Add name to list of exported symbols */	
			/* This list is printed in loader ifile */
			nexpsyms++;	

			/* make sure that expsyms[] is large 
			 * enough to hold new exported name */
			if (nexpsyms > maxnexpsyms) {
				maxnexpsyms += OBJCHUNK;
				if ((expsyms=(char **)realloc((char *)expsyms,
					sizeof(char *) * maxnexpsyms)) == NULL)
					fatal("Out of space");
			}
			expsyms[nexpsyms-1]= stralloc(iname);
			if (hasmeta (expsyms [nexpsyms-1]) < 0)
				fatal ("%s, line %d:  syntax error in regular expression\n",specname,lineno);
			
		}
		break;

	default:
		fatal("%s, line %d: missing #<directive> line",specname,lineno);
	}
}


void
directive(lptr)
char	*lptr;	/* pointer into current line */
{
	int	i;
	char	*str,
		*rest,
		*tname;
	long	addr;
	char	*err;

	lptr++;
	SKIPWHITE(lptr);

	if (*lptr == '#')
		/* comment line */
		return;

	/* if #export linker (or hide) is not followed by one or more symbols */
	/* Flags set to true in directive() and false when symbol line encountered */
	/* in spec() 	*/
	if ((someexp == TRUE) || (somehide == TRUE))
		fatal("%s, line %d: must have symbols after #export or #hide directive\n",specname,lineno);

	rest= gettok(lptr);
	for (i=0; i < NUMDIRS; i++) { 
		if (strcmp(lptr, dirs[i].dname) == 0)
			break;
	}
	if (i == NUMDIRS)
		fatal("%s, line %d: illegal directive line", specname, lineno);

	lptr= rest;
	SKIPWHITE(lptr);

	switch(dirs[i].didx)
	{
	case ADDRESS:
		/* #address- section start address specification line */
		curtype= BAD;
		if (maketarg == FALSE)
			break;
		aflag = TRUE;

		/* first get section name */
		if ((rest=gettok(lptr)) == NULL)
			fatal("%s, line %d: bad #address line",specname,lineno);
		tname=lptr;

		/* then get section's start address */
		lptr=rest;
		if (gettok(lptr) != NULL)
			fatal("%s, line %d: bad #address line",specname,lineno);

		addr= strtol(lptr,&err,0);
		if (*err != '\0')
			fatal("%s, line %d: bad address in #address line",
								specname,lineno);
		if (addr % SEGBOUND != 0)
			fatal("%s, line %d: bad #address line- section must start on segment boundary",specname,lineno);

		/* add start address of section to ifile */
		(void)fprintf(ifil,"\t%s %s:\t{}\n",tname, lptr);

		if (strcmp(tname,".text") == 0)
			txtflag = TRUE;
		else if (strcmp(tname,".data") == 0)
			dataflag = TRUE;
		else if (strcmp(tname,".bss") == 0)
			bssflag = TRUE;
		break;

	case TARGET:
		if (tflag == TRUE)
			fatal("%s, line %d: multiple #target directives",specname,lineno);
		/* #target- specifies pathname of target on target machine */
		curtype= BAD;
		tflag = TRUE;

		/* get pathname of target (on target machine) */
		if (gettok(lptr) != NULL || *lptr=='\0')
			fatal("%s, line %d: bad #target line",specname,lineno);
		trgpath= stralloc(lptr);
		break;

	case BRANCH:
		if (bflag == TRUE)
			fatal("%s, line %d: multiple #branch directives",specname,lineno);
		/* #branch- start of branch table specificaton */
		curtype= BRANCHL;
		if (maketarg == FALSE)
			break;
		/* should be no other token on the line */
		if (lptr != NULL)
			fatal("%s, line %d: bad #branch line",specname,lineno);
		break;

	case OBJECTS:
		/* #objects- start of list of input objects */
		if (lptr != NULL)		/* Look for noload keyword */
		{
			extern int nflag;
			gettok(lptr);
			if (strcmp(lptr,"noload") != 0)
				fatal("%s, line %d: bad #objects line",specname,lineno);
			if (maketarg == FALSE)
				fatal(
"%s, line %d: \"#objects noload\" not implemented under \"-n\" option\n\
(Omit the \"-n\" option to build a target, use chkshlib(1) to compare\n\
it against the target you were using for this \"-n\" run.)\n",
				specname, lineno);

			curtype = SLIBS;	/* List of shared libraries */
		}
		else				/* #objects line */
		{
			if (oflag == TRUE)
				fatal("%s, line %d: multiple #objects directives",specname,lineno);
			oflag = TRUE;		/* Only one #objects line allowed */
			curtype= OBJECTL;
		}
		break;

	case EXPORT:
		rest = gettok(lptr);	/* Save returned value from gettok */
		if (*lptr == '\0')	/* Must have linker */
			fatal("%s, line %d: must have keyword linker after #export directive",specname, lineno);
		if (strcmp (lptr,"linker") == 0)
			curtype = EXPL;
		else
		{
			if (strcmp (lptr,"runtime") == 0)
				fatal("%s, line %d: Not applicable to static shared libraries",specname, lineno);
			else
				fatal("%s, line %d: must have keyword linker after #export directive",specname, lineno);
		}	
		if (rest != NULL)	/* '*' allowed after {runtime,linker} */
		{
			lptr = rest;
			rest = gettok(lptr);
			if ((rest != NULL) || (*lptr == '\0'))
				fatal("%s, line %d: usage #export linker [*]",specname, lineno);
			if (strcmp (lptr,"*") != 0)
			{
				fatal("%s, line %d: usage #export linker [*]",specname, lineno);
			}
			allexp = TRUE;	/* All symbols exported - global flag */
		}
		else
			someexp = TRUE;	/* Some symbols exported */
		break;

	case HIDE:
		rest = gettok(lptr);	/* Save returned value from gettok */
		if (*lptr == '\0')	/* Must have linker */
			fatal("%s, line %d: must have keyword linker after #hide directive",specname, lineno);
		if (strcmp (lptr,"linker") == 0)
			curtype = HIDL;		/* Hide linker */
		else
		{
			if (strcmp (lptr,"runtime") == 0)
				fatal("%s, line %d: not applicable to static shared libraries",specname, lineno);
			else
				fatal("%s, line %d: must have keyword linker after #hide directive",specname, lineno);
		}	
		if (rest != NULL)	/* '*' allowed after {runtime,linker} */
		{
			lptr = rest;
			if ((gettok(lptr) != NULL) || (*lptr == '\0'))
				fatal("%s, line %d: usage #hide linker [*]",specname, lineno);
			if (strcmp (lptr,"*") != 0)
				fatal("%s, line %d: usage #hide linker [*]",specname, lineno);
			allhide = TRUE;	/* All symbols hidden - global flag */
		}
		else
			somehide = TRUE;	/* Some symbols hidden */
		hidef=TRUE;	/* Hiding symbols, so ld will be called for each object file */
		break;

	case INIT:
		/* #init- initialization declaration line */
		curtype= INITL;
		if (makehost == FALSE)
			break;

		/* first finish with current .init section */
		if (pinit != NULL) {
			(void)fclose(assemf);
			if (execute(asname,asname,"-dl","-o",pinit->initname,
								ainit,(char *)0))
				fatal("Internal %s invocation failed",asname);
		}

		if (gettok(lptr) != NULL || *lptr=='\0')
			fatal("%s, line %d: bad #init line",specname,lineno);

		/* now get new file for next .init section */
		str= getbase(lptr);
		pinit= newinitinfo(stralloc(str), inits);
		inits= pinit;

		/* Open a temporary file for the assembly code
		 * of the .init section of the current object */
		if ((assemf= fopen(ainit,"w")) == NULL)
			fatal("Cannot open temp file");

		/* Print leading lines of assembly file */
		prlal(assemf, pinit->objname);
		break;

	case IDENT:
		/* #ident- ident string specification */
		curtype= BAD;
		if ((idstr= getident(lptr)) == NULL)
			fatal("%s, line %d: bad #ident line",specname,lineno);
		break;

	default:
		fatal(stderr,"%s, line %d: illegal directive\n",specname,lineno);
	}
}


/* gettok() parses the input lines of the specification file.  Its function is
 * somewhat similar to strtok(3) in that it places a null character in str after
 * the first token.  The token separators are white space or a terminating '##'.
 * Gettok() also returns a pointer to the next token if one exists. Otherwise,
 * it returns NULL.
 */
char *
gettok(lptr)
char	*lptr;
{
	char	*str;

	/* see if lptr starts with 2 #'s */
	SKIPWHITE(lptr);
	if (*lptr == '#') {
		str= lptr;
		str++;
		SKIPWHITE(str);
		if (*str == '#')
			return(NULL);
	}

	/* skip pointer over token */
	while (!isspace(*lptr)) {
		if (*lptr == '\0')
			return(NULL);
		lptr++;
	}

	*lptr= '\0';
	lptr++;

	/* find beginning of next token */
	SKIPWHITE(lptr);
	if (*lptr == '\0')
		return(NULL);
	else if (*lptr == '#') {
		str= lptr;
		str++;
		SKIPWHITE(str);
		if (*str == '#')
			return(NULL);
	}
	return(lptr);
}



/* getident() expects a quoted string as input and returns a string containing
 * the quote.  This routine is used to determine if #ident lines have the 
 * proper format.
 */
char *
getident(lptr)
char	*lptr;
{
	char	*str;

	SKIPWHITE(lptr);
	if (*lptr != '"') {
		return(NULL);
	}
	lptr++;
	if ((str=strchr(lptr,'"')) == NULL) {
		return(NULL);
	}
	*str= '\0';

	/* make sure there is nothing else on the line */
	str++;
	SKIPWHITE(str);
	if (*str == '#') {
		str++;
		SKIPWHITE(str);
		if (*str != '#')
			return(NULL);
	} else if (*str != '\0')
		return(NULL);

	return(stralloc(lptr));
}

/*
 * Return 1 if "s" contains special characters that
 * make "s" a r.e.;
 * return -1 if it's a malformed r.e.;
 * return 0 if it contains no metacharacters.
 */

hasmeta(s)
	char *s;
{
	int foundmeta = 0;
	enum { inbracket, notinbracket} state = notinbracket;

	do
	{
		switch (*s)
		{
		    case '*':
		    case '?':
			foundmeta = 1;
			break;
		    case '\\':	
			s++; /* Ignore next character */
			if (*s == '\0')
				return(-1);
			break;
		    case '[':
			if (state == inbracket) return(-1);
			state = inbracket;
			foundmeta = 1;
			break;
		    case ']':
		    case '-':
		    case '!':
			if (state == notinbracket) return(-1);
			foundmeta = 1;
			if (*s == ']') state = notinbracket;
			break;
		}
	} while (*s++ != '\0');
	if (state == inbracket) return(-1);
	return(foundmeta);
}

/* newehstruct
	
	Create a new node for the hash table explst.
								*/

ehstruct *newehstruct(name, type, next)
char	*name;
int	type;
ehstruct	*next;
{
	
	ehstruct *tnode;

	if ((tnode = (ehstruct *)malloc(sizeof(ehstruct))) == NULL)
		fatal("Out of space for export/hide hash table");

	tnode->symname = name;
	tnode->type = type;
	tnode->found = FALSE;
	tnode->next = next;
	return(tnode);
}
