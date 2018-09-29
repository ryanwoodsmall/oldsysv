	.file	"pause.s"
#	@(#)pause.s	1.3
# C library - pause

	.set	pause,29
	.globl	_pause
	.globl	cerror

	.align	2
_pause:
	MCOUNT
	addr	pause,r0
	addr	4(sp),r1
	svc
	bfc	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
