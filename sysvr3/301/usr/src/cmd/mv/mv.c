/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)mv:mv.c	1.22"
/*
 * Combined mv/cp/ln command:
 *	mv file1 file2
 *	mv dir1 dir2
 *	mv file1 ... filen dir1
 */



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

#define BLKSIZE	4096
#define	DOT	"."
#define	DELIM	'/'
#define EQ(x,y)	!strcmp(x,y)
#define	FALSE	0
#define MODEBITS 07777
#define TRUE 1

char	*malloc();
char	*dname();
char	*strrchr();
int	utime();
extern	int errno;
extern  char *optarg;
extern	int optind, opterr;
struct stat s1, s2;
int cpy = FALSE;	
int mve = FALSE;	
int lnk = FALSE;	
char	*cmd;
int	silent = 0;

main(argc, argv)
register char *argv[];
{
	register int c, i, r, errflg = 0;
	
	/*
	 * Determine command invoked (mv, cp, or ln) 
	 */
	
	if (cmd = strrchr(argv[0], '/'))
		++cmd;
	else
		cmd = argv[0];
	
	/*
	 * Set flags based on command.
	 */
	 
	  if (EQ(cmd, "mv"))
	  	mve = TRUE;
	  else if (EQ(cmd, "ln")) 
		   lnk = TRUE;
	  	else
		   cpy = TRUE;   /* default */
	
	/*
	 * Check for options:
	 * 	cp file1 [file2 ...] target
	 *	ln [-f] file1 [file2 ...] target
	 *	mv [-f] file1 [file2 ...] target
	 *	mv [-f] dir1 target
	 */
	 
	if (cpy) {
		while ((c = getopt(argc, argv,"")) != EOF) 
			errflg++;
	} else {

		while ((c = getopt(argc, argv,"f")) != EOF) 
	 	switch(c) {
			case 'f':
				silent++;
				break;
			default:
				errflg++;
		}
	}
	
	/*
	 * Check for sufficient arguments 
	 * or a usage error.
	 */

	argc -= optind;	 
	argv  = &argv[optind];
	
	if (argc < 2) {
		fprintf(stderr,"%s: Insufficient arguments (%d)\n",cmd,argc);
		usage();
	}
	
	if (errflg != 0)
		usage();
	
	/*
	 * If is is a move command and the
	 * first argument is a directory
	 * then (mv dir1 dir2).
	 */
	 
	stat(argv[0], &s1);
	if (mve) 
		if (ISDIR(s1) && argc == 2) {
	        
			/*
	            	 * Call mv_dir to do actual directory move.
	            	 * NOTE: mv_dir must belong to root 
			 *       and have the set uid bit on.
	        	 */
	        
			execl( "/usr/lib/mv_dir", "mv", argv[0], argv[1], 0 );
	        	fprintf(stderr, "%s:  cannot exec() /usr/lib/mv_dir\n", cmd );
	        	exit(2);
		}
	
	/*
	 * If there is more than a source and target,
	 * the last argument (the target) must be a directory
	 * which really exists.
	 */
	 
	if (argc > 2) {
		if (stat(argv[argc-1], &s2) < 0) {
			fprintf(stderr, "%s: %s not found\n", cmd, argv[argc-1]);
			exit(2);
		}
		
		if (!ISDIR(s2)) {
			fprintf(stderr,"%s: Target must be directory\n",cmd);
			usage();
		}
	}	
	
	/*
	 * Perform a multiple argument mv|cp|ln by
	 * multiple invocations of move().
	 */
	 
	r = 0;
	for (i=0; i<argc-1; i++)
		r += move(argv[i], argv[argc-1]);
	
	/* 
	 * Show errors by nonzero exit code.
	 */
	 
	 exit(r?2:0);
}

