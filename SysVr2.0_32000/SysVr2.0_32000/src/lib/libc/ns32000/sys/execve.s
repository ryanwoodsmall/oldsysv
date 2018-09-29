	.file	"execve.s"
#	@(#)execve.s	1.3
# C library -- execve

# execve(file, argv, env);

# where argv is a vector argv[0] ... argv[x], 0
# last vector element must be 0

	.set	exece,59
	.globl	_execve
	.globl	cerror

_execve:
	MCOUNT
	addr	exece,r0
	addr	4(sp),r1
	svc
	jump	cerror
