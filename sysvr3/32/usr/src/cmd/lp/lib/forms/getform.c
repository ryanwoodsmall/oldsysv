/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/forms/getform.c	1.5"
#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include "lp.h"
#include "fs.h"
#include "form.h"

long next_read=0;

int getform(form,formp,alertp,alignfilep)
char *form;
FORM *formp;
FALERT *alertp;
FILE **alignfilep;
{
    FILE *formfopen();
    FILE *fp;
    DIR *dirp;
    FALERT *returned,*getalert();
    struct dirent *nextent;
    struct stat statbuf;
    char *pathptr,*filepath,*retval;
    char *formdir,*name,*lineptr;
    char line[BUFSIZ + 1];
    int opttag[NFSPEC];
    int fd,len;
    void free();
    char *malloc(),*makepath(),*getbasedir();

    if (STREQU(NAME_ALL,form)) {
	if (formp == NULL) {
	    errno = EINVAL;
	    lp_errno = LP_ENULLPTR;
	    return(-1);
	    }
	formdir = getbasedir((char *)0);

	if ((dirp = Opendir(formdir)) == NULL) 
		return(-1);	/* Ooops, we failed! */

	free(formdir);
	if (!next_read) {
	    nextent = Readdir(dirp);
	    }
	else {
	    Seekdir(dirp,next_read);
	    nextent = Readdir(dirp);
	    } 
	/* Skip over UNIX files "." and ".." */
	while (STREQU(nextent->d_name,".") || STREQU(nextent->d_name,"..")) {
	    nextent = Readdir(dirp);
	    }

	if (nextent == NULL) {
	   if (!next_read)
		errno = EFAULT;
	   Closedir(dirp);
	   return(-1);
	}

	formp->name = strdup(nextent->d_name);
	name = strdup(nextent->d_name);
	next_read = Telldir(dirp);
	Closedir(dirp);
	}
    else {
	name = strdup(form);
	}



    if (formp != NULL ) {
	if ((fp = formfopen(name,DESCRIBE,"r",0)) == NULL) {
	    if (errno == ENOENT) errno = EBADF;
	    if (errno == ENOTDIR) errno = ENOENT;
	    return(-1);
	    }
	/*
	Null out comment and alignment pattern content type pointers in
	case the form does not have a comment or alignment pattern. 
	This is necesary since the comment and alignment pattern are stored
	in seperate files from the describe file and formp is generally used
	only in conjunction with describe file activities. The comment and
	content pointers are necessary for adding a form primarily.
	*/
	formp->comment = formp->conttype = (char *)NULL;
    	if (scform(name,formp,fp,1,opttag) == -1)
	   return(-1);
    	close_lpfile(fp);
    	if ((pathptr = getbasedir(name)) == NULL) {
	    return(-1);
	    }
	if ((filepath = makepath(pathptr,COMMENT,(char *)0)) == NULL) {
	    free(pathptr);
	    return(-1);
	    }
	free(pathptr);
	if ( (fd = Open(filepath,O_RDONLY)) != -1) {
	    if (Fstat(fd,&statbuf) == -1)
	    	return(-1);
	    if (statbuf.st_size) {
	        formp->comment = malloc(statbuf.st_size + 1);
	        Read(fd,formp->comment,statbuf.st_size);
	        formp->comment[statbuf.st_size] = (char)NULL;
		}
	    Close(fd);	
	    }

	}

    if (alertp != NULL ) {
	if ((pathptr = getbasedir((char *)NULL)) == NULL)
	    return(-1);
	if ((returned = getalert(pathptr,name)) == NULL) {
	    if (errno == ENOENT) {
	    	alertp->shcmd = (char *)NULL;
		alertp->Q = alertp->W = -1;
		}
	    else {
		if (errno == ENOTDIR) /* If form does not exist have ENOENT set */
		    errno = ENOENT;
		return(-1);
		}
	    }
	else {
	    alertp->shcmd = strdup(returned->shcmd);
	    alertp->Q = returned->Q;
	    alertp->W = returned->W;
	    }
	}

    if (alignfilep != NULL )
	*alignfilep = formfopen(name,ALIGN_PTRN,"r",0);
    free(name);
    return(0);
}
