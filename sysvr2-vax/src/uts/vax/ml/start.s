# @(#)start.s	6.2
# Initialization
#	IPL == 1F
#	MAPEN == off
#	SCBB, PCBB not set
#	SBR, SLR not set
#	ISP, KSP not set
#
	.set	MCKVEC,4	# offset into Scbbase of machine check vector
	.globl	start
start:
	.word	0x0000
	mfpr	$SID,r5			# get system id
	bicl2	$0x00ffffff,r5		# check cpu only
	cmpl	$0x01000000,r5		# VAX-11/780
	beql	star
# VAX-11/750 (COMET) only
	movw	$750,_cputype
	movl	$XCmchk+ISTK,Scbbase+MCKVEC
	brb	vax
# VAX-11/780 (STAR) only
star:
	movw	$780,_cputype
	movl	$XSmchk+ISTK,Scbbase+MCKVEC
vax:
	mtpr	$Scbbase-0x80000000,$SCBB	# set SCBB
	movl	$ispstend,sp	# set ISP
#
# Setup restart parameter block
	moval	Rpbbase,r6
	movl	$Rpbbase-0x80000000,RPB_BASE(r6)
	movl	$restart-0x80000000,RPB_RESTART(r6)
	clrl	RPB_CHKSUM(r6)
	moval	restart,r0
	movl	$0x1f,r1
chksum:
	addl2	(r0)+,RPB_CHKSUM(r6)
	sobgtr	r1,chksum
	clrl	RPB_FLAG(r6)

	movl	Scbbase+MCKVEC,r5	# save machine check entry
	movab	startint+ISTK,Scbbase+MCKVEC	# set new vector address
#
# size memory in 64kb chunks, save number of bytes in r7
	mtpr	$HIGH-1,$IPL	# allow machine check interrupts
	clrl	r7
sizmem:		# loop until machine check
	tstl	(r7)
	acbl	$8096*1024-1,$64*1024,r7,sizmem
	brb 	startint
	.align	4
startint:
	cmpw	_cputype,$780
	beql	mc780
mc750:
	mtpr	$0xf,$MCESR
	brb	mcall
mc780:
	mtpr	$0,$SBIFS	# clear err first pass bit
mcall:
	movl	r5,Scbbase+MCKVEC	# restore machine check vector
	movl	$ispstend,sp		# reset ISP
#
	bisw2	$0x0fff,_trap		# set trap to save r0-r11
	bisw2	$0x0fff,_syscall	# set trap to save r0-r11
	bisw2	$0x003f,_clock		# set clock to save r0-r5
	bisw2	$0x003f,_conrint
	bisw2	$0x003f,_conxint
	bisw2	$0x003f,_cdtrint
	bisw2	$0x003f,_cdtxint
	bisw2	$0x003f,_printf		# call printf from assembler safely
	bisw2	$0x0fff,_panic		# set panic to save r0-r11 -- sometimes
					# useful to know

# clear rest of memory
	movab	_edata,r0
	subl3	$8,r7,r1
star1:
	clrq	(r0)
	acbl	r1,$8,r0,star1
#
	jmp	*$star2		# set pc to system virtual address
star2:
	pushab	star3
	pushl	r7
# init system pt entries, I/O adapters, memory maps, proc 0
	calls	$2,_vadrspace
	ldpctx			# switch to proc 0
	rei			# on kernal stack
star3:
# init SBI control regs, VAX-11/780 only
	cmpw	_cputype,$780
	bneq	star4
	mtpr	$0x00040000,$SBIFS
	mtpr	$0x00008000,$SBIER
star4:
	mtpr	$0xf03,$TXDB	# clear warm boot flag
swr1:
	mfpr	$TXCS,r0
	tstb	r0
	bgeq	swr1
	mtpr	$0xf04,$TXDB	# clear cold boot flag
# startup, fork off /etc/init
# main returns pc for new process.
	calls	$0,_main
# proc 0 loops forever, but children of proc 0 (init, vhand ...)
# return to be continued as described below.
#
# check for user or system, GOTO user process, CALL system process.
	bbs	$31,r0,star5	# branch if system address
	pushl	$PS_CURMOD|PS_PRVMOD	# psl, user mode, ipl= 0
	pushl	r0		# pc, returned
	rei			# now on user stack
	.globl	star5
star5:
	pushl	$PS_PRVMOD	# kernel mode
	pushab	star6		# pc
	rei			# prv mode user ???
star6:
	calls	$0,(r0)
	pushl	$0x3
	calls	$0,_machill
	halt		# should not get here
