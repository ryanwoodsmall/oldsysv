/ @(#)dump_hp.s	1.1
/ dump for RP04
/ save registers in low core and
/ write all core onto swap area.

.data
.globl	dump_hp
.globl	_hp_addr, _cputype, _swapdev
dump_hp:
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


2:
	clr	r1
	mov	_hp_addr,r0
	mov	_swapdev,r2
	ash	$-3,r2
	mov	r2,10(r0)
	mov	$21,(r0)
	mov	$10000,32(r0)
1:
	cmp	$70.,_cputype
	bne	2f
	mov	r1,r2
	ash	$-7,r2
	mov	r2,50(r0)
2:
	mov	$-512.,2(r0)
	mov	r1,r2
	ash	$9.,r2
	mov	r2,4(r0)
	mov	r1,r3
	clr	r2
	add	$5000.,r3
	adc	r2
	div	$22.,r2
	mov	r3,r4
	mov	r2,r3
	clr	r2
	div	$19.,r2
	swab	r3
	bis	r3,r4
	mov	r4,6(r0)
	mov	r2,34(r0)
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
	add	$2,r1
	br	1b
2:
	bit	$4000,10(r0)
	beq	1b
	br	.
