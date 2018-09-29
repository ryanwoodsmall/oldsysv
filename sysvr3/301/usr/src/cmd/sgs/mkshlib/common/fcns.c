/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkshlib:common/fcns.c	1.3"

#include <stdio.h>
#include <signal.h>
#include <varargs.h>
#include "paths.h"
#include "filehdr.h"
#include "ldfcn.h"
#include "syms.h"
#include "reloc.h"
#include "scnhdr.h"
#include "shlib.h"
#include "hst.h"
#include "trg.h"

#define MAXARGS	500	/* maxinum number of args to a call to execute() */

/*
 * getpref() gets the prefix of the mkshlib command and returns a pointer to a string
 * holding the prefix
 */
char *
getpref(cmd)
char	*cmd;
{
	char	*pref;
	char	*pptr;
	int	cmdlen;

	pptr= getbase(cmd);
	cmdlen= strlen(pptr);
	if (cmdlen < 7 || strncmp(pptr+(cmdlen-7),"mkshlib",7) != 0)
		fatal("command name must end in \"mkshlib\"");
	pref= stralloc(pptr);
	pref[cmdlen-7]='\0';
	return(pref);
}


/* getbase() gets the base filename of the supplied pathname */
char *
getbase(fnam)
char	*fnam;
{
	char	*pptr;

	if ((pptr= strrchr(fnam,'/')) == NULL)
		pptr= fnam;
	else
		pptr++;
	return(pptr);
}

/* concat() allocates space to concatenate pref and name and returns a pointer
 * to the concatenated string
 */
char *
concat(pref, name)
char	*pref,
	*name;
{
	char	*p;
	if ((p = malloc((unsigned)(strlen(prefix) + strlen(name) + 1))) == NULL)
		fatal("Out of space");
	(void)strcpy(p, pref);
	(void)strcat(p, name);
	return(p);
}

/* Hash function for all hash tables */
long
hash(name, tabsize)
char *name;
int	tabsize;
{
	char	*tstr;
	long	i;
	unsigned long	hashval=0;

	for (tstr=name, i=1; *tstr != '\0'; tstr++, i++)
		hashval+= (*tstr) * i;

	return(hashval % tabsize);
}


/* Copy size bytes from the object file, ldfil, to the file, ffil. */
int
copy(ldfil, ffil,  size )
	LDFILE	*ldfil;
	FILE	*ffil;
	register long	size;
{
	char	buffer[BUFSIZ];
	int	n;

	while (size && (n = FREAD(buffer,sizeof(char),BUFSIZ,ldfil)) > 0) {
		if (size/n == 0)
			n= size;
		size-= n;
		if (fwrite(buffer,sizeof(char),n,ffil) != n)
			return(FAILURE);
	}
	if (size != 0)
		return(FAILURE);
	return(SUCCESS);
}


/* stralloc() returns a pointer to a copy of str */
char *
stralloc(str)
char	*str;
{
	char	*tptr;

	if ((tptr= malloc((unsigned)(strlen(str)+1))) == NULL)
		fatal("Out of space");

	(void)strcpy(tptr, str);
	return(tptr);
}


/* This function gets the names of all necessary temporary files */
void
mktmps()
{
	extern int	errno;
	if (maketarg == TRUE) {
		ifil1name=tempnam(TMPDIR,"trg5"); /* name of ifile for initial 
						   * load */
		ifil2name=tempnam(TMPDIR,"trg1"); /* name of ifile for final load */
		tpltnam=  tempnam(TMPDIR,"trg2"); /* name of intermediate partially
						   * loaded target */
		pltname=  tempnam(TMPDIR,"trg3"); /* name of partially loaded 
						   * target */
		assemnam= tempnam(TMPDIR,"trg4"); /* name of assembly file to 
						   * hold branch table */
		btname=   tempnam(TMPDIR,"trg6"); /* name of object to hold branch 
						   * table */
		if (!(ifil1name && ifil2name && tpltnam && pltname && assemnam &&
			btname))
			fatal("Cannot create temp file name");
	}

	if (makehost == TRUE) {
		moddir=  tempnam(TMPDIR,"hst1"); /* name of temp directory for
						  * holding modified objects */
		defname= tempnam(TMPDIR,"hst2"); /* name of lib. definition file */
		ainit=   tempnam(TMPDIR,"hst3"); /* name of aseembly file for 
						  * creating .init sections */
		if (!(moddir && defname && ainit))
			fatal("Cannot create temp file name");
	}
	errno=0; /* reset errno to prevent erroneous perror() messages */
}


/* This function enables signal catching so that temp files will be removed
 * before an abnormal termination of the program */
void
catchsigs()
{
	if (signal(SIGHUP, SIG_IGN) == SIG_DFL)
		(void)signal(SIGHUP, (int (*)())dexit);
	if (signal(SIGINT, SIG_IGN) == SIG_DFL)
		(void)signal(SIGINT, (int (*)())dexit);
	if (signal(SIGTERM, SIG_IGN) == SIG_DFL)
		(void)signal(SIGTERM, (int (*)())dexit);
}


/*VARARGS0*/
void
fatal(va_alist)
va_dcl
{
	extern int	errno;

	va_list	args;
	char	*fmt;
	static int	first=TRUE;

	va_start(args);
	(void)fprintf(stderr,"%s: ",progname);
	fmt= va_arg(args, char *);
	(void)vfprintf(stderr, fmt, args);
	va_end(args);
	if (errno != 0)
		perror(" ");
	else
		(void)putc('\n',stderr);

	if (first) {
		first= FALSE;
		dexit();
	} else
		exit(1);
}


/* dexit() deletes all temporary files and exits */
void
dexit()
{
	rmtmps();
	exit(1);
}


/* This function removes all temporary files. */
void
rmtmps()
{
	if (maketarg == TRUE) {
		(void)unlink(ifil1name);
		(void)unlink(ifil2name);
		(void)unlink(tpltnam);
		(void)unlink(pltname);
		(void)unlink(assemnam);
		(void)unlink(btname);
	}
	if (makehost == TRUE) {
		(void)unlink(defname);
		(void)unlink(ainit);
		if (execute("rm","rm","-r","-f",moddir,(char *)0))
			fatal("Internal rm invocation failed");
	}
}


/* This function invokes execvp to execute a command. */
/*VARARGS0*/
int
execute(va_alist)
va_dcl
{
	va_list	ap;
	char	*file;
	char	*args[MAXARGS];
	int	argno=0;
	int	pid,
		w,
		status;

	va_start(ap);
	file = va_arg(ap, char *);
	while ((args[argno++] = va_arg(ap,char *)) != (char *)0)
		if (argno >= MAXARGS)
			fatal("Too many args to %s for internal invocation",file);
	va_end(ap);

	if ((pid = fork()) == 0) {
		(void)execvp(file, args);
		fatal("Cannot exec %s", file);
	} else if (pid == -1)
		fatal("Process table full - try again later");

	while ((w = wait(&status)) != pid && w != -1) ;

	if (w == -1)
		fatal("Lost %d - No child process!",pid);
	else {
		if ((w = status & 0xff) != 0) {
			if (w != SIGINT)
				fatal("Fatal error in %s;  Status 0%o",file,status);
		}
	}
	return((status >> 8) & 0xff);
}
