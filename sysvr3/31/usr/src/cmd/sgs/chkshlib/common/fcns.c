/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)chkshlib:common/fcns.c	1.4"

#include <stdio.h>
#include <signal.h>
#include "filehdr.h"
#include "ldfcn.h"
#include "string.h"
#include "reloc.h"
#include "syms.h"
#include "scnhdr.h"
#include "chkshlib.h"

long
hash(name)
char *name;
{
	char	*tstr;
	long	i;
	long	hashval=0;

	for (tstr=name, i=1; *tstr != '\0'; tstr++, i++)
		hashval+= (*tstr) * i;

	return(hashval % TABSIZ);
}

/* stralloc() returns a pointer to a copy of str */
char *
stralloc(str)
char	*str;
{
	char	*tptr;

	tptr = chk_malloc((unsigned)strlen(str)+1);

	(void)strcpy(tptr, str);
	return(tptr);
}


/* filedata_alloc() returns a pointer to a file_data stucture */
file_data *
filedata_alloc()
{

	file_data	*fdptr;

	fdptr = (file_data *)chk_malloc((unsigned)sizeof (file_data));
	fdptr->f_targ = NULL;

	return(fdptr);
}

/* targname_alloc() returns a pointer to a targ_name stucture */
targ_name *
targname_alloc()
{

	targ_name	*tptr;

	tptr = (targ_name *)chk_malloc((unsigned)sizeof (targ_name));

	return(tptr);
}

/* chk_malloc is chkshlib's malloc routine */
char *
chk_malloc (value)
unsigned value;
{
	char *sptr;

	if ((sptr = malloc(value)) == NULL)
		printerr2("Out of space");
	return(sptr);
}

/* chk2_malloc is special malloc routine called from function store_lib */
/* when checking .lib section, if data is corrrupt, malloc fail with big number */
char *
chk2_malloc (value)
unsigned value;
{
	char *sptr;

	if ((sptr = malloc(value)) == NULL)
		printerr2("Out of space or Corrupt .lib section");
	return(sptr);
}
