/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpforms.c	1.10"
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include "lp.h"
#include "msgs.h"
#include <sys/types.h>
#include <sys/stat.h>
#include "fs.h"

#define WHO_AM_I	I_AM_LPFORMS
#include "oam.h"

#include "form.h"

#define OPT_LIST	"A:f:F:lQ:u:xW:"

#define ALLOW	"allow:"
#define DENY	"deny:"
#define DTMPDIR	"/usr/tmp"
#define TPFX	"tfo"

extern char *sys_errlist[];
extern int sys_nerr;

main(argc,argv)
int argc;
char *argv[];
{
    char *malloc(),*fgets(),*getbasedir(),**getlist();
    void usage(),wralign(),wralert(),rmform(),disable_alert(),
	scan_errors(),getf_errors(),putf_errors(),emptydir_error();
    int notify_spooler();
    extern int is_user_admin();
    long strtol();
    static FORM a_form;
    static FALERT newalert = { (char *)0, -1, -1 };
    static FALERT oldalert = { (char *)0, -1, -1 };
    struct stat statbuf;
    FILE *fp,*alignfp;
    int c,FFLAG,AFLAG,XFLAG,LFLAG;
    char *tmpfile;
    char *rest;
    char *uarg;
    char **ulist;
    int OPTIONS;
    short FRN_ALL,FRN_ANY,FRN_NONE; /* Flags for the form reserved names */
    char *formname;
    extern char *optarg;
    extern int optind, optopt, opterr;
    extern int errno;

    OPTIONS = FFLAG = AFLAG = LFLAG = XFLAG = 0;
    FRN_ALL = FRN_ANY = FRN_NONE = 0;
    uarg = NULL;
    fp = NULL;

    if (!is_user_admin()) {
	LP_ERRMSG(ERROR, E_LP_NOTADM);
	exit(1);
	}

    opterr = 0;
    while ((c = getopt(argc,argv,OPT_LIST)) != EOF) {

	/*
	 * These options take values; "getopt()" passes values
	 * that begin with a dash without checking if they're
	 * options. If a value is missing, we want to complain
	 * about.
	 */
	switch (c) {
	case 'W':
	case 'Q':
		/*
		 * These options take numeric values, which might be
		 * negative. Negative values are handled later, but
		 * here we just screen them.
		 */
		(void)strtol(optarg, &rest, 10);
		if (!rest || !*rest)
			break;
		/*FALLTHROUGH*/
	case 'f':
	case 'F':
	case 'A':
	case 'u':
		{
			char			*cp;

			if (!*optarg) {
				(cp = "-X")[1] = c;
				LP_ERRMSG1 (ERROR, E_LP_NULLARG, cp);
				exit (1);
			}
			if (*optarg == '-') {
				(cp = "-X")[1] = c;
				LP_ERRMSG1 (ERROR, E_LP_OPTARG, cp);
				exit (1);
			}
		}
		break;
	}

	switch (c) {
	case 'x': XFLAG++;
		OPTIONS++;
		break;
	case 'l': LFLAG++;
		OPTIONS++;
		break;
	case 'f': if (FFLAG)
		    LP_ERRMSG1(ERROR,E_LP_2MANY,'f');
		  FFLAG++;
		  formname = strdup(optarg);
		  if (!syn_name(formname)) {
		      LP_ERRMSG1(ERROR,E_LP_NOTNAME,formname);
		      exit(1);
		      }
		  if (STREQU(NAME_ANY,formname))
		      FRN_ANY = 1;
		  if (STREQU(NAME_NONE,formname))
		      FRN_NONE = 1;
		  if (STREQU(NAME_ALL,formname))
		      FRN_ALL = 1;
		break;
	case 'A': OPTIONS++;
		  if (STREQU(NAME_LIST,optarg))
		    AFLAG = 2;
		  else {
		    AFLAG = 1;
		    newalert.shcmd = strdup(optarg);
		    }
		  break;
	case 'F': OPTIONS++;
		  if (fp != NULL)
		     LP_ERRMSG1(WARNING,E_LP_2MANY,'F');
		  if ((fp = fopen(optarg,"r")) == NULL) {
		      LP_ERRMSG1(ERROR,E_FOR_OPEN,optarg);
		      exit(1);
		      }
	    	   if (Stat(optarg,&statbuf) == -1) {
			LP_ERRMSG(ERROR,E_LP_ACCESS);
	    		exit(1);
			}
	      	  if (statbuf.st_size == 0)
		      LP_ERRMSG(WARNING,E_FOR_EMPTYFILE);
		  break;
	case 'Q': OPTIONS++;
		  if (STREQU(optarg, NAME_ANY))
			newalert.Q = 1;
		  else {
			newalert.Q = (int) strtol(optarg,&rest,10);
			if (newalert.Q <= 0) {
				LP_ERRMSG1 (ERROR, E_LP_NEGARG, 'Q');
				exit(1);
			}
			if (rest && *rest) {
				LP_ERRMSG1 (ERROR, E_LP_GARBNMB, 'Q');
				exit (1);
			}
		  }
		  break;
	case 'W': OPTIONS++;
		  if (STREQU(optarg, NAME_ONCE))
			newalert.W = 0;
		  else {
			newalert.W = (int) strtol(optarg,&rest,10);
			if (newalert.W < 0) {
				LP_ERRMSG1 (ERROR, E_LP_NEGARG, 'W');
				exit(1);
			}
			if (rest && *rest) {
				LP_ERRMSG1 (ERROR, E_LP_GARBNMB, 'W');
				exit (1);
			}
		  }
		  break;
	case 'u': OPTIONS++;
		  if (uarg != NULL)
		     LP_ERRMSG1(WARNING,E_LP_2MANY,'u');
		  uarg = strdup(optarg);
		  break;
	default:
		if (optopt == '?') {
			usage ();
			exit (0);
		} else {
			char			*cp;

			(cp = "-X")[1] = optopt;
			if (strchr(OPT_LIST, optopt))
				LP_ERRMSG1 (ERROR, E_LP_OPTARG, cp);
			else
				LP_ERRMSG1 (ERROR, E_LP_OPTION, cp);
			exit (1);
		}
	}
	}
    if (optind < argc) {
	if (cs_strcmp(argv[optind],"-") == 0){ /* stdin token */
	    if (fp != NULL) {
		LP_ERRMSG2(ERROR,E_LP_AMBIG,"-F","-");
	    	exit(1);
	    	}
	    OPTIONS++;
	    fp = stdin;
	    }
	else
	    LP_ERRMSG1(WARNING,E_FOR_EXTRAARG,argv[optind]);
	}

    if (!FFLAG) { /* Flag error if no form chosen */
	LP_ERRMSG(ERROR,E_FOR_FORMNAME);
	exit(1);
	}

    if (!OPTIONS) { /* Flag error if no options chosen */
	LP_ERRMSG(ERROR,E_FOR_NOACT);
	exit(1);
	}

    if ( XFLAG ) {
	if ( XFLAG != OPTIONS) {
	    LP_ERRMSG(ERROR,E_FOR_XONLY);
	    exit(1);
	    }
	else if ( FRN_ANY || FRN_NONE) {
	    LP_ERRMSG(ERROR,E_FOR_ANYNONE);
	    exit(1);
	    }
	rmform(formname);
	exit(0);
	}

    if ( fp != NULL) {
	if ( FRN_ANY || FRN_NONE) {
	    LP_ERRMSG(ERROR,E_FOR_ANYNONE);
	    exit(1);
	    }
	if ((tmpfile = tempnam(DTMPDIR,TPFX)) == NULL) {
	    LP_ERRMSG(ERROR,E_FOR_CTMPFILE);
	    exit(1);
	    }
	copyfile(fp,tmpfile);
	fclose(fp);
	if ((fp = fopen(tmpfile,"r")) == NULL) {
	    fprintf(stderr,"Unable to read tempfile\n");
	    exit(1);
	    }
	if (FRN_ALL) { /* change all forms in LP forms database as requested */
	    while(getform(formname,&a_form,NULL,NULL) == 0) {
	    	if (chform(a_form.name,&a_form,fp) == -1) 
		    scan_errors(a_form.name);
		putform(a_form.name,&a_form,NULL,NULL);
		notify_spooler(S_LOAD_FORM,R_LOAD_FORM,a_form.name);
		rewind(fp);
		}
	    emptydir_error();
	    }
	else { 
	    if (getform(formname,&a_form,NULL,NULL) == -1) { /* Add new form */
		if (errno == ENOENT) {
	            if (addform(formname,&a_form,fp) == -1) 
		    	scan_errors(formname);
		    }
		else
		    getf_errors(formname);
		}
	    else /* Change an existing form */
	    	if (chform(formname,&a_form,fp) == -1) 
		    scan_errors(formname);
	    fclose(fp);
	    if (putform(formname,&a_form,NULL,NULL) == -1)
		putf_errors(formname);
	    notify_spooler(S_LOAD_FORM,R_LOAD_FORM,formname);
	    } 
	fclose(fp);
	Unlink(tmpfile);
	free(tmpfile);
	}

    if ((AFLAG == 1) || (newalert.Q != -1) || (newalert.W != -1)) {
	if (FRN_NONE) {
	    LP_ERRMSG(ERROR,E_FOR_NONE);
	    exit(1);
	    }
	if (*formname == (char)NULL) /* Null formname has same meaning as "any" */
	    FRN_ANY = 1;
	if (FRN_ANY || FRN_ALL) { /* change all requested form alerts */
	    while(getform(NAME_ALL,&a_form,&oldalert,NULL) == 0) {
		if ( (FRN_ANY && (oldalert.shcmd == (char *)NULL)) || FRN_ALL) {
		    if (validate_alert(a_form.name,&newalert)) {
			/* Insure previous old alert values are not propagated thru */
		    	if (FRN_ANY) { 
			    oldalert.Q = -1;
			    oldalert.W = -1;
			    }
	    	    	chfalert(&oldalert,&newalert);
		    	if (putform(a_form.name,NULL,&oldalert,NULL) == -1) {
		    	    if (errno == EINVAL)
				LP_ERRMSG1(ERROR,E_FOR_NOSHCMD,a_form.name);
			    }
		    	else
			    notify_spooler(S_LOAD_FORM,R_LOAD_FORM,a_form.name);
		    	}
		    }
		}
	    emptydir_error();
	    }
	else { 
	    if (getform(formname,NULL,&oldalert,NULL) == -1)
		getf_errors(formname);
	    if (validate_alert(formname,&newalert)) {
	    	chfalert(&oldalert,&newalert);
		if (putform(formname,NULL,&oldalert,NULL) == -1) {
		    if (errno == EINVAL)
		        LP_ERRMSG1(ERROR,E_FOR_NOSHCMD,formname);
			exit(1);
		    }
		notify_spooler(S_LOAD_FORM,R_LOAD_FORM,formname);
		}
	    } 
	}

    if (AFLAG == 2) {
	if ( FRN_ANY || FRN_NONE) {
	    LP_ERRMSG(ERROR,E_FOR_ANYNONE);
	    exit(1);
	    }
	if (FRN_ALL) { /* list all forms in LP forms database */
	    while(getform(formname,&a_form,&oldalert,NULL) == 0) {
		fprintf(stdout,"\nForm name: %s\n",a_form.name);
		wralert(&oldalert);
	    	}
	    emptydir_error();
	    }
	else { /* list requested form */
	    if (getform(formname,NULL,&oldalert,NULL) == -1)
		getf_errors(formname);
	    wralert(&oldalert);
	    }
	}

    if ( uarg != NULL) {
	if (STRNEQU(uarg,ALLOW,strlen(ALLOW))) {
	    uarg += strlen(ALLOW);
	    if ((ulist = getlist(uarg,LP_WS,LP_SEP)) == NULL ) {
		LP_ERRMSG(ERROR,E_FOR_NULLLIST);
		exit(1);
		}
	    if (allow_user_form(ulist,formname) == -1) {
		if (errno == ENOENT) {
		     LP_ERRMSG1(ERROR,E_LP_NOFORM,formname);
	    	     exit(1);
	    	     }
		if (errno == EACCES) {
		     LP_ERRMSG(ERROR,E_FOR_ACCES);
	    	     exit(1);
	    	     }
		}
	    }
	else if (STRNEQU(uarg,DENY,strlen(DENY))) {
	    uarg += strlen(DENY);
	    if ((ulist = getlist(uarg,LP_WS,LP_SEP)) == NULL ) {
		LP_ERRMSG(ERROR,E_FOR_NULLLIST);
		exit(1);
		}
	    if (deny_user_form(ulist,formname) == -1) {
		if (errno == ENOENT) {
		     LP_ERRMSG1(ERROR,E_LP_NOFORM,formname);
	    	     exit(1);
	    	     }
		if (errno == EACCES) {
		     LP_ERRMSG(ERROR,E_FOR_ACCES);
	    	     exit(1);
	    	     }
		}
	    }
	else {
	    LP_ERRMSG(ERROR,E_FOR_UALLOWDENY);
	    exit(1);
	    }
	notify_spooler(S_LOAD_FORM,R_LOAD_FORM,formname);
	freelist(ulist);
	}	
    if (LFLAG) {
	if ( FRN_ANY || FRN_NONE) {
	    LP_ERRMSG(ERROR,E_FOR_ANYNONE);
	    exit(1);
	    }
	if (FRN_ALL) { /* list all forms in LP forms database */
	    while(getform(formname,&a_form,NULL,&alignfp) == 0) {
		fprintf(stdout,"\nForm name: %s\n",a_form.name);
	    	wrform(&a_form,stdout);
		if (alignfp != NULL) wralign(stdout,alignfp);
	    	}
	    emptydir_error();
	    }
	else { /* list requested form */
	    if (getform(formname,&a_form,NULL,&alignfp) == -1)
		getf_errors(formname);
	    wrform(&a_form,stdout);
	    if (alignfp != NULL) wralign(stdout,alignfp);
	    }
	}
    exit(0);
}

