/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mv:mv_dir.c	1.7"
/*
 * mv_dir command:  move a directory within its parent directory
 *    mv_dir dir1 dir2
 * to build:
 *    cc mv_dir.c -o mv_dir
 *    mv mv_dir /bin/mv_dir
 *    cd /bin
 *    chown root mv_dir
 *   chmod u+s mv_dir
 */


#include	<signal.h>
#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<errno.h>

#define FTYPE(A)	(A.st_mode)
#define FMODE(A)	(A.st_mode)
#define	IDENTICAL(A,B)	(A.st_dev==B.st_dev && A.st_ino==B.st_ino)
#define ISBLK(A)	((A.st_mode & S_IFMT) == S_IFBLK)
#define ISCHR(A)	((A.st_mode & S_IFMT) == S_IFCHR)
#define ISDIR(A)	((A.st_mode & S_IFMT) == S_IFDIR)
#define ISFIFO(A)	((A.st_mode & S_IFMT) == S_IFIFO)
#define ISREG(A)	((A.st_mode & S_IFMT) == S_IFREG)

#define	DOT	"."
#define	DOTDOT	".."
#define	DELIM	'/'
#define EQ(x,y)	!strcmp(x,y)

char	*malloc();
char	*dname();
char	*strrchr();
extern	errno;

static	struct stat s1, s2;

static	char	*cmd;

main(argc, argv)
char	*argv[];
{
	register char *source, *target;
	register char *q, *p;
	int last;

	/*
	 * Determine command name.
	 */

	if (cmd = strrchr(argv[0], '/'))
		++cmd;
	else
		cmd = argv[0];

	/*
	 * Check for usage errors.
	 *   1) Insufficient arguments.
	 *   2) source isn't a directory.
	 */

	if (argc != 3) {
		fprintf(stderr,"Incorrect number of arguments (%d).\n",argc);
		usage();
	}
	
	source = argv[1];
	target = argv[2];
	
	/* 
	 * While source or target have trailing 
	 * DELIM (/), remove them (unless only "/")
	 */

	while (((last = strlen(source)) > 1)
	    &&  (source[last-1] == DELIM))
		 source[last-1]=NULL;
	
	while (((last = strlen(source)) > 1)
	    &&  (source[last-1] == DELIM))
		 source[last-1]=NULL;
	
	
	stat(source, &s1);
	
	if (!ISDIR(s1)) {
		fprintf(stderr,"Source (%s) is not a directory.\n",source);
		usage();
	}
	
	/*
	 * Make sure target does not exist.
	 */ 
	
	if (stat(target, &s2) >= 0) {
		fprintf(stderr, "%s: %s exists\n", cmd, target);
		exit(2);
	}
	
	/* 
	 * If source's name is the same as target's name, it is an error.
	 */

	if (EQ(source, target)) {
		fprintf(stderr, "%s: ?? source == target, source exists and target doesn't\n", cmd);
		exit(2);
	}
	
	/*
	 * Set p equal to file name of source's complete path.
	 */

	 p = dname(source);
	
	/*
	 * We can not rename "." or ".." or "" or anything ending in '/'.
	 */

	if (EQ(p, DOT) || EQ(p, DOTDOT) || !*p || p[strlen(p)-1]=='/') {
		fprintf(stderr, "%s: cannot rename %s\n", cmd, p);
		exit(2);
	}
	
	/*
	 * Set p to parent of source.
	 * Set q to parent of target.
	 */ 

	if ((p = malloc(strlen(source)+4)) == NULL) {
		fprintf(stderr,"%s: Insufficient memory space.\n",cmd);
		exit(3);
	}
	strcpy(p,source);
	pname(p);
	
	if ((q = malloc(strlen(target)+4)) == NULL) {
		fprintf(stderr,"%s: Insufficient memory space.\n",cmd);
		exit(3);
	}
	strcpy(q,target);
	pname(q);
	
	/* 
	 * Parents for both source and target must exist.
	 */

	if (stat(p, &s1) < 0) {
		fprintf(stderr, "%s: cannot locate parent of source (%s).\n", cmd, p);
		exit(2);
	}
	
	if (stat(q, &s2) < 0) {
		fprintf(stderr, "%s: cannot locate parent of target (%s).\n", cmd, q);
		exit(2);
	}
	
	/*
	 * Parents of source and target must be identical.
	 */

	if (!IDENTICAL(s1,s2)) {
		fprintf(stderr, "%s: directory rename only\n", cmd);
		exit(2);
	}
	
	/*
	 * If user doesn't have access to parent, then directory
	 * can not be renamed.
	 */

	if (access(p, 2) < 0) {
		fprintf(stderr, "%s: no write access to %s\n", cmd, p);
		exit(2);
	}

	/*
	 * Everything looks good so free up parent info.
	 */

	free(p);
	free(q);
	
	/*
	 *	Ignore signals that could cause the directories to be left
	 *	linked together.
	 */
	signal(SIGHUP, SIG_IGN);
	signal(SIGINT, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
	
	/*
	 * First, link source to target.
	 */

	if (link(source, target) < 0) {
		fprintf(stderr, "%s: cannot link %s and %s\n", cmd,
			source, target);
		exit(2);
	}
	
	/* 
	 * Next, unlink target.
	 */
	
	if (unlink(source) < 0) {
		fprintf(stderr, "%s: ?? cannot unlink %s\n", cmd, source);
		exit(2);
	}
	exit(0);
}

pname(name)
register char *name;
/*
 * This function returns the parent (i.e., the
 * specified path minus the last entry), or "."
 * if in user's current working directory.
 */
{
	register c;
	register char *p, *q;

	p = q = name;
	while (c = *name++)
		if (c == DELIM)
			q = name-1;
	if (q == p && *q == DELIM)
		q++;
	*q = 0;
	if (!*p) 
		strcpy(p,DOT);
	return;
}

char *
dname(name)
register char *name;
/* 
 * This function extracts the last name (file or
 * directory) from the specified path.
 */
{
	register char *p;

	p = name;
	while (*p)
		if (*p++ == DELIM && *p)
			name = p;
	return name;
}

usage()
{
	fprintf(stderr, "Usage: %s dir1 dir2\n", cmd);
	exit(2);
}
