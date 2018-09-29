	.file	"nexect.s"
#	@(#)nexect.s	1.3
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
	MCOUNT
#	bispsw	$0x10		# set tbit
	addr	execve,r0
	addr	4(sp),r1
	svc
	jump	cerror
