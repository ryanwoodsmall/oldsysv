	.file	"pause.s"
#	@(#)pause.s	1.5
# C library - pause

	.set	pause,29
.globl	_pause
.globl	cerror

	.align	2
_pause:
	.word	0x0000
	MCOUNT
	chmk	$pause
	bcc	.noerror
	jmp	cerror
.noerror:
	clrl	r0
	ret
