	.file	"nexect.s"
#	@(#)nexect.s	1.5
# C library -- exect

# exect(file, argv, env);

# where argv is a vector argv[0] ... argv[x], 0
# last vector element must be 0

# The same as execve except that it sets the TBIT causing
# a trace trap on the first instruction of the executed process,
# to give a chance to set breakpoints.

	.set	execve,59
.globl	_exect
.globl	cerror

_exect:
	.word	0x0000
	MCOUNT
	bispsw	$0x10		# set tbit
	chmk	$execve
	jmp 	cerror
