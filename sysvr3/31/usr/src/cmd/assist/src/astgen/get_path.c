/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)tools:get_path.c	1.7"

/*	This routine establishes the pathname where help files for
*	astgen can be found.  If $ASSISTLIB is defined, the files are
*	assumed to be in a directory called "astgen" in that.  If not,
*	the files are assumed to be in /usr/muse/lib/astgen
*/

#include "../forms/muse.h"

get_path()
{
	char *getenv(), *s;

	if ((s=(char*)getenv("ASSIST")) != NULL)
           sprintf(muse,"%s/",s);
	else sprintf(muse,"/usr/assist/");

	if ((s=(char*)getenv("ASSISTLIB")) != NULL)
		sprintf(muselib,"%s/astgen/",s);
	else sprintf(muselib,"%slib/astgen/",muse);
}
