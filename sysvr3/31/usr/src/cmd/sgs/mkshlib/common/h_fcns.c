/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/h_fcns.c	1.7"

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

/* newstab() creates a new stab structure
 *
 * If a symbol by a given name is already there, then look
 * at the previous version:
 * a) if one's an external (and the other one isn't),
 *    keep the information on the external.
 * b) if both versions are external, complain.
 * c) There shouldn't be a third case.
 *
 * Note that *WE* have to allocate space to store the string.
 */
stab *
newstab(name, value, sclass,  next)
char	*name;
long	value;
long	sclass;
stab	*next;
{
	stab	**ptr;
	stab	*tnode;

	for (ptr = &next; *ptr != NULL; ptr = &(ptr[0]->next))
		if (strcmp(ptr[0]->name, name) == 0)	/* A match! */
		{
			if (sclass == C_EXT && ptr[0]->sclass == C_STAT)
			{
				tnode = ptr[0];
				break;
			}
			if (sclass == C_STAT)
				return(next);	/* nothing to do */
			if (sclass == C_EXT && ptr[0]->sclass == C_EXT)
			{
				warn("Symbol %s appears in target more than once - ignoring all but first occurence", name);
				return(next);
			}
		}
	if (*ptr == NULL)
	{
		if ((tnode= (stab *)malloc(sizeof(stab))) == NULL)
			fatal("Out of space");
		tnode->name = stralloc(name);
		tnode->next=NULL;
		*ptr = tnode;
	}
	tnode->absaddr= value;
	tnode->sclass = sclass;
	return(next);
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


/* getdpre() gets the prefix for the file definition symbols.  The file
 * definition symbols are used to firm up the links between archive members
 * which contain symbol resolutions for other archive members.  We want
 * to prevent the  user from interupting the reference chain by defining a
 * symbol which is already defined in the archive. The file definition symbols
 * are used to force the linking of an entire reference chain by placing in each file
 * unresolved references to the file definition symbol of each object which
 * defines a symbol in the current file. That is, if file B defines a symbol
 * used in file A, then A will get an unresolved reference to B's file definition
 * symbol.  It is important that the file definition symbols are given names
 * which will never appear in another shared library or in a user's program.
 * This feat is attempted by giving the file definition symbols the following
 * format:  
 *		<library path>[<object file name>]
 *	Example:
 *		/shlib/libc_s[t.o]
 *	target path name, the internal archive member name, and object file name
 */
char *
getdpre()
{
	char  *pre;

	if ((pre = malloc(strlen(trgpath)+3)) == NULL)
		fatal("Out of space");
	(void)sprintf(pre,"%s", trgpath);
	return(pre);
}

/*
 * Return the name of that we want the temporary object
 * file that contains the lib definition symbol to be.
 *
 * We use the name ".../Xxxx" (where "xxx" is the last component
 * of the target name) as an object file that will contain the
 * ".lib" section - this object file is the last member of the
 * host archive we'll be creating, and we need to be able to
 * predict the name of that member.
 *
 * So, for example, the target name "/shlib/libc_s" produces
 * a temp file of ".../Xlibc_s". (The "X" keeps us a little
 * safer, in case the target is in the current directory and
 * someone does an "ar x" later on. Embarrassing.)
 */

char *getdefname()
{
	char *ret;
	int i;
	char *base;
	int len; 

	base = getbase(trgpath == NULL ? trgname : trgpath);

	len = strlen(base) + strlen(moddir)
		+ 2 + 1;	/* for "/X" + '\0' */
	if ((ret = malloc(len)) == NULL)
		fatal("Cannot allocate space for temporary file");
	(void) sprintf(ret, "%s/X%s", moddir, base);

/*
 * Check for a problem - since we will be putting this object
 * in the sme directory and archive as the objects, we need
 * to look for name conflicts.
 */
	base = getbase(ret);
	for (i = 0; i < numobjs; i++)
	{
		char *p = getbase(trgobjects[i]);
		if (strcmp(p, base) == 0)
			fatal("Object file cannot be named %s - please rename it",
				p);
	}

	return(ret);
}
