# @(#)sbi.s	6.1
# SBI error trap catcher
	.align	4
Xsbi:
	pushl	$0
	brb	xsbi
	.align	4
	pushl	$1
	brb	xsbi
	.align	4
	pushl	$2
	brb	xsbi
	.align	4
	pushl	$3
	brb	xsbi
	.align	4
	pushl	$4
	brb	xsbi
# save SBI registers
xsbi:
	pushr	$0x3f
	movl	$16,r0
xsbis:
	mfpr	$SBIS,-(sp)
	sobgtr	r0,xsbis
	mfpr	$SBITA,-(sp)
	mfpr	$SBIER,-(sp)
	mfpr	$SBIMT,-(sp)
	mfpr	$SBISC,-(sp)
	mfpr	$SBIFS,-(sp)
	pushl	116(sp)
	pushl	116(sp)
	pushl	116(sp)
	calls	$24,_sbierr
# reset SBI
	mfpr	$SBIER,r0
	mtpr	r0,$SBIER
	mfpr	$SBIFS,r0
	bicl2	$0x2000000,r0
	mtpr	r0,$SBIFS
	mfpr	$SBISC,r0
	bbc	$31,r0,xsbi1
	mtpr	$0x00200000,$SBIMT
	mtpr	$0x0,$SBISC
xsbi1:
	popr	$0x3f
	tstl	(sp)+
	rei
