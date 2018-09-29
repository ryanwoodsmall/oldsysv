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
/* @(#)pkon.c	1.2 */
pkon(i,j) int i,j; { if (i == j); return(1); } /* shuts up lint */
pkoff(i) int i; { if (i); return; } /*shuts up lint */
