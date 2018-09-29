	.file	"lseek.s"
#	@(#)lseek.s	1.3
# C library -- lseek

# error = lseek(file, offset, ptr);

	.set	lseek,19
	.globl	_lseek
	.globl	cerror

_lseek:
	MCOUNT
	addr	lseek,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	ret	0
