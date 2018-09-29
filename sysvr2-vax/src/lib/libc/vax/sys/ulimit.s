	.file	"ulimit.s"
#	@(#)ulimit.s	1.5
# C library -- ulimit

	.set	ulimit,63
.globl	_ulimit
.globl	cerror

_ulimit:
	.word	0x0000
	MCOUNT
	chmk	$ulimit
	bcc	.noerror
	jmp	cerror
.noerror:
	ret
