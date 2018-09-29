/ @(#)csubr.s	1.1
.globl	csav
csav:
	mov	r5,(sp)
	mov	sp,r5		/ context switch!
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	tst	-(sp)
	mov	r0,pc

.globl cret
cret:
	mov	r5,r2
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc
