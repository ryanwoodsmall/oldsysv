/ @(#)dump_rk.s	1.1
/ Disk dump for RK05

RKER	= 2
RKCS	= 4
RKWC	= 6
RKBA	= 10
RKDA	= 12

/ save registers in low core and
/ write all core onto RK05

.globl	dump_rk
.globl	_dumpdev
.globl	_rk_addr
.globl	_cputype
dump_rk:
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

/ dump memory onto RK05

2:
	mov	_rk_addr,r0
	clr	RKBA(r0)
	movb	_dumpdev,r1
	bic	$177770,r1
	ash	$13.,r1
	mov	r1,RKDA(r0)

1:
	mov	$-[1024.*28.],RKWC(r0)
	mov	$3,RKCS(r0)
	tst	RKER(r0)
	bgt	1b

	br	.
