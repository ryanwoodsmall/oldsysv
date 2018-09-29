	.file	"chroot.s"
#	@(#)chroot.s	1.5
#  C library -- chroot
 
#  error = chroot(string);
 
	.set	chroot,61
 
.globl	_chroot
.globl	cerror

_chroot:
	.word	0x0000
	MCOUNT
	chmk	$chroot
	bcc	.noerror
	jmp	cerror
.noerror:
	ret
