	.data
	.align	4
	.globl	dumpstack
	.space	58*4		# seperate stack for tape dumps
dumpstack: 
	.space	4
	.text
	.align	512
# Produce a core image dump on mag tape
	.globl	_do_dump
_do_dump:
	bicpsrw	$0xf00		# disable ints, Supv, ISP
	movd	$0xfffe00,r1	# icu addr
	movb	$0xff,0x14(r1)
	movb	$0xff,0x16(r1)	# mask all
	lprd	sp,$dumpstack
	save	[r0,r1,r2,r3,r4,r5,r6,r7]
	sprd	fp,tos
	smr	eia,r0
	smr	msr,r1
	save	[r0,r1]
	movd	_dump,r0
	jsr	r0
	jsr	_do_bpt		# no return
	bpt
