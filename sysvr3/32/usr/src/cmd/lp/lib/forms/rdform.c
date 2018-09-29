/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/forms/rdform.c	1.6"
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include "lp.h"
#include "form.h"

int linenum;

int addform(form_name,formp,usrfp)
char *form_name;
FORM *formp;
FILE *usrfp; 
{
    char *malloc();
    FILE *formfopen(),*fp;
    void free();
    int setopt[NFSPEC];
    int i;


    	if (scform(form_name,formp,usrfp,0,setopt) == -1)
	    return(-1);
	/* When a form is added an empty deny file is created as a default */
	fp = formfopen(form_name,DENYFILE,"w",1);
	close_lpfile(fp);
	for ( i = 0; i < NFSPEC; i++)
	    if (setopt[i] == 0) {
		switch (i) {
		case FO_PLEN: 	formp->plen.val = DPLEN;
				formp->plen.sc = 0;
				break;
		case FO_PWID: 	formp->pwid.val = DPWIDTH;
				formp->pwid.sc = 0;
				break;
		case FO_NP: 	formp->np = DNP;
				break;
		case FO_LPI: 	formp->lpi.val = DLPITCH;
				formp->lpi.sc = 0;
				break;
		case FO_CPI: 	formp->cpi.val = DCPITCH;
				formp->cpi.sc = 0;
				break;
		case FO_CHSET: 	formp->chset = strdup(DCHSET);
				formp->mandatory = 0;
		  		break;
		case FO_RCOLOR: formp->rcolor = strdup(DRCOLOR);
				break;
		case FO_CMT: 	formp->comment = (char *)NULL;
				break;
		case FO_ALIGN: 	formp->conttype = (char *)NULL;
				break;
		}
		}
	return(0);
}

int chform(form_name,cform,usrfp)
char *form_name;
FORM *cform;
FILE *usrfp; /* *usrfp is NULL iff form_name is a registered lp form */
{
    int setopt[NFSPEC];

    return(scform(form_name,cform,usrfp,0,setopt));
}
	
int chfalert(old,new)
FALERT *new,*old;
{
    char *p;

    if (new->shcmd != NULL) {
	if (old->shcmd != NULL) free(old->shcmd);
	old->shcmd = strdup(new->shcmd);
	}
    if (new->Q != -1)
	old->Q = new->Q;
    else {
	/* Iff both alerts not specified(new alert!) Install alert default */
	if (old->Q == -1) old->Q = 1;
	}
    if (new->W != -1)
	old->W = new->W;
    else {
	/* Iff both alerts not specified(new alert!) Install alert default */
	if (old->W == -1) old->W = 0;
	}
    return(0);
}

int nscan(s,var)
char *s;
SCALED *var;
{
    double strtod();
    char *ptr;

    if ( ( var->val = (float)strtod(s,&ptr)) <= 0) {
	errno = EBADF;
	lp_errno = LP_EBADSDN;
	return(-1);
	}

    if (ptr) {
	switch(*ptr) {
	case 0:
	case ' ':
	case '\n': var->sc = 0;
		   break;
	case 'c':
	case 'i':  var->sc = *ptr;
		   break;
	default:   errno = EBADF;
		   lp_errno = LP_EBADSDN;
		   return(-1);
	}
	}

    return(0);
}

int stripnewline(line)
char *line;
{
    char *strchr();
    register char *p;
    
    p = strchr(line,0);
    while ( isspace(*--p));
    p[1] = 0;
}

char *getbasedir(formname) 
char *formname;
{
    char *path;
    if (!Lp_A_Forms) {
	getadminpaths(LPUSER);
	if (!Lp_A_Forms)
	    return(0);
	}
    path = makepath(Lp_A_Forms,formname, (char *)0);
    return(path); 
}

char *mkcmd_usr(cmd)
char *cmd;
{
    char *cmdstr,*login;
    
    if ((login = cuserid((char *)NULL)) == NULL)
	return(NULL);
    if ((cmdstr = malloc(strlen(cmd) + 1 + strlen(login) + 1)) == NULL)
	return(NULL);
    strcpy(cmdstr,cmd);
    strcat(cmdstr," ");
    strcat(cmdstr,login);
    return(cmdstr);
}

char *mkformdir(formname)
char *formname;
{
    char *getbasedir(),*makepath();
    char *formdir;
    int oldmask;
    char *path;

	formdir = getbasedir((char *)NULL);
	/* Create forms directory if it doesn't exist */
	if ( Access(formdir)!= 0)
	    mkdir_lpdir(formdir,0775);
	/* Create formname directory if it doesn't exist */
	path = makepath(formdir,formname,(char *)0);
	if ( Access(path)!= 0)
	    mkdir_lpdir(path,0775);
	free(formdir);
	return(path);
}

