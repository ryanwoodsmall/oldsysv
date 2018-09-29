	.file	"ecvt.s"
.data
.text
.globl	_ecvt
_ecvt:
	.enter
	movqd	0, tos
	movd	24(fp), tos
	movd	20(fp), tos
	movd	16(fp), tos
	movl	8(fp), tos
	jsr	_cvt
	adjspb	$-24
	br	.L13
.L13:
	exit	[]
	ret	0
	enter	[], 0
.data
.text
.globl	_fcvt
_fcvt:
	.enter
	movqd	1, tos
	movd	24(fp), tos
	movd	20(fp), tos
	movd	16(fp), tos
	movl	8(fp), tos
	jsr	_cvt
	adjspb	$-24
	br	.L17
.L17:
	exit	[]
	ret	0
	enter	[], 0
.data
_buf:
	.space	328
	.double	0x0
.text
_cvt:
	.enter
	movd	20(fp),r7
	movd	$_buf, r6
	movd	16(fp),r0
	addd	$_buf, r0
	movd	r0, r5
	cmpl	8(fp),$0l0.00000000000000000000e+00
	bge	.L9999
	movqd	1,r0
	br	.L9998
.L9999:
	movqd	0,r0
.L9998:
	movd	r0, 0(24(fp))
	movd	0(24(fp)), r1
	cmpqd	0, r1
	beq	.L24
	negl	8(fp),f0
	movl	f0, 8(fp)
.L24:
	movqb	0, _buf
	movqd	0, 0(r7)
	cmpl	8(fp),$0l0.00000000000000000000e+00
	beq	.L25
.data
.align	4
.L26:
	.long	0l1.00000000000000000000e+32
	.double	32
	.long	0l1.00000000000000000000e+16
	.double	16
	.long	0l1.00000000000000000000e+08
	.double	8
	.long	0l1.00000000000000000000e+04
	.double	4
	.long	0l1.00000000000000000000e+02
	.double	2
	.long	0l1.00000000000000000000e+01
	.double	1
.text
	movd	$.L26, r4
	addqd	1, 0(r7)
	cmpl	8(fp),$0l9.00719925474099260000e+15
	blt	.L27
.L30:
	cmpl	8(fp),0(r4)
	blt	.L32
.L33:
	divl	0(r4),8(fp)
	addd	8(r4),0(r7)
	cmpl	8(fp),0(r4)
	bge	.L33
.L32:
.L29:
	movd	r4, r0
	addr	12(r4), r4
	cmpqd	1, 8(r0)
	blt	.L30
.L28:
	br	.L34
.L27:
	cmpl	8(fp),$0l1.00000000000000000000e+01
	blt	.L35
	movl	$0l1.00000000000000000000e+01, -8(fp)
	br	.L37
.L38:
	movl	-16(fp), -8(fp)
.L37:
	movl	-8(fp),f0
	mull	$0l1.00000000000000000000e+01,f0
	movl	f0, -16(fp)
	cmpl	-16(fp),8(fp)
	ble	.L38
.L36:
.L41:
	movd	
