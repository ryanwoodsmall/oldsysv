/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.sys:src/sys/itoa.c	1.1"

char *
itoa(n, base)
int	n;
int	base;
{
	register char	*p;
	register int	minus;
	static char	buf[36];

	p = &buf[36];
	*--p = '\0';
	if (n < 0) {
		minus = 1;
		n = -n;
	}
	else
		minus = 0;
	if (n == 0)
		*--p = '0';
	else
		while (n > 0) {
			*--p = "0123456789abcdef"[n % base];
			n /= base;
		}
	if (minus)
		*--p = '-';
	return p;
}
