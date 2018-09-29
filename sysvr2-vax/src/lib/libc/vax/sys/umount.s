	.file	"umount.s"
#	@(#)umount.s	1.5
# C library -- umount

	.set	umount,22
.globl	_umount
.globl	cerror

_umount:
	.word	0x0000
	MCOUNT
	chmk	$umount
	bcc 	.noerror
	jmp 	cerror
.noerror:
	clrl	r0
	ret
