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
/*	@(#)defs.c	1.2	*/

#include	"defs.h"

struct layer 	*layers[MAX_LAYERS];

int max_index = 0;
int chan = 0;
int fildes[2];
	
char cntlf[]	= "/dev/sxt/000";
int	cntl_chan_fd;
	
struct utmp	  *u_entry;
struct termio ttysave;

int uid;
int gid;
