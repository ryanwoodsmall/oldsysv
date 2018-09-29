	.globl	_fuword
	.globl	_fuiword
_fuword:
_fuiword:
	enter	[r5],4
	movd	8(fp),r3	# user address
	movd	r3,r1
	andd	$0x3ff,r1	# page offset
	cmpd	r1,$0x3fc
	bgt	fu2p		# overlaps 2 pages ?

	bsr	chkin
	bfs	fuerr		# bad address?

	movd	_copypte,r2
	movmd	r5,r2,2		# copy pte's
	movd	_copyvad,r2
	addd	r1,r2		# virtual addr
	lmr	eia,r2
	addr	3(r2),r0
	lmr	eia,r0

	movd	0(r2),r0
fuout:	exit	[r5]
	ret	0
fuerr:	movqd	-1,r0
	br	fuout

fu2p:	# very rare case - just use copyin
	movqd	-1,-4(fp)	# pick up -1 if copyin fails
	movqd	4,tos
	addr	-4(fp),tos
	movd	r3,tos
	jsr	_copyin
	adjspb	$-12
	movd	-4(fp),r0
	br	fuout

	.globl	_suword
	.globl	_suiword
_suword:
_suiword:
	enter	[r5],0
	movd	8(fp),r3	# user address
	movd	r3,r1
	andd	$0x3ff,r1	# page offset
	cmpd	r1,$0x3fc
	bgt	su2p		# overlaps 2 pages ?

	bsr	chkout
	bfs	suerr		# bad address?

	movd	_copypte,r2
	movmd	r5,r2,2		# copy pte's
	movd	_copyvad,r2
	addd	r1,r2		# virtual addr
	lmr	eia,r2
	addr	3(r2),r0
	lmr	eia,r0

	movd	12(fp),0(r2)
	movqd	0,r0
suout:	exit	[r5]
	ret	0
suerr:	movqd	-1,r0
	br	suout

su2p:	# very rare case - just use copyout
	movqd	4,tos
	movd	r3,tos
	addr	12(fp),tos
	jsr	_copyout
	adjspb	$-12
	br	suout

	.globl	_fubyte
_fubyte:
	enter	[r5],4
	movd	8(fp),r3	# user address
	movd	r3,r1
	andd	$0x3ff,r1	# page offset

	bsr	chkin
	bfs	fuerr		# bad address?

	movd	_copypte,r2
	movmd	r5,r2,2		# copy pte's
	movd	_copyvad,r2
	addd	r1,r2		# virtual addr
	lmr	eia,r2

	movzbd	0(r2),r0
	br	fuout
	.globl	_subyte
_subyte:
	enter	[r5],0
	movd	8(fp),r3	# user address
	movd	r3,r1
	andd	$0x3ff,r1	# page offset

	bsr	chkout
	bfs	suerr		# bad address?

	movd	_copypte,r2
	movmd	r5,r2,2		# copy pte's
	movd	_copyvad,r2
	addd	r1,r2		# virtual addr
	lmr	eia,r2

	movb	12(fp),0(r2)
	movqd	0,r0
	br	suout
