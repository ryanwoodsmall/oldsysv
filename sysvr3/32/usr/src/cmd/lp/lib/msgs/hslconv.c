/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/msgs/hslconv.c	1.2"
/*
**      LINTLIBRARY
*/

# include	<string.h>

char		_lp_hextable[17] = "0123456789ABCDEF";

char	*
ltos(s,l)
char	*s;
unsigned long	l;
{
    int	i = 7;

    while (i >=	0)
    {
	s[i--] = _lp_hextable[l % 16];
	l /= 16;
    }
    s += 8;
    return(s);
}
char	*
htos(s, h)
char	*s;
unsigned short	h;
{
    int	i = 3;

    while (i >= 0)
    {
	s[i--] = _lp_hextable[h % 16];
	h /= 16;
    }
    s += 4;
    return(s);
}

unsigned long
stol(s)
char	*s;
{
    int		i = 0;
    unsigned long	l = 0;

    while (i < 8)
    {
	l <<= 4;
	l += strchr(_lp_hextable, s[i++]) - _lp_hextable;
    }
    return(l);
}

unsigned short
stoh(s)
char	*s;
{
    int		i = 0;
    short	unsigned h = 0;

    while (i < 4)
    {
	h <<= 4;
	h += strchr(_lp_hextable, s[i++]) - _lp_hextable;
    }
    return(h);
}
