/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)nlp:include/users.h	1.2"

struct user_ {
    short priority_limit;    /* highest priority (lowest value) allowed */
};

typedef struct user_ USER;

int putuser(), deluser();

USER *getuser();

#define LEVEL_DFLT 20
#define LIMIT_DFLT 0

#define TRUE  1
#define FALSE 0

#define PRI_MAX 39
#define	PRI_MIN	 0

#define LPU_MODE 0644

struct user_priority {
    short	deflt;		/* priority to use when not specified */
    short	deflt_limit;	/* priority limit for users not
				   otherwise specified */
    char	**users[PRI_MAX - PRI_MIN + 1];
};
