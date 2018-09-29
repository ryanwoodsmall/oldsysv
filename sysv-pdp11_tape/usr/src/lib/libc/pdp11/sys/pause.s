/	@(#)pause.s	1.3
/ C library - pause

.globl	_pause, cerror
.pause = 29.

_pause:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	sys	.pause
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
