	.file	"stty.s"
#	@(#)stty.s	1.5
# C library -- stty

.globl	_stty
.globl	cerror

	.set	stty,31

_stty:
	.word	0x0000
	MCOUNT
	chmk	$stty
	bcc	.noerror
	jmp	cerror
.noerror:
	ret
