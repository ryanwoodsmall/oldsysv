	.file	"getpid.s"
#	@(#)getpid.s	1.3
# getpid -- get process ID

	.set	getpid,20
	.globl	_getpid

_getpid:
	MCOUNT
	addr	getpid,r0
	addr	4(sp),r1
	svc
	ret	0
