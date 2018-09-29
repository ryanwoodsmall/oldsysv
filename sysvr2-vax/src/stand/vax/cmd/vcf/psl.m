#	@(#)psl.m	1.1
#	VAX program status longword

	.set	PS_C,0x1		# carry bit
	.set	PS_V,0x2		# overflow bit
	.set	PS_Z,0x4		# zero bit
	.set	PS_N,0x8		# negative bit
	.set	PS_T,0x10		# trace enable bit
	.set	PS_IV,0x20		# integer overflow enable bit
	.set	PS_FU,0x40		# floating point underflow enable bit
	.set	PS_DV,0x80		# decimal overflow enable bit
	.set	PS_IPL,0x1f0000		# interrupt priority level
	.set	HIGH,0x1f		# highest priority
	.set	PS_PRVMOD,0xc00000	# previous mode
	.set	PS_CURMOD,0x3000000	# current mode
	.set	PS_IS,0x4000000		# interrupt stack
	.set	PS_FPD,0x8000000	# first part done
	.set	PS_TP,0x40000000	# trace pending
	.set	PS_CM,0x80000000	# compatibility mode

