	.file	"read.s"
#	@(#)read.s	1.3
# C library -- read

# nread = read(file, buffer, count);
# nread ==0 means eof; nread == -1 means error

	.set	read,3
	.globl	_read
	.globl	cerror

_read:
	MCOUNT
	addr	read,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	ret	0
