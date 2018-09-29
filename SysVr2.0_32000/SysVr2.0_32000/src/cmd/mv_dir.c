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
** mv_dir command:  move a directory within its parent directory
**    mv_dir dir1 dir2
to build:
    cc mv_dir.c -o mv_dir
    mv mv_dir /bin/mv_dir
    cd /bin
    chown root mv_dir
    chmod u+s mv_dir
*/

char xxxsccsid[] = "@(#)mv_dir.c	1.2";

#include    <signal.h>
#include    <stdio.h>
#include    <sys/types.h>
#include    <sys/stat.h>
#include    <errno.h>

#define EQ(x,y)    !strcmp(x,y)
#define    DOT    "."
#define    DOTDOT    ".."
#define    DELIM    '/'
#define    MAXN    128

char    *pname();
char    *dname();
char    *strrchr();
extern    errno;

static    struct    stat s1, s2;

static    char    *cmd;

main(argc, argv)
char    *argv[];
{
    register char *source, *target;
    register char *p;

    if (cmd = strrchr(argv[0], '/'))
        ++cmd;
    else
        cmd = argv[0];


    if (argc != 3)
        usage();
    stat(argv[1], &s1);
    if ((s1.st_mode & S_IFMT) != S_IFDIR)
        usage();

    source = argv[1];
    target = argv[2];
    if (stat(target, &s2) >= 0) {
        fprintf(stderr, "%s: %s exists\n", cmd, target);
        exit(2);
    }
    if (EQ(source, target)) {
        fprintf(stderr, "%s: ?? source == target, source exists and target doesn't\n", cmd);
        exit(2);
    }
    p = dname(source);
    if (EQ(p, DOT) || EQ(p, DOTDOT) || !*p || p[strlen(p)-1]=='/') {
        fprintf(stderr, "%s: cannot rename %s\n", cmd, p);
        exit(2);
    }
    p = pname(source);
    if (stat(p, &s1) < 0 || stat(pname(target), &s2) < 0) {
        fprintf(stderr, "%s: cannot locate parent\n", cmd);
        exit(2);
    }
    if (s1.st_dev != s2.st_dev || s1.st_ino != s2.st_ino) {
        fprintf(stderr, "%s: directory rename only\n", cmd);
        exit(2);
    }
    if (access(p, 2) < 0) {
        fprintf(stderr, "%s: no write access to %s\n", cmd, p);
        exit(2);
    }

    /*
        ignore signals that could cause the directories to be left
        linked together.
    */
    signal(SIGHUP, SIG_IGN);
    signal(SIGINT, SIG_IGN);
    signal(SIGQUIT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    if (link(source, target) < 0) {
        fprintf(stderr, "%s: cannot link %s and %s\n", cmd,
            source, target);
        exit(2);
    }
    if (unlink(source) < 0) {
        fprintf(stderr, "%s: ?? cannot unlink %s\n", cmd, source);
        exit(2);
    }
    exit(0);
}

char *
pname(name)
register char *name;
{
    register c;
    register char *p, *q;
    static    char buf[MAXN];

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
    fprintf(stderr, "Usage: %s dir1 dir2\n", cmd);
    exit(2);
}
