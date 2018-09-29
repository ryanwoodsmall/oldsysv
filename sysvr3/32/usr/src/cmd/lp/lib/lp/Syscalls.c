/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/lp/Syscalls.c	1.3"

# include	<sys/types.h>
# include	<sys/stat.h>
# include	<errno.h>

extern int	errno;

_Access(s, i)
char	*s;
int	i;
{			/**/
    register int	n;
    
    while((n = access(s, i)) == -1 && errno == EINTR);
    return(n);
}

_Chdir(s)
char    *s;
{
    register int        n;

    while((n = chdir(s)) == -1 && errno == EINTR);
    return(n);
}

_Chmod(s, i)
char    *s;
int     i;
{
    register int        n;

    while((n = chmod(s, i)) == -1 && errno == EINTR);
    return(n);
}

_Chown(s, i, j)
char    *s;
int     i;
int     j;
{
    register int        n;

    while((n = chown(s, i, j)) == -1 && errno == EINTR);
    return(n);
}

_Close(i)
int     i;
{
    register int        n;

    while((n = close(i)) == -1 && errno == EINTR);
    return(n);
}

_Creat(s, i)
char    *s;
int     i;
{
    register int        n;

    while((n = creat(s, i)) == -1 && errno == EINTR);
    return(n);
}

/*VARARGS2*/
_Fcntl(i, j, k)
int     i;
int     j;
int     k;
{
    register int        n;
    while((n = fcntl(i, j, k)) == -1 && errno == EINTR);
    return(n);
}


/*
**	Code for Fork() is in cmd/lpsched/exec.c
*/


_Fstat(i, st)
int		i;
struct stat	*st;
{
    register int        n;

    while((n = fstat(i, st)) == -1 && errno == EINTR);
    return(n);
}

_Link(s1, s2)
char    *s1;
char	*s2;
{
    register int        n;

    while((n = link(s1, s2)) == -1 && errno == EINTR);
    return(n);
}

/*VARARGS2*/
_Mknod(s, i, j)
char    *s;
int     i;
int     j;
{
    register int        n;
    while((n = mknod(s, i, j)) == -1 && errno == EINTR);
    return(n);
}

/*VARARGS2*/
_Open(s, i, j)
char    *s;
int     i;
int	j;
{			/**/
    register int        n;

    while((n = open(s, i, j)) == -1 && errno == EINTR);
    return(n);
}

_Read(i, s, j)
int     i;
char    *s;
unsigned	j;
{
    register int        n;

    while((n = read(i, s, j)) == -1 && errno == EINTR);
    return(n);
}

_Stat(s, st)
char		*s;
struct stat	*st;
{
    register int        n;

    while((n = stat(s, st)) == -1 && errno == EINTR);
    return(n);
}

_Unlink(s)
char    *s;
{
    register int        n;

    while((n = unlink(s)) == -1 && errno == EINTR);
    return(n);
}

_Wait(i)
int     *i;
{
    register int        n;

    while((n = wait(i)) == -1 && errno == EINTR);
    return(n);
}

_Write(i, s, j)
int     i;
char    *s;
unsigned	j;
{			/**/
    register int        n;
    while((n = write(i, s, j)) == -1 && errno == EINTR);
    return(n);
}
