	.file	"acct.s"
#	@(#)acct.s	1.3
# C library -- acct

	.set	acct,51
	.globl	_acct
	.globl	cerror

_acct:
	MCOUNT
	addr	acct,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	ret	0
