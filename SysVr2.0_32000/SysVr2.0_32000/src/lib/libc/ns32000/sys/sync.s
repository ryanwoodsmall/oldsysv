	.file	"sync.s"
#	@(#)sync.s	1.3
#  C library -- sync

	.set	sync,36
	.globl	_sync

_sync:
	MCOUNT
	addr	sync,r0
	addr	4(sp),r1
	svc
	ret	0
