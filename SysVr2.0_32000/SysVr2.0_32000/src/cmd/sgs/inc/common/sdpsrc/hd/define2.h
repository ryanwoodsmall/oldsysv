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
/*
 * static char ID_def2h[] = "@(#) define2.h: 1.1 1/7/82";
 */

/*
 * Users should use but not change these defined constants
 *
 *
 *
 *	CREATE		tells sdpinit() when the address space paging
 *				files do not yet exist
 *	EMPTY		null itemid
 *	RNLY		permission for lock and unlock;
 *				no intent to change item
 *	WRTN		permission for lock and unlock;
 *				intent is to change item
 */

#define CREATE		1
#define EMPTY		(-1)
#define RNLY		1
#define WRTN		2
