	.file	"sync.s"
#	@(#)sync.s	1.5
#  C library -- sync

	.set	sync,36
.globl	_sync

_sync:
	.word	0x0000
	MCOUNT
	chmk	$sync
	ret
