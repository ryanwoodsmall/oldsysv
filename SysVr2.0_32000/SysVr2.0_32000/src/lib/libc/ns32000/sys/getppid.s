	.file	"getppid.s"
#	@(#)getppid.s	1.3
# getppid -- get parent process ID

	.set	getpid,20
	.globl	_getppid

_getppid:
	MCOUNT
	addr	getpid,r0
	addr	4(sp),r1
	svc
	movd	r1,r0
	ret	0
