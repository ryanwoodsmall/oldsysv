/ @(#)misc.s	1.1
.globl	_display
_display:
	dec	dispdly
	bge	2f
	clr	dispdly
	mov	PS,-(sp)
	mov	$HIPRI,PS
	mov	CSW,r1
	bit	$1,r1
	beq	1f
	bis	$PRUSER,PS
	dec	r1
1:
	mov	r1,-(sp)
	jsr	pc,_fuword
	tst	(sp)+
	mov	r0,CSW
	mov	(sp)+,PS
	cmp	r0,$-1
	bne	2f
	mov	$60.,dispdly		/ 1 sec delay after CSW fault
2:
	rts	pc

.globl	_addupc
_addupc:
	mov	r2,-(sp)
	mov	6(sp),r2	/ base of prof with base,leng,off,scale
	mov	4(sp),r0	/ pc
	sub	4(r2),r0	/ offset
	clc
	ror	r0
	mov	6(r2),r1
	clc
	ror	r1
	mul	r1,r0		/ scale
	ashc	$-14.,r0
	inc	r1
	bic	$1,r1
	cmp	r1,2(r2)	/ length
	bhis	1f
	add	(r2),r1		/ base
	mov	nofault,-(sp)
	mov	$2f,nofault
	mfpd	(r1)
	add	12.(sp),(sp)
	mtpd	(r1)
	br	3f
2:
	clr	6(r2)
3:
	mov	(sp)+,nofault
1:
	mov	(sp)+,r2
	rts	pc

.globl	_spl0, _spl1, _spl4, _spl5, _spl6, _spl7, _splx
_spl0:
	mov	PS,r0
	spl	0
	rts	pc

_spl1:
	mov	PS,r0
	spl	1
	rts	pc

_spl4:
	mov	PS,r0
	spl	4
	rts	pc

_spl5:
	mov	PS,r0
	spl	5
	rts	pc

_spl6:
	mov	PS,r0
	spl	6
	rts	pc

_spl7:
	mov	PS,r0
	spl	HIGH
	rts	pc

_splx:
	mov	2(sp),PS
	rts	pc
