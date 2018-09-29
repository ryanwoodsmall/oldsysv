/	@(#)ffltpr.s	1.2
/ C library-- fake floating output

.globl	fltcvt
fltcvt:
	add	$8,r4
	movb	$'?,(r3)+
	mov	r3,r0
	rts	pc
