#	@(#)nargs.s	1.1
# C library -- nargs
#
	.text
	.globl	_nargs
_nargs:
	.word	0x0000
	movzbl	*8(fp),r0	# 8(fp) is old ap
	ret
