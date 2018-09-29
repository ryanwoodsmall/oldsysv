#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- setjmp, longjmp
.ident	"@(#)libc-m32:gen/setjmp.s	1.3"

#	longjmp(a,v)
# will generate a "return(v)" from 
# the last call to
# 	setjmp(a)
# by restoring r3-pc from 'a'
# and doing a return
#
	.globl	setjmp

setjmp:
	MCOUNT
	MOVW	0(%ap),%r0
	MOVW	%r3,0(%r0)
	MOVW	%r4,4(%r0)
	MOVW	%r5,8(%r0)
	MOVW	%r6,0xc(%r0)
	MOVW	%r7,0x10(%r0)
	MOVW	%r8,0x14(%r0)
	MOVW	-4(%sp),0x18(%r0)	# old %ap
	MOVW	-8(%sp),0x1c(%r0)	# old %pc
	MOVW	%ap,0x20(%r0)		# old %sp
	MOVW	%fp,0x24(%r0)		# old %fp
	CLRW	%r0
	RET

	.globl	longjmp

longjmp:
	MCOUNT
	MOVW	4(%ap),%r0
	MOVW	0(%ap),%r1
	MOVW	0(%r1),%r3
	MOVW	4(%r1),%r4
	MOVW	8(%r1),%r5
	MOVW	0xc(%r1),%r6
	MOVW	0x10(%r1),%r7
	MOVW	0x14(%r1),%r8
	MOVW	0x18(%r1),%ap
	MOVW	0x1c(%r1),%r2
	MOVW	0x20(%r1),%sp
	MOVW	0x24(%r1),%fp
	TSTW	%r0
	jne	L1
	MOVW	&1,%r0
L1:
	JMP	0(%r2)
