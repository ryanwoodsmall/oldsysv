/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:com/version.c	1.14"

version()
{
#if u3b
	error("AT&T Technologies 3B-20 sdb  Advanced Programming Utilities Issue 1.1");
#else
#if u3b5 || u3b15
	error("AT&T Technologies 3B-5 sdb  Advanced Programming Utilities Issue 1.1");
#else
#if vax
	error("VAX-11/780 sdb  Advanced Programming Utilities Issue 1.1");
#else
#if u3b2
	error("AT&T Technologies 3B-2 sdb  Advanced Programming Utilities Issue 1.1");
#endif
#endif
#endif
#endif
}
