	.file	"execle.s"
#	@(#)execle.s	1.3
# C library -- execle

# execle(file, arg1, arg2, ... , env);

	.globl	_execle
	.globl	_execve

_execle:
	MCOUNT
	enter	[],0
	addr	12(fp),r0
loop:
	cmpqd	0,0(r0)
	addqd	4,r0
	bne	loop

	movd	0(r0),tos	# env
	addr	12(fp),tos	# argv
	movd	8(fp),tos	# file
	jsr	_execve
	exit	[]
	ret	0
