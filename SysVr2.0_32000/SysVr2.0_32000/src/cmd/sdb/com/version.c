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
 	/* @(#) version.c: 1.9 10/14/83 */

version()
{
#if u3b
	error("WECo 3B-20 sdb System V Release 2.0");
#else
#if u3b5
	error("WECo 3B-5 sdb System V Release 2.0");
#else
#if vax
	error("VAX-11/780 sdb System V Release 2.0");
#else
#if ns32000
	error("NS32032 System V Release 2.0");
#endif
#endif
#endif
#endif
}