/**
 ** usage() - PRINT LPFORMS USAGE MESSAGE
 **/

void usage()
{
#define	P	(void) printf ("%s\n",
#define	X	);
P "usage:"								X
P ""									X
P "  (add or change form)"						X
P "    lpforms -f form-name [options]"					X
P "	[-F path-name | -]			(form definition)"	X
P "	[-u allow:user-list | deny:user-list]	(who's allowed to use)"	X
P "	[-A mail | write | shell-command]	(alert definition)"	X
P "	[-Q threshold]				(# needed for alert)"	X
P "	[-W interval]				(minutes between alerts)"X
P ""									X
P "  (list form)"							X
P "    lpforms -f form-name -l"						X
P ""									X
P "  (delete form)"							X
P "    lpforms -f form-name -x"						X
P ""									X
P "  (examine alerting)"						X
P "    lpforms -f form-name -A list"					X
P ""									X
P "  (stop alerting)"							X
P "    lpforms -f form-name -A quiet		(temporarily)"		X
P "    lpforms -f form-name -A none		(for good)"		X
	return;
}

/**
 ** emptydir_error() - PRINT ERROR MESSAGE FOR NO DIRECTORY ENTRY CONDITION
 **/

void emptydir_error()
{
	if (errno == EFAULT) {
		LP_ERRMSG(WARNING,E_FOR_NOFORMS);
		exit(1);
	}
	return;
}

