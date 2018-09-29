	.file	"nice.s"
#	@(#)nice.s	1.3
# C library-- nice

# error = nice(hownice)

	.set	nice,34
	.globl	_nice
	.globl	cerror

_nice:
	MCOUNT
	addr	nice,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	ret	0
