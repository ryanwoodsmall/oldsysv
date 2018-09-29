	.file	"chroot.s"
#	@(#)chroot.s	1.3
#  C library -- chroot
 
#  error = chroot(string);
 
	.set	chroot,61
 
	.globl	_chroot
	.globl	cerror

_chroot:
	MCOUNT
	addr	chroot,r0
	addr	4(sp),r1
	svc
	bfc	noerror
	jump	cerror
noerror:
	ret	0
