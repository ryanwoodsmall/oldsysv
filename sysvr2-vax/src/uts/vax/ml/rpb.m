# @(#)rpb.m	6.1
#
#	restart parameter block
#


	.set	RPB_BASE,0	# physical address of rpb
	.set	RPB_RESTART,4	# physical address of restart routine
	.set	RPB_CHKSUM,0x8	# sum of 1f words of restart routine
	.set	RPB_FLAG,0xc	# restart available
	.set	RPB_ISP,0xa4	# interrupt stack pointer
	.set	RPB_PCBB,0xa8	# physical address of current pcb
	.set	RPB_SBR,0xac	# physical address of system page table
	.set	RPB_SCBB,0xb0	# physical address of SCB
	.set	RPB_SISR,0xb4	# software interrupt summary register
	.set	RPB_SLR,0xb8	# save SPT length
