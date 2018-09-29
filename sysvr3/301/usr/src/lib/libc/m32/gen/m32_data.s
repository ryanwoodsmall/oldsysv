#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.ident	"@(#)libc-m32:gen/m32_data.s	1.5"
	.file   "m32_data.s"
# This file contains
# the definition of the
# global symbols errno and _old.fp
# 
# int errno = 0;
# int _old.fp = 0;

	.globl	errno

	.data
	.align	4
errno:	
	.word 	0
_m4_ifdef_(`SHLIB',
`	.globl	d.vect
	.globl	_old.fp
d.vect:
	.zero	32 * 4
_old.fp:
	.word	0
',
`')