int scform(form_nm,formp,fp,LP_FORM,opttag)
char *form_nm;
FORM *formp;
FILE *fp;
short LP_FORM;
int *opttag;
{
    char *getbasedir();
    char *retval;
    char line[BUFSIZ + 1];
    char *lineptr;
    char *p;
    char temp[15];	/* temporary string */
    long begblk,endblk,blklen;
    int i,j,n;
    int CMTEND,MATCH;

    linenum = 0;
    for ( i = 0; i < NFSPEC; opttag[i] = 0, i++);
    retval = fgets(line,BUFSIZ,fp);
    linenum++;
    while ( retval != NULL ) {
	CMTEND = MATCH = 0;
	for ( i = 0; i < NFSPEC && !MATCH; i++) { 
	    n = strlen(fspec[i]);
	    if ( (j = cs_strncmp(fspec[i],retval,n)) == 0)
		MATCH = 1;
	    }
	i--;

	if (!MATCH) {/* Unidentified input line */
	    errno = EBADF;
	    lp_errno = LP_EBADHDR; 
	    return(-1);
	    }
	opttag[i] = 1;
	lineptr = line;
	lineptr += n;
	while (*lineptr == ' ' || *lineptr == '\t')
	    lineptr++;
	switch (i) {
	    case FO_PLEN:   if (nscan(lineptr,&(formp->plen)) == -1)
				return(-1);
		    	    break;
	    case FO_PWID:   if (nscan(lineptr,&(formp->pwid)) == -1)
				return(-1);
		    	    break;
	    case FO_LPI:    if (nscan(lineptr,&(formp->lpi)) == -1)
				return(-1);
		    	    break;
	    case FO_CPI:    stripnewline(lineptr);
			    if (STREQU(lineptr, NAME_PICA)) {
				formp->cpi.val = 10;
				formp->cpi.sc = 0;
			    } else if (STREQU(lineptr, NAME_ELITE)) {
				formp->cpi.val = 12;
				formp->cpi.sc = 0;
			    } else if (STREQU(lineptr, NAME_COMPRESSED)) {
				formp->cpi.val = 9999;
				formp->cpi.sc = 0;
			    } else if (nscan(lineptr,&(formp->cpi)) == -1)
				return(-1);
		    	    break;
	    case FO_NP:     if ( ( formp->np = (int)strtol(lineptr,&p,10)) <= 0) {
				errno = EBADF;
				lp_errno = LP_EBADINT;
				return(-1);
				}
			    if (*p!= '\n') {
				errno = EBADF;
				lp_errno = LP_EBADINT;
				return(-1);
				}
		    	    break;
    
	    case FO_CHSET:  stripnewline(lineptr);
			    p = strchr(lineptr,',');
			    formp->mandatory = 0;
			    if (p) {
				do
				    *(p)++ = NULL;
				while (*p && isspace(*p));

		    	    	if (CS_STREQU(MANSTR,p))
				    formp->mandatory = 1;
				else {
				    errno = EBADF;
				    lp_errno = LP_EBADARG;
				    return(-1);
				    } 
				}
			    if (!syn_name(lineptr)) {
				errno = EBADF;
				lp_errno = LP_EBADNAME;
				return(-1);
				}
		            formp->chset = strdup(lineptr);		
		    	    break;
	    case FO_RCOLOR: stripnewline(lineptr);
			    if (!syn_name(lineptr)) {
				errno = EBADF;
				lp_errno = LP_EBADNAME;
				return(-1);
				}
		            formp->rcolor = strdup(lineptr);		
		    	    break;
	    case FO_CMT:    endblk = begblk = ftell(fp);
			    if (*lineptr != '\n') {
				errno = EBADF;
				lp_errno = LP_ETRAILIN;
				return(-1);
				}
    	    	    	    while ( (retval != NULL) && !CMTEND ) {
				endblk = ftell(fp);
    		    		retval = fgets(line,BUFSIZ,fp);
				linenum++;
				for ( j = 0; j < NFSPEC && !CMTEND; j++) { 
	    		    	n = strlen(fspec[j]);
	    		    	if ( cs_strncmp(fspec[j],retval,n) == 0)
			            CMTEND = 1;
	    		        }
			    }
			    if (!CMTEND)
			    	endblk = ftell(fp);
		    	    /* Store comment block */
		    	    blklen = endblk - begblk;
	    		    fseek(fp,begblk,0);
		    	    if (blklen == 0) {
	    			formp->comment = NULL;
				errno = EBADF;
				lp_errno = LP_ENOCMT;
				return(-1);
				}
		    	    else {
				if ((formp->comment = malloc(blklen + 1)) == NULL)
			    	    return(-1);
	    		    	fread(formp->comment,1,(int) blklen,fp);
			    	formp->comment[(int) blklen] = '\0';
	    			}
			    linenum--;
		    	    break;
	    case FO_ALIGN:  stripnewline(lineptr);
			    if (!syn_type(lineptr)) {
				errno = EBADF;
				lp_errno = LP_EBADCTYPE;
				return(-1);
				}
		            formp->conttype = (*lineptr) ? strdup(lineptr) :
						strdup(DCONTYP);
			    if (formp->conttype == (char *)NULL)
				return(-1);
			    if (!LP_FORM) {
				begblk = ftell(fp);
				(void) fseek(fp,(long) 0,2);
				if ((ftell(fp) - begblk) > 0)
				    fseek(fp,begblk,0);
				else {
				    errno = EBADF;
				    lp_errno = LP_ENOALP;
				    return(-1);
				    }
			        if (putform(form_nm,NULL,NULL,&fp) == -1) {
				    formp->conttype = (char *)NULL;
				    return(-1);
				    }
				}
		    	    retval = NULL;
		    	    break;	
			
	    } /* End switch for tagged lines */


	if (retval != NULL) {
	    retval = fgets(line,BUFSIZ,fp);
	    linenum++;
	    }
	} /* End while for identifying input lines */
    return(0);
}
