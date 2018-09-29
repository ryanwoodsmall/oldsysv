#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- exit
.ident	"@(#)libc-m32:gen/cuexit.s	1.5"

# exit(code)
# code is return in r0 to system

	.set	_exit,1*8

	.globl	exit

exit:
	MCOUNT
#
#The following M4 macro is for shared libraries.
#If SHLIB is defined, substitute _cleanup for a pointer to _cleanup (_libc__cleanup)
#
 	CALL	0(%sp),_cleanup
	MOVW	&4,%r0
	MOVW	&_exit,%r1
	GATE
