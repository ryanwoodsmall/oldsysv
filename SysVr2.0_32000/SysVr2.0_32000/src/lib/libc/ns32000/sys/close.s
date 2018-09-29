	.file	"close.s"
#	@(#)close.s	1.3
# C library -- close

# error =  close(file);

	.set	close,6
	.globl	_close
	.globl	cerror

_close:
	MCOUNT
	addr	close,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