move(source, target)
char *source, *target;
{
	register last, c, i;
	char	*buf = (char *)NULL;
	int from, to, ct, oflg;
	char fbuf[BLKSIZE];
	
	struct	utimbuf	{
		time_t	actime;
		time_t	modtime;
		};
	struct utimbuf *times;

	/* 
	 * While source or target have trailing 
	 * DELIM (/), remove them (unless only "/")
	 */

	while (((last = strlen(source)) > 1)
	    &&  (source[last-1] == DELIM))
		 source[last-1]=NULL;
	
	while (((last = strlen(target)) > 1)
	    &&  (target[last-1] == DELIM))
		 target[last-1]=NULL;
	
	/*
	 * Make sure source file exists.
	 */
	 
	if (stat(source, &s1) < 0) {
		fprintf(stderr, "%s: cannot access %s\n", cmd, source);
		return(1);
	}
	
	/* 
	 * Make sure source file is not a directory,
	 * we don't move() directories...
	 */
	
	if (ISDIR(s1)) {
		fprintf(stderr, "%s : <%s> directory\n", cmd, source);
		return(1);
	}
	
	/*
	 * If it is a move command and we don't have write access 
	 * to the source's parent then goto s_unlink for the error
	 * message.
	 */

	if ((mve)
	  && accs_parent(source, 2) == -1)
		goto s_unlink;
	
	/*
	 * If stat fails, then the target doesn't exist,
	 * we will create a new target with default file type of regular.
 	 */	
	
	FTYPE(s2) = S_IFREG;
	
	if (stat(target, &s2) >= 0) {
		
		/*
		 * If target is a directory,
		 * make complete name of new file
		 * within that directory.
		 */

		if (ISDIR(s2)) {
			if ((buf = malloc(strlen(target) + strlen(dname(source)) + 4)) == NULL) {
				fprintf(stderr,"%s: Insufficient memory to %s %s\n ",cmd,cmd,source);
				exit(3);
			}
			sprintf(buf, "%s/%s", target, dname(source));
			target = buf;
		}
		
		/*
		 * If filenames for the source and target are 
		 * the same and the inodes are the same, it is
		 * an error.
		 */
		
		if (stat(target, &s2) >= 0) {
			if (IDENTICAL(s1,s2)) {
				fprintf(stderr, "%s: %s and %s are identical\n", cmd, source, target);
				if (buf != NULL)
					free(buf);
				return(1);
			}
			
			/*
			 * Because copy doesn't unlink target,
			 * treat it separately.
			 */
			
			if(cpy)
				goto skip;
			
			/* 
			 * Prevent super-user from overwriting a directory
			 * structure with file of same name.
			 */
			 
			 if (mve && ISDIR(s2)) {
				fprintf(stderr, "%s: Cannot overwrite directory %s\n", cmd, target);
				if (buf != NULL)
					free(buf);
				return(1);
			}
			
			/*
			 * If the user does not have access to
			 * the target, ask him----if it is not
			 * silent and user invoked command 
			 * interactively.
			 */
			
			if (access(target, 2) < 0 
			 && isatty(fileno(stdin))
			 && !silent) {
				fprintf(stderr, "%s: %s: %o mode?", cmd, target,
					FMODE(s2) & MODEBITS);
			
				/* Get response from user. Based on
				 * first character, make decision.
				 * Discard rest of line.
				 */
				
				i = c = getchar();
				while (c != '\n' && c != EOF)
					c = getchar();
				if (i != 'y') {
					if (buf != NULL)
						free(buf);
					return(1);
				}
			}
			
			/*
			 * Attempt to unlink the target.
			 */
			 
			 if (unlink(target) < 0) {
				fprintf(stderr, "%s: cannot unlink %s\n", cmd, target);
				if (buf != NULL)
					free(buf);
				return(1);
			}
		}
	}
skip:
	/* 
	 * Either the target doesn't exist,
	 * or this is a copy command ...
	 */
	 
	if (cpy || link(source, target) < 0) {

		/*
		 * If link failed, and command was 'ln'
		 * send out appropriate error message.
		 */
		 
		if (lnk) {
			if(errno == EXDEV)
				fprintf(stderr, "%s: different file system\n", cmd);
			else
				fprintf(stderr, "%s: no permission for %s\n", cmd, target);
			if (buf != NULL)
				free(buf);
			return(1);
		}
		
		/* 
		 * Attempt to open source for copy.
		 */
		 
		if((from = open(source, 0)) < 0) {
			fprintf(stderr, "%s: cannot open %s\n", cmd, source);
			if (buf != NULL)
				free(buf);
			return (1);
		}
		
		/* 
		 * Save a flag to indicate target existed.
		 */
		
		oflg = access(target, 0) == -1;
		
		/* 
		 * Attempt to create a target.
		 */
		
		if((to = creat (target, 0666)) < 0) {
			fprintf(stderr, "%s: cannot create %s\n", cmd, target);
			if (buf != NULL)
				free(buf);
			return (1);
		}
		
		/*
		 * Block copy from source to target.
		 */
		 
		/*
		 * If we created a target,
		 * set its permissions to the source
		 * before any copying so that any partially copied
		 * file will have the source's permissions (at most)
		 * or umask permissions whichever is the most restrictive.
		 */
		 
		if (oflg)
			chmod(target, FMODE(s1));
		
		while((ct = read(from, fbuf, BLKSIZE)) != 0)
			if(ct < 0 || write(to, fbuf, ct) != ct) {
				fprintf(stderr, "%s: bad copy to %s\n", cmd, target);
				/*
				 * If target is a regular file,
				 * unlink the bad file.
				 */
				 
				if (ISREG(s2))
					unlink(target);
				if (buf != NULL)
					free(buf);
				return (1);
			}
		
		/*
		 * If it was a move, leave times alone.
		 */
		if (mve) {
			times = (struct utimbuf *) malloc((unsigned) sizeof(struct utimbuf) + 2);
			times->actime = s1.st_atime;
			times->modtime = s1.st_mtime;
			utime(target, times);
			free(times);
		}
		close(from), close(to);
		
	}
	
	/* 
	 * If it is a copy or a link,
	 * we don't have to remove the source.
	 */
	 
	if (!mve) {
		if (buf != NULL)
			free(buf);
		return (0);
	}
	
	/* 
	 * Attempt to unlink the source.
	 */
	 
	if (unlink(source) < 0) {
s_unlink:
		fprintf(stderr, "%s: cannot unlink %s\n", cmd, source);
		if (buf != NULL)
			free(buf);
		return(1);
	}
	if (buf != NULL)
		free(buf);
	return(0);
}


