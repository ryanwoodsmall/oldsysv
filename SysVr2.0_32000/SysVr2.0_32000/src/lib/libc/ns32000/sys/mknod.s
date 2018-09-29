	.file	"mknod.s"
#	@(#)mknod.s	1.3
# C library -- mknod

# error = mknod(string, mode, major.minor);

	.set	mknod,14
	.globl	_mknod
	.globl	cerror

_mknod:
	MCOUNT
	addr	mknod,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
