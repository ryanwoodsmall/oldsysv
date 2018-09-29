#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- close
.ident	"@(#)libc-m32:sys/close.s	1.3"

# error =  close(file);

	.set	_close,6*8

	.globl	close
	.globl	_cerror

close:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_close,%r1
	GATE
	jgeu	noerror
	JMP 	_cerror
noerror:
	CLRW	%r0
	RET
