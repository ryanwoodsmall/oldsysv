# @(#)rpb.s	6.1
# Restart parameter block
#
	.text
# Assume that this is at physical location 0
	.globl	Rpbbase
Rpbbase:
	.long	Rpbbase
	.long	0
	.long	0
	.long	0
	.align	512

 
	.globl	_sigcode
_sigcode:	# signal assist - save regs 0-5 plus r6 for compatibility mode
	calls	$3,1(pc)
	rei
	.word	0x7f
	callg	(ap),*12(ap)
	ret
