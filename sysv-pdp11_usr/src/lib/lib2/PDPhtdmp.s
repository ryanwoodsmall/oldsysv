/ @(#)PDPhtdmp.s	1.2
/ Mag tape dump for TU16
/ save registers in low core and
/ write all core onto mag tape.

.data
.globl	_htdmp
.globl	_ht_addr, _cputype
_htdmp:
	inc	$-1
	bne	2f

/ save regs r0,r1,r2,r3,r4,r5,r6,KIA6
/ starting at abs location 4

	mov	r0,4
	mov	$6,r0
	mov	r1,(r0)+
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	cmp	$40.,_cputype
	beq	1f
	mov	*$172374,(r0)+
	br	2f
1:
	mov	*$172354,(r0)+

/ dump memory onto tape, 9-track, 800bpi

2:
	clr	r1
	clr	r3
1:
	mov	_ht_addr,r0
	mov	$1300,32(r0)
1:
	cmp	$70.,_cputype
	bne	2f
	mov	r1,r2
	ash	$-7,r2
	mov	r2,34(r0)
2:
	mov	$-512.,6(r0)
	mov	r1,r2
	ash	$9.,r2
	mov	r2,4(r0)
	mov	$-256.,2(r0)
	mov	r1,r2
	asl	r2
	bic	$!1400,r2
	bis	$61,r2
	mov	r2,(r0)
2:
	tstb	(r0)
	bge	2b
	tst	(r0)
	blt	2f
	inc	r1
	br	1b
2:
	bit	$4000,10(r0)
	beq	2f
	inc	r3
2:
	mov	$40011,(r0)
	mov	$-1,6(r0)
	mov	$33,(r0)
2:
	tstb	12(r0)
	bge	2b
	tst	r3
	bne	1f
	mov	$25,(r0)
2:
	tstb	12(r0)
	bge	2b
	mov	$40011,(r0)
	br	1b
1:
	mov	$27,(r0)
2:
	tstb	12(r0)
	bge	2b
	mov	$7,(r0)
	br	.
