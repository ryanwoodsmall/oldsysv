	.file	"execl.s"
#	@(#)execl.s	1.5
# C library -- execl

# execl(file, arg1, arg2, ... , 0);

.globl	_execl
.globl	_execv

_execl:
	.word	0x0000
	MCOUNT
	pushab	8(ap)
	pushl	4(ap)
	calls	$2,_execv
	ret
