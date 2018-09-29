#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

# C library -- wait
.ident	"@(#)libc-m32:sys/wait.s	1.3"

# pid = wait(0);
#   or,
# pid = wait(&status);

# pid == -1 if error
# status indicates fate of process, if given

	.set	_wait,7*8

	.globl	wait
	.globl  _cerror

	.align	1
wait:
	MCOUNT
	MOVW	&4,%r0
	MOVW	&_wait,%r1
	GATE
	jgeu 	noerror
	JMP 	_cerror
noerror:
	TSTW	0(%ap)		# status desired?
	je	nostatus	# no
	MOVW	%r1,*0(%ap)	# store child's status
nostatus:
	RET
