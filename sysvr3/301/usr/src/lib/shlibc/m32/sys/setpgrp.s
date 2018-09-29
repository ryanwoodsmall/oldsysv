#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- setpgrp, getpgrp
.ident	"@(#)libc-m32:sys/setpgrp.s	1.3"

	.set	_setpgrp,39*8

	.globl	setpgrp
	.globl	getpgrp

setpgrp:
	MCOUNT
	PUSHW	&1
	CALL	-4(%sp),pgrp
	RET

getpgrp:
	PUSHW	&0
	CALL	-4(%sp),pgrp
	RET

pgrp:
	MOVW	&4,%r0
	MOVW	&_setpgrp,%r1
	GATE
	RET
