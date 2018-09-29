#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#  C library -- umask
.ident	"@(#)libc-m32:sys/umask.s	1.3"
 
#  omask = umask(mode);
 
	.set	_umask,60*8

	.globl	umask
	.globl	_cerror

umask:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_umask,%r1
	GATE
	jgeu	noerror
	JMP	_cerror
noerror:
	RET
