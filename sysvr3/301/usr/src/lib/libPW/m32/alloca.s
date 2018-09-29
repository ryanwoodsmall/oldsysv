#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"alloca.s"
.ident	"@(#)libPW:m32/alloca.s	1.3"


#	ptr = alloca(nbytes) - allocate automatic storage

	.globl	alloca

alloca:
	save	&0
	movw	0(%ap), %r1		# get nbytes
	addw2	&0x00000003, %r1	# round up to multiple of 4
	andw2	&0xFFFFFFFC, %r1
	movw	%ap, %r0		# return value
	movw	-36(%fp), %r2		# get return address
	movw	-32(%fp), %ap		# restore ap
	movw	-28(%fp), %fp		# restore fp
	addw3	%r1, %r0, %sp		# grow stack
	jmp	0(%r2)			# return
