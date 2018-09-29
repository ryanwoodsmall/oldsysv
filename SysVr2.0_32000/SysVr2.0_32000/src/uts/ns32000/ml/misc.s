	.globl	_addupc
_addupc:
	enter	[r3,r5],8
	movd	12(fp),r2	#u_prof
	movd	8(fp),r0	#pc
	subd	8(r2),r0	#adjust pc
	cmpqd	0,r0
	bgt	addret
	movd	12(r2),-8(fp)
	meid	r0,-8(fp)
	movd	-6(fp),r0	#scaled result
	cmpqw	0,-2(fp)	#high bits of result
	bne	addret
	addqd	1,r0
	bicb	$1,r0
	cmpd	r0,4(r2)
	bhs	addret
	addd	0(r2),r0
#if movus/movsu works add:
#	wrval	r0
#	bfs	adderr
	tbitb	$0, r0
	bfs	adderr
#if movus/movsu worked, the following code would be
#	movusw	r0,-2(fp)
#	addw	16(fp),-2(fp)
#	movsuw	-2(fp),r0
	movd	r0,r3
	andd	$0x3ff,r0	# page offset
	bsr	chkout
	bfs	adderr

	movd	_copypte,r1
	movmd	r5,r1,2		# copy pte's

	movd	_copyvad,r1
	addd	r0,r1
	lmr	eia,r1

	addw	16(fp),0(r1)
#end replacement code
	br	addret
adderr:
	movqd	0,12(r2)
addret:
	exit	[r3,r5]
	ret	0

	.globl	_searchdir
_searchdir:
	enter	[r5,r6,r7],0
	movqd	0,r6		# pointer to empty slot
	movd	12(fp),r7	# directory length in bytes
	ashd	$-4,r7		# convert to number of slots
	cmpqd	0,r7
	beq	sdone
	movd	8(fp),r5	# pointer to directory
s_top:
	cmpqw	0,0(r5)		# directory entry empty?
	beq	sempty
	movqd	7,r0		# word count
	movd	16(fp),r1	# target
	addr	2(r5),r2	# directory entry name
	cmpsw
	beq	smatch
scont:
	addr	16(r5),r5
	acbd	-1,r7,s_top	# loop
	br	sdone
sempty:
	cmpqd	0,r6
	bne	scont
	movd	r5,r6		# save empty slot
	br	scont
smatch:
	subd	8(fp),r5	# convert to offset
	movd	r5,r0
	br	sout
sdone:
	movqd	-1,r0
	cmpqd	0,r6
	beq	sout
	subd	8(fp),r6	# convert to offset
	movd	r6,r0
sout:
	exit	[r5,r6,r7]
	ret	0
