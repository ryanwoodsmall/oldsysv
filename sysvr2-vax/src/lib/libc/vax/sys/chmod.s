	.file	"chmod.s"
#	@(#)chmod.s	1.5
# C library -- chmod

# error = chmod(string, mode);

	.set	chmod,15
.globl	_chmod
.globl	cerror

_chmod:
	.word	0x0000
	MCOUNT
	chmk	$chmod
	bcc 	.noerror
	jmp 	cerror
.noerror:
	clrl	r0
	ret
