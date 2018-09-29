/	@(#)sync.s	1.3
/ C library -- synch()

.globl	_sync
.sync = 36.

_sync:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	sys	.sync
	mov	(sp)+,r5
	rts	pc
