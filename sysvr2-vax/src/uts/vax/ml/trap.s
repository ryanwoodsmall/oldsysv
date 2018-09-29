# @(#)trap.s	6.3
	.set	PCB_P0LR, 84	# byte offset of P0LR in PCB
	.align	4
Xchme:
Xchms:
Xchmu:
	tstl	(sp)+
	rei

	.align	4
XCmchk:				# Machine check entry for VAX-11/750
	mtpr	$1,$CADR	# disable cache
	mtpr	$0,$TBIA	# invalidate translation buffer
	brb	Xmacheck
	.align	4
XSmchk:				# Machine check entry for VAX-11/780
	mtpr	$0x00218000,$SBIMT	# disable cache
	.globl	Xmacheck
Xmacheck:
	pushr	$0x3f
	moval	24(sp),r0
	addl2	(r0)+,r0
	cmpl	(r0),$copymov
	bnequ	mach1	# unexpected machine check, panic

	cmpw	_cputype,$780
	beql	Smach
# VAX-11/750 machine check error clear
	mfpr	$MCESR,r0
	mtpr	r0,$MCESR	# all bits are write 1 to clear
	mtpr	$0,$CADR	# enable cache
	brb	mach
# VAX-11/780 machine check error clear
Smach:
	mfpr	$SBIFS,r0
	bicl2	$0x02000000,r0	# clear error first pass, prevent double errors
	mtpr	r0,$SBIFS
	mfpr	$SBIER,r0
	mtpr	r0,$SBIER
	mtpr	$0x00200000,$SBIMT	# enable cache
mach:
	popr	$0x3f
	addl2	(sp)+,sp
	movl	$eret,(sp)
	bicl2	$PS_FPD,4(sp)
	rei
mach1:
	cmpw	_cputype,$780
	bnequ	Cmach1			# VAX-11/750
	callg	24(sp),_macheck
	halt
# This is a debug strap. Type C<cr> and see if system picks up.
# VAX-11/780 machine check error clear (SBI stuff)
	mfpr	$SBIFS,r0
	bicl2	$0x02000000,r0
	mtpr	r0,$SBIFS
	mfpr	$SBIER,r0
	mtpr	r0,$SBIER
	mtpr	$0x00200000,$SBIMT
	brb	mach2
Cmach1:
	callg	24(sp),_machck
	tstl	r0
	bneq	mdch2		# restartable machine check
	halt
mdch2:
# This is a debug strap. Type C<cr> and see if system picks up.
# VAX-11/750 machine check error clear
	mfpr	$MCESR,r0
	mtpr	r0,$MCESR
	mtpr	$0,$CADR
	mtpr	$0, $TBIA
mach2:	popr	$0x3f
	addl2	(sp)+,sp
	rei

	.data
	.align	4
KSPmsg:
	.byte	'K,'e,'r,'n,'a,'l,' ,'s,'t,'a,'c,'k,' ,'i,'n,'v,'a,'l,'i,'d,' ,'=,' ,'%,'x, 012,0
.text

	.align	4
	.globl	Xkspinv
Xkspinv:
	pushr	$0x3f
	mfpr	$KSP,-(sp)
	pushab	KSPmsg
	calls	$2,_printf
	mfpr	$KSP, KSPsave
	.data
	.align	4
	.globl	KSPsave
KSPsave:
	.long	0x0
	.text
	halt

	.align	4
Xtimein:
	pushr	$0x3f
	calls	$0,_timein
	brw	int_ret

# Massbus adapter interrupts
	.align	8
	.globl	_Xmba
_Xmba:
	pushr	$0x3f
	movl	$0,r0
	brb	xmba
	.align	8
	pushr	$0x3f
	movl	$1,r0
	brb	xmba
	.align	8
	pushr	$0x3f
	movl	$2,r0
	brb	xmba
	.align	8
	pushr	$0x3f
	movl	$3,r0
	brb	xmba
xmba:
	incl	_intlvl
	pushl	_mbadev[r0]
	movl	_mbaintr[r0],r0
	calls	$1,(r0)
	brw	int_ret

# Unibus adapter interrupts, VAX-11/780 only
	.align	8
	.globl	_Xuba	# The following is, in effect, a jump table for
			# the system control block (scb) set up in ubainit.
_Xuba:		#  UBA interrupts
	pushr	$0x3f
	movl	$0,r1	# uba 0 (nexus 3)
	brb	xuba
	.align	8
	pushr	$0x3f
	movl	$1,r1	# uba 1 (nexus 4)
	brb	xuba
	.align	8
	pushr	$0x3f
	movl	$2,r1	# uba 2 (nexus 5)
	brb	xuba
	.align	8
	pushr	$0x3f
	movl	$3,r1	# uba 3 (nexus 6)
	brb	xuba

#
#
xuba:
	incl	_intlvl
	mfpr	$IPL,r2		# get br level
	movl	_ubavad[r1],r0	# get uba virtual address
	movl	UBA_BR4-20*4(r0)[r2],r3	# get unibus device vector
	bleq	ubasrv	 # branch if zero vector or UBA service required

