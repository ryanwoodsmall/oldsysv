/ @(#)clist.s	1.1
.globl	_getc, _putc
.globl	_cfreelist
.globl	_wakeup

_getc:
	mov	r2,-(sp)
	mov	4(sp),r2
	mov	PS,-(sp)
	spl	6
	dec	(r2)+
	blt	2f
	mov	(r2),r1
	tst	(r1)+
	movb	(r1),r0
	incb	(r1)
	cmpb	(r1)+,(r1)+
	beq	3f
	add	r1,r0
	movb	(r0),r0
	bic	$!377,r0
1:
	mov	(sp)+,PS
	mov	(sp)+,r2
	rts	pc
2:
	inc	-(r2)
	mov	$-1,r0
	br	1b
3:
	add	r1,r0
	movb	(r0),r0
	bic	$!377,r0
	tst	-(r1)
	mov	-(r1),(r2)+
	bne	4f
	clr	(r2)
4:
	mov	_cfreelist,(r1)
	mov	r1,_cfreelist
	tst	_cfreelist+4
	beq	1b
	mov	r0,-(sp)
	clr	_cfreelist+4
	mov	$_cfreelist,-(sp)
	jsr	pc,_wakeup
	tst	(sp)+
	mov	(sp)+,r0
	br	1b

_putc:
	mov	r2,-(sp)
	mov	6(sp),r2
	mov	PS,-(sp)
	spl	6
	mov	4(r2),r1
	beq	3f
1:
	add	$3,r1
	movb	(r1),r0
	cmp	r0,_cfreelist+2
	bge	6f
	inc	(r2)
	incb	(r1)+
	add	r1,r0
	movb	6(sp),(r0)
	clr	r0
2:
	mov	(sp)+,PS
	mov	(sp)+,r2
	rts	pc
3:
	mov	_cfreelist,r1
	beq	5f
	mov	r1,2(r2)
4:
	mov	(r1),_cfreelist
	clr	(r1)
	clr	2(r1)
	mov	r1,4(r2)
	br	1b
5:
	mov	$-1,r0
	br	2b
6:
	mov	r1,r0
	mov	_cfreelist,r1
	beq	5b
	mov	r1,-3(r0)
	br	4b

.globl	_getcf, _putcf
_getcf:
	mov	PS,-(sp)
	spl	6
	mov	_cfreelist,r0
	beq	1f
	mov	(r0),_cfreelist
	clr	(r0)
	mov	_cfreelist+2,r1
	swab	r1
	mov	r1,2(r0)
1:
	mov	(sp)+,PS
	rts	pc

_putcf:
	mov	2(sp),r0
	mov	PS,-(sp)
	spl	6
	mov	_cfreelist,(r0)
	mov	r0,_cfreelist
	tst	_cfreelist+4
	beq	1f
	clr	_cfreelist+4
	mov	$_cfreelist,-(sp)
	jsr	pc,_wakeup
	tst	(sp)+
1:
	mov	(sp)+,PS
	rts	pc

.globl	_getcb, _putcb
_getcb:
	mov	r2,-(sp)
	mov	4(sp),r2
	mov	PS,-(sp)
	spl	6
	mov	2(r2),r0
	beq	1f
	tst	(r0)+
	movb	(r0)+,r1
	movb	(r0),r0
	sub	r1,r0
	sub	r0,(r2)+
	mov	(r2),r0
	mov	(r0),(r2)+
	bne	1f
	clr	(r2)
1:
	mov	(sp)+,PS
	mov	(sp)+,r2
	rts	pc

_putcb:
	mov	r2,-(sp)
	mov	4(sp),r0
	mov	6(sp),r2
	mov	PS,-(sp)
	spl	6
	mov	4(r2),r1
	bne	1f
	mov	r0,2(r2)
	br	2f
1:
	mov	r0,(r1)
2:
	mov	r0,4(r2)
	clr	(r0)+
	movb	(r0)+,r1
	movb	(r0),r0
	sub	r1,r0
	add	r0,(r2)
	clr	r0
	mov	(sp)+,PS
	mov	(sp)+,r2
	rts	pc
