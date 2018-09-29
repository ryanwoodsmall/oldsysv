/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/forms/delform.c	1.2"
#include <stdio.h>
#include <errno.h>
#include <sys/types.h>
#include "lp.h"
#include "fs.h"
#include "form.h"

int delform(formname)
char *formname;
{
    void free();
    char *getbasedir();
    register int type;
    char *formpath,*nullstr;
    char savedir[BUFSIZ];

    type = formtype(formname);
    switch (type) {
    case NO_FORM:   errno = ENOENT;
		    return(-1);
    case ALLFORMS: /* remove all forms in LP forms database */
	    	    if (curdir(savedir) != 0)
			return(-1);
		    nullstr = (char *) NULL;
	    	    formpath = getbasedir(nullstr);
	    	    if (Chdir(formpath) != 0) {
			errno = EACCES;
			return(-1);
			}
	    	    if (system("rm -r -f *") < 0) {
			errno = EACCES;
			return(-1);
			}
		    free(formpath);
	    	    if (Chdir(savedir) != 0)
			return(-1);
		    break;
    case REG_FORM:
	    	    if (curdir(savedir) != 0)
			return(-1);
    		    formpath = getbasedir(formname);
	    	    if (Chdir(formpath) != 0) {
			errno = EACCES;
			return(-1);
			}
	    	    if (system("rm -r -f *") < 0) {
			errno = EACCES;
			return(-1);
			}
	    	    if (Chdir(savedir) != 0)
			return(-1);
	    	    if (Rmdir(formpath) != 0) {
			errno = EACCES;
			return(-1);
			}
		    free(formpath);
		    break;
	}
    return(0);
}

int formtype(formname)
char *formname;
{
    void free();
    char *getbasedir();
    char *formpath;

    if (STREQU(NAME_ALL,formname))
	return(ALLFORMS);
    formpath = getbasedir(formname);
    if (Access(formpath,0) != 0) {
    	free(formpath);
	return(NO_FORM);
	}
    free(formpath);
    return(REG_FORM);
}
