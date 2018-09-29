/ @(#)copy.s	1.1
.globl	_copyio
_copyio:
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	mov	10(sp),r0
	mov	12(sp),r1
	ashc	$10.,r0
	mov	PS,-(sp)
	mov	$PRUSER+NOCLK,PS
	mov	r0,mmr
	mov	$1000,r0
	ashc	$6,r0
	mov	r0,r4
	clr	r0
	mov	16(sp),r1
	mov	20(sp),r2
	mov	22(sp),r3
	bic	$177771,r3
	ashc	$3,r0
	asl	r0
	add	cbase(r3),r0
	mov	40(r0),mmr+2
	mov	(r0)+,mmr+4
	bit	$17,r0
	bne	1f
	sub	$20,r0
1:
	mov	40(r0),mmr+6
	mov	(r0),mmr+8.
	mov	UISA0+[4*2],-(sp)
	mov	UISA0+[3*2],-(sp)
	mov	UISA0+[2*2],-(sp)
	mov	UISD0+[4*2],-(sp)
	mov	UISD0+[3*2],-(sp)
	mov	UISD0+[2*2],-(sp)
	mov	mmr,UISA0+[4*2]
	mov	$77406,UISD0+[4*2]
	mov	mmr+2,UISA0+[2*2]
	mov	mmr+4,UISD0+[2*2]
	mov	mmr+6,UISA0+[3*2]
	mov	mmr+8.,UISD0+[3*2]
	mov	$2,r0
	ashc	$13.,r0
	bit	$1,36(sp)
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
	bit	$1,r1
	bne	1f
	bit	$1,r0
	beq	gcete
	dec	r0
	br	gcote
1:
	dec	r1
	dec	r2
	bit	$1,r0
	beq	gceto
gcoto:
	dec	r0
	mfpi	(r1)
	mov	(sp)+,r3
	mfpi	(r0)+
	movb	r3,(sp)
	mtpi	(r1)+
gcete:
	asr	r2
	beq	2f
1:
	mfpi	(r0)+
	mtpi	(r1)+
	sob	r2,1b
2:
	bcc	9f
	mfpi	(r0)
	mov	(sp)+,r3
	br	8f
gceto:
	mfpi	(r0)
	mov	(sp)+,r3
	mfpi	(r1)
	movb	r3,1(sp)
	mtpi	(r1)+
gcote:
	mfpi	(r0)+
	mov	(sp)+,r3
	mov	r2,gccnt
	asr	r2
	beq	2f
1:
	mfpi	(r0)+
	mov	(sp),r4
	clrb	r3
	bisb	r4,r3
	swab	r3
	mov	r3,(sp)
	mtpi	(r1)+
	mov	r4,r3
	sob	r2,1b
2:
	asr	gccnt
	bcc	9f
	swab	r3
8:
	mfpi	(r1)
	movb	r3,(sp)
	mtpi	(r1)
9:
	clr	r0
1:
	mov	(sp)+,nofault
	mov	(sp)+,UISD0+[2*2]
	mov	(sp)+,UISD0+[3*2]
	mov	(sp)+,UISD0+[4*2]
	mov	(sp)+,UISA0+[2*2]
	mov	(sp)+,UISA0+[3*2]
	mov	(sp)+,UISA0+[4*2]
	mov	(sp)+,PS
	mov	(sp)+,r2
	mov	(sp)+,r3
	mov	(sp)+,r4
	rts	pc
cerr:
	mov	$-1,r0
	br	1b

.data
cbase:
	UISD0
	KISD0
	UISD0
	KISD0
gccnt:
	0
.text

.globl	_copyseg
_copyseg:
	mov	PS,-(sp)
	mov	UISA0,-(sp)
	mov	UISA0+[1*2],-(sp)
	mov	$PRUSER+NOCLK,PS
	mov	10(sp),UISA0
	mov	12(sp),UISA0+[1*2]
	mov	UISD0,-(sp)
	mov	UISD0+[1*2],-(sp)
	mov	$6,UISD0
	mov	$6,UISD0+[1*2]
	mov	r2,-(sp)
	clr	r0
	mov	$8192.,r1
	mov	$32.,r2
1:
	mfpi	(r0)+
	mtpi	(r1)+
	sob	r2,1b
	mov	(sp)+,r2
	mov	(sp)+,UISD0+[1*2]
	mov	(sp)+,UISD0
	mov	(sp)+,UISA0+[1*2]
	mov	(sp)+,UISA0
	mov	(sp)+,PS
	rts	pc

.globl	_clearseg
_clearseg:
	mov	PS,-(sp)
	mov	UISA0,-(sp)
	mov	$PRUSER+NOCLK,PS
	mov	6(sp),UISA0
	mov	UISD0,-(sp)
	mov	$6,UISD0
	clr	r0
	mov	$32.,r1
1:
	clr	-(sp)
	mtpi	(r0)+
	sob	r1,1b
	mov	(sp)+,UISD0
	mov	(sp)+,UISA0
	mov	(sp)+,PS
	rts	pc

.globl	_clear
_clear:
	mov	PS,-(sp)
	mov	r2,-(sp)
	mov	6(sp),r0
	mov	10(sp),r1
	ashc	$10.,r0
	mov	UISA0+[4*2],-(sp)
	mov	UISD0+[4*2],-(sp)
	mov	$PRUSER+NOCLK,PS
	mov	r0,UISA0+[4*2]
	mov	$77406,UISD0+[4*2]
	mov	$1000,r0
	ashc	$6,r0
	mov	16(sp),r1
	bit	$1,r0
	beq	1f
	dec	r1
	dec	r0
	mfpi	(r0)
	clrb	1(sp)
	mtpi	(r0)+
1:
	mov	r1,r2
	asr	r1
	beq	2f
1:
	clr	-(sp)
	mtpi	(r0)+
	sob	r1,1b
2:
	asr	r2
	bcc	1f
	mfpi	(r0)
	clrb	(sp)
	mtpi	(r0)+
1:
	mov	(sp)+,UISD0+[4*2]
	mov	(sp)+,UISA0+[4*2]
	mov	(sp)+,r2
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
	mfpi	(r0)+
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
	mfpi	(r0)+
	mov	(sp)+,(r1)+
	sob	r2,1b
	br	4f
2:
	mfpi	(r0)+
	movb	(sp),(r1)+
	swab	(sp)
	movb	(sp)+,(r1)+
	sob	r2,2b
4:
	asr	r3
	bcc	7f
	mfpi	(r0)
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
	mfpi	(r1)
	movb	(r0)+,1(sp)
	mtpi	(r1)+
	dec	r2
1:
	mov	r2,r3
	asr	r2
	beq	4f
	bit	$1,r0
	bne	2f
1:
	mov	(r0)+,-(sp)
	mtpi	(r1)+
	sob	r2,1b
	br	4f
2:
	movb	(r0)+,-(sp)
	movb	(r0)+,1(sp)
	mtpi	(r1)+
	sob	r2,2b
4:
	asr	r3
	bcc	7b
	mfpi	(r1)
	movb	(r0)+,(sp)
	mtpi	(r1)
	br	7b
