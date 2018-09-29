/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)libc-m32:fp/fp_data.c	1.3"
/*
 * contains the definitions
 * of the global symbols used
 * by the floating point environment
 */
#define P754_NOFAULT    1
#include "shlib.h"
#ifdef SHLIB

#include <ieeefp.h>
 int 			_fp_hw = 0;
 int 			_asr = 0;
 fp_ftype 		_fpftype = (fp_ftype)0;
 struct fp_fault* 	_fpfault = (struct fp_fault *)0;
#endif
