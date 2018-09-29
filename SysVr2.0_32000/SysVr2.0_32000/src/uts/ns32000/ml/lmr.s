	.globl	_lmr
_lmr:
	movd	4(sp),r0	#which
	movd	8(sp),r1	#val
	checkd	r0,LCH,r0
	bfs	LOUT
LC:	casew	LD[r0:w]
LD:
	.word	L0-LC
	.word	L1-LC
	.word	L2-LC
	.word	L3-LC
	.word	L4-LC
	.word	L5-LC
	.word	L6-LC
LCH:	.double	6,0
L0:
	lmr	eia,r1
	br	LOUT
L1:
	lmr	msr,r1
	br	LOUT
L2:
	lmr	ptb0,r1
	br	LOUT
L3:
	lmr	ptb1,r1
	br	LOUT
L4:
	lmr	bpr0,r1
	br	LOUT
L5:
	lmr	bpr1,r1
	br	LOUT
L6:
	lmr	bcnt,r1
LOUT:
	ret	0
	.globl	_smr
_smr:
	movd	4(sp),r0	#which
	checkd	r0,XCH,r0
	bfs	XOUT
XC:	casew	XD[r0:w]
XD:
	.word	X0-XC
	.word	X1-XC
	.word	X2-XC
	.word	X3-XC
	.word	X4-XC
	.word	X5-XC
	.word	X6-XC
XCH:	.double	6,0
X0:
	smr	eia,r0
	br	XOUT
X1:
	smr	msr,r0
	br	XOUT
X2:
	smr	ptb0,r0
	br	XOUT
X3:
	smr	ptb1,r0
	br	XOUT
X4:
	smr	bpr0,r0
	br	XOUT
X5:
	smr	bpr0,r1
	br	XOUT
X6:
	smr	bcnt,r0
XOUT:
	ret	0

	.globl	_invsatb
_invsatb:
	movd	8(sp),r0
	andd	$0xfffc00,r0
	cmpqd	0,4(sp)
	beq	Sa
	ord	$0x80000000,r0
Sa:
	lmr	eia,r0
	addr	512(r0),r0
	lmr	eia,r0
	ret	0
