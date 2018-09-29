	.file	"ulimit.s"
#	@(#)ulimit.s	1.3
# C library -- ulimit

	.set	ulimit,63
	.globl	_ulimit
	.globl	cerror

_ulimit:
	MCOUNT
	addr	ulimit,r0
	addr	4(sp),r1
	svc
	bfc	noerror
	jump	cerror
noerror:
	ret	0
