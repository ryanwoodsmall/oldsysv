	.file	"cuexit.s"
#	@(#)cuexit.s	1.4
# C library -- exit

# exit(code)
# code is return in r0 to system

	.set	exit,1
.globl	_exit
.globl	__cleanup

	.align	2
_exit:
	.word	0x0000
	MCOUNT
	calls	$0,__cleanup
	chmk	$exit
	halt
