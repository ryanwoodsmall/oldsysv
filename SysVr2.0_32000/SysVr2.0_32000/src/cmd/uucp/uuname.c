/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
/* @(#)uuname.c	1.6 */
#include "uucp.h"
 
/*
 * returns a list of all remote systems.
 * options:
 *	-l	-> returns only the local system name.
 *	-v 	-> description of the system is also displayed.
 * If ADMIN does not exist, then ignore -v option.
 * All known systems are stored in SYSFILE, while description in ADMIN.
 */
char	**Env;
main(argc,argv, envp)
char *argv[];
char	**envp;
int argc;
{
	FILE *np, *fd;
	register short lflg, vflg, c;
	char s[300], prev[20], s1[20], s2[300], name[20], *desc;

	Env = envp;
	lflg = vflg = 0;
	while ((c=getopt(argc, argv, "lv")) != EOF )
		switch (c) {
		case 'l':
			lflg++;
			break;
		case 'v':
			vflg++;
			break;
		case '?':
			fprintf(stderr, "usage: uuname [-l] [-v]\n");
			exit(2);
		}
 
	if ( (vflg) && ((fd=fopen(ADMIN, "r")) == NULL)) 
		vflg=0;
	if (lflg) {
		uucpname(name);

		/*
		 * initialize to null string
		 */
		desc = "";	
		printf("%s",name);
		if (vflg) {
			while (fgets(s, 300, fd) != NULL) {
				sscanf(s, "%[^\t] %[^\t\n]", s1, s2);
				if (strncmp(name, s1, SYSNSIZE) == 0) { 
					printf(" %s", s2);
					break;
				}
			}
		} 
		printf("\n");
		exit(0);
	}
	if ((np=fopen(SYSFILE, "r")) == NULL) {
		fprintf(stderr, "File \" %s \" is protected\n", SYSFILE);
		exit(2);
	}
 
	while (fgets(s, 300, np) != NULL) {
		if((s[0] == '#') || (s[0] == ' ') || (s[0] == '\t') || 
			(s[0] == '\n'))
			continue;
		sscanf(s, "%s", name);
		if (strcmp(name,prev)==SAME)	continue;
		desc = "";
		printf("%s",name);
		if (vflg) { 

			/*
			 * rewind
			 */
			fseek(fd, 0L,0);	
			while (fgets(s, 300, fd) != NULL) {
				sscanf(s, "%[^\t] %[^\t\n]", s1, s2);
				if (strcmp(name, s1) == 0) {
					printf(" %s", s2);
					break;
				}
			}
		}
		printf("\n");
		strcpy(prev,name);
	}
	exit(0);
}


