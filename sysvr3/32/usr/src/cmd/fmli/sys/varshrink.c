/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.sys:src/sys/varshrink.c	1.2"

#include	<stdio.h>
#include	"wish.h"
#include	"var_arrays.h"

/*
 * shrink the actual space used by a v_array as much as possible
 * note that this requires the process to allocate more space
 * before giving some back, so it may actually INCREASE the data
 * segment size of the process.  If used, array_shrink should be
 * called before adding things to other v_arrays, since perhaps
 * one of them can take advantage of the freed space.
 */
struct v_array *
array_shrink(array)
struct v_array	array[];
{
	register struct v_array	*ptr;
	register struct v_array	*newptr;

	ptr = v_header(array);
	if ((newptr = (struct v_array *)realloc(ptr, sizeof(struct v_array) + ptr->tot_used * ptr->ele_size)) == NULL)
		return NULL;
	return v_body(ptr);
}
