	.globl	_icuinit
	.globl	_vector
_icuinit:
	movb	0xffe880,r0	#tweak memory board error regs
	movb	0xffe680,r0
	movb	0xffe480,r0
	movqb	1,0xffee00	#tweak diag reg.
	movb	$0x3f,0xffee00

	addr	_vector,r2
	addr	32,r0
	sprw	mod,r1		#current module
lpmod:
	movw	r1,-4(r2)[r0:d]
	acbb	-1,r0,lpmod	#stuff module into vectors

	sprd	intbase,r1	# rom monitor base
	cmpqd	0,_kdebug
	beq	norom
# borrow monitor vector for bpt
	movd	32(r1),32(r2)
norom:
	lprd	intbase,r2

# set up icu registers
	movd	$0xfffe00,r2	# address of ICU
	movb	$0x80,0x2c(r2)
	movb	$0x40,0x30(r2)
	movb	$0x78,0x32(r2)
	movb	$0x7d,0x34(r2)
	movb	$0x1,0x36(r2)
	movb	$0x40,0x38(r2)
	movb	$0x78,0x3a(r2)
	movb	$0x7d,0x3c(r2)
	movb	$0x1,0x3e(r2)
	movb	$0x42,0x20(r2)
	movb	$0x0,0x2c(r2)
	movb	$0x11,0x2e(r2)
	movb	$0xff,0x24(r2)
	movb	$0xff,0x28(r2)
	movb	$0xff,0x2a(r2)
	movb	$0x0,0x22(r2)
	movb	$0x0,0x26(r2)
	movb	$0x0,0x18(r2)
	movb	$0x10,0x2(r2)
	movb	$0x40,0xc(r2)
	movb	$0x40,0xe(r2)
	movb	$0xe5,0x4(r2)
	movb	$0xcb,0x6(r2)
	movb	$0x40,0x8(r2)
	movb	$0x20,0xa(r2)
	movb	$0x0,0x1c(r2)
	movb	$0xff,0x14(r2)
	movb	$0xff,0x16(r2)
	movb	$0x2,0x20(r2)
	ret	0

	.globl	_icuenable
_icuenable:
	movd	$0xfffe00,r0
	bicb	4(sp),0x14(r0)
	bicb	5(sp),0x16(r0)
	ret	0

	.globl	_icudisable
_icudisable:
	movd	$0xfffe00,r1
	movb	0x16(r1),r0
	lshw	$8,r0
	movb	0x14(r1),r0
	movw	r0,r2
	orw	4(sp),r2
	movb	r2,0x14(r1)
	lshw	$-8,r2
	movb	r2,0x16(r1)
	ret	0

# interrupt vector table
	.align	4
_vector:
	.word	0
	.word	h0
	.word	0
	.word	h1
	.word	0
	.word	h2
	.word	0
	.word	h3
	.word	0
	.word	h4
	.word	0
	.word	h5
	.word	0
	.word	h6
	.word	0
	.word	h7
	.word	0
	.word	h8
	.word	0
	.word	h9
	.word	0
	.word	ha
	.word	0
	.word	hb
	.word	0
	.word	hc
	.word	0
	.word	hd
	.word	0
	.word	he
	.word	0
	.word	hf
	.word	0
	.word	v0
	.word	0
	.word	v1
	.word	0
	.word	v2
	.word	0
	.word	v3
	.word	0
	.word	v4
	.word	0
	.word	v5
	.word	0
	.word	v6
	.word	0
	.word	v7
	.word	0
	.word	v8
	.word	0
	.word	v9
	.word	0
	.word	va
	.word	0
	.word	vb
	.word	0
	.word	vc
	.word	0
	.word	vd
	.word	0
	.word	ve
	.word	0
	.word	vf

h0:	addr	0x0,tos
	br	alltraps
h1:	addr	0x1,tos
	br	alltraps
h3:	addr	0x3,tos
	br	alltraps
h4:	addr	0x4,tos
	br	alltraps
# system call
h5:	addr	0x5,tos
	movd	r0,tos		#save r0
	tbitb	$0,15(sp)	# from user mode?
	bfc	suptrap
	movmd	tos,USTACK-16,4	#move stuff to ublk stack
	adjspb	$-16		#pop intr stack
	bispsrw	$0x200		#use usp
	sprd	sp,r0		#r0 = usp
	lprd	sp,$USTACK-16	#ublock
	save	[r0,r1,r2,r3]
	jsr	_syscall
	br	comret
