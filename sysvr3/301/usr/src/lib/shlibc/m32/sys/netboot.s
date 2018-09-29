#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

.ident	"@(#)libc-m32:sys/netboot.s	1.2"

	.set	_netboot,68*8

	.globl	netboot
	.globl	_cerror

netboot:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_netboot,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	RET
