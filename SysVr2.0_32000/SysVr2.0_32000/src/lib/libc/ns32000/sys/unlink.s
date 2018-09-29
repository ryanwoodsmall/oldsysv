	.file	"unlink.s"
#	@(#)unlink.s	1.3
# C library -- unlink

# error = unlink(string);

	.set	unlink,10
	.globl	_unlink
	.globl	cerror

_unlink:
	MCOUNT
	addr	unlink,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
