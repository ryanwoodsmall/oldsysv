	.file	"write.s"
#	@(#)write.s	1.5
# C library -- write

# nwritten = write(file, buffer, count);
# nwritten == -1 means error

	.set	write,4
.globl	_write
.globl  cerror

_write:
	.word	0x0000
	MCOUNT
	chmk	$write
	bcc 	.noerror
	jmp 	cerror
.noerror:
	ret