alltraps:
	movd	r0,tos		#save r0
	tbitb	$0,15(sp)	# from user mode?
	bfc	suptrap
	movmd	tos,USTACK-16,4	#move stuff to ublk stack
	adjspb	$-16		#pop intr stack
	bispsrw	$0x200		#use usp
	sprd	sp,r0		#r0 = usp
	lprd	sp,$USTACK-16	#ublock
	save	[r0,r1,r2,r3]
call_tr:
	jsr	_trap
comret:
	restore	[r0,r1,r2,r3]
	lprd	sp,r0		#restore usp
	bicpsrw	$0x200		#intr stack
	adjspb	$16
	movmd	USTACK-16,tos,4
	movd	tos,r0
	adjspb	$-4
goout:
	tbitb	$0,7(sp)		# from user?
	bfc	rett_adr
	movb	$0xb,7(sp)	# fix hardware problem
rett_adr:
	rett	0
# supervisor mode trap - we will panic, so dont switch stacks
suptrap:
	sprd	sp,r0
	save	[r0,r1,r2,r3]
	jsr	_trap
	bpt			# should never return

h6:	addr	0x6,tos
	br	alltraps
h7:	addr	0x7,tos
	br	alltraps
h8:	addr	0x8,tos
	br	alltraps
h9:	addr	0x9,tos
	br	alltraps
ha:	addr	0xa,tos
	br	alltraps
hb:	addr	0xb,tos
	br	alltraps
hc:	addr	0xc,tos
	br	alltraps
hd:	addr	0xd,tos
	br	alltraps
he:	addr	0xe,tos
	br	alltraps
hf:	addr	0xf,tos
	br	alltraps


vf:
	tbitb	$0,softype
	bfc	vf2
	bicb	$1,softype
	# soft clock
	save	[r0,r1,r2,r3]
	movd	_cpl,tos
	movqd	1,_cpl
	bispsrw	$0x800
	jsr	_timein
	movd	tos,_cpl
	restore	[r0,r1,r2,r3]
	bicpsrw	$0x800

vf2:	tbitb	$0,7(sp)	# trap from user mode?
	bfc	ur
	cmpqb	0,softype
	bne	uf
ur:
	movb	$0,0xfffe12
	br	goout
uf:	tbitb	$1,softype
	bfc	af
	addr	15,tos	# resched
	br	pd
af:
	tbitb	$2,softype
	bfc	ur
	addr	18,tos	# addup
pd:
	movqb	0,softype
	movb	$0,0xfffe12	#clear int pending bit (doing rett)
	bispsrw	$0x800		#enable ints
	br	alltraps

	.data
	.globl	_waitloc
	.text

	.globl	_spl0
	.globl	_spl4
	.globl	_spl5
	.globl	_spl6
	.globl	_spl7
	.globl	_splhi
	.globl	_splx
_spl0:
	movqd	0,r1
	br	xspl

_spl4:
	movqd	4,r1
	br	xspl

_spl5:
	movqd	5,r1
	br	xspl

_spl6:
	movqd	6,r1
	br	xspl

_spl7:
_splhi:
	movqd	7,r1
	br	xspl

_splx:
	movd	4(sp),r1
# fall through

xspl:
	addr	0xfffe10,r2
	movd	_cpl,r0
	bicpsrw	$0x800
	movd	r1,_cpl
	movw	pl_isr[r1:w],r1
	movb	r1,0(r2)
	lshw	$-8,r1
	movb	r1,2(r2)
	bispsrw	$0x800
	ret	0

	.globl	_idle
_idle:
	jsr	_spl0
	movd	$0xfffe00,r2	#make sure ICU is still sane (hardware bug)
	cmpqb	2,0x20(r2)
	beq	icu_ok
	movqb	2,0x20(r2)
icu_ok:
	wait
_waitloc:
	movd	r0,tos
	jsr	_splx
	adjspb	$-4
	ret	0

	.globl	_timepoke
_timepoke:
	orb	$1,softype
	movb	$0x8f,0xfffe0e
	ret	0

	.globl	_profon
_profon:
	orb	$4,softype
	movb	$0x8f,0xfffe0e
	ret	0

	.globl	_kdebug
	.globl	_cpl
	.globl	softype
	.data
softype:
	.space	4
	.align	2
_cpl:
	.space	4
_kdebug:
	.double	0
	.text
