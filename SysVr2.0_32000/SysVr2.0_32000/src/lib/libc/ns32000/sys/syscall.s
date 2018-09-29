	.file	"syscall.s"
#	@(#)syscall.s	1.3
# C library -- syscall

#  Interpret a given system call

	.globl	_syscall
	.globl	cerror

_syscall:
	MCOUNT
	movd	4(sp),r0	# syscall number
	addr	8(sp),r1	# one fewer arguments
	svc
	bfc	noerror
	jump	cerror
noerror:
	ret	0
