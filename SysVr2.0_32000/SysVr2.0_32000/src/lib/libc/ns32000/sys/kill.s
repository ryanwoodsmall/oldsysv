	.file	"kill.s"
#	@(#)kill.s	1.3
# C library -- kill

	.set	kill,37
	.globl	_kill
	.globl	cerror

_kill:
	MCOUNT
	addr	kill,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
