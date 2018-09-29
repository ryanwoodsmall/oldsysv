/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dump:common/filedefs.h	1.3"
struct filelist {
	char		*file;
	LDFILE		*ldptr;
	struct filelist	*nextitem;
};

#define FILELIST	struct filelist
#define LISTSZ	sizeof(FILELIST)

/*
*/
