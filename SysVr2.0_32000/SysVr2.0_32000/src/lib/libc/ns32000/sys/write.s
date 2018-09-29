	.file	"write.s"
#	@(#)write.s	1.3
# C library -- write

# nwritten = write(file, buffer, count);
# nwritten == -1 means error

	.set	write,4
	.globl	_write
	.globl	cerror

_write:
	MCOUNT
	addr	write,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	ret	0
