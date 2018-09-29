	.file	"setjmp.s"
#	@(#)setjmp.s	1.6
# C library -- setjmp, longjmp

#	longjmp(a,v)
# will generate a "return(v)" from
# the last call to
#	setjmp(a)
# by restoring r6-pc from 'a'
# and doing a return.
#

.globl	_setjmp
.globl	_longjmp

	.align	2
_setjmp:
	.word	0x0000
	MCOUNT
	movl	4(ap),r0
	movq	r6,(r0)+
	movq	r8,(r0)+
	movq	r10,(r0)+
	movq	8(fp),(r0)+		# ap, fp
	movab	8(ap),(r0)+		# sp
	movl	16(fp),(r0)		# pc
	clrl	r0
	ret

	.align	2
_longjmp:
	.word	0x0000
	MCOUNT
	movl	8(ap),r0		# return(v)
	movl	4(ap),r1
	cmpl	28(r1),12(fp)		# if called within same function,
	bneq	.L0			# don't restore register variables
	addl2	$24,r1			# just step past them
	jbr	.L1
.L0:
	movq	(r1)+,r6
	movq	(r1)+,r8
	movq	(r1)+,r10
.L1:
	movq	(r1)+,r12		# ap, fp
	movl	(r1)+,sp
	tstl	r0
	bneq	.L2
	movzbl	$1,r0
.L2:
	jmp	*(r1)
