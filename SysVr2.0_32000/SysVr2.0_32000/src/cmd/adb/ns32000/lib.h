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
/*	@(#)lib.h	1.2	*/
/*
 *  Library call external declarations
 */

extern STRING environ;
extern int    errno;

extern char *ctime();
extern int  exect();
extern int  fprintf();
extern void perror();
extern int  setjmp();
extern int  sprintf();
