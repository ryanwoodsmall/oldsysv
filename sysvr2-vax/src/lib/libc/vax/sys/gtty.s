	.file	"gtty.s"
#	@(#)gtty.s	1.5
# C library -- gtty

	.set	gtty,32
.globl	_gtty
.globl	cerror

_gtty:
	.word	0x0000
	MCOUNT
	chmk	$gtty
	bcc	.noerror
	jmp	cerror
.noerror:
	ret
