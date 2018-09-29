	.set	_u,UDOT
	.globl	_u

	.text
	.globl start
	.double	0,0,0,0,0,0,0,0	# space for zero module and monitor module
start:
	setcfg	[m,i,f]
	bicpsrw	$0xf00
	lprw	mod,$0
	addr	intstack,r5
	lprd	sp,r5
	lprd	fp,r5
# figure out size of memory and clear it
	addr	_edata,r6
	addd	$0x3ffff,r6
	andd	$0xfffc0000,r6
loopm:	movqd	0,0(r6)
	movd	0(r6),r0
	cmpqd	0,r0
	bne	outm
	movqd	-1,0(r6)
	movd	0(r6),r0
	cmpqd	-1,r0
	bne	outm
	addd	$0x40000,r6
	br	loopm
outm:
	addr	_edata,r1
	addr	4(r1),r2
	movd	r6,r0
	subd	r2,r0
	ashd	$-2,r0	#r0 = count of words
	movqd	0,0(r1)
	movsd		#clear memory
# set up virtual memory
	movd	r6,tos
	jsr	_vadrspace
	adjspb	$-4
#
	movd	$USTACK,r0
	bispsrw	$0x200	#user stack pointer
	lprd	sp,r0
	lprd	fp,r0
	bispsrw	$0x800	#enable int
	jsr	_main
	lprd	sp,$512
	lprd	fp,$512
	bicpsrw	$0x200	#use intr stack
	movw	$0xb00,tos	# psw U|S|I
	movqw	0,tos		#mod
	movqd	0,tos		#pc
	rett	0
	.data
	.align	2
	.space	128*4
intstack:
	.text