accs_parent(name, amode)
register char *name;
register int amode;
{
	register c;
	register char *p, *q;
	char *buf;

	/*
	 * Allocate a buffer for parent.
	 */
	
	if ((buf = malloc(strlen(name) + 2)) == NULL) {
		fprintf(stderr,"%s: Insufficient memory space.\n",cmd);
		exit(3);
	}
	p = q = buf;
	
	/* 
	 * Copy name into buf and set 'q' to point to the last
	 * delimiter within name.
	 */
	 
	while (c = *p++ = *name++)
		if (c == DELIM)
			q = p-1;
	
	/*
	 * If the name had no '\' or was "\" then leave it alone,
	 * otherwise null the name at the last delimiter.
	 */
	 
	if (q == buf && *q == DELIM)
		q++;
	*q = NULL;
	
	/*
	 * Find the access of the parent.
	 * If no parent specified, use dot.
	 */
	 
	c = access(buf[0] ? buf : DOT,amode);
	free(buf);
	
	/* 
	 * Return access for parent.
	 */
	
	return(c);
}

char *
dname(name)
register char *name;
{
	register char *p;

	/* 
	 * Return just the file name given the complete path.
	 * Like basename(1).
	 */
	 
	p = name;
	
	/*
	 * While there are characters left,
	 * set name to start after last
	 * delimiter.
	 */
	 
	while (*p)
		if (*p++ == DELIM && *p)
			name = p;
	return name;
}

usage()
{
	register char *opt;
	
	/*
	 * Display usage message.
	 */
	 
	opt = cpy ? "" : " [-f]";
	fprintf(stderr, "Usage: %s%s f1 f2\n       %s%s f1 ... fn d1\n", 
		cmd, opt, cmd, opt);
	if(mve)
		fprintf(stderr, "       mv [-f] d1 d2\n");
	exit(2);
}
