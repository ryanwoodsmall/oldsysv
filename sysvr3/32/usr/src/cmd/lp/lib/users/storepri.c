/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/users/storepri.c	1.3"
/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/


#include <stdio.h>
#include "lp.h"
#include "users.h"

/*
Inputs:
Outputs:
Effects:
*/
print_tbl(ppri_tbl)
struct user_priority *ppri_tbl;
{
    int limit;

    printf("Default priority: %d\n", ppri_tbl->deflt);
    printf("Priority limit for users not listed below: %d\n", ppri_tbl->deflt_limit);
    printf("Priority  Users\n");
    printlist_setup ("", "", ",", "\n");
    for (limit = PRI_MIN; limit <= PRI_MAX; limit++) {
	if (ppri_tbl->users[limit - PRI_MIN]) {
	    printf("   %2d     ", limit);
	    printlist(stdout, ppri_tbl->users[limit - PRI_MIN]);
	}
    }
}
/*
Inputs:
Outputs:
Effects:
*/
output_tbl(f, ppri_tbl)
FILE *f;
struct user_priority *ppri_tbl;
{
    int limit;
    char **ptr;
    int ucnt;

    fprintf(f, "%d\n%d:\n", ppri_tbl->deflt, ppri_tbl->deflt_limit);
    printlist_setup ("	", "\n", "", "");
    for (limit = PRI_MIN; limit <= PRI_MAX; limit++)
	if (ppri_tbl->users[limit - PRI_MIN]) {
	    fprintf(f, "%d:", limit);
	    printlist(f, ppri_tbl->users[limit - PRI_MIN]);
	}
}
