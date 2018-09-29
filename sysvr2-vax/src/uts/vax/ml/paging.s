# @(#)paging.s	1.2

	.globl _pfault
	.globl _vfault

#	Protection  fault
	.align	4
	.globl	Xprotflt
Xprotflt:
	jbc $26,3*4(sp),Xpf2		# must not occur on interrupt stack
	pushl	$0x01
	calls	$0,_machill
	halt
Xpf2:
	pushr $0x3f			# 6 "scratch regs" of C compiler
	movl 7*4(sp),r0		# vaddr
	jsb mptea			# fetch addr of pte
	jeql Xpf1			# bad addr, no pte
	movl 9*4(sp),-(sp)		# psl
	pushl r0
	movq 8*4(sp),-(sp)	# code, vaddr
	calls $4,_pfault	# pfault(code,vaddr,ptea,psl)
	tstl r0
Xpf1:
# Condition codes must be set here.  Z=user error, N=page read error
	popr $0x3f				# popr preserves condition codes
	jeql cwlost			# jump if user error
	jlss pgrdflt			# jump if page read error
	addl2 $8,sp				# code, vaddr
	mtpr	$0x18, $IPL		# just in case the fault occured at
					# a non-zero ipl. If we didn't od
					# this in such a case, the rei
					# would fail.
	rei
cwlost:
	blbs	(sp),Xvalflt		# check for pt length violation
	cmpl	8(sp),$copymov
	beql	nofault
	movl	$PROTFLT,(sp)		# replace fault param word
	jbr 	alltraps

#	Segmentation fault

	.set	ref, 0x600000		# ref | ndref
	.set	valid, 0x80600000	# ref | ndref | valid
	.align	4
	.globl	Xvalflt
Xvalflt:
	jbc $26,3*4(sp),Xsf2		# must not occur on interrupt stack
	pushl	$0x02
	calls	$0,_machill
	halt
Xsf2:
	pushr	$0x3f			# 6 "scratch regs" of C compiler
	movl	7*4(sp),r0		# vaddr
	jsb	mptea			# fetch addr of pte
	jeql	Xsf1			# bad addr, no pte
	bitl	$ref, (r0)		# check for software ref bit
	jeql	Xsf3			# truly invalid page
	bisl2	$valid, (r0)		# set pte valid
	popr	$0x3f			# pop registers
	addl2	$8, sp			# code, vaddr
	mtpr	$0x18, $IPL		# just in case the fault occured at
					# a non-zero ipl. If we didn't od
					# this in such a case, the rei
					# would fail.
	rei

Xsf3:
	movl	9*4(sp),-(sp)		# ps
	pushl	r0
	movq	8*4(sp),-(sp)		# code, vaddr
	calls	$4,_vfault		# vfault(code,vaddr,ptea,ps)
	tstl r0
Xsf1:
# Condition codes must be set here.  Z=user error, N=page read error
	popr	$0x3f			# popr preserves condition codes
	jeql	dplost			# jump if user error
	jlss	pgrdflt			# jump if page read error
	addl2	$8,sp			# code, vaddr
	mtpr	$0x18, $IPL		# just in case the fault occured at
					# a non-zero ipl. If we didn't od
					# this in such a case, the rei
					# would fail.
	rei

dplost:
	cmpl	8(sp),$copymov
	beql	nofault
	movl	$SEGFLT,(sp)
	jbr 	alltraps

pgrdflt:
	cmpl	8(sp), $copymov
	beql	nofault
	movl	$PGRDFLT,(sp)
	jbr	alltraps

nofault:
	addl2	$8,sp
	movl	$eret,(sp)
	bicl2	$PS_FPD,4(sp)
	mtpr	$0x18, $IPL		# just in case the fault occured at
					# a non-zero ipl. If we didn't od
					# this in such a case, the rei
					# would fail.
	rei

# paging utilities 

_mapa:	.globl _mapa
	.word 0x0000
	movl 4(ap),r0
	bsbb mptea
	ret

# compute addess of pte which maps (r0); enter via bsbb or bsbw.
# this should be microcoded
	.globl mptea
mptea:
	jbcc $31,r0,pfusr	# jump if user space
	mfpr $SBR,r2		# pt base
	bisl2 $0x80000000,r2
	mfpr $SLR,r3		# pt length
	ashl $-9,r0,r0		# page #
	cmpl r0,r3
	jgeq pfnacc		# pt length violation
	moval (r2)[r0],r0	# pte address
	rsb
pfusr:
	jbsc $30,r0,pfP1	# jump if P1 space
	mfpr $P0BR,r2
	mfpr $P0LR,r3
	ashl $-9,r0,r0		# page #
	cmpl r0,r3
	jgeq pfnacc			# pt length violation
	moval (r2)[r0],r0	# pte address
	movl	r0, r1		# hold the address
	bsbb	mptea		# see if the address is reasonable
	jeql	pfnacc
	tstl	(r0)
	jeql	pfnacc
	movl	r1, r0
	rsb
pfP1:
	mfpr $P1BR,r2
	mfpr $P1LR,r3
	ashl $-9,r0,r0
	cmpl r0,r3
	jlss pfnacc
	moval (r2)[r0],r0	# pte address
	movl	r0, r1		# hold the address
	bsbb	mptea		# see if the address is reasonable
	jeql	pfnacc
	tstl	(r0)
	jeql	pfnacc
	movl	r1, r0
	rsb
pfnacc:
	clrl r0
	rsb
