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
#ifdef OSDEBUG
#define ASSERT(EX) if (EX) ; else assfail("EX", __FILE__, __LINE__)
#else
#define ASSERT(EX)
#endif

#define PFLOGON 0
#define PHASHLOGON 0
#define SWLOGON 0
#define LOCKLOGON 0
#define SPTLOGON 0
