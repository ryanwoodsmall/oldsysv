	.file	"setuid.s"
#	@(#)setuid.s	1.5
# C library -- setuid

# error = setuid(uid);

	.set	setuid,23
.globl	_setuid
.globl  cerror

_setuid:
	.word	0x0000
	MCOUNT
	chmk	$setuid
	bcc 	.noerror
	jmp 	cerror
.noerror:
	clrl	r0
	ret
