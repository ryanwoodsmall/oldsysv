/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/msgs/_getmessage.c	1.3"
/*
**      LINTLIBRARY
*/

# include	<varargs.h>
# include	<memory.h>
# include	<string.h>
# include	<errno.h>
# include	"msgs.h"

extern char	*_lp_msg_fmts[];
extern int	errno;

/* VARARGS */
int
_getmessage(buf, rtype, arg)
char	*buf;
short	rtype;
va_list	arg;
{
    char	*endbuf;
    char	*fmt;
    char	**t_string;
    int		temp = 0;
    long	*t_long;
    short	*t_short;
    short	etype;

    if (buf == (char *)0)
    {
	errno = ENOSPC;
	return(-1);
    }

    /*
     * We assume that we're given a buffer big enough to hold
     * the header.
     */

    endbuf = buf + (long)stoh(buf + HEAD_SIZE);
    if ((buf + CONTROL_LEN) > endbuf)
    {
	errno = ENOMSG;
	return(-1);
    }

    etype = stoh(buf + HEAD_TYPE);
    if (etype < 0 || etype > LAST_UMESSAGE)
    {
	errno = EBADMSG;
        return(-1);
    }

    if (etype != rtype)
    {
	if (rtype > LAST_UMESSAGE && rtype <= LAST_IMESSAGE)
	    fmt = _lp_msg_fmts[rtype];
	else
	{
	    errno = EINVAL;
	    return(-1);
	}
    }
    else
	fmt = _lp_msg_fmts[etype];

    buf += HEAD_LEN;
    while (*fmt != '\0')
	switch(*fmt++)
	{
	    case 'H':
	        if ((buf + 4) > endbuf)
		{
		    errno = ENOMSG;
		    return(-1);
		}

		t_short = (short *) va_arg(arg, short *);
		*t_short = stoh(buf);
		buf += 4;
		break;

	    case 'L':
		if ((buf + 8) > endbuf)
		{
		    errno = ENOMSG;
		    return(-1);
		}

		t_long = (long *) va_arg(arg, long *);
		*t_long = stol(buf);
		buf += 8;
		break;

	    case 'S':
		if ((buf + 4) > endbuf)
		{
		    errno = ENOMSG;
		    return(-1);
		}

		t_string = (char **) va_arg(arg, char **);
		temp = stoh(buf);
		buf += 4;
		if ((buf + temp) > endbuf)
		{
		    errno = ENOMSG;
		    return(-1);
		}

		*t_string = buf;
		buf += temp;
		break;
	}
    return(etype);
}
