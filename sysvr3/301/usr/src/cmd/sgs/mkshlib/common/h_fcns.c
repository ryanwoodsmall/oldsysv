/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/h_fcns.c	1.2"

#include <stdio.h>
#include "filehdr.h"
#include "reloc.h"
#include "syms.h"
#include "scnhdr.h"
#include "ldfcn.h"
#include "shlib.h"
#include "hst.h"


/* newobj() creates a new obj structure */
obj *
newobj()
{
	obj	*tnode;

	if ((tnode= (obj *)calloc(1, sizeof(obj))) == NULL)
		fatal("Out of space");
	return(tnode);
}

/* newsymlst() creates a new symlst structure */
symlst *
newsymlst(symptr,next)
SYMENT	*symptr;
symlst	*next;
{
	symlst	*tnode;

	if ((tnode= (symlst *)malloc(sizeof(symlst))) == NULL)
		fatal("Out of space");
	tnode->symptr= symptr;
	tnode->next=next;
	return(tnode);
}

/* newscnlst() creates a new scnlst structure */
scnlst *
newscnlst()
{
	scnlst	*tnode;

	if ((tnode= (scnlst *)calloc(1, sizeof(scnlst))) == NULL)
		fatal("Out of space");
	return(tnode);
}

/* newstab() creates a new stab structure */
stab *
newstab(name, value, next)
char	*name;
long	value;
stab	*next;
{
	stab	*tnode;

	if ((tnode= (stab *)malloc(sizeof(stab))) == NULL)
		fatal("Out of space");

	tnode->name= name;
	tnode->absaddr= value;
	tnode->next=next;
	return(tnode);
}

/* newarstab() creates a new arstab structure */
arstab *
newarstab(name, objptr, next)
char	*name;
obj	*objptr;
arstab	*next;
{
	arstab	*tnode;

	if ((tnode= (arstab *)malloc(sizeof(arstab))) == NULL)
		fatal("Out of space");

	tnode->name= name;
	tnode->objptr= objptr;
	tnode->next=next;
	return(tnode);
}

/* newinitinfo() allocates and sets a new initinfo structure */
initinfo *
newinitinfo(objname, next)
char		*objname;
initinfo	*next;
{
	extern int	errno;

	initinfo	*tnode;
	char		*name;

	if ((tnode= (initinfo *)malloc(sizeof(initinfo))) == NULL)
		fatal("Out of space");

	if ((name= tempnam(moddir,"init")) == NULL)
		fatal("Cannot get name for temp file");
	errno= 0; /* reset errno after call to tempnam() */
	tnode->initname= makename(moddir,getbase(name));
	free(name);

	tnode->objname= objname;
	tnode->next=next;
	return(tnode);
}



/* This routine returns a pointer to a null terminated string holding the name
 * of the symbol in the structure, *symptr. Such a string will already exist unless
 * the symbol name is 8 characters long. The routine will copy all names which are
 * less than or equal to 8 characters (i.e, do not have a string table entry)
 * into a new array and adds a terminating null character.
 */
char *
getname(symptr)
SYMENT	*symptr;
{
	char	*str;

	if (symptr->n_zeroes == 0)
		str= symptr->n_nptr;
	else {
		if ((str= malloc(SYMNMLEN+1)) == NULL)
			fatal("Out of space");
		(void)strncpy(str,symptr->n_name,SYMNMLEN);
		str[SYMNMLEN]= '\0';
	}
	return(str);
}


/* getdsuf() gets the suffix for the file definition symbols.  The file
 * definition symbols are used to firm up the links between archive members
 * which have contain symbol resolutions for other archive members.  We want
 * to prevent the  user from interupting the reference chain by defining a
 * symbol which is already defined in the archive. The file definition symbols
 * are used to force the linking of an entire reference chain by placing in each file
 * unresolved references to the file definition symbol of each object which
 * defines a symbol in the current file. That is, if file B defines a symbol
 * used in file A, then A will get an unresolved reference to B's file definition
 * symbol.  It is important that the file definition symbols are given names
 * which will never appear in another shared library or in a user's program.
 * This feat is attempted by giving the file definition symbols the following
 * format:  <filename>.<suffix> where <suffix> is formed thusly;
 * <current date and time>.<pid>. 
 */
char *
getdsuf()
{
	static char  suf[25]; /* must be long enough to hold 2 longs (in hex) + 2 */
	long	date;
	int	pid;

	date= time((long *) 0);
	pid= getpid();
	(void)sprintf(suf,"%x.%x", date, pid);
	return(suf);
}
