	.file	"ioctl.s"
#	@(#)ioctl.s	1.3
# C library -- ioctl

	.set	ioctl,54
	.globl	_ioctl
	.globl	cerror

_ioctl:
	MCOUNT
	addr	ioctl,r0
	addr	4(sp),r1
	svc
	bfc	noerror
	jump	cerror
noerror:
	ret	0
