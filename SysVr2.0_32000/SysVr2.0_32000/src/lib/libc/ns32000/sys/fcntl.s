	.file	"fcntl.s"
#	@(#)fcntl.s	1.3
# C library -- fcntl

	.set	fcntl,62
	.globl	_fcntl
	.globl	cerror

_fcntl:
	MCOUNT
	addr	fcntl,r0
	addr	4(sp),r1
	svc
	bfc	noerror
	jump	cerror
noerror:
	ret	0
