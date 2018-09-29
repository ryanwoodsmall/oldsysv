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
/*
** Combined mv/cp/ln command:
**	mv file1 file2
**	mv dir1 dir2
**	mv file1 ... filen dir1
*/

char xxxsccsid[] = "@(#)mv.c	1.14";


#include	<stdio.h>
#include	<sys/types.h>
#include	<sys/stat.h>
#include	<errno.h>

#define EQ(x,y)	!strcmp(x,y)
#define	DOT	"."
#define	DOTDOT	".."
#define	DELIM	'/'
#define	MAXN	128
#define MODEBITS 07777
#if u370
#define BLKSIZE   4096
#else
#define BLKSIZE    2048
#endif

char	*pname();
char	*dname();
char	*strrchr();
int	utime();
extern	errno;

struct	stat s1, s2;

char	*cmd;
int	f_flag = 0;

main(argc, argv)
register char *argv[];
{
	register i, r;
	if (cmd = strrchr(argv[0], '/'))
		++cmd;
	else
		cmd = argv[0];
	if (!EQ(cmd, "cp") && !EQ(cmd, "mv") && !EQ(cmd, "ln")) {
		fprintf(stderr, "%s: command must be named cp|mv|ln--defaults to `cp'\n", cmd);
		cmd = "cp";
	}

	if (EQ(argv[1], "-f") && (EQ(cmd, "mv") || EQ(cmd, "ln")))
		++f_flag, --argc, ++argv;

	if (argc < 3)
		usage();
	stat(argv[1], &s1);
	if (((s1.st_mode & S_IFMT) == S_IFDIR) && EQ(cmd, "mv") && argc == 3) {
	        if (!EQ(cmd, "mv"))
	            usage();
	        if (argc != 3)
	            usage();
	        /*
	            invoke mv_dir to do actual directory move.
	            mv_dir must belong to root and have the set uid bit on.
	        */
	        execl( "/usr/lib/mv_dir", "mv", argv[1], argv[2], 0 );
	        fprintf( stderr, "%s:  cannot exec() /usr/lib/mv_dir\n", cmd );
	        exit(2);
	}
/*	setuid(getuid());	*/
	if (argc > 3)
		if (stat(argv[argc-1], &s2) < 0) {
			fprintf(stderr, "%s: %s not found\n", cmd, argv[argc-1]);
			exit(2);
		} else if ((s2.st_mode & S_IFMT) != S_IFDIR)
			usage();
	r = 0;
	for (i=1; i<argc-1; i++)
		r += move(argv[i], argv[argc-1]);
	exit(r?2:0);
}

move(source, target)
char *source, *target;
{
	register c, i;
	char	buf[MAXN];

	if (stat(source, &s1) < 0) {
		fprintf(stderr, "%s: cannot access %s\n", cmd, source);
		return(1);
	}
	if ((s1.st_mode & S_IFMT) == S_IFDIR) {
		fprintf(stderr, "%s : <%s> directory\n", cmd, source);
		return(1);
	}
	if (EQ(cmd, "mv") && access(pname(source), 2) == -1)
		goto s_unlink;
	s2.st_mode = S_IFREG;
	if (stat(target, &s2) >= 0) {
		if ((s2.st_mode & S_IFMT) == S_IFDIR) {
			sprintf(buf, "%s/%s", target, dname(source));
			target = buf;
		}
		if (stat(target, &s2) >= 0) {
			if (s1.st_dev==s2.st_dev && s1.st_ino==s2.st_ino) {
				fprintf(stderr, "%s: %s and %s are identical\n", cmd,
						source, target);
				return(1);
			}
			if(EQ(cmd, "cp"))
				goto skip;
			if (access(target, 2) < 0 && isatty(fileno(stdin))
			&& !f_flag) {
				fprintf(stderr, "%s: %s: %o mode ", cmd, target,
					s2.st_mode & MODEBITS);
				i = c = getchar();
				while (c != '\n' && c != EOF)
					c = getchar();
				if (i != 'y')
					return(1);
			}
			if (unlink(target) < 0) {
				fprintf(stderr, "%s: cannot unlink %s\n", cmd, target);
				return(1);
			}
		}
	}
skip:
	if (EQ(cmd, "cp") || link(source, target) < 0) {
		int from, to, ct, oflg;
		char fbuf[BLKSIZE];

		if (EQ(cmd, "ln")) {
			if(errno == EXDEV)
				fprintf(stderr, "%s: different file system\n", cmd);
			else
				fprintf(stderr, "%s: no permission for %s\n", cmd, target);
			return(1);
		}
		if((from = open(source, 0)) < 0) {
			fprintf(stderr, "%s: cannot open %s\n", cmd, source);
			return 1;
		}
		oflg = access(target, 0) == -1;
		if((to = creat (target, 0666)) < 0) {
			fprintf(stderr, "%s: cannot create %s\n", cmd, target);
			return 1;
		}
		while((ct = read(from, fbuf, BLKSIZE)) != 0)
			if(ct < 0 || write(to, fbuf, ct) != ct) {
				fprintf(stderr, "%s: bad copy to %s\n", cmd, target);
				if((s2.st_mode & S_IFMT) == S_IFREG)
					unlink(target);
				return 1;
			}
		if (EQ(cmd, "mv")) {
			struct	utimbuf	{
				time_t	actime;
				time_t	modtime;
				};
			struct utimbuf *times;

			times = (struct utimbuf *) malloc((unsigned) sizeof(struct utimbuf));
			times->actime = s1.st_atime;
			times->modtime = s1.st_mtime;
			utime(target, times);
			free(times);
		}
		close(from), close(to);
		if (oflg)
			chmod(target, s1.st_mode);
	}
	if (!EQ(cmd, "mv"))
		return 0;
	if (unlink(source) < 0) {
s_unlink:
		fprintf(stderr, "%s: cannot unlink %s\n", cmd, source);
		return(1);
	}
	return(0);
}


char *
pname(name)
register char *name;
{
	register c;
	register char *p, *q;
	static	char buf[MAXN];

	p = q = buf;
	while (c = *p++ = *name++)
		if (c == DELIM)
			q = p-1;
	if (q == buf && *q == DELIM)
		q++;
	*q = 0;
	return buf[0]? buf : DOT;
}

char *
dname(name)
register char *name;
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
	register char *opt;
	opt = EQ(cmd, "cp") ? "" : " [-f]";
	fprintf(stderr, "Usage: %s%s f1 f2\n       %s%s f1 ... fn d1\n", 
		cmd, opt, cmd, opt);
	if(EQ(cmd, "mv"))
		fprintf(stderr, "       mv [-f] d1 d2\n");
	exit(2);
}
