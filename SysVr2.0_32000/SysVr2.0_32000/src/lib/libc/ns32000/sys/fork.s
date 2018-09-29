	.file	"fork.s"
#	@(#)fork.s	1.3
# C library -- fork

# pid = fork();

# r1 == 0 in parent process, r1 == 1 in child process.
# r0 == pid of child in parent, r0 == pid of parent in child.

	.set	fork,2
	.globl	_fork
	.globl	cerror

_fork:
	MCOUNT
	addr	fork,r0
	addr	4(sp),r1
	svc
	bfc	forkok
	jump	cerror
forkok:
	cmpqd	0,r1
	beq	parent
	movqd	0,r0
parent:
	ret	0
