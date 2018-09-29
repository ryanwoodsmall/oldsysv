/	@(#)nice.s	1.3
/ C library-- nice

/ error = nice(hownice)

.globl	_nice, cerror
.nice = 34.

_nice:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(sp),r0
	sys	.nice
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
