/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */

#ident	"@(#)vm:xx/mnucheck.c	1.2"

#include	<stdio.h>
#include 	<string.h>
#include 	<ctype.h>
#include	"wish.h"


#define FAILURE	4
#define EXISTS  2
#define INVALID 3
#define SUCCESS 0

extern char *optarg;
char	*string;
char	*name=NULL;

main(argc,argv)
int argc;
char *argv[];
{

	int     ret;
	int 	opt;
	char	hpath[PATHSIZ], path[PATHSIZ], *home, *vmsys, *str, *getenv();
	FILE    *fp, *nfp;

	home=getenv("HOME");
	vmsys=getenv("VMSYS");

	while((opt=getopt(argc,argv,"s:n:")) != EOF)
		switch(opt) {
		case 's':
			string=optarg;
			break;
		case 'n':
			name=optarg;
			break;
		}

	if (string == NULL || strchr(string, '\040') || strlen(string) == 0)
		return(INVALID);

	str = string;
	for (; *str != '\0'; str++)
		if (isalnum(*str) ||  *str == '_')
			continue;
		else
			return(FAILURE);

	if (strcmp(string,name) == 0)
		return(SUCCESS);

	sprintf(hpath,"%s/pref/services",home);
	sprintf(path, "%s/OBJECTS/Menu.serve",vmsys);

	if(access(hpath, 00) == 0) {
		if ((fp=fopen(hpath, "r")) == NULL) {
			fprintf(stderr, "Cannot open file %s",hpath);
			return(FAILURE);
		}

		if (mread(fp)) {
			fclose(fp);
			return(EXISTS);
		}

	}
	if(access(path, 00) == 0) {
		if ((nfp=fopen(path,"r")) == NULL) {
			fprintf(stderr, "Cannot open file %s",path);
			return(FAILURE);
		}

		if (mread(nfp)) {
			fclose(nfp);
			return(EXISTS);
		}
	}
	fclose(fp);
	fclose(nfp);
	return(SUCCESS);
}

mread(fp)
FILE *fp;
{
	char buf[BUFSIZ], *label, *mname;

	while(fp && (fgets(buf, BUFSIZ,fp) != NULL)) {
		if (*buf == '\n' || *buf == '#' )
			continue;

		label=strtok(buf, "=");
		if (strcmp(label,"name") != 0)
			continue;

		mname=strtok(NULL,"\n");
		if (strcmp(mname,string) == 0)
			return(EXISTS);
	}
	return(SUCCESS);
}
