	.file	"chown.s"
#	@(#)chown.s	1.3
# C library -- chown

# error = chown(string, owner);

	.set	chown,16
	.globl	_chown
	.globl	cerror

_chown:
	MCOUNT
	addr	chown,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
