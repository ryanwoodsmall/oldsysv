/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/rdspec.c	1.6"

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
	"ident",	IDENT
};

#define	NUMDIRS	(sizeof(dirs)/sizeof(dirs[0]))

/* parameters specifying the current type of specification line */
#define	BAD	0
#define	BRANCHL	1
#define	OBJECTL	2
#define	INITL	3

#define	OBJCHUNK	500

/* Static variables needed for reading the shared library specification file. */
static FILE	*ifil;		/* pointer to ifile */
static FILE	*assemf;	/* pointer to ainit */
static int	aflag=FALSE,	/* indicates if #address directive was given */
		tflag=FALSE,	/* indicates if #target directive was given */
		bflag=FALSE,	/* indicates if #branch directive was given */
		oflag=FALSE,	/* indicates if #objects directive was given */
		txtflag=FALSE,	/* indicates if start address of .text was given */
		dataflag=FALSE,	/* indicates if start address of .data was given */
		bssflag=FALSE;	/* indicates if start address of .bss was given */
static initinfo *pinit=NULL;	/* hold init info. concerning current object file */
static int	lineno,
		curtype=BAD;	/* describes current type line being processed */

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
			directive(tptr);
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
		if (!txtflag && !qflag)
			(void)fprintf(stderr,"Warning: start address of the .text section of the target is not specified\n");
		if (!dataflag && !qflag)
			(void)fprintf(stderr,"Warning: start address of the .data section of the target is not specified\n");
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
	long	hval;		/* hash value of tname in btlst */
	char	*import;	/* name of imported symbol */
	char	*pimport;	/* name of pointer to imported symbol */

	static int 	btordsiz=MAXBT;	/* current number of slots allocated in
					 * btorder[] */
	static int	maxnumobjs=OBJCHUNK;	/* current number of slots 
						 * allocated in trgobjects[] */

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
		break;

	case OBJECTL:
		/* object file specification line */
		oflag= TRUE;

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

	case INITL:
		/* initialization specification line */
		if (makehost == FALSE)
			break;

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
		initpr(assemf, import, pimport);
		break;

	default:
		fatal("%s, line %d: missing #branch, #object or #init line",specname,lineno);
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
		/* #target- specifies pathname of target on target machine */
		curtype= BAD;
		if (maketarg == FALSE)
			break;
		tflag = TRUE;

		/* get pathname of target (on target machine) */
		if (gettok(lptr) != NULL || *lptr=='\0')
			fatal("%s, line %d: bad #target line",specname,lineno);
		trgpath= stralloc(lptr);
		break;

	case BRANCH:
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
		/* should be no other token on the line */
		if (lptr != NULL)
			fatal("%s, line %d: bad #objects line",specname,lineno);
		curtype= OBJECTL;
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
