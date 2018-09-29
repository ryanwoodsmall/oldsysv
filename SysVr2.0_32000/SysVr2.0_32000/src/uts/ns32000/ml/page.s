	.globl	h2
	.globl	usrabt
	.globl	p_retry
	.globl	chkin
	.globl	chkout

# code to handle an MMU abort

h2:
	sprb	psr,6(sp)	#aborts may not save psr properly
				#NOTE: this fix breaks single stepping
				#remove when chip bug fixed!
h2_again:
	movqd	2,tos		#trap type
	movd	r0,tos		#save r0
	tbitb	$0,15(sp)	#from kernal mode (currently illegal)
	bfs	usrabt
	cmpd	$rett_adr,8(sp) #was trying to do rett?
	bne	suptrap
	adjspb	$-16		#pretend we got to user before trap
	br	h2_again
usrabt:
	movmd	tos,USTACK-16,4 #copy to user stack
	adjspb	$-16		#pop int. stack
	bispsrw	$0x200		#user stack pointer
	sprd	sp,r0		#save old stack pointer
	lprd	sp,$USTACK-16	#set user stack pointer
	save	[r0,r1,r2,r3]	#save scratch regs
	smr	eia,r3
	smr	msr,r2
	andd	$0xffffff,r3	#ignore top byte
	rdval	r3
	bfs	try_sp
	movd	_upte2,r0
	movd	r3,r1
	ashd	$-7,r1
	andd	$0xfffff8,r1
	addd	r1,r0		#address of user pte
	movd	r0,tos
	movd	r3,tos
	movb	r2,r0		#low part of msr
	andb	$0x38,r0	#TET
	cmpb	$0x8,r0		#prot fault?
	beq	c_pf
	jsr	_vfault
	br	c_ret
c_pf:
	movb	$16,USTACK-12	#change type code
	jsr	_pfault
# apparently there is a hardware bug with RO protection faults -
#if EIAHACK
# the EIA may be garbage when it should be equal to the stack pointer
# the following hack tries to get around this
# BEGIN EIA HACK
	adjspb	$-8
	cmpqd	0,r0
	blt	comret		#normal case - fault fixed
# try sp as fault address
try_sp:
	movd	USTACK-20,r3	#user sp
p_retry:
	rdval	r3
	bfs	call_tr		#forget it
	movd	_upte2,r0
	movd	r3,r1
	ashd	$-7,r1
	andd	$0xfffff8,r1
	addd	r1,r0		#address of user pte
	movd	r0,tos
	movd	r3,tos
	jsr	_pfault
	adjspb	$-8
	cmpqd	0,r0
	blt	comret		#2nd time worked!
# if on page boundary, try sp+3
	movd	r3,r0
	andd	$0x3ff,r0
	cmpd	r0,$0x3fd
	blt	call_tr		#give up
	addqd	3,r3
	br	p_retry
#END EIA HACK
#endif
c_ret:
	adjspb	$-8
	cmpqd	0,r0
	blt	comret		# ok - return
#if EIAHACK
	bgt	rd_bad
	movd	USTACK-20,r2	#user sp
	andd	$0xfffe00,r2
	andd	$0xfffe00,r3
	cmpd	r2,r3
#endif
	beq	call_tr		#call trap.c
#if EIAHACK
	br	try_sp
rd_bad:
#endif
	movb	$17,USTACK-12	#change type code to indicate page read err
	br	call_tr

# internal routines to check access of a user address
# on input:
#	r3 - user virtual address
# on output:
#	r5 - address of pte
#	flag set if error

# check read of user address

chkin:
	save	[r0,r1,r2,r3]
#high byte must be zero
	movd	$0xff000000,r0
	andd	r3,r0
	cmpqd	0,r0
	bne	ck_bad
#check that at least reads would be ok if PG_V
	rdval	r3
	bfs	ck_bad
#compute pte addr
	movd	r3,r5
	ashd	$-7,r5
	andd	$0x1fff8,r5
	addd	_upte2,r5
	movb	0(r5),r0
	andb	$5,r0
	cmpqb	5,r0
	beq	cki_ok		#valid and PG_UR or PG_UW ?
# simulate fault
	movd	r5,tos
	movd	r3,tos
	jsr	_vfault
	adjspb	$-8
	cmpqd	1,r0
	bne	ck_bad
cki_ok:
	bicpsrb	$0x20	# clear flag
	orb	$0x8,0(r5)	# set ref bit
ck_ret:
	restore	[r0,r1,r2,r3]
	ret	0
ck_bad:
	bispsrb	$0x20	# set flag
	br	ck_ret

# check write of user address

chkout:
	save	[r0,r1,r2,r3]
#high byte must be zero
	movd	$0xff000000,r0
	andd	r3,r0
	cmpqd	0,r0
	bne	ck_bad
#check that at least reads would be ok if PG_V
	rdval	r3
	bfs	ck_bad
#compute pte addr
	movd	r3,r5
	ashd	$-7,r5
	andd	$0x1fff8,r5
	addd	_upte2,r5
cko_again:
	movb	0(r5),r0
	andb	$7,r0
	cmpqb	7,r0
	beq	cko_ok		#valid and PG_UW ??
# simulate fault
	movd	r5,tos
	movd	r3,tos
	tbitb	$0,0(r5)	# valid?
	bfs	cko_pf
	jsr	_vfault
	br	cko_dc
cko_pf:
	jsr	_pfault
cko_dc:
	adjspb	$-8
	cmpqd	1,r0
	bne	ck_bad
	br	cko_again
cko_ok:
	bicpsrb	$0x20	# clear flag
	orb	$0x18,0(r5)	# set ref and mod bits
	br	ck_ret
