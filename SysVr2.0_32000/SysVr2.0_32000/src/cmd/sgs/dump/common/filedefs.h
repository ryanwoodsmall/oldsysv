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
struct filelist {
	char		*file;
	LDFILE		*ldptr;
	struct filelist	*nextitem;
};

#define FILELIST	struct filelist
#define LISTSZ	sizeof(FILELIST)

/*
*     static char	ID_filedefs[ ] = "@(#) filedefs.h: 1.1 1/11/82";
*/
