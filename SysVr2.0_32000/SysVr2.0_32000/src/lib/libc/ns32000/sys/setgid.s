	.file	"setgid.s"
#	@(#)setgid.s	1.3
# C library -- setgid

# error = setgid(uid);

	.set	setgid,46
	.globl	_setgid
	.globl	cerror

_setgid:
	MCOUNT
	addr	setgid,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
