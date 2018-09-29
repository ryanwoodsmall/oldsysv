/	@(#)maus.s	1.3
/ C library -- maus

/ mdes  = getmaus(string, mode)
/ error = freemaus(mausdes)
/ vaddr = enabmaus(mdes)
/ error = dismaus(vaddr)
/ mdes	= switmaus(mdes, vaddr)
/ mdes	= switmaus(-1, vaddr)
/ vaddr = switmaus(mdes,-1)
/ error = switmaus(-1,-1)
/
/ error indication is a -1 return in all cases

.globl	_getmaus, _enabmaus, _dismaus, _freemaus, _switmaus, cerror

maus = 58.
freem = 3
switm = 4

_switmaus:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	6(r5),0f
	br	1f

_dismaus:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	$-1,r1
	mov	4(r5),0f
	br	2f

_enabmaus:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	$-1,0f
1:
	mov	4(r5),r1
2:
	movb	r1,mdes		/ Save MAUS descriptor.
	mov	$switm,r0
	sys	0; 9f
	bec	1f
	jmp	cerror

/ Successful call - set up proper return to user.

1:
	cmpb	$-1,mdes	/ was mdes given ?
	bne	1f
	decb	mdes
	mov	0f,r0
	cmp	$-1,r0		/ check vaddr given by caller
	bne	1f
	inc	r0
	br	2f
1:
	mov	r0,r1
	ash	$-13.,r1
	bic	$177770,r1
	mov	r2,-(sp)
	movb	sdes(r1),r2	/ r2 = old mdes
	movb	mdes,sdes(r1)	/ save new mdes

/ Return old MAUS descriptor or vaddr as necessary.

	cmp	$-1,0f
	beq	1f		/ no vaddr given
	mov	r2,r0		/ return old MAUS descriptor.
1:
	mov	(sp)+,r2
2:
	mov	(sp)+,r5
	rts	pc

_freemaus:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	$freem,r0
	br	1f

_getmaus:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	6(r5),r0
1:
	mov	4(r5),0f
2:
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc

.data

mdes:	. = .+1
sdes:	.byte	-2,-2,-2,-2,-2,-2,-2,-2

.even
9:
	sys	maus; 0:..
