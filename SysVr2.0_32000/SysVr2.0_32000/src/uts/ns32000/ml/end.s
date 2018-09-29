# Bootstrap program executed in user mode
# to bring up the system.
	.globl	_icode
	.globl	_szicode
	.set	exec,11
_icode:
ic0:
	addr	ic1-ic0,tos
	addr	ic2-ic0,tos
	addr	0(sp),r1
	addr	exec,r0
	svc
	movqd	1,r0
	tbitd	$0,r0
	flag
ic2:
	.byte	"/etc/init",0	#HANS
ic1:	.double	ic2-ic0
	.double	0
ic9:
_szicode:
	.double	ic9-ic0

	.data
	.globl	_cputype
	.align	1
_cputype:
	.word	16032
	.text
