	.file	"unlink.s"
#	@(#)unlink.s	1.5
# C library -- unlink

# error = unlink(string);

	.set	unlink,10
.globl	_unlink
.globl  cerror

_unlink:
	.word	0x0000
	MCOUNT
	chmk	$unlink
	bcc 	.noerror
	jmp 	cerror
.noerror:
	clrl	r0
	ret
