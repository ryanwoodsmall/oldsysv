	.file	"stty.s"
#	@(#)stty.s	1.3
# C library -- stty

	.globl	_stty
	.globl	cerror

	.set	stty,31

_stty:
	MCOUNT
	addr	stty,r0
	addr	4(sp),r1
	svc
	bfc	noerror
	jump	cerror
noerror:
	ret	0
