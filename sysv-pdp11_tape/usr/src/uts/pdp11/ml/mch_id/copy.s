/ @(#)copy.s	1.1
.globl	_copyio
_copyio:
	mov	PS,-(sp)
	mov	$4340,PS
	mov	sp,r5
	cmp	(r5)+,(r5)+
	mov	(r5)+,r0
	mov	(r5)+,r1
	ashc	$10.,r0
	mov	r0,SISA0+[4*2]
	mov	$1000,r0
	ashc	$6,r0
	mov	r0,r4
	clr	r0
	mov	(r5)+,r1
	mov	(r5)+,r2
	mov	(r5),r3
	bic	$177771,r3
	ashc	$3,r0
	asl	r0
	add	cbase(r3),r0
	mov	40(r0),SISA0+[2*2]
	mov	(r0)+,SISD0+[2*2]
	bit	$17,r0
	bne	1f
	sub	$20,r0
1:
	mov	40(r0),SISA0+[3*2]
	mov	(r0),SISD0+[3*2]
	mov	$2,r0
	ashc	$13.,r0
	bit	$1,(r5)
	bne	1f
	mov	r4,r1
	br	2f
1:
	mov	r0,r1
	mov	r4,r0
gcio:
2:
	mov	nofault,-(sp)
	mov	$cerr,nofault
	mov	$44340,PS
	bit	$1,r1
	bne	1f
	bit	$1,r0
	beq	gcete
	br	gcote
1:
	bit	$1,r0
	beq	gceto
gcoto:
	dec	r2
	movb	(r0)+,(r1)+
gcete:
	asr	r2
	beq	2f
1:
	mov	(r0)+,(r1)+
	sob	r2,1b
2:
	bcc	9f
	movb	(r0)+,(r1)+
	br	9f
gceto:
gcote:
1:
	movb	(r0)+,(r1)+
	sob	r2,1b
9:
	clr	PS
	clr	r0
1:
	mov	(sp)+,nofault
	mov	(sp)+,PS
	rts	pc
cerr:
	clr	PS
	mov	$-1,r0
	br	1b

.data
cbase:
	UDSD0
	KDSD0
	UISD0
	KISD0
.text

.globl	_copyseg
_copyseg:
	mov	PS,-(sp)
	spl	7
	mov	4(sp),SISA0+[5*2]
	mov	6(sp),SISA0+[4*2]
	mov	r2,-(sp)
	mov	$120000,r0
	mov	$100000,r1
	mov	$32.,r2
	mov	$40340,PS
1:
	mov	(r0)+,(r1)+
	sob	r2,1b
	clr	PS
	mov	(sp)+,r2
	mov	(sp)+,PS
	rts	pc

.globl	_clearseg
_clearseg:
	mov	PS,-(sp)
	spl	7
	mov	4(sp),SISA0+[5*2]
	mov	$40340,PS
	mov	$120000,r0
	mov	$32.,r1
1:
	clr	(r0)+
	sob	r1,1b
	clr	PS
	mov	(sp)+,PS
	rts	pc


.globl	_clear
_clear:
	mov	PS,-(sp)
	mov	$4340,PS
	mov	sp,r5
	cmp	(r5)+,(r5)+
	mov	(r5)+,r0
	mov	(r5)+,r1
	ashc	$10.,r0
	mov	r0,SISA0+[4*2]
	mov	$1000,r0
	ashc	$6,r0
	mov	(r5)+,r1
	mov	$44340,PS
	bit	$1,r0
	beq	1f
	dec	r1
	clrb	(r0)+
1:
	mov	r1,r2
	asr	r1
	beq	2f
1:
	clr	(r0)+
	sob	r1,1b
2:
	asr	r2
	bcc	1f
	clrb	(r0)+
1:
	clr	PS
	mov	(sp)+,PS
	rts	pc

.globl	_copyin, _copyout
_copyin:
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	6(sp),r0
	mov	10(sp),r1
	mov	12(sp),r2
	mov	nofault,-(sp)
	mov	$9f,nofault
	bit	$1,r0
	beq	1f
	dec	r0
	mfpd	(r0)+
	movb	1(sp),(r1)+
	tst	(sp)+
	dec	r2
1:
	mov	r2,r3
	asr	r2
	beq	4f
	bit	$1,r1
	bne	2f
1:
	mfpd	(r0)+
	mov	(sp)+,(r1)+
	sob	r2,1b
	br	4f
2:
	mfpd	(r0)+
	movb	(sp),(r1)+
	swab	(sp)
	movb	(sp)+,(r1)+
	sob	r2,2b
4:
	asr	r3
	bcc	7f
	mfpd	(r0)
	movb	(sp)+,(r1)+
7:
	clr	r0
8:
	mov	(sp)+,nofault
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc
9:
	mov	$-1,r0
	br	8b

_copyout:
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	6(sp),r0
	mov	10(sp),r1
	mov	12(sp),r2
	mov	nofault,-(sp)
	mov	$9b,nofault
	bit	$1,r1
	beq	1f
	dec	r1
	mfpd	(r1)
	movb	(r0)+,1(sp)
	mtpd	(r1)+
	dec	r2
1:
	mov	r2,r3
	asr	r2
	beq	4f
	bit	$1,r0
	bne	2f
1:
	mov	(r0)+,-(sp)
	mtpd	(r1)+
	sob	r2,1b
	br	4f
2:
	movb	(r0)+,-(sp)
	movb	(r0)+,1(sp)
	mtpd	(r1)+
	sob	r2,2b
4:
	asr	r3
	bcc	7b
	mfpd	(r1)
	movb	(r0)+,(sp)
	mtpd	(r1)
	br	7b
