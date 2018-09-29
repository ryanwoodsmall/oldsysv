/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mkdir:mkdir.c	1.7"
/*
** make directory.
** If -m is used with a valid mode, directories will be
** created in that mode.  Otherwise, the default mode will
** be 777 possibly altered by the process's file mode creation
** mask.
** If -p is used, make the directory as well as
** its non-existing parent directories.
*/

#include	<signal.h>
#include	<stdio.h>
#include	<errno.h>
#include	<string.h>

extern int opterr, optind,  errno;
extern char *optarg,  *sys_errlist[];

char *strrchr();

main(argc, argv)
char *argv[];
int argc;
{
	int i, c, pflag, errflg, mflag, mode;
	char *d, *p, *endptr, *m, *ptr;

	pflag = 0;
	mflag = 0;
	errflg = 0;
	mode= 0777;

	while ((c=getopt(argc, argv, "m:p")) != EOF)
		switch (c) {
			case 'm':
				mflag++;
				m = optarg;
				for(i=0; *m != '\0'; m++)
					if(*m < '0' || *m > '7') {
						fprintf(stderr,"Invalid mode.\n");
						exit(2);
						}
				umask(0);
				mode = (int) strtol(optarg, (char **)NULL, 8);
				break;
			case 'p':
				pflag++;
				break;
			case '?':
				errflg++;
				break;
		}
	if(argc < 2 || errflg) {
		fprintf(stderr, "mkdir: usage: mkdir [-m mode] [-p] dirname ...\n");
		exit(2);
	}
	argc -= optind;
	argv = &argv[optind];
        while(argc--) {
		d = *argv++;

			/* Skip extra slashes at the end of path */

		while ((endptr=strrchr(d, '/')) != NULL){
			p=endptr;
			p++;
			if (*p == '\0')
				*endptr='\0';
			else
				break;
		}

			/* When -p is set */
		if (pflag){ 
			if (mkdirp(d,mode) <0) {
				fprintf(stderr, "mkdir: \"%s\": %s\n",d,sys_errlist[errno]);
				continue;
			}
			continue;
		}
			/* No -p. Make only one directory */

				/* Check write permission of parent*/

					/* Parent is current directory */

		if ((ptr=strrchr(d,'/')) == NULL)
			if (access(".",02)) {
				fprintf(stderr, "mkdir: \"%s\": %s\n",d,sys_errlist[errno]);
				continue;
			} else;
		else {
					/* Parent is / */
			if (ptr == d) {
				if (access("/",02)) {
					fprintf(stderr, "mkdir: \"%s\": %s\n",d,sys_errlist[errno]);
					continue;
				} else;
					/* Parent is not / */
			} else {
				*ptr = '\0';
				if (access(d,02)) {
					*ptr='/';
					fprintf(stderr, "mkdir: \"%s\": %s\n",d,sys_errlist[errno]);
					continue;
				}
				*ptr='/';
			}
		}
				/* Set id to real uid and gid */
 
		if (setuid(getuid()) || setgid(getgid())) {
			fprintf(stderr, "mkdir:  Failed to set effective user/group ids to real user/group ids");
			continue;
		}
				/* Make the directory */

		if ((mkdir(d, mode)) < 0) {
                   	fprintf(stderr, "mkdir:  Failed to make directory \"%s\"; %s\n", d, sys_errlist[errno]);  
			continue;
		}
	}
        exit(errno? 2: 0);
}
