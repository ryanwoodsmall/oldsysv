	.file	"getppid.s"
#	@(#)getppid.s	1.5
# getppid -- get parent process ID

	.set	getpid,20
.globl	_getppid

_getppid:
	.word	0x0000
	MCOUNT
	chmk	$getpid
	movl	r1,r0
	ret
