/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1985 AT&T
 *	All Rights Reserved
 *
 */

#ident	"@(#)vm.vt:src/vt/vt.h	1.7"

struct vt {
	char	*title;
	WINDOW	*win;
	WINDOW  *subwin;
	vt_id	next;
	vt_id	prev;
	int	number;
	int	flags;
};
/* les */
#define	WORK_LEN	7
#define	DATE_LEN	48

#define VT_USED		1
#define VT_DIRTY	2	/* contents of window changed */
#define VT_BDIRTY	4	/* border of window changed */
#define VT_TDIRTY	8	/* title of window changed */
#define VT_ADIRTY	16	/* "arrows" of window changed */
#define VT_ANYDIRTY	(VT_DIRTY | VT_BDIRTY | VT_TDIRTY | VT_ADIRTY)

extern vt_id		VT_front;
extern vt_id		VT_back;
extern vt_id		VT_curid;
extern struct vt	*VT_array;

/* attribute array */
extern chtype		Attr_list[];
#define highlights(x)	((chtype) Attr_list[x])
