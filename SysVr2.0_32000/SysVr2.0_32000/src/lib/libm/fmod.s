	.file	"fmod.s"
.data
.text
.globl	_fmod
_fmod:
	br	.L48
.L49:
	cmpl	8(fp),$0l0.00000000000000000000e+00
	bge	.L9999
	negl	8(fp),f0
	movl	f0, -8(fp)
	br	.L9998
.L9999:
	movl	8(fp), -8(fp)
.L9998:
	cmpl	16(fp),$0l0.00000000000000000000e+00
	bge	.L9997
	negl	16(fp),f0
	br	.L9996
.L9997:
	movl	16(fp),f0
.L9996:
	movl	-8(fp),f2
	subl	f0,f2
	cmpl	f2,-8(fp)
	bne	.L50
	movl	$0l0.00000000000000000000e+00,f0
	br	.L47
.L50:
	addr	-8(fp), r0
	movd	r0, tos
	movl	8(fp),f0
	divl	16(fp),f0
	movl	f0, tos
	jsr	_modf
	adjspb	$-12
	floorl
