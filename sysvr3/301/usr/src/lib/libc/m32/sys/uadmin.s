#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


.ident	"@(#)libc-m32:sys/uadmin.s	1.3"
# C library -- uadmin

# file = uadmin(cmd,fcn[,mdep])

# file == -1 means error

	.set	_uadmin,55*8

	.globl	uadmin
	.globl  _cerror

uadmin:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_uadmin,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET
