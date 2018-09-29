/ @(#)dump_tm.s	1.1
/ Mag tape dump for TU10
/ save registers in low core and
/ write all core onto mag tape.

.data
.globl	dump_tm
.globl	_tm_addr
.globl	_cputype
dump_tm:
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
	mov	_tm_addr,r0
	tst	(r0)+
	mov	r1,r2
	ash	$9.,r2
	mov	r2,4(r0)
	mov	$-512.,2(r0)
	mov	r1,r2
	ash	$-3,r2
	bic	$!60,r2
	bis	$60005,r2
	mov	r2,(r0)
2:
	tstb	(r0)
	bge	2b
	tst	(r0)+
	blt	2f
	inc	r1
	br	1b
2:
	bit	$10000,6(r0)
	bne	2b
	tstb	-4(r0)
	bge	2f
	inc	r3
2:
	mov	$-1,(r0)
	mov	$60013,-(r0)
2:
	tstb	(r0)
	bge	2b
	tst	r3
	beq	1b
	mov	$60007,(r0)
2:
	tstb	(r0)
	bge	2b
	mov	$60017,(r0)
	br	.
