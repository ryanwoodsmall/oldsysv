	.file	"mknod.s"
#	@(#)mknod.s	1.5
# C library -- mknod

# error = mknod(string, mode, major.minor);

	.set	mknod,14
.globl	_mknod
.globl  cerror

_mknod:
	.word	0x0000
	MCOUNT
	chmk	$mknod
	bcc 	.noerror
	jmp 	cerror
.noerror:
	clrl	r0
	ret
