	.file	"plock.s"
#	@(#)plock.s	1.3
# C library -- plock

# error = plock(op)

	.set	plock,45
	.globl	_plock
	.globl	cerror

_plock:
	MCOUNT
	addr	plock,r0
	addr	4(sp),r1
	svc
	bfc	noerror
	jump	cerror
noerror:
	ret	0
