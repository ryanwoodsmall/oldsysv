	.file	"execl.s"
#	@(#)execl.s	1.3
# C library -- execl

# execl(file, arg1, arg2, ... , 0);

	.globl	_environ
	.globl	_execl
	.globl	_execv

_execl:
	MCOUNT
	enter	[],0
	movd	_environ,tos
	addr	12(fp),tos
	movd	8(fp),tos
	jsr	_execv
	adjspb	$-12
	exit	[]
	ret	0
