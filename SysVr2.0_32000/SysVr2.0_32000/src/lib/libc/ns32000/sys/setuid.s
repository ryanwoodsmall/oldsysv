	.file	"setuid.s"
#	@(#)setuid.s	1.3
# C library -- setuid

# error = setuid(uid);

	.set	setuid,23
	.globl	_setuid
	.globl	cerror

_setuid:
	MCOUNT
	addr	setuid,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
