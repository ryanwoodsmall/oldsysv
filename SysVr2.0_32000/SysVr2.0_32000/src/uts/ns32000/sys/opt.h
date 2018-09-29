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
/* @(#)opt.h	6.1 */
#if MESG==0
msgsys(){nosys();}
msginit(){return(0);}
#endif

#if SEMA==0
semsys(){nosys();}
seminit(){}
semexit(){}
#endif

#if SHMEM==0
shmsys(){nosys();}
shmexec(){}
shmexit(){}
shmfork(){}
shmreset(){}
#endif
