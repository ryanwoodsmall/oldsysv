	.file	"ptrace.s"
#	@(#)ptrace.s	1.3
# ptrace -- C library

#	result = ptrace(req, pid, addr, data);

	.set	ptrace,26
	.globl	_ptrace
	.globl	cerror
	.globl  _errno

_ptrace:
	MCOUNT
	movqd	0,_errno
	addr	ptrace,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	ret	0
