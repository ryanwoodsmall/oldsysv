/ @(#)bufio.s	1.1
.globl	_bigetc, _biputc
_bigetc:
	mov	PS,-(sp)
	jsr	pc,7f
	mfpi	(r0)
	mov	(sp)+,r0
	tst	r1
	bpl	1f
	swab	r0
1:
	bic	$!377,r0
	br	9f

_biputc:
	mov	PS,-(sp)
	jsr	pc,7f
	mfpi	(r0)
	tst	r1
	bpl	1f
	movb	16(sp),1(sp)
	br	8f
1:
	movb	16(sp),(sp)
	br	8f

.globl	_biget, _biput
_biget:
	mov	PS,-(sp)
	jsr	pc,7f
	mfpi	(r0)
	mov	(sp)+,r0
	br	9f

_biput:
	mov	PS,-(sp)
	jsr	pc,7f
	mov	14(sp),-(sp)
	br	8f

.globl	_bigetl, _biputl
_bigetl:
	mov	PS,-(sp)
	jsr	pc,7f
	mfpi	(r0)+
	mfpi	(r0)
	mov	(sp)+,r1
	mov	(sp)+,r0
	br	9f

_biputl:
	mov	PS,-(sp)
	jsr	pc,7f
	mov	14(sp),-(sp)
	mtpi	(r0)+
	mov	16(sp),r1
	mov	r1,-(sp)
8:
	mtpi	(r0)
	mov	14(sp),r0
9:
	mov	(sp)+,UISD0
	mov	(sp)+,UISA0
	mov	(sp)+,PS
	rts	pc
7:
	mov	6(sp),r0
	mov	20(r0),r1
	mov	16(r0),r0
	add	10(sp),r1
	adc	r0
	ashc	$10.,r0
	mov	(sp),-(sp)
	mov	(sp),-(sp)
	mov	UISA0,4(sp)
	mov	$PRUSER+NOCLK,PS
	mov	r0,UISA0
	mov	UISD0,2(sp)
	mov	$77406,UISD0
	clr	r0
	ashc	$5,r0
	asl	r0
	rts	pc