/**
 ** scan_errors() - PRINT LPFORMS ERRORS IN READING DESCRIBE FILE INPUT
 **/

void scan_errors(name)
char *name;
{
	if (errno == EBADF) {
		switch(lp_errno) {
		case LP_EBADSDN:  LP_ERRMSG1(ERROR,E_FOR_BADSCALE,linenum);
				  break;
		case LP_EBADINT:  LP_ERRMSG1(ERROR,E_FOR_BADINT,linenum);
				  break;
		case LP_EBADNAME: LP_ERRMSG1(ERROR,E_FOR_NOTNAME,linenum);
				  break;
		case LP_EBADARG:  LP_ERRMSG1(ERROR,E_FOR_BADCHSETQUALIFIER,linenum);
				  break;
		case LP_ETRAILIN: LP_ERRMSG1(ERROR,E_FOR_TRAILIN,linenum);
				  break;
		case LP_ENOCMT:   LP_ERRMSG1(ERROR,E_FOR_NOCMT,linenum);
				  break;
		case LP_EBADCTYPE:LP_ERRMSG1(ERROR,E_FOR_NOTCTYPE,linenum);
				  break;
		case LP_ENOALP:   LP_ERRMSG1(ERROR,E_FOR_NOALIGNPTRN,linenum);
				  break;
		case LP_EBADHDR:  LP_ERRMSG1(ERROR,E_FOR_BADHDR,linenum);
				  break;
		default: 	  LP_ERRMSG2(ERROR,
					     E_FOR_UNKNOWN,
				             name,
					     sys_errlist[errno]);		
		}
		}
	else
		LP_ERRMSG2(ERROR,
			   E_FOR_UNKNOWN,
			   name,
			   (errno < sys_nerr? sys_errlist[errno]: "unknown"));
	exit(1);
}

