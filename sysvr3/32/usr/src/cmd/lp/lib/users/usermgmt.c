/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:lib/users/usermgmt.c	1.6"

#include <stdio.h>
#include "lp.h"
#include "users.h"

static loaded = 0;
static struct user_priority *ppri_tbl;
struct user_priority *ld_priority_file();
static USER usr;

char *PRIORITY;
char *makepath();

int
putuser(user, pri_s)
char *user;
USER *pri_s;
{
    FILE *f;

    if (!loaded) {
	if (!PRIORITY)
	    PRIORITY = makepath(getspooldir(), USERSFILE, (char *)0);
	if (!(ppri_tbl = ld_priority_file(PRIORITY))) return(-1);
	loaded = 1;
    }

    if (!add_user(ppri_tbl, user, pri_s->priority_limit)) {
	endpwent();
	return(-1);
    }
    endpwent();

    if (!(f = open_lpfile(PRIORITY, "w", LPU_MODE)))
	return(-1);
    output_tbl(f, ppri_tbl);
    close_lpfile(f);
    return(0);
}

USER *
getuser(user)
char *user;
{
    int limit;

    /* root and lp do not get a limit */
    if (STREQU(user, "root")
     || STREQU(user, LPUSER)) {
	usr.priority_limit = 0;
	return(&usr);
    }

    if (!loaded) {
	if (!PRIORITY)
	    PRIORITY = makepath(getspooldir(), USERSFILE, (char *)0);
	if (!(ppri_tbl = ld_priority_file(PRIORITY))) return((USER *)0);
	loaded = 1;
    }

    for (limit = PRI_MIN; limit <= PRI_MAX; limit++)
	if (searchlist(user, ppri_tbl->users[limit - PRI_MIN])) {
	    usr.priority_limit = limit;
	    return(&usr);
	}

    usr.priority_limit = ppri_tbl->deflt_limit;
    return(&usr);
}

int
deluser(user)
char *user;
{
    FILE *f;

    if (!loaded) {
	if (!PRIORITY)
	    PRIORITY = makepath(getspooldir(), USERSFILE, (char *)0);
	if (!(ppri_tbl = ld_priority_file(PRIORITY))) return(-1);
	loaded = 1;
    }

    del_user(ppri_tbl, user);
    if (!(f = open_lpfile(PRIORITY, "w", LPU_MODE)))
	return(-1);

    output_tbl(f, ppri_tbl);
    close_lpfile(f);
    return(0);
}

int
getdfltpri()
{
    if (!loaded) {
	if (!PRIORITY)
	    PRIORITY = makepath(getspooldir(), USERSFILE, (char *)0);
	if (!(ppri_tbl = ld_priority_file(PRIORITY))) return(-1);
	loaded = 1;
    }

    return (ppri_tbl->deflt);
}

trashusers()
{
    int limit;

    if (loaded) {
	if (ppri_tbl) {
	    for (limit = PRI_MIN; limit <= PRI_MAX; limit++)
		freelist (ppri_tbl->users[limit - PRI_MIN]);
	    ppri_tbl = 0;
	}
	loaded = 0;
    }
}

