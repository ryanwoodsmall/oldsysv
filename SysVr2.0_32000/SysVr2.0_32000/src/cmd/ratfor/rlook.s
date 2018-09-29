	.file	"rlook.c"
.data
.globl	_hshtab
.comm	_hshtab,404
.globl	_hshval
.comm	_hshval,4
.text
.globl	_lookup
_lookup:
	br	.L18
.L19:
.data
.align	4
.L20:
	.space	16
.text
	movd	8(fp), r7
	movqd	0, _hshval
	cmpqb	0, 0(r7)
	beq	.L22
.L23:
	movxbd	0(r7),r0
	addd	r0,_hshval
	addqd	1, r7
	cmpqb	0, 0(r7)
	bne	.L23
.L22:
	remd	$101,_hshval
	movd	_hshval, r0
	ashd	$2,r0
	movd	_hshtab(r0), r5
	cmpqd	0, r5
	beq	.L25
.L26:
	movd	8(fp), r7
	movd	0(r5), r6
	br	.L28
.L29:
	movd	r6, r0
	addqd	1, r6
	cmpqb	0, 0(r0)
	bne	.L30
	movd	r5, r0
	br	.L17
.L30:
.L28:
	movd	r7, r0
	addqd	1, r7
	cmpb	0(r0),0(r6)
	beq	.L29
.L27:
	movd	12(r5), r5
	cmpqd	0, r5
	bne	.L26
.L25:
	movd	$.L20, r0
	br	.L17
.L17:
	exit	[r5,r6,r7]
	ret	0
.L18:
	enter	[r5,r6,r7], 0
	br	.L19
.data
.text
.globl	_install
_install:
	br	.L32
.L33:
	movd	8(fp), tos
	jsr	_lookup
	adjspb	$-4
	movd	r0, r7
	movd	r7, r1
	cmpqd	0, 0(r1)
	bne	.L34
	addr	16, tos
	jsr	_malloc
	adjspb	$-4
	movd	r0, r7
	movd	8(fp), tos
	jsr	_copy
	adjspb	$-4
	movd	r0, 0(r7)
	movd	12(fp), tos
	jsr	_copy
	adjspb	$-4
	movd	r0, 4(r7)
	movd	16(fp), 8(r7)
	movd	_hshval, r0
	ashd	$2,r0
	movd	_hshtab(r0), 12(r7)
	movd	_hshval, r0
	ashd	$2,r0
	movd	r7, _hshtab(r0)
	movd	4(r7), r0
	br	.L31
.L34:
	movd	4(r7), tos
	jsr	_free
	adjspb	$-4
	movd	12(fp), tos
	jsr	_copy
	adjspb	$-4
	movd	r0, 4(r7)
	movd	4(r7), r0
	br	.L31
.L31:
	exit	[r7]
	ret	0
.L32:
	enter	[r7], 0
	br	.L33
.data
.text
.globl	_copy
_copy:
	br	.L37
.L38:
	movd	8(fp),r7
	movd	r7, tos
	jsr	_strlen
	adjspb	$-4
	addqd	1, r0
	movd	r0, tos
	jsr	_malloc
	adjspb	$-4
	movd	r0, r5
	movd	r5, r6
	br	.L41
.L42:
.L41:
	movd	r7, r0
	addqd	1, r7
	movd	r5, r1
	addqd	1, r5
	movb	0(r0), 0(r1)
	movxbd	0(r1), r2
	cmpqd	0, r2
	bne	.L42
.L40:
	movd	r6, r0
	br	.L36
.L36:
	exit	[r5,r6,r7]
	ret	0
.L37:
	enter	[r5,r6,r7], 0
	br	.L38
.data
