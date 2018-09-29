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
/*	saghdr.h 1.3 of 5/10/84	*/
#include <stdio.h>
static char Sccssag[]="@(#)saghdr.h	1.3";
#define	NPTS	100
#define NFLD	9
#define	FLDCH	10
#ifndef	DEBUG
#define	DEBUG	0
#endif

struct	entry	{
	char	tm[9];
	float	hr;
	float	val;
	char	qfld[8];
	};

struct	array	{
	char	hname[56];
	struct	entry	ent[NPTS];
	};


struct	c	{
	char	name[60];
	char	op;
	struct	array	*dptr;
	};

struct	p	{
	char	spec[60];
	struct	c	c[5];
	char	mn[10], mx[10];
	float	min, max;
	int	jitems;
	int	mode;
	};
