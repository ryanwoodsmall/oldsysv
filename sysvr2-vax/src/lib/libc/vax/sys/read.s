	.file	"read.s"
#	@(#)read.s	1.5
# C library -- read

# nread = read(file, buffer, count);
# nread ==0 means eof; nread == -1 means error

	.set	read,3
.globl	_read
.globl  cerror

_read:
	.word	0x0000
	MCOUNT
	chmk	$read
	bcc 	.noerror
	jmp 	cerror
.noerror:
	ret
