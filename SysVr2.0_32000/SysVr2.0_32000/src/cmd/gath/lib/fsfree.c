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
static char Sccsid[] = "@(#)fsfree.c	1.1";

fsfree(d,v)
 int v[];
 {int fsb[8];
  register int t;
  t=ustat(d,fsb);
  if (fsb[0]<0) fsb[0]=10000;
  if (fsb[1]<0) fsb[1]=10000;
  *v++=fsb[0];
  *v++=fsb[1];
  return (t);};