/**
 ** getf_errors() - HANDLE GETFORM ERRORS
 **/

void getf_errors(name)
char *name;
{
	switch(errno) {
	case ENOTDIR:  LP_ERRMSG(ERROR,E_FOR_NOFORMSDIR);
			  break;
	case ENOENT: LP_ERRMSG1(ERROR,E_LP_NOFORM,name);
				  break;
	case EBADF:  LP_ERRMSG1(ERROR,E_FOR_BADF,name);
				  break;
	case EACCES: LP_ERRMSG(ERROR,E_FOR_ACCES);
				  break;
	default:     LP_ERRMSG2(ERROR,
		     		E_FOR_UNKNOWN,
		     		name,
		     		(errno < sys_nerr? sys_errlist[errno]: "unknown"));
	}
	exit(1);
}

/**
 ** putf_errors() - HANDLE PUTFORM ERRORS
 **/

void putf_errors(name)
char *name;
{

	switch(errno) {
	case EINVAL: LP_ERRMSG1(ERROR,E_LP_EMPTY,"given for alignment pattern");
				  break;
	default:     LP_ERRMSG2(ERROR,
		     		E_FOR_UNKNOWN,
		     		name,
		     		(errno < sys_nerr? sys_errlist[errno]: "unknown"));
	}
	exit(1);
}

