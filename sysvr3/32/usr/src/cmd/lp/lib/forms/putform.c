/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/forms/putform.c	1.4"
#include <stdio.h>
#include <errno.h>
#include "lp.h"
#include "form.h"

extern char **environ;

extern void free();

int putform(form,formp,alertp,alignfilep)
char *form;
FORM *formp;
FALERT *alertp;
FILE **alignfilep;
{
    FILE *formfopen();
    FILE *fp;
    int wralign();
    char *getbasedir();
    char *path;
    int size;

    if (STREQU(NAME_ALL,form)) {
	errno = EINVAL;
	return(-1);
	}

    if (formp != NULL ) {
	if ((fp = formfopen(form,DESCRIBE,"w",1)) == NULL)
	    return(-1);
	else {
    	    wrform(formp,fp);
    	    close_lpfile(fp);
	    }
	if (formp->comment != NULL) {
	    if ((fp = formfopen(form,COMMENT,"w",0)) == NULL)
	    	return(-1);
	    fprintf(fp,"%s",formp->comment);
	    close_lpfile(fp);
	    }
	}

    if (alertp != NULL ) {
	if ((path = getbasedir((char *)NULL)) == NULL)
	    return(-1);
	if (putalert(path,form,alertp) == -1)
	    return(-1);
	free(path);
	}

    if (alignfilep != NULL ) {
	if ((fp = formfopen(form,ALIGN_PTRN,"w",1)) == NULL)
	    return(-1);
	else {
	    size = wralign(fp,*alignfilep);
	    close_lpfile(fp);
	    if (!size) {
		Unlink(makepath(getbasedir(form),ALIGN_PTRN,(char *) 0));
		Unlink(makepath(getbasedir(form),(char *) 0));
		errno = EINVAL;
		lp_errno = LP_ENOALP;
		return(-1);
		}
	    }
	}
    return(0);
}

FILE *formfopen(formname,file,type,dircreat)
char *formname;
char *file;
char *type;
register int dircreat;
{
    char *malloc(),*makepath(),*mkformdir();
    FILE *fp;
    char *filepath,*pathptr;

	/* If positive form directory is crteated if it doesn't exist */
	if (dircreat)
	    pathptr = mkformdir(formname);
	else {
    	    if ((pathptr = getbasedir(formname)) == NULL)
	    	return(NULL);
	    if (access(pathptr, 00) == -1) {
		if (errno == ENOENT) errno = ENOTDIR;
		return(NULL);
		}
	    }
	filepath = makepath(pathptr,file,(char *)0);
    	if (filepath == NULL) {
	    free(filepath);
	    free(pathptr);
	    return(NULL);
	    }
	free(pathptr);
    	if ( (fp = open_lpfile(filepath,type, 
	     (STREQU(file,ALIGN_PTRN) ) ? MODE_NOREAD : MODE_READ))
		== NULL) {
	    free(filepath);
	    return(NULL);
	    }
	free(filepath);
	return(fp);
}