ubanorm:
	movl	_UNIvec(r3),r1	# get interrupt service routine address
	beql	ubastray	# NULL denotes stray
	extzv	$27,$4,r1,-(sp)	# controller code is in bits 27-30
	bicl2	$0x78000000,r1	# clear code
	calls	$1,(r1)		# call device interrupt handler
	brw	int_ret

.globl	_ubasrv
ubasrv:
	beql	uba0		# if zero vector, goto uba0
	cmpl	24(sp),$copymov
	beql	uba1
ubas0:
	pushl	r1		# unibus adapter number
	pushl	r3		# unibus device vector
	calls	$2,_ubasrv
	brb	uba2
uba0:
	incl	_ubazero
	brb	uba2
uba1:
	bitl	$1,8(r0)	# test for UBSSYNTO (slave sync timeout)
				# in uba status register
	beql	ubas0		# not set--print error, clear
	movl	$1,8(r0)	# clear UBA SSYNTO
	popr	$0x3f
	movl	$eret,(sp)
	bicl2	$PS_FPD,4(sp)
	decl	_intlvl
	rei

.globl	_ubastray
ubastray:
	pushl	r1		# unibus adapter number
	pushl	r3		# uba device vector
	calls	$2,_ubastray
uba2:
	popr	$0x3f
	decl	_intlvl
	rei

# Unibus direct vectored interrupts, VAX-11/750 only
	.align	4
Xubint:		#  Unibus device interrupts
	pushr	$0x3f
	movl	24(sp),r3	# get address this is coming from
	subl2	$Xunibus+3,r3	# make vectors zero origin.
	movl	_UNIvec(r3),r1	# get interrupt service routine address
	beql	ubstray		# NULL denotes stray
	extzv	$27,$4,r1,-(sp)	# controller code is in bits 27-30
	bicl2	$0x78000000,r1	# clear code
	calls	$1,(r1)
	popr	$0x3f
	tstl	(sp)+		# get rid of extra longword
	brw	int_ret1
ubstray:
	pushl	$0		# always uba 0 on comet
	pushl	r3
	calls	$2,_ubastray
	decl	_intlvl
	popr	$0x3f
	tstl	(sp)+		# get rid of extra longword
	rei

# Console receiver interrupt

	.align	4
Xconrint:
	incl	_intlvl
	calls	$0,_conrint
	brb 	int_ret1

# Console transmit interrupt
	.align	4
Xconxint:
	incl	_intlvl
	calls	$0,_conxint
	brb 	int_ret1

# Console DEC Tape II receive interrupt
	.align	4
Xcdtrint:
	calls	$0,_cdtrint
	brb	int_ret1

# Console Dec Tape II transmit interrupt
	.align	4
Xcdtxint:
	calls	$0,_cdtxint
	brb	int_ret1

# Clock interrupt
	.align	4
	.globl	Xclock
Xclock:
	incl	_intlvl
	calls	$0,_clock
	brb 	int_ret1

# Common code for interrupts
# At this point, the interrupt stack looks like:
#
#		 ___________
#		|    r0     | :isp
#		|-----------|
#		|    ...    |
#		|-----------|
#		|    r5     |
#		|-----------|
#		|    pc     |
#		|-----------|
#		|    psl    |
#		|___________|

int_ret:
	popr	$0x3f		# restore regs 0 - 5
int_ret1:
	tstl	_intlvl
	beql	nodec
	decl	_intlvl
nodec:
	bbssi	$0,idleflag,int_r0	# set idle escape flag (no wait instr)
	bbssi	$0,idleflag,int_r0	# set idle escape flag (no wait instr)
int_r0:
	bitl	$PS_CURMOD,4(sp)	# interrupt from user mode?
	beql	int_r1		# no, from kernal, just rei
	tstb	_runrun		# should we reschedule?
	beql	int_r1		# no, just rei
#
# If here, interrupt from user mode, and time to reschedule.
#	to do this, we set a software level 3 interrupt to
#	change to kernal mode, switch stacks,
#	and format kernal stack for a `qswitch' trap to force
#	a reschedule.
#
	mtpr	$0x18,$IPL	# make sure interrupt doesn't happen now
	mtpr	$3,$SIRR	# request level 3 software interrupt
int_r1:
	rei 			# return to interrupted process
#
# Trap and fault vector routines
#
#
#	Quiescent state (Software level 1 interrupt)
#
	.align	4
Xquiescent:
	pushr	$0x3fff
	calls	$0,_pwr
	popr	$0x3fff
	rei
#
#	AST trap (Software level 2 interrupt)
#
	.align 4
Xasttrap:
	insv	$4,$24,$3,_u+PCB_P0LR
	mtpr	$4,$ASTLVL		# restore ast level
	bitl	$PS_CURMOD,4(sp)	# check current mode against psl	
	bnequ	ast1			# if kernel mode, return
	rei				# (Can happen since resched comes
					#  in at interrupt level 3)

