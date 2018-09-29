/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/hp7220.d/hpd.d/sbn.c	1.2"
#include <stdio.h>
#include "dev.h"

sbn(i) 
int i;
{
/*
 *  sbn translates integer i into single byte format
 */
	i=i & 077;
	if(i>=32) putchar(i);  else putchar(i|0100);
}
