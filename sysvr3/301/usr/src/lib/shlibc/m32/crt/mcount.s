#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# count subroutine called during profiling
.ident	"@(#)libc-m32:crt/mcount.s	1.3"
#
# calling sequence:	MOVW	&a_word,%r0
#			JSB	_mcount
#			.data
#		a_word:	.word	0
#

	.globl	_mcount
	.globl	countbase

_mcount:
	MOVW	0(%r0),%r1		# a_word to %r1
	jne	add1			# skip initialization if not zero

	MOVW	countbase,%r1		# get stack base in %r1
	REQL				# return if counting not turned on yet

	ADDW2	&8,countbase		# allocate a new pc,count pair
	MOVW	-4(%sp),0(%r1)		# remember pc
	ADDW2	&4,%r1
	MOVW	%r1,0(%r0)		# store &pair+4 in a_word

add1:
	INCW	0(%r1)			# count function call
	RSB
