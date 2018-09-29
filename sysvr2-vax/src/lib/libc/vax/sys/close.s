	.file	"close.s"
#	@(#)close.s	1.5
# C library -- close

# error =  close(file);

	.set	close,6
.globl	_close
.globl	cerror

_close:
	.word	0x0000
	MCOUNT
	chmk	$close
	bcc 	.noerror
	jmp 	cerror
.noerror:
	clrl	r0
	ret
