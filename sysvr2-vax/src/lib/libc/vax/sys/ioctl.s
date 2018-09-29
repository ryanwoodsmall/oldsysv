	.file	"ioctl.s"
#	@(#)ioctl.s	1.5
# C library -- ioctl

	.set	ioctl,54
.globl	_ioctl
.globl	cerror

_ioctl:
	.word	0x0000
	MCOUNT
	chmk	$ioctl
	bcc	.noerror
	jmp	cerror
.noerror:
	ret
