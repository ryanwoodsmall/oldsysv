#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#ident	"@(#)kern-port:debug/arg.s	1.1"
	.file "arg.s"
	.text
#
#
#	calculate the number of arguments our caller was passed and
#	return this value.  
#
	.globl argcount

argcount:
	subw3	&36,%fp,%r0	# r0 = fp - 9 * sizeof(char *)
	subw2	-4(%sp),%r0	# r0 -= oldap
	divw2	&4,%r0		# convert from bytes to words
	RET
