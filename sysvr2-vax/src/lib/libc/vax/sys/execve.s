	.file	"execve.s"
#	@(#)execve.s	1.5
# C library -- execve

# execve(file, argv, env);

# where argv is a vector argv[0] ... argv[x], 0
# last vector element must be 0

	.set	exece,59
.globl	_execve
.globl	cerror

_execve:
	.word	0x0000
	MCOUNT
	chmk	$exece
	jmp 	cerror
