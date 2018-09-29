#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- utime
.ident	"@(#)libc-m32:sys/utime.s	1.3"
 
#  error = utime(string,timev);
 
	.set	_utime,30*8

	.globl	utime
	.globl	_cerror
 
utime:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_utime,%r1
	GATE
	jgeu	noerror
	JMP	_cerror
noerror:
	RET