/**
 ** validate_alert() - PROCESS ALERT FOR QUIET,WRITE, AND MAIL
 **/

static int		validate_alert(form,alertp)
char *form;
FALERT *alertp;
{
	char *login,*tmpcmd;

	if (STREQU(NAME_QUIET,alertp->shcmd)) {
		disable_alert(form);
		return(0);
		}
	if (STREQU(MAIL,alertp->shcmd) || STREQU(WRITE,alertp->shcmd)) {
		if ((login = getname()) == NULL)
			return(0);
		tmpcmd = strdup(alertp->shcmd);
		free(alertp->shcmd);
		if ((alertp->shcmd = malloc(strlen(tmpcmd) + 1 + strlen(login) + 1))
			== NULL)
			return(0);
		strcpy(alertp->shcmd,tmpcmd);
		free(tmpcmd);
		strcat(alertp->shcmd," ");
		strcat(alertp->shcmd,login); 
		}
	return(1);
}

/**
 ** notify_spooler() - NOTIFY SPOOLER OF ACTION ON FORMS DB
 **/

int		notify_spooler (sendmsg,replymsg,form)
int sendmsg,replymsg;
char *form;
{
	char			*msgbuf;

	int			mtype;

	int			size;

	short			status;

	/*
	 * If the attempt to open a message queue to the
	 * Spooler fails, assume it isn't running and just
	 * return--don't say anything, `cause the user may
	 * know. Any other failure deserves an error message.
	 */

	if (mopen() == -1)
		return(-1);

	size = putmessage (NULL, sendmsg,form);
	msgbuf = malloc(size);
	(void)putmessage (msgbuf, sendmsg,form);

	if (msend(msgbuf) == -1) {
		LP_ERRMSG(ERROR,E_LP_MSEND);
		(void)mclose ();
		exit(1);
		}
 
	if (mrecv(msgbuf, size) == -1) {
		LP_ERRMSG(ERROR,E_LP_MRECV);
		(void)mclose ();
		exit(1);
		}

	mtype = getmessage(msgbuf, replymsg, &status);
	free(msgbuf);
	(void)mclose ();
	if (mtype != replymsg) {
		LP_ERRMSG(ERROR,E_LP_BADREPLY);
		exit (1);
		}

	if (status == MOK)
		return(MOK);

	if (sendmsg == S_LOAD_FORM)
		switch(status) {
		case MNOSPACE:	LP_ERRMSG(ERROR,E_FOR_NOSPACE);
				break;
		case MNOPERM:	LP_ERRMSG(ERROR,E_LP_NOTADM);
				break;
		/* The following two error conditions should have
		   already been trapped, so treat them as bad status
		   should they occur.				*/
		case MNODEST:
		case MERRDEST:
		default: 	LP_ERRMSG1(ERROR,E_LP_BADSTATUS,status);
		}

	if (sendmsg == S_UNLOAD_FORM)
		switch(status) {
		case MBUSY: 	LP_ERRMSG1(ERROR,E_FOR_FORMBUSY,form);
			    	break;
		case MNODEST:	return(MNODEST);
				break;
		case MNOPERM:	LP_ERRMSG(ERROR,E_LP_NOTADM);
				break;
		default:	LP_ERRMSG(ERROR,E_LP_BADSTATUS);
		}
	exit(1);
}

/**
 ** rmform(name) - REMOVE FORM if possible
 **/

void		rmform(name)
char *name;
{
	switch(notify_spooler(S_UNLOAD_FORM,R_UNLOAD_FORM,name)) {
	case -1: 	if (anyrequests()) {
	    			LP_ERRMSG(ERROR,E_FOR_MOPENREQX);
	    			exit(1);
	    			}

	case MNODEST: 	if (delform(name) == -1) {
				if (errno == ENOENT)
					LP_ERRMSG1(ERROR,E_LP_NOFORM,name);
				else
					LP_ERRMSG2(ERROR,
		     			E_FOR_UNKNOWN,
		     			name,
		     			(errno < sys_nerr? sys_errlist[errno]:
					 "unknown"));
				exit(1);
				}
				break;

	case MOK:	if (delform(name) == -1) {
	    			LP_ERRMSG(ERROR,E_FOR_DELSTRANGE);
	    			exit(1);
	    			}
	}
}

/**
 ** disable_alert() - HAVE SPOOLER DISABLE ALERT ON FORM
 **/

static void		disable_alert (name)
	char			*name;
{
	char			*msgbuf;

	int			mtype;

	int			size;

	short			status;

	/*
	 * If the attempt to open a message queue to the
	 * Spooler fails, assume it isn't running and just
	 * return--don't say anything, `cause the user may
	 * know. Any other failure deserves an error message.
	 */

	if (mopen() == -1)
		return;

	size = putmessage (NULL, S_QUIET_ALERT,name,QA_FORM);
	msgbuf = malloc(size);
	(void)putmessage (msgbuf, S_QUIET_ALERT,name,QA_FORM);

	if (msend(msgbuf) == -1) {
		LP_ERRMSG(ERROR,E_LP_MSEND);
		(void)mclose ();
		exit(1);
		}	
 
	if (mrecv(msgbuf, size) == -1) {
		LP_ERRMSG(ERROR,E_LP_MRECV);
		(void)mclose ();
		exit(1);
		}
 
	mtype = getmessage(msgbuf, R_QUIET_ALERT, &status);
	free(msgbuf);
	(void)mclose ();
	if (mtype != R_QUIET_ALERT) {
		LP_ERRMSG(ERROR,E_LP_BADREPLY);
		exit (1);
		}

	switch (status) {
	case MOK:
		break;

	case MNODEST:	/* not quite, but not a lie either */
	case MERRDEST:
		LP_ERRMSG1 (WARNING, E_LP_NOQUIET, name);
		break;

	case MNOPERM:	/* taken care of up front */
	default:
		LP_ERRMSG1 (ERROR, E_LP_BADSTATUS, status);
		exit (1);
		/*NOTREACHED*/
	}

	return;

}

/*
 * copyfile(stream, name) -- copy stream to file "name"
 */

copyfile(stream, name)
FILE *stream;
char *name;
{
    FILE *ostream;
    int i;
    char buf[BUFSIZ];

    if((ostream = fopen(name, "w")) == NULL) {
	fprintf(stderr, "cannot create temp file %s", name);
	return;
    }

    Chmod(name, 0600);
    while((i = fread(buf, sizeof(char), BUFSIZ, stream)) > 0) {
	fwrite(buf, sizeof(char), i, ostream);
	if(feof(stream)) break;
    }

    fclose(ostream);
}
