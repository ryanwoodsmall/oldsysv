	.file	"acct.s"
#	@(#)acct.s	1.5
# C library -- acct

	.set	acct,51
.globl	_acct
.globl  cerror

_acct:
	.word	0x0000
	MCOUNT
	chmk	$acct
	bcc 	.noerror
	jmp 	cerror
.noerror:
	ret
