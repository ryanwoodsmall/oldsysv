	.file	"times.s"
#	@(#)times.s	1.3
# C library -- times

	.set	times,43
	.globl	_times
	.globl	cerror

_times:
	MCOUNT
	addr	times,r0
	addr	4(sp),r1
	svc
	bfc	noerror
	jump	cerror
noerror:
	ret	0
