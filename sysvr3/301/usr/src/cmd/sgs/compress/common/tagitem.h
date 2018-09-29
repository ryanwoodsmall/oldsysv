/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)compress:common/tagitem.h	1.8"

typedef struct tagitem TAGITEM;

/*	tagitem.h contains structure that is used for comparing tags
 *	in the compress utility.  it also defines a macro to verity
 *	that a file contains the long IS25 object file format.
 */

struct tagitem {
	long	oldloc;		/*  old loc in symbol table  */
	int	nentrys;	/*  # of entries of tag      */
	TAGITEM	*realtag;	/*  ptr to "real" struct     */
	long	newloc;		/*  loc of tag after compression */
	SYMENT	*symptr;	/* temp storage of structure mems */
};


void	exit();
char	*calloc();

#define TRUE	1
#define FALSE	0
