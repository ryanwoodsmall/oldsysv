# @(#)dump.s	6.1
	.data
	.align	512
	.globl	__sdata
__sdata:	# start of data
	.globl	dumpstack
	.space	58*4		# seperate stack for tape dumps
dumpstack: 
	.space	4
	.text
# 0x400
# Produce a core image dump on mag tape
	.align	512
	.globl	dump
dump:
	movl	sp,dumpstack	# save stack pointer
	movab	dumpstack,sp	# reinit stack
	mfpr	$PCBB,-(sp)	# save u-area pointer
	mfpr	$MAPEN,-(sp)	# save value
	mfpr	$IPL,-(sp)
	mtpr	$0,$MAPEN	# turn off memory mapping
	mtpr	$HIGH,$IPL	# disable interrupts
	pushr	$0x3fff		# save regs 0 - 13
	calls	$0,*_dump
	halt
