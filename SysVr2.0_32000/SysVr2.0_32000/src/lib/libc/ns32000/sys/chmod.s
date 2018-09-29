	.file	"chmod.s"
#	@(#)chmod.s	1.3
# C library -- chmod

# error = chmod(string, mode);

	.set	chmod,15
	.globl	_chmod
	.globl	cerror

_chmod:
	MCOUNT
	addr	chmod,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
