/	@(#)dup.s	1.3
/ C library -- dup

/	f = dup(of [ ,nf])
/	f == -1 for error

.globl	_dup, cerror
.dup = 41.

_dup:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	6(r5),r1
	sys	.dup
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
