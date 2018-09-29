/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ipc:nnlist.c	1.2"
#include	<stdio.h>
#include	<filehdr.h>
#include	<syms.h>
#include	<ldfcn.h>

/*	We need the following because of the insanity in syms.h
**	You cannot include syms.h and nlist.h and use the n_name
**	field of both.  Nice going guys.
*/


typedef struct nlist
{
	char		*nl_name;	/* symbol name */
	long		nl_value;	/* value of symbol */
	short		nl_scnum;	/* section number */
	unsigned short	nl_type;	/* type and derived type */
	char		nl_sclass;	/* storage class */
	char		nl_numaux;	/* number of aux. entries */
} nlist_t;

/*	This is the same as the old nlist function
**	except that the fields n_scnum, n_type, and
**	n_sclass may be set non-zero in the nlist
**	structure.  In this case, the appropriate
**	fields must match as well as the name.
*/


nlist(fname, nlp)
register char		*fname;
register nlist_t	*nlp;
{
	SYMENT			syment;
	int			nlcnt;
	register nlist_t	*tnlp;
	register LDFILE		*ldptr;
	register int		i;

	/*	Set the value fields of all of the nlist
	**	structures to zero and, at the same time
	**	count how many we have.
	*/

	nlcnt = 0;
	tnlp = nlp;

	while(tnlp->nl_name  &&  *tnlp->nl_name != '\0'){
		tnlp++->nl_value = 0;
		nlcnt++;
	}

	/*	Try to open the file whose symbol table
	**	we want to read.
	*/

	if((ldptr = ldopen(fname, NULL)) == NULL)
		return(-1);
	
	/*	Seek to the start of the symbol table.
	**	This will fail if the file was stripped.
	*/

	if(ldtbseek(ldptr) == FAILURE){
		ldclose(ldptr);
		return(-1);
	}

	/*	Read each symbol table entry in the file.
	**	For each one, see if it matches any of
	**	the entries in the supplied namelist
	**	array.  When all namelist entries have
	**	been found, quite.
	*/

	i = 0;

	while(ldtbread(ldptr, i++, &syment) == SUCCESS){
		if(match(ldptr, nlp, &syment)){
			if(--nlcnt == 0)
				break;
		}
	}

	/*	Close the file.  Return indication of
	**	whether we found all of the files or not.
	*/

	ldclose(ldptr);
	if(nlcnt == 0)
		return(0);
	else
		return(-1);
}

/*	See if a symbol table entry matches any of the
**	namelist entries in the supplied list.
*/


match(ldptr, nlp, sp)
register LDFILE		*ldptr;
register nlist_t	*nlp;
register SYMENT		*sp;
{
	register char	*snp;
	char		*ldgetname();

	/*	Get the correct name for the symbol table
	**	entry.  This may be in a funny place (the
	**	string table) because of flexnames.
	*/

	if((snp = ldgetname(ldptr, sp)) == NULL)
		return(0);
	
	/*	Loop through all of the namelist entries
	**	in the array.
	*/

	while(nlp->nl_name  &&  *nlp->nl_name != '\0'){

		/*	If the names don't match, forget
		**	this namelist entry.
		*/

		if(strcmp(nlp->nl_name, snp) != 0){
			nlp++;
			continue;
		}

		/*	If a particular section number was
		**	specified and they don't match,
		**	then forget this entry.
		*/

		if(nlp->nl_scnum  &&  nlp->nl_scnum != sp->n_scnum){
			nlp++;
			continue;
		}

		/*	If a particular type was
		**	specified and they don't match,
		**	then forget this entry.
		*/

		if(nlp->nl_type  &&  nlp->nl_type != sp->n_type){
			nlp++;
			continue;
		}

		/*	If a particular storage class was
		**	specified and they don't match,
		**	then forget this entry.
		*/

		if(nlp->nl_sclass  &&  nlp->nl_sclass != sp->n_sclass){
			nlp++;
			continue;
		}
		
		/*	We have found a match.  Copy all of the
		**	interesting fields and return.
		*/

		nlp->nl_value = sp->n_value;
		nlp->nl_scnum = sp->n_scnum;
		nlp->nl_type = sp->n_type;
		nlp->nl_sclass = sp->n_sclass;
		nlp->nl_numaux = sp->n_numaux;
		return(1);
	}
	return(0);
}
