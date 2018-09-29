/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:cmd/lpsched/files.c	1.4"

#include "lpsched.h"
#include "sys/stat.h"

/**
 ** chfiles() - CHANGE OWNERSHIP OF FILES, RETURN TOTAL SIZE
 **/

off_t
chfiles(list, uid, gid)
char	**list;
int	uid;
int	gid;
{
    size_t	total;
    struct stat	stbuf;
    char	*file;

    total = 0;

    while(file = *list++)
    {
	if (strncmp(Lp_Temp, file, strlen(Lp_Temp)) == 0)
	{
	    (void) Chmod(file, 0600);
	    (void) Chown(file, (int)uid, (int)gid);
	}

	if (Stat(file, &stbuf) == -1)
	    return(-1);

	switch (stbuf.st_mode & S_IFMT) {
	case 0:
	case S_IFREG:
	    break;

	case S_IFIFO:
	    if (!isadmin(uid))
		return(-1);
	    /*
	     * If any of the files is a FIFO, the size indicator
	     * becomes meaningless. On the other hand, returning
	     * a total of zero causes the request to be rejected,
	     * so we return something > 0.
	     */
	    stbuf.st_size = 1;
	    break;

	case S_IFDIR:
	case S_IFCHR:
	case S_IFBLK:
	default:
	    return(-1);
	}

	total += stbuf.st_size;
    }
    return(total);
}

/**
 ** rmfiles() - DELETE/LOG FILES FOR DEFUNCT REQUEST
 **/

void
rmfiles(rp, log_it)
RSTATUS	*rp;
int log_it;
{
    char	**file	= rp->request->file_list;
    char	*path;
    char	*p;
    char	num[20];
    static FILE	*logfp	= 0;
    int		reqfd;
    int		count	= 0;


    if (rp->req_file) {

	   /*
	    * The secure request file is removed first to prevent
	    * reloading should the system crash while in rmfiles().
	    */
	    path = makepath(Lp_Requests, rp->req_file, (char *)0);
	    (void) Unlink(path);
	    free(path);

	    /*
	     * Copy the request file to the log file, if asked,
	     * or simply remove it.
	     */
	    path = makepath(Lp_Temp, rp->req_file, (char *)0);
	    if (log_it && rp->secure && rp->secure->req_id) {
		if (!logfp)
		    logfp = open_lpfile(Lp_ReqLog, "a", MODE_NOREAD);
		if (logfp && (reqfd = Open(path, O_RDONLY)) != -1) {
		    register int	n;
		    char		buf[BUFSIZ];

		    fprintf (
			logfp,
			"= %s, uid %d, gid %d, size %ld, %s",
			rp->secure->req_id,
			rp->secure->uid,
			rp->secure->gid,
			rp->secure->size,
			ctime(&(rp->secure->date))
		    );
		    if (rp->slow)
			fprintf (logfp, "x %s\n", rp->slow);
		    if (rp->fast)
			fprintf (logfp, "y %s\n", rp->fast);
		    if (rp->printer && rp->printer->printer->name)
			fprintf (logfp, "z %s\n", rp->printer->printer->name);
		    while ((n = Read(reqfd, buf, BUFSIZ)) > 0)
			fwrite (buf, 1, n, logfp);
		    Close (reqfd);
		    fflush (logfp);
		}
	    }
	    (void)Unlink (path);
	    free (path);
    }

    if (file)
	while(*file)
	{
		/*
		 * The copies of user files.
		 */
		if (STRNEQU(Lp_Temp, *file, strlen(Lp_Temp)))
		    (void) Unlink(*file);
		count++;
		file++;
	}

    if (rp->secure && rp->secure->req_id) {
	p = getreqno(rp->secure->req_id);

	/*
	 * The filtered files. We can't rely on just the RS_FILTERED
	 * flag, since the request may have been cancelled while
	 * filtering. On the other hand, just checking "rp->slow"
	 * doesn't mean that the files exist, because the request
	 * may have been canceled BEFORE filtering started. Oh well.
	 */
	if (rp->slow)
	    while(count > 0)
	    {
		sprintf(num, "%d", count--);
		path = makestr(Lp_Temp, "/F", p, "-", num, (char *)0);
		Unlink(path);
		free(path);
	    }

	/*
	 * The notify/error file.
	 */
	path = makepath(Lp_Temp, p, (char *)0);
	(void) Unlink(path);
	free(path);
    }
}

/**
 ** _alloc_file() - ALLOCATE FILES FOR A REQUEST
 **/

#define	SEQF_DEF_START	1
#define	SEQF_DEF_END	999999
#define	SEQF_DEF_INCR	1
#define	SEQF		".SEQF"

char	*
_alloc_files(num, prefix, uid, gid)
int	num;
char	*prefix;
ushort	uid;
ushort	gid;
{
    static short	started = 0;
    static FILE		*fp;
    static long		start;
    static long		end;
    static long		incr;
    static long		curr;
    static char		base[15];
    static char		fmt[30];
    char		*file;
    char		*cp;
    int			fd;
    int			plus;

    if (num > 999999)
	return(0);

    if (!prefix)
    {
	if (!started)
	{
	    file = makepath(Lp_Temp, SEQF, (char *)0);
	    if ((fp = fopen(file, "r+")) == NULL)
		if ((fp = fopen(file, "w")) == NULL)
		    fail(CANNOT_OPEN_FILE, file, PERROR);

	    (void) rewind(fp);
	    if (fscanf(fp, "%ld:%ld:%ld:%ld\n", &start, &end, &incr, &curr) != 4)
	    {
		start = SEQF_DEF_START;
		end = SEQF_DEF_END;
		curr = start;
		incr = SEQF_DEF_INCR;
	    }

	    if (start < 0)
		start = SEQF_DEF_START;

	    if (end > 999999)
		end = SEQF_DEF_END;

	    if (curr < start || curr > end)
		curr = start;
	    (void) sprintf(fmt, "%ld:%ld:%ld:%%ld\n", start, end, incr);
	    started++;
	}

	(void) sprintf(base, "%d-999999", curr);

	if ((curr += incr) >= end)
	    curr = start;

	(void) rewind(fp);
	(void) fprintf(fp, fmt, curr);
	(void) fflush(fp);
	plus = 0;
    }
    else
    {
	if (strlen(prefix) > 6)
	    return(0);
	(void) sprintf(base, "F%s-999999", prefix);
	plus = 1;
    }

    if (!(file = makepath(Lp_Temp, base, (char *)0)))
	fail(MEMORY_ALLOC_FAILED);;
    
    cp = strrchr(file, '-') + 1;
    while(num--)
    {
	(void) sprintf(cp, "%d", num + plus);
	if ((fd = Open(file, O_CREAT|O_TRUNC, 0600)) == -1)
	{
	    free(file);
	    return(0);
	}
	else
	{
	    Close(fd);
	    Chown(file, (int)uid, (int)gid);
	}
    }
    free(file);

    if ((cp = strrchr(base, '-')))
	*cp = 0;
    return(base);
}
