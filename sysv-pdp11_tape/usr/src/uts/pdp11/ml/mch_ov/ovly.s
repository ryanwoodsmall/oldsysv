/ @(#)ovly.s	1.1
/	ovcsav, ovcret, ovhndlr for overlayed UNIX

.globl	ovcsav, ovcret, ovhndlr, __ovno, __ova, __ovd

.data

__ovno:	0		/ current overlay number
__ova:	. = . + 16.	/ physical addresses for overlays
__ovd:	. = . + 16.	/ mm descriptors for overlays

.text

ovcsav	=	csav	/ the same at present

ovcret:
	bit	$1,2(r5)
	bne	1f
	tst	(r5)
	bpl	2f

/ normal return
/ mimic cret

	mov	r5,r2
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc

/ return to root segment
/ note return to root segment, mimic cret

1:	clr	__ovno
/*	clr	KISA0+[7*2]
/*	clr	KISD0+[7*2]
	mov	r5,r2
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2
	mov	r5,sp
	mov	(sp)+,r5
	dec	(sp)
	rts	pc

/ return to an overlay segment
/ fix overlay addressing, restore R5, mimic cret

2:	mov	(r5),r2
	ash	$-11.,r2
.if	OVDEBUG
	tst	r2
	ble	8f-2
	cmp	$14.,r2
	bge	8f
	0
8:
.endif
	mov	r2,__ovno
	mov	__ova(r2),KISA0+[7*2]
	mov	__ovd(r2),KISD0+[7*2]
	bic	$174000,(r5)
	bis	$140000,(r5)	/ r5 now restored
	mov	r5,r2
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc		/ return to overlay segment

/	on entry
/		r1	number of overlay to be entered (times 2)
/		r0	virtual entry address PLUS 4
/		(sp)	return pc

ovhndlr:

/ mimic csav entry sequence

	mov	r5,-(sp)	/ NOTE difference to csav
	mov	sp,r5		/ context switch!
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	tst	-(sp)

/ setup for entry to new overlay segment

	mov	__ovno,r2
	bne	1f		/ must save overlay number

	inc	2(r5)		/ set LOW bit saved PC
	br	2f

1:	bic	$174000,(r5)	/ top 5 bits r5 KNOWN, make room for ovno
	ash	$11.,r2
	bis	r2,(r5)		/ save current overlay number

2:
.if	OVDEBUG
	tst	r1
	ble	8f-2
	cmp	$14.,r1
	bge	8f
	0
8:
.endif
	mov	r1,__ovno	/ note new overlay number
	mov	__ova(r1),KISA0+[7*2]
	mov	__ovd(r1),KISD0+[7*2]

3:	mov	r0,pc		/ enter overlay, skipping ovcsav call
