	.globl	_bcopy
_bcopy:
	movd	4(sp),r1	#from
	movd	8(sp),r2	#to
	movd	12(sp),r0	#byte count
	ashd	$-2,r0		#word count
	movsd
	movqd	3,r0
	andb	12(sp),r0	#leftover bytes
	movsb
	ret	0

# internal bcopy - r0,r1,r2 already set on input
bcopy:
	movw	r0,tos		# save low byte
	ashd	$-2,r0
	movsd
	movqd	3,r0
	andw	tos,r0
	movsb
	ret	0

	.globl	_copyseg
	.globl	_copypage
_copyseg:
_copypage:
	movd	_copypte,r0
	movd	4(sp),r1
	ord	$PG_V+PG_KW,r1
	movd	r1,0(r0)
	ord	$0x200,r1
	movd	r1,4(r0)
	movd	8(sp),r1
	ord	$PG_V+PG_KW,r1
	movd	r1,8(r0)
	ord	$0x200,r1
	movd	r1,12(r0)
	movd	_copyvad,r1
	addr	1024(r1),r2
	lmr	eia,r1
	addr	512(r1),r0
	lmr	eia,r0
	lmr	eia,r2
	addr	512(r2),r0
	lmr	eia,r0
	movd	$256,r0
	movsd
	ret	0

	.globl	_clearseg
	.globl	_clearpage
_clearseg:
_clearpage:
	movd	_copypte,r0
	movd	4(sp),r1
	ord	$PG_V+PG_KW,r1
	movd	r1,0(r0)
	ord	$0x200,r1
	movd	r1,4(r0)
	movd	_copyvad,r1
	lmr	eia,r1
	addr	512(r1),r2
	lmr	eia,r2
	addr	4(r1),r2
	movd	$255,r0
	movqd	0,0(r1)
	movsd
	ret	0

	.globl	_copyout
_copyout:
	enter	[r3,r4,r5],0
	movd	12(fp),r3	#to addr
	movd	16(fp),r4	#count
	cmpqd	0,r4
	beq	cpdone

colp3:
	movd	r3,r1		#to addr
	andd	$0x3ff,r1	#page offset
	movd	$1024,r0
	subd	r1,r0		#bytes in current page
	cmpd	r4,r0
	bge	coall		#want to do whole rest of page ?
	movd	r4,r0
coall:
	bsr	chkout
	bfs	ceret
	movd	_copypte,r2
	movmd	r5,r2,2		#set copypte
	movd	_copyvad,r5
	lmr	eia,r5
	movd	r5,r2
	addd	r1,r2		#virtual to addr
	addr	512(r5),r5
	lmr	eia,r5

	movd	r0,r5		#save byte count
	movd	8(fp),r1	#from addr
	bsr	bcopy

	addd	r5,r3
	addd	r5,8(fp)
	subd	r5,r4
	cmpqd	0,r4
	bne	colp3

cpdone:	movqd	0,r0
cpexit:	exit	[r3,r4,r5]
	ret	0
ceret:	movqd	-1,r0
	br	cpexit

	.globl	_copyin
_copyin:
	enter	[r3,r4,r5],0
	movd	8(fp),r3	#from addr
	movd	16(fp),r4	#count
	cmpqd	0,r4
	beq	cpdone

cilp3:
	movd	r3,r2		#from addr
	andd	$0x3ff,r2	#page offset
	movd	$1024,r0
	subd	r2,r0		#bytes in current page
	cmpd	r4,r0
	bge	ciall		#want to do whole rest of page ?
	movd	r4,r0
ciall:
	bsr	chkin
	bfs	ceret
	movd	_copypte,r1
	movmd	r5,r1,2		#set copypte
	movd	_copyvad,r5
	lmr	eia,r5
	movd	r5,r1
	addd	r2,r1		#virtual from addr
	addr	512(r5),r5
	lmr	eia,r5

	movd	r0,r5		#save byte ciunt
	movd	12(fp),r2	#to addr
	bsr	bcopy

	addd	r5,r3
	addd	r5,12(fp)
	subd	r5,r4
	cmpqd	0,r4
	bne	cilp3
	br	cpdone

	.globl	_bzero
_bzero:	movd	4(sp),r1	# addr
	movd	8(sp),r0	# bount
	ashd	$-2,r0		# int count
	cmpqd	0,r0
	bge	bz2b		# < 1 double
	addqd	-1,r0
	movqd	0,0(r1)
	addr	4(r1),r2
	movsd
	movd	r2,r1
bz2b:	movqd	3,r0
	andb	8(sp),r0	# remaining bytes
	cmpqd	0,r0
	bne	bz2b2
	ret	0
bz2b2:	movqb	0,0(r1)
	addqd	1,r1
	acbd	-1,r0,bz2b2
	ret	0

# this routine validates pages for physio
# currently just returns 0 for failure, 1 for success
#
# useracc(vaddr, length, code)
#
# if code&1 doing a READ
# if code&0x10 lock pages
	.globl	_useracc
_useracc:
	enter	[r3,r4,r5],0
	movd	8(fp),r3	#user address
	movd	12(fp),r4	#count
	cmpqd	0,r4
	beq	uadone

ualp3:
	movd	r3,r1		#user addr
	andd	$0x3ff,r1	#page offset
	movd	$1024,r0
	subd	r1,r0		#bytes in current page
	cmpd	r4,r0
	bge	uall		#want to do whole rest of page ?
	movd	r4,r0
uall:
	tbitb	$0,16(fp)	#test READ bit
	bfs	uread
	bsr	chkin
	br	uchkd
uread:
	bsr	chkout
uchkd:
	bfs	ueret
	tbitb	$4,16(fp)	#test B_PHYS bit
	bfc	uno_lck
	orb	$0x1,3(r5)	#set lock bit
uno_lck:
	addd	r0,r3
	subd	r0,r4
	cmpqd	0,r4
	bne	ualp3

uadone:	movqd	1,r0
uaexit:	exit	[r3,r4,r5]
	ret	0
ueret:	movqd	0,r0
	br	uaexit

# upgclr(vaddr)
# clear the rest of a user page starting at vaddr
	.globl	_upgclr
_upgclr:
	enter	[r3,r5],0
	movd	8(fp),r3	#user addr

	movd	r3,r1		#user addr
	andd	$0x3ff,r1	#page offset
	movd	$1024,r0
	subd	r1,r0		#bytes in current page
	bsr	chkout
	bfs	ucret
	movd	_copypte,r2
	movmd	r5,r2,2		#set copypte
	movd	_copyvad,r5
	lmr	eia,r5
	addd	r5,r1		#virtual user addr
	addr	512(r5),r5
	lmr	eia,r5

	addr	1(r1),r2
	movqb	0,0(r1)
	addqd	-1,r0
	cmpqd	0,r0
	bge	ucdone
	movsb

ucdone:	movqd	0,r0
ucexit:	exit	[r3,r5]
	ret	0
ucret:	movqd	-1,r0
	br	ucexit
