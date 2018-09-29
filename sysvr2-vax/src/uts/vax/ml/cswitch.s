# @(#)cswitch.s	6.1
# save regs and ret loc in save area - return 0
	.align	4
	.globl	_save
_save :		# save(save_area)
	.word	0x0
	mtpr	$HIGH,$IPL
	movl	4(ap),r0	# save area addr
	movab	2*4(ap),sp	# restore stack to val before 'save' call
	movl	8(fp),ap	# restore ap	"	"	"
	movl	16(fp),r1	# restore pc	"	"	"
	movl	12(fp),fp	# restore fp	"	"	"
	movq	r6,(r0)+
	movq	r8,(r0)+
	movq	r10,(r0)+
	movq	ap,(r0)+	# ap & fp
	movl	sp,(r0)+
	movl	r1,(r0)+	# ret loc of call to 'save'
	movpsl	-(sp)
	pushl	r1
	svpctx		# save reg's -> PCB
	movpsl	-(sp)	# set up for return
	bicl2	$PS_IS|PS_IPL,(sp)	# undo SVPCTX
	pushl	r1	# ret loc
	clrl	r0
	rei

# switch to another process's '_u' area - return 1
	.align	4
	.globl	_resume
_resume :		# resume(proc_addr,save_addr)
	.word	0x0
	mtpr	$HIGH,$IPL	# inhibit interrupts
	movl	8(ap),retloc
	ashl	$9,4(ap),r0
	mtpr	r0,$PCBB
	movab	resmsp,sp
	ldpctx
	movl	retloc,r1	# 'ssav' or 'qsav' addr
	movq	(r1)+,r6
	movq	(r1)+,r8
	movq	(r1)+,r10
	movq	(r1)+,ap
	movl	(r1)+,sp
	movl	$1,r0
	mtpr	$0,$IPL
	jmp	*(r1)+	# return to caller at 'save' address

	.data
	.align	4
retloc:	.space	1*4
	.space	4*4
.globl	resmsp
resmsp:
	.space 4

	.text
	.align	4
	.globl	_setjmp
_setjmp:
	.word	0x0000
	movl	4(ap),r0
	movq	r6,(r0)+
	movq	r8,(r0)+
	movq	r10,(r0)+
	movq	8(fp),(r0)+
	movab	8(ap),(r0)+
	movl	16(fp),(r0)
	clrl	r0
	ret

	.align	4
	.globl	_longjmp
_longjmp:
	.word	0x0000
	movq	4(ap),r1
	movq	(r1)+,r6
	movq	(r1)+,r8
	movq	(r1)+,r10
	movq	(r1)+,ap
	movl	(r1)+,sp
	movl	r2,r0
	bneq	lj1
	incl	r0
lj1:
	jmp	*(r1)
