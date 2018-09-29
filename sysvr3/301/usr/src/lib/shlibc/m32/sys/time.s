#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- time
.ident	"@(#)libc-m32:sys/time.s	1.3"

# tvec = time(tvec);

	.set	_time,13*8

	.globl	time

time:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_time,%r1
	GATE
	TSTW	0(%ap)
	je	nostore
	MOVW	%r0,*0(%ap)
nostore:
	RET
