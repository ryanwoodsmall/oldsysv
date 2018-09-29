#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


	.ident	"@(#)kern-port:boot/olboot/misc.s	1.1"
#
#	OLBOOT ENTRY POINT
#
#	Olboot is called by mboot (which is operating on the IAU
#	stack).  The IAU stack is only 1K (0x30100 - 0x30500) for
#	release 1.2 of the 3B5.  This code will switch the stack
#	to address &sstack (length estack-sstack); the stack
#	bounds in the current PCB will also be adjusted.
#

	.globl	sstack
	.globl	estack
	.globl	main
	.globl	start
start:
	SAVE	%r8
	MOVW	16(%pcbp),%r8
	MOVAW	estack-8,16(%pcbp)	# set stack upper bound

	MOVW	%sp,%r0
	MOVAW	sstack,%sp
	CALL	0(%r0),main		# use CALL so unwinding is easy

	MOVW	%r8,16(%pcbp)
	RESTORE	%r8
	RET
