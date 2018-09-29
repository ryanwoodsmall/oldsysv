#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

#  C library -- chroot
.ident	"@(#)libc-m32:sys/chroot.s	1.3"
 
#  error = chroot(string);
 
	.set	_chroot,61*8
 
	.globl	chroot
	.globl	_cerror

chroot:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_chroot,%r1
	GATE
	jgeu	noerror
	JMP	_cerror
noerror:
	RET
