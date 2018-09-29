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
/*	@(#)sys.h	1.2	*/
/*
 *  External declarations for system call library routines
 */

extern int  close();
extern int  creat();
extern int  execl();
extern int  exit();
extern int  fork();
extern long lseek();
extern int  open();
extern int  ptrace();
extern int  read();
extern char *sbrk();
extern int  (* signal())();
extern int  wait();
extern int  write();
