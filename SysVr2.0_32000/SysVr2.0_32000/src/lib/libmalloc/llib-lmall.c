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
/*	@(#)llib-lmall.c	1.1	*/
#include "malloc.h"
/*	Lint Library for Malloc(3x)	*/
/*	MALLOC(3X)	*/
/*	malloc, calloc, realloc and free are checked
/*	by the c library lint file
*/
int mallopt (cmd, value) int cmd, value; { return cmd+value; }
struct mallinfo mallinfo () { struct mallinfo s; return (s); }
