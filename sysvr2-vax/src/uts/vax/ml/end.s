# @(#)end.s	6.2
# define user area virtual address
	.set	usize,4		# size of user area, in pages
	.set	_u,0x80000000 - usize*512
	.globl	_u
	.data
# Bootstrap program executed in user mode
# to bring up the system.
	.globl	_icode
	.globl	_szicode
	.set	exec, 11
	.align	4
_icode:
ic0:
	pushab	ic1
	pushab	ic2
	pushl	$2
	movl	sp,ap
	chmk	$exec
ic00:
	brb	ic00
	.align	4
ic1:
	.long	ic2-ic0
	.long	0
ic2:
	.byte	'/,'e,'t,'c,'/,'i,'n,'i,'t,0
	.align	4
ic9:
_szicode:
	.long	ic9-_icode

	.align	4
	.globl	_cputype
_cputype:
	.word	780

	.align	4
	.globl	ispstack
ispstack:
	.space	512*2
ispstend:
	.space	4
