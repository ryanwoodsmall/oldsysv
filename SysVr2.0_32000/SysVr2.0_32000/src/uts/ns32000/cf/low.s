	.data
	.align	2
	.globl	pl_isr
pl_isr:
	.word	0
	.word	0xffff8000
	.word	0xffff8000
	.word	0xffff8000
	.word	0x200
	.word	0x8
	.word	0x2
	.word	0x1
	.set	M4,0x2
	.set	A4,0xfffe12
	.set	M5,0x8
	.set	A5,0xfffe10
	.set	M6,0x2
	.set	A6,0xfffe10
	.set	M7,0x1
	.set	A7,0xfffe10
	.text
	.globl	v1
v1:	save	[r0,r1,r2,r3]
	movd	_cpl,tos
	movqd	6,_cpl
	bispsrw	$0x800
	jsr	_clock
	br	intret
	.globl	v9
v9:	save	[r0,r1,r2,r3]
	movd	_cpl,tos
	movqd	4,_cpl
	bispsrw	$0x800
	jsr	_conrint
	br	intret
	.globl	va
va:	save	[r0,r1,r2,r3]
	movd	_cpl,tos
	movb	$M4,A4
	movqd	4,_cpl
	bispsrw	$0x800
	jsr	_conxint
	br	intret
	.globl	v0
v0:	movd r0,tos
	movzbd 0,r0
	br vint
	.globl	v2
v2:	movd r0,tos
	movzbd 0x2,r0
	br vint
	.globl	v3
v3:	save	[r0,r1,r2,r3]
	movd	_cpl,tos
	movqd	5,_cpl
	bispsrw	$0x800
	jsr	_dcintr
	br	intret
	.globl	v4
v4:	save	[r0,r1,r2,r3]
	movd	_cpl,tos
	movb	$M5,A5
	movqd	5,_cpl
	bispsrw	$0x800
	jsr	_tcintr
	br	intret
	.globl	v5
v5:	movd r0,tos
	movzbd 0x5,r0
	br vint
	.globl	v6
v6:	movd r0,tos
	movzbd 0x6,r0
	br vint
	.globl	v7
v7:	movd r0,tos
	movzbd 0x7,r0
	br vint
	.globl	v8
v8:	movd r0,tos
	movzbd 0x8,r0
	br vint
	.globl	vb
vb:	movd r0,tos
	movzbd 0xb,r0
	br vint
	.globl	vc
vc:	save	[r0,r1,r2,r3]
	movd	_cpl,tos
	movb	$M4,A4
	movqd	4,_cpl
	bispsrw	$0x800
	jsr	_lprint
	br	intret
	.globl	vd
vd:	save	[r0,r1,r2,r3]
	movd	_cpl,tos
	movb	$M4,A4
	movqd	4,_cpl
	bispsrw	$0x800
	jsr	_lpxint
	br	intret
	.globl	ve
ve:	save	[r0,r1,r2,r3]
	movd	_cpl,tos
	movb	$M4,A4
	movqd	4,_cpl
	bispsrw	$0x800
	jsr	_siointr
	br	intret
intret:
	bicpsrw	$0x800
	movd	tos,r1
	tbitb	$0,23(sp)		#trap from user mode?
	bfc	realout
	movb	$0xb,23(sp)	#set PSR bits (hardware bug)
	cmpqb	0,_runrun
	beq	realout
	orb	$2,softype
	movb	$0x8f,0xfffe0e	#generate resched int
	movqb	0,_runrun
realout:
	movd	r1,_cpl
	movw	pl_isr[r1:w],r1
	movb	r1,0xfffe10
	lshw	$-8,r1
	movb	r1,0xfffe12
	restore	[r0,r1,r2,r3]
	rett	0
vint:	save	[r1,r2,r3]
	movd	r0,tos
	jsr	_logstray
	adjspb	$-4
	restore	[r0,r1,r2,r3]
	reti