ast1:	pushl	$0			# dummy code 
	pushl	$ASTTRAP
	brw	alltraps
#
# Force a user level trap so profiling can take place.
#
	.align	4
	.globl	_profon
_profon:
	.word	0x0
	insv	$3,$24,$3,_u+PCB_P0LR
	mtpr	$3,$ASTLVL
	ret

#
#	Reschedule trap (Software level 3 interrupt)
#
	.align	4
	.globl	Xresched
Xresched:
	movpsl	-(sp)	# get ps
	bicl2	$PS_IPL,(sp)	# lower ipl
	pushab	resc1		# push pc
	rei			# lower ipl, jump to resc1

resc1:
	pushl	$0		# dummy code
	pushl	$RESCHED	# type
	brw 	alltraps	# merge

#	privileged instruction fault
	.align	4
Xprivinflt:
	pushl	$0		# push dummy code
	pushl	$PRIVINFLT	# push type
	brw 	alltraps	# merge

#	xfc instruction fault
	.align	4
Xxfcflt:
	pushl	$0		# push dummy code value
	pushl	$XFCFLT		# push type value
	brw 	alltraps	# merge

#	reserved operand fault
	.align	4
Xresopflt:
	pushl	$0		# push dummy code value
	pushl	$RESOPFLT	# push type value
	brw 	alltraps	# merge

#	reserved addressing mode fault
	.align	4
Xresadflt:
	pushl	$0		# push dummy code value
	pushl	$RESADFLT	# push type value
	brw 	alltraps	# merge with common code

#	bpt instruction fault
	.align	4
Xbptflt:
	pushl	$0		# push dummy code value
	pushl	$BPTFLT		# push type value
	brw 	alltraps	# merge with common code

#	Compatibility mode fault
	.align	4
Xcompatflt:
	pushl	$COMPATFLT	# push type value
	brw	alltraps

#	Trace trap
	.align	4
Xtracep:
	pushl	$0		# push dummy code value
	pushl	$TRCTRAP	# push type value
	brw 	alltraps	# go do it

#	Arithmitic trap
	.align	4
Xarithtrap:
	pushl	$ARITHTRAP	# push type value
	brb 	alltraps	# merge with common code

# Corrected memory Read Data and Read Data Substitute
	.align	4
Xcrdrds:
	cmpw	_cputype,$780
	beql	XScrd
	pushl	$0
	brb	Xcmi
XScrd:
	pushl	$1
	brw	xsbi

# Memory Write Timeout
	.align	4
Xmwt:
	cmpw	_cputype,$780
	beql	XSmwt
	pushl	$1
# VAX 11/750 Memory Interconnect common code
Xcmi:
	pushr	$0x3f
	pushl	32(sp)	#psl
	pushl	32(sp)	#pc
	pushl	32(sp)	#type
	calls	$3,_cmierr
	popr	$0x3f
	tstl	(sp)+
	rei

XSmwt:
	pushl	$4
	brw	xsbi

#	Protection  fault
#	moved to paging.s

# CHMK trap (syscall trap)
#	stack after CALLS to _trap:
#
#		 ___________
#		| 0x2FFF0000| :ksp
#		|-----------|
#		|    ap     |
#		|-----------|
#		|    fp     |
#		|-----------|
#		|    pc     |
#		|-----------|
#		|    r0     |
#		|-----------|
#		|    ...    |
#		|-----------|
#		|    r11    |
#		|-----------|
#		|    0x0    |
#		|-----------|
#		|    usp    |
#		|-----------|
#		|    type   |
#		|-----------|
#		|    code   |
#		|-----------|
#		|    pc     |
#		|-----------|
#		|    psl    |
#		|___________|
#

	.align	4
	.globl	Xsyscall
Xsyscall:
	pushl	$SYSCALL	# push type value
	mfpr	$USP,-(sp)	# get usp
# Call _trap with wrong number of arguments
#  so args not popped by ret
	calls	$0,_syscall
# Restore
	mtpr	(sp)+,$USP	# restore usp
	addl2	$8,sp		# pop type, code
	rei
.align 4
	.globl	alltraps
alltraps:
# Prepare arguments to _trap, note that type has already been pushed
	mfpr	$USP,-(sp)	# get usp
# Call _trap with wrong number of arguments
#  so args not popped by ret
	calls	$0,_trap
# Restore
	mtpr	(sp)+,$USP	# restore usp
	addl2	$8,sp		# pop type, code
	rei

	.align	4
_idle:	.globl	_idle
	.word	0x0000
	mtpr	$0,$IPL		# enable interrupts
waitloc:	blbc	idleflag,waitloc	# loop until interrupt
	bbcci	$0,idleflag,idle1	# clear idle escape flag
idle1:
	ret
	.data
	.globl	_waitloc
	.align	4
_waitloc:	.long	waitloc
idleflag:	.long	0

	.globl	_intlvl
_intlvl:  .long	0
	.text
