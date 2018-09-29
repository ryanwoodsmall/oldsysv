/	@(#)setjmp.s	1.4
/ C library -- setjmp, longjmp

/	longjmp(a,v)
/ will generate a "return(v)" from
/ the last call to
/	setjmp(a)
/ by restoring sp, r5, pc from `a'
/ and doing a return.
/

.globl	_setjmp
.globl	_longjmp

_setjmp:
	MCOUNT
	mov	2(sp),r0
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	(sp),(r0)
	clr	r0
	rts	pc

_longjmp:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r1
	mov	6(r5),r0
	bne	1f
	inc	r0		/ force non-zero return value
1:
	cmp	(sp),(r1)	/ if longjmp called from same function,
	beq	2f		/ don't restore register variables
1:
	cmp	(r5),(r1)
	beq	1f
	mov	(r5),r5
	bne	1b
	br	2f		/ old r2-r4 not available
1:
	mov	r5,r2
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2
2:
	mov	(r1)+,r5
	mov	(r1)+,sp
	mov	(r1),(sp)
	rts	pc
