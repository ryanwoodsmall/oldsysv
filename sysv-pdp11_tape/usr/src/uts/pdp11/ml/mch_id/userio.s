/ @(#)userio.s	1.1
.globl	_fubyte, _subyte
.globl	_fuword, _suword
.globl	_fuiword, _suiword
nofulow:
_fubyte:
	mov	2(sp),r0
	asr	r0
	bcs	1f
	asl	r0
	mfpd	(r0)
	mov	(sp)+,r0
	bic	$!377,r0
	rts	pc
1:
	asl	r0
	mfpd	(r0)
	mov	(sp)+,r0
	clrb	r0
	swab	r0
	rts	pc

_fuword:
	mfpd	*2(sp)
	mov	(sp)+,r0
	rts	pc

_fuiword:
	mfpi	*2(sp)
	mov	(sp)+,r0
	rts	pc

_subyte:
	mov	2(sp),r1
	bic	$1,r1
	mfpd	(r1)
	cmp	r1,4(sp)
	beq	1f
	movb	6(sp),1(sp)
	br	2f
1:
	movb	6(sp),(sp)
2:
	mtpd	(r1)
	clr	r0
	rts	pc

_suword:
	mov	4(sp),r0
	mov	r0,-(sp)
	mtpd	*2(sp)
	rts	pc

_suiword:
	mov	4(sp),r0
	mov	r0,-(sp)
	mtpi	*2(sp)
	rts	pc
nofuhigh:

nofuerr:
	mov	$-1,r0
	rts	pc
