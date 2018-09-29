/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

/*
 * Copyright  (c) 1986 AT&T
 *	All Rights Reserved
 */
#ident	"@(#)vm.sys:src/sys/varchkapnd.c	1.1"

#include	<stdio.h>
#include	"wish.h"
#include	"var_arrays.h"

/*
 * like array_append, but creates array if NULL
 */
struct v_array *
array_check_append(size, array, element)
unsigned size;
struct v_array	array[];
char	*element;
{
	if (array == NULL)
		array = array_create(size, 8);
	return array_append(array, element);
}
