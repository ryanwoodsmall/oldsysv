/	@(#)plock.s	1.3
/ C library -- lock

/ lock(operation);
/	4 = unlock process text or data
/	1 = lock process text & data
/	2 = lock only process text
/	3 = lock process data

.globl _plock
.globl cerror
utssys = 57.
lock = 105.

_plock:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	utssys; lock
	bec	8f
	jmp	cerror
8:
	clr	r0
	mov	(sp)+,r5
	rts	pc
