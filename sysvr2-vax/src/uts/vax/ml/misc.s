# @(#)misc.s	6.2
	.align	4
_addupc:	.globl	_addupc
	.word	0x0000
	movl	8(ap),r2	# &u.u_prof
	subl3	8(r2),4(ap),r0	# corrected pc
	blss	addret
	extzv	$1,$31,r0,r0	# logical right shift
	extzv	$1,$31,12(r2),r1	# ditto for scale
	emul	r1,r0,$0,r0
	ashq	$-14,r0,r0
	tstl	r1
	bneq	addret
	incl	r0
	bicb2	$1,r0
	cmpl	r0,4(r2)	# length
	bgequ	addret
	addl2	(r2),r0		# base
	probew	$3,$2,(r0)
	beql	adderr
	addw2	12(ap),(r0)
addret:
	ret
adderr:
	clrl	12(r2)
	ret

	.align	4
_spl1:	.globl	_spl1
	.word	0x0000
	mfpr	$IPL,r0		# get IPL value
	mtpr	$2,$IPL		# disable RESCHED & AST interrupts
	ret

	.align	4
_spl4:	.globl	_spl4
	.word	0x0000
	mfpr	$IPL,r0
	mtpr	$0x14,$IPL	# disable bus level 4 interrupts
	ret

	.align	4
_spl5:	.globl	_spl5
	.word	0x0000
	mfpr	$IPL,r0
	mtpr	$0x15,$IPL	# disable bus level 5 interrupts
	ret

	.align	4
_spl6:	.globl	_spl6
_spl7:	.globl	_spl7
	.word	0x0000
	mfpr	$IPL,r0
	mtpr	$0x18,$IPL	# disable bus level 7 and clock interrupts
	ret

	.align	4
_splhi:	.globl	_splhi
	.word	0x0000
	mfpr	$IPL,r0
	cmpl	r0,$0x17
	bgeq	splh
	mtpr	$0x17,$IPL
splh:
	ret

	.align	4
_spl0:	.globl	_spl0
	.word	0x0000
	mfpr	$IPL,r0
	mtpr	$0,$IPL
	ret

	.align	4
_splx:	.globl	_splx
	.word	0x0000
	mfpr	$IPL,r0
	mtpr	4(ap),$IPL
	ret

#	searchdir(buf, n, target) - search a directory for target
#	returns offset of match, or empty slot, or -1

	.globl	_searchdir

	.text
_searchdir:
	.word	0x1fc				# save regs 4 thru 8, r2 and r3
	movl	4(ap), r8			# pointer to directory
	movl	8(ap), r7			# directory length in bytes
	movl	$16, r6				# sizeof(struct direct)
	movl	$14, r5				# DIRSIZ for cmpcc
	movl	$0, r4				# pointer to empty slot

s_top:
	cmpl	r7, r6				# count less than 16?
	blss	sdone				# jump if true
	cmpw	0(r8), $0			# directory entry empty?
	beql	sempty				# jump if true
	cmpc3	r5, 2(r8), *12(ap)		# compare
	beql	smatch				# jump if match
scont:
	addl2	r6, r8				# increment directory pointer
	subl2	r6, r7				# decrement size
	brb	s_top				# keep looking

sempty:
	cmpl	r4, $0				# do we need an empty slot?
	bneq	scont				# jump if no
	movl	r8, r4				# save current offset
	brb	scont				# and goto to next entry

smatch:
	subl2	4(ap), r8			# convert to offset
	movl	r8, r0				# return offset
	ret

sdone:
	mcoml	$0, r0				# save failure return
	cmpl	r4, $0				# empty slot found?
	beql	sfail				# jump if false
	subl2	4(ap), r4			# convert to offset
	movl	r4, r0				# return empty slot
sfail:
	ret
