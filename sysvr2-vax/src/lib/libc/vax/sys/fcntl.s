	.file	"fcntl.s"
#	@(#)fcntl.s	1.5
# C library -- fcntl

	.set	fcntl,62
.globl	_fcntl
.globl	cerror

_fcntl:
	.word	0x0000
	MCOUNT
	chmk	$fcntl
	bcc	.noerror
	jmp	cerror
.noerror:
	ret
