/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)graf:src/dev.d/tek4000.d/td.d/devinit.c	1.3"
/* <: t-5 d :> */
#include <stdio.h>
#include "td.h"
#include "dev.h"
#define ERASE       printf("\033\014")  /*  ESC NP  */
#define GRAPHMODE   printf("\035")      /*  GS  */
extern struct options {
	int eopt;  /*  erase  */
	int win;  /*  flag for window options ( 'u' or 'r')  */
}op;

devinit()
{
	extern char *nodename;
/*
 *  devinit initializes the tektronix terminal for displaying
 */
	if(op.eopt) {
		ERASE;
		sleep(1);
	}
	GRAPHMODE;
}
