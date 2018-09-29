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
struct  _Acttab
{
        String  act_name;
        Routine act_func;
};

typedef struct  _Acttab Acttab;         /* new type: Acttab */

/* external Acttabs: */

extern  Acttab  act_tab[];      /* Verb actions */

extern  Routine act_get();
extern	char	*act_lk();
