	.file	"umount.s"
#	@(#)umount.s	1.3
# C library -- umount

	.set	umount,22
	.globl	_umount
	.globl	cerror

_umount:
	MCOUNT
	addr	umount,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
