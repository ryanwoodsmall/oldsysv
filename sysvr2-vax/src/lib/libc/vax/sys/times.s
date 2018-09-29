	.file	"times.s"
#	@(#)times.s	1.5
# C library -- times

	.set	times,43
.globl	_times
.globl	cerror

_times:
	.word	0x0000
	MCOUNT
	chmk	$times
	bcc	.noerror
	jmp	cerror
.noerror:
	ret
