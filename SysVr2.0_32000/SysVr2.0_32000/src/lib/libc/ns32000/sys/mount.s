	.file	"mount.s"
#	@(#)mount.s	1.3
# C library -- mount

# error = mount(dev, file, flag)

	.set	mount,21
	.globl	_mount
	.globl	cerror

_mount:
	MCOUNT
	addr	mount,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
