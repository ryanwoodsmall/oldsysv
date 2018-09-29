/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)ld:m32/special1.c	1.4"

#include <stdio.h>

#include "system.h"
#include "structs.h"
#include "extrns.h"



#if TRVEC
void
chktvorg(org, tvbndadr)

long	org, *tvbndadr;
{
	/*
	 * check user-supplied .tv origin for legality
	 * if illegal, side-effect tvspec.tvbndadr
	 *  and issue warning message
	 */

			if( (org & 0xf) != 0 )
				yyerror("tv origin (%10.0lx) must be a multiple of 16", org);
			*tvbndadr = (org + 0xfL) & ~0xfL;

}
#endif

void
specflags(flgname, argptr)
char *flgname;
char **argptr;
{

	/*
	 * process special flag specifications for m32ld
	 * these flags have fallen through switch of argname in ld00.c
	 */

	switch ( *flgname ) {

		default:
			yyerror("unknown flag: %s", flgname);

		}
}
