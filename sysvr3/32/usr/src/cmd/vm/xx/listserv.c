/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)vm:xx/listserv.c	1.5"

#include <stdio.h>
#include <string.h>
#include "wish.h"
#include "exception.h"

main(argc,argv)
int argc;
char *argv[];
{
	FILE *fp;
	char *home, *getenv(), *label, *name, *penv;
	char hpath[PATHSIZ], buf[BUFSIZ], path[PATHSIZ], *opt;
	int flag=0, cond=0;

	penv=argv[argc-1];
	while(--argc > 0 && (*++argv)[0] == '-')
		for(opt=argv[0]+1; *opt != '\0'; opt++)
		switch(*opt) {
			case 'd':
				flag=1;
				break;
			case 'l': /* used to create the rmenu */
				flag=2;
				break;
			case 'm':
				flag=3;
				break;
			default:
				break;
		}
	home=getenv(penv);
	if (strcmp(penv,"HOME") == 0)
		sprintf(hpath, "%s/pref/services",home);
	else
		sprintf(hpath, "%s/OBJECTS/Menu.serve",home);

	if ((fp=fopen(hpath,"r")) == NULL) {
		fprintf(stderr,"Cannot open file %s",hpath);
		return FAIL;
	}

	while(fp && (fgets(buf,BUFSIZ,fp) != NULL)) {
		if (*buf == '\n' || *buf == '#' )
			continue;

		label = strtok(buf,"=");
		if (strcmp(label,"name") != 0)
			continue;

		name=strtok(NULL,"\n");

		sprintf(path,"%s/bin/%s.ins",home,name);

		if (access(path,00) == 0) {
			 cond=1;
			 if (flag == 2)  {
				printf("%s\n",name);
				continue;
			 }
			printf("name=%s\n",name);
			printf("lininfo=%s\n",name);
			if (flag == 1 )
				printf("action=OPEN FORM $VMSYS/OBJECTS/Form.del $LININFO\n");
			else if (flag == 3 )
				printf("action=OPEN FORM $VMSYS/OBJECTS/Form.mod $LININFO\n");
			else 
				if (strcmp(penv,"HOME") == 0)
					printf("action=`run $HOME/bin/%s.ins`\n",name);
				else
					printf("action=`run $VMSYS/bin/%s.ins`\n",name);
		}
	}
	if (!cond && strcmp(penv, "HOME") == 0)
		printf("init=`message No Personal Services`false\n");
}
