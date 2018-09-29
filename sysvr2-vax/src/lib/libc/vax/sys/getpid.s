	.file	"getpid.s"
#	@(#)getpid.s	1.5
# getpid -- get process ID

	.set	getpid,20
.globl	_getpid

_getpid:
	.word	0x0000
	MCOUNT
	chmk	$getpid
	ret
