/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)vm:xx/delserve.c	1.1"

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "wish.h"
#include "exception.h"

main(argc,argv)
int argc;
char *argv[];
{
	FILE *fp, *nfp;
	char *home, *getenv(), *mktemp(), *penv;
	char *mname, item[BUFSIZ], action[BUFSIZ], buf[BUFSIZ]; 
	char hpath[PATHSIZ], spath[PATHSIZ], tpath[PATHSIZ];

	if (argc < 2) {
		fprintf(stderr,"Arguments invalid");
		return FAIL;
	}

	/* Initialize arguments needed to create installation script */
	mname=argv[1];
	penv=argv[2];

	home=getenv(penv);
	sprintf(hpath, "%s/bin/%s.ins",home,mname);
	if (strcmp(penv,"HOME") == 0)
		sprintf(spath, "%s/pref/services",home);
	else
		sprintf(spath,"%s/OBJECTS/Menu.serve",home);
	sprintf(item,"name=%s\n",mname);

	if (strcmp(penv, "HOME") == 0)
		sprintf(action, "action=`run $HOME/bin/%s.ins`\n",mname);
	else
		sprintf(action, "action=`run $VMSYS/bin/%s.ins`\n",mname);

	/* if file exist copy it to old.<name> */
	if (access(hpath, 00) == 0) 
		unlink(hpath);
	
	/* Update the service file */
	sprintf(tpath, "/tmp/servXXXXXX");
	mktemp(tpath);

	if ((fp=fopen(spath,"r")) == NULL) {
		fprintf(stderr, "Cannot open file %s",spath);
		return FAIL;
	}

	if ((nfp=fopen(tpath,"w")) == NULL) {
		fprintf(stderr, "Cannot open file %s",tpath);
		return FAIL;
	}

	while(fp && (fgets(buf,BUFSIZ,fp) != NULL)) {
		if(strcmp(buf,item) != 0 && strcmp(buf,action) != 0)
			fprintf(nfp,"%s",buf);

	}
	fclose(nfp);
	fclose(fp);
	copyfile(tpath,spath);

}




/*
 * copy a file
 */
FILE *
cpfile(from, to)
char	*from;
char	*to;
{
	register int	c;
	register FILE	*src;
	register FILE	*dst;

	if ((src = fopen(from, "r")) == NULL)
		return NULL;
	if ((dst = fopen(to, "w+")) == NULL) {
		fclose(src);
		return NULL;
	}
	while ((c = getc(src)) != EOF)
		putc(c, dst);
	if (ferror(src)) {
		fclose(src);
		fclose(dst);
		unlink(to);
		return NULL;
	}
	fclose(src);
	return dst;
}

copyfile(from, to)
char *from;
char *to;
{
	FILE *fp;

	if (fp = cpfile(from, to)) {
		fclose(fp);
		return(0);
	}
	return(-1);
}
