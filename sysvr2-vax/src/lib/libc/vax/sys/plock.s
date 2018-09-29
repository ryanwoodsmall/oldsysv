	.file	"plock.s"
#	@(#)plock.s	1.5
# C library -- plock

# error = plock(op)

	.set	plock,45
.globl	_plock
.globl	cerror

_plock:
	.word	0x0000
	MCOUNT
	chmk	$plock
	bcc	.noerror
	jmp	cerror
.noerror:
	ret
