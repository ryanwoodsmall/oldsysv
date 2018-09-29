# @(#)power.s	6.1
# Power Fail Interrup Handler

	.align	4
Xpowfail:
	tstl	pftime
	bneq	pfwait
	pushr	$0x3fff
#
# Save registers in restart parameter block
#
	moval	Rpbbase,r5
	movl	_sbrpte,r3
	movl	(r3),r4			# save pte of phys 0
	movl	$PG_V|PG_KW,(r3)
	mtpr	r5,$TBIS
	mfpr	$PCBB,RPB_PCBB(r5)
	mfpr	$SCBB,RPB_SCBB(r5)
	mfpr	$SBR,RPB_SBR(r5)
	mfpr	$SISR,RPB_SISR(r5)
	mfpr	$SLR,RPB_SLR(r5)
	mfpr	$TODR,pftime
#
# Save all other volatile processor registers on the current stack (ISP)
#
	mfpr	$KSP,-(sp)
	mfpr	$ESP,-(sp)
	mfpr	$SSP,-(sp)
	mfpr	$USP,-(sp)
	mfpr	$ASTLVL,-(sp)
	mfpr	$PMR,-(sp)
	mfpr	$P0BR,-(sp)
	mfpr	$P0LR,-(sp)
	mfpr	$P1BR,-(sp)
	mfpr	$P1LR,-(sp)
	movl	sp,RPB_ISP(r5)
	movl	r4,(r3)			# restore pte
	movl	$-1,_pwr_act
	incl	_pwr_cnt
pfwait:
	brb	pfwait
#
# restore state and restart after power on
#
	.set POWERUP,3
	.align	512
	.globl	_clkset
restart:
	cmpl	ap,$POWERUP
	beql	powerup
	pushab	pfmsg
	calls	$1,_printf
# bug in powerfail? loop to avoid LSI reboot
pflp1:
	brb	pflp1
powerup:
	tstl	pftime
	bneq	pfup
	pushab	pfmsg
	calls	$1,_printf
pflp2:
	brb	pflp2
pfup:
	movab	-512(sp),r5
	bicl2	$1,RPB_FLAG(r5)
	mtpr	RPB_SBR(r5),$SBR
	mtpr	RPB_SLR(r5),$SLR
	mtpr	RPB_SISR(r5),$SISR
	mtpr	RPB_SCBB(r5),$SCBB
	mtpr	RPB_PCBB(r5),$PCBB
	movl	RPB_ISP(r5),r6
	jmp	*$restmap
restmap:
	mtpr	$0,$TBIA
	mtpr	$1,$MAPEN
	movl	r6,sp
	mtpr	(r6)+,$P1LR
	mtpr	(r6)+,$P1BR
	mtpr	(r6)+,$P0LR
	mtpr	(r6)+,$P0BR
	mtpr	(r6)+,$PMR
	mtpr	(r6)+,$ASTLVL
	mtpr	(r6)+,$USP
	mtpr	(r6)+,$SSP
	mtpr	(r6)+,$ESP
	mtpr	(r6)+,$KSP
	pushl	r6
	cmpw	_cputype,$780
	bneq	pfup2
# clear SBI errors on 780 only
	mtpr	$0x00040000,$SBIFS
	mtpr	$0x00008000,$SBIER
pfup2:
	mtpr	$0xf03,$TXDB		# clear warm boot
	calls	$0,_clkstart
	movl	_time,-(sp)
	calls	$1,_clkset
	movl	(sp)+,sp
	clrl	pftime
	popr	$0x3fff
	mtpr	$1,$SIRR
	rei

	.data
	.align	4
	.globl	pftime
pftime:	.space	1*4
	.globl	_pwr_act
_pwr_act:	.space	1*4
	.globl	_pwr_cnt
_pwr_cnt:	.space	1*4
pfmsg:
	.byte	012,'P,'o,'w,'e,'r,' ,'F,'a,'i,'l,' ,'E,'r,'r,'o,'r,012,0
	.text
