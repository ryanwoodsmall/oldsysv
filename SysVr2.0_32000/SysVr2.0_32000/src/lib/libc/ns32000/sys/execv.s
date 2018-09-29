	.file	"execv.s"
#	@(#)execv.s	1.3
# C library -- execv

# execv(file, argv);

# where argv is a vector argv[0] ... argv[x], 0
# last vector element must be 0

	.globl	_environ
	.globl	_execv
	.globl	 _execve
_execv:
	MCOUNT
	enter	[],0
	movd	_environ,tos
	movd	12(fp),tos
	movd	8(fp),tos
	jsr	_execve
	adjspb	$-12
	exit		[]
	ret	0
