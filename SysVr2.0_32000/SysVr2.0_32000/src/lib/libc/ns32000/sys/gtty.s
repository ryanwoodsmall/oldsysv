	.file	"gtty.s"
#	@(#)gtty.s	1.3
# C library -- gtty

	.set	gtty,32
	.globl	_gtty
	.globl	cerror

_gtty:
	MCOUNT
	addr	gtty,r0
	addr	4(sp),r1
	svc
	bfc	noerror
	jump	cerror
noerror:
	ret	0
