#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


	.ident	"@(#)kern-port:ml/cswitch.s	10.2"
	.globl	setjmp

setjmp:
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
	MOVW	0(%ap),%r0
	MOVW	0(%r0),%r3
	MOVW	4(%r0),%r4
	MOVW	8(%r0),%r5
	MOVW	0xc(%r0),%r6
	MOVW	0x10(%r0),%r7
	MOVW	0x14(%r0),%r8
	MOVW	0x18(%r0),%ap
	MOVW	0x1c(%r0),%r1
	MOVW	0x20(%r0),%sp
	MOVW	0x24(%r0),%fp
	CLRW	%r0
	INCW	%r0
	JMP	0(%r1)
