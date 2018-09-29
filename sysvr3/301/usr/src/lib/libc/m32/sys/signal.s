#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.ident	"@(#)libc-m32:sys/signal.s	1.21"
	.file	"signal.s"
#   C-library signal() routine recoded for IEEE floating-point
# REMEMBER: SIGFPE is special in this environment!
#
#   In SVR3, five new system calls are added to this file. They are
#   sigset(), sighold(), sigrelse(), sigignore() and sigpause().	

# **********
# Constants defined by the hardware and the OS
# **********
	.set	SYSGATE,1*4	# first-level gate entry for system call
				# (*4 since each 1st level is 1 word)
	.set	SIGNAL,48*8	# second-level gate entry for signal
				# system call (*8 since each second-
				# level gate entry is two words long) 
	.set	EINVAL,22	# errno value for invalid system call
	.set	SIGILL,4
	.set	SIGFPE,8	# signal number for floating point

	# code no longer uses SIG_DFL, but does depend on it being 0
	.set	SIG_IGN,1	# "ignore" signal
	.set	SIG_HOLD,2	# "hold" signal

	# The following 5 definitions are also defined in signal.h
	.set	SIGDEFER,0x100	# "defer" signal after caught one
	.set	SIGHOLD,0x200	# "hold" signal
	.set	SIGRELSE,0x400	# "release" signal
	.set	SIGIGNORE,0x800	# "ignore" signal
	.set	SIGPAUSE,0x1000	# "release" held signal and pause

	.set	ASR_CSC,0x20000	# <CSC> bit must be set when writing ASR

# **********
# Global function names
# **********
	.globl	signal		# defined below
	.globl	sigset
	.globl	sighold
	.globl	sigignore
	.globl	sigrelse
	.globl	sigpause
	.globl	_cerror		# common C library error return
_m4_ifdef_(`SHLIB',
`	.globl	_libc__getflthw
',
`	.globl	_getflthw	# floating-point pre-handler
')

# **********
# Global data
# **********
	.globl	_fp_hw		# floating-point hardware equippage
	.globl	d.vect		# this table is looked into by fcrt0.s
_m4_ifdef_(`SHLIB',
`
',
`	.comm	_old.fp,4	# flag for old trap floating point
')

#
# ******************
# Definition of sighold()
# ******************
#
# sighold(n)		/* hold signal n */
#
sighold:
	MCOUNT

	# is the signal number valid?  (0 < n < NSIG)
	CMPW	&0,0(%ap)
	jle	.ninvalid
	CMPW	0(%ap),&NSIG
	jle	.ninvalid

	ORW2	&SIGHOLD,0(%ap)		# turn on SIGHOLD bit
	jmp 	.nost

#
# ******************
# Definition of sigrelse()
# ******************
#
# sigrelse(n)		/* release signal n if there is one held */
#

sigrelse:
	MCOUNT

	# is the signal number valid?  (0 < n < NSIG)
	CMPW	&0,0(%ap)
	jle	.ninvalid
	CMPW	0(%ap),&NSIG
	jle	.ninvalid

	ORW2	&SIGRELSE,0(%ap)	# turn on SIGRELSE bit
	jmp	.nost

#
#************************
# Definition of sigignore()
# **********************
#
# sigignore(n)
#
#
sigignore:
	MCOUNT

	# is the signal number valid?  (0 < n < NSIG)
	CMPW	&0,0(%ap)
	jle	.ninvalid
	CMPW	0(%ap),&NSIG
	jle	.ninvalid

	ORW2	&SIGIGNORE,0(%ap)		# turn on SIGIGNORE
	jmp	.nost

#
#************************
# Definition of sigpause()
# **********************
#
# sigpause(n)
#
#
sigpause:
	MCOUNT

	# is the signal number valid?  (0 < n < NSIG)
	CMPW	&0,0(%ap)
	jle	.ninvalid
	CMPW	0(%ap),&NSIG
	jle	.ninvalid

	ORW2	&SIGPAUSE,0(%ap)		# turn on SIGPAUSE bit
	jmp	.nost
.nost:						# common gate instruction
	movw	&SYSGATE,%r0
	movw	&SIGNAL,%r1
	GATE
	jlu	_cerror
	RET

.ninvalid:				# common error routine
	movw &EINVAL,%r0		#put error number in r0
	jmp	_cerror

#
# ************
# Definition of sigset()
# ************
#
# sigset(n, SIG_DFL); 	/* default action on signal n */
# sigset( n, SIG_IGN);	/* ignore signal n		*/
# sigset( n, handler);	/* call user handler on signal	*/
#
sigset:
	MCOUNT
	SAVE	%r8
	movw 	&1,%r2	# use r2 as the flag for sigset
	jmp 	.sig1

#
# **********
# Definition of signal()
# **********
#
# signal( n, SIG_DFL);	/* default action on signal n	*/
# signal( n, SIG_IGN);	/* ignore signal n		*/
# signal( n, handler);	/* call user handler on signal	*/

signal:
	MCOUNT
	SAVE	%r8
	movw 	&0,%r2	# specify that this is signal()
.sig1:

	# is the signal number valid?  (0 < n < NSIG)
	CMPW	&0,0(%ap)
	jle	.invalid
	CMPW	0(%ap),&NSIG
	jle	.invalid

	ALSW3	&2,0(%ap),%r1	# convert signal number into offset
				# into a table of 4-byte vectors

	PUSHW	d.vect(%r1)	# save old user entry at 0(%fp)
	movaw	d.vect(%r1),%r8

	TSTW	_old.fp		# is old floating point code present?
	je	.sig2		# no, go ahead
	CMPW	0(%ap),&SIGILL	# does the user want to change the
				# disposition of SIGILL ?
	jne	.sig2		# no, go ahead.
				# Otherwise we do not tell the kernel
	movw	4(%ap),(%r8)	# just put the user label in d.vect table
	movw	0(%fp),%r0	# and return the last user label
	RESTORE	%r8
	RET

.sig2:
	# if the signal action is changing from anything to DFL, IGN
	# or HOLD,
	# must call the OS before updating d.vect[]
	cmpw	4(%ap),&SIG_HOLD
	jleu	.sig3

	# update d.vect[n] before issuing the system call
	movw	4(%ap),(%r8)

	# and replace user entry by signal's own tvect[n]
	mulw3	&TSIZE,0(%ap),%r0	# index into tvect[] table
	movaw	tvect(%r0),4(%ap)
.sig3:
	movw 	0(%ap),%r1
	CLRB	setflg(%r1)		# setflg ==1 for sigset
					# setflg == 0 for signal
	TSTW	%r2			# if signal, don't turn on the
					# SIGDEFER flag
	je	.ost
	ORW2	&SIGDEFER,0(%ap)	# turn on SIGDEFER bit for sigset
	movb	&1,setflg(%r1)		# turn on setflg to remember
					# that common code was entered
					# from sigset instead of signal
.ost:
	movw	&SYSGATE,%r0
	movw	&SIGNAL,%r1
	GATE
	jlu	._cerror0		# if kernel returned error

	# The kernel will return SIG_IGN, SIG_DFL, SIG_HOLD or the old handler.
	# This may be different from the value 0(%fp) we pushed IF
	# a signal has been handled, re-setting the kernel to SIG_DFL
	# while the signal code still thinks a handler is attached.
	# ALWAYS believe the kernel.
	cmpw	%r0,&SIG_HOLD		# if SIG_DFL, SIG_IGN or SIG_HOLD
	jleu	.around			# return this action to user

	# kernel returned the old handler (tvect[n], not d.vect[n])
	# so replace with the old value of d.vect[n] off the stack
	movw	0(%fp),%r0

.around:
	# if we did not update d.vect[] above and 4(%ap) is DFL or IGN,
	# and we need to update d.vect[] here.
	cmpw	4(%ap),&SIG_IGN
	jgu	.around2
	movw	4(%ap),(%r8)

.around2:
	RESTORE	%r8
	RET

.invalid:
	movw	&EINVAL,%r0	# put error number in %r0
._cerror0:
	RESTORE	%r8		# go to common library error return,
	jmp	_cerror		# which moves %r0 to errno, returns -1

#
# The following is two tables of transfer vectors.
# Each table has one entry for each valid signal number.
#    tvect[] is the table of signal entry points from the kernel.
#    d.vect[] is the corresponding table of user handlers.
# NSIG == number of vectors, so the  highest signal number is NSIG-1.
# tvect[0] is unused, but makes the calculations above easier.
#


	.set	NSIG,0
tvect:
	BSBH	savr;	.set	NSIG,NSIG + 1	# entry 0, unused
			.set	TSIZE,.-tvect	# except to size BSBH
.entry2:
	BSBH	savr;	.set	NSIG,NSIG + 1	# 1: SIGHUP
	BSBH	savr;	.set	NSIG,NSIG + 1	# 2: SIGINT
	BSBH	savr;	.set	NSIG,NSIG + 1	# 3: SIGQUIT
	BSBH	savr;	.set	NSIG,NSIG + 1	# 4: SIGILL
	BSBH	savr;	.set	NSIG,NSIG + 1	# 5: SIGTRAP
	BSBH	savr;	.set	NSIG,NSIG + 1	# 6: SIGABRT
	BSBH	savr;	.set	NSIG,NSIG + 1	# 7: SIGEMT
	BSBH	fpsavr;	.set	NSIG,NSIG + 1	# 8: SIGFPE <-**
	BSBH	savr;	.set	NSIG,NSIG + 1	# 9: SIGKILL <unused>
	BSBH	savr;	.set	NSIG,NSIG + 1	#10: SIGBUS
	BSBH	savr;	.set	NSIG,NSIG + 1	#11: SIGSEGV
	BSBH	savr;	.set	NSIG,NSIG + 1	#12: SIGSYS
	BSBH	savr;	.set	NSIG,NSIG + 1	#13: SIGPIPE
	BSBH	savr;	.set	NSIG,NSIG + 1	#14: SIGALRM
	BSBH	savr;	.set	NSIG,NSIG + 1	#15: SIGTERM
	BSBH	savr;	.set	NSIG,NSIG + 1	#16: SIGUSR1
	BSBH	savr;	.set	NSIG,NSIG + 1	#17: SIGUSR2
	BSBH	savr;	.set	NSIG,NSIG + 1	#18: SIGCLD
	BSBH	savr;	.set	NSIG,NSIG + 1	#19: SIGPWR
	BSBH	savr;	.set	NSIG,NSIG + 1	#20: SIGWIND
	BSBH	savr;	.set	NSIG,NSIG + 1	#21: SIGPHONE
	BSBH	savr;	.set	NSIG,NSIG + 1	#22: SIGSEL

# define the parallel set of user vectors
# NOTE: the contents of d.vect[i] will not correspond to the OS's idea:
#	1) for SIGKILL, where any attempt to catch, or ignore causes
#	   the kernel to return EINVAL to the signal() call.
#	2) if signal() is used and a signal has been caught,
#	   changing the handler to SIG_DFL.
#	3) if sigset() is used and a signal has been caught,
#	   holding subsequent signals of the same type.

	.data	# define the parallel set of user vectors
	.align	4
_m4_ifdef_(`SHLIB',
`',
`d.vect:	.zero	NSIG * 4	# and initialize to SIG_DFL
')
setflg:	.zero	NSIG 		# sigset flag vector
	.text
#
# **********
# Entry point from kernel for SIGFPE
# **********
#
# FPSAVR: entry point for SIGFPE exceptions
#
# before the call to _getflt, the stack looks like:
#
#	%sp ->	|		|
#		+===============+
#		| d.vect[SIGFPE]	|
#		+---------------+
#		| r0ptr		| >------
#		+===============+	|
#		| user PC (r15)	|	|
#		+---------------+	|
#		| "isp"		|	|
#		+---------------+	|
#		| "pcpb"	|	|
#		+---------------+	|
#	------<	| user sp (r12)	|	|
#	|	+---------------+	|
#	|	| user psw(r11)	|	|
#	|	+---------------+	|
#	|	| user ap (r10)	|	|
#	|	+---------------+	|
#	|	| user fp (r09)	|	|
#	|	+---------------+	|
#	|	| user r8	|	|
#	|	+---------------+	|
#	|	| user r7	|	|
#	|	+---------------+	|
#	|	| user r6	|	|
#	|	+---------------+	|
#	|	| user r5	|	|
#	|	+---------------+	|
#	|	| user r4	|	|
#	|	+---------------+	|
#	|	| user r3	|	|
#	|	+---------------+	|
#	|	| user r2	|	|
#	|	+---------------+	|
#	|	| user r1	|	|
#	|	+---------------+	|
#	|	| user r0	| <------
#	|	+===============+
#	|	| BSBH return	|
#	|	+===============+
#	|	| user PSW	| : GATE
#	|	+---------------+
#	------> | user PC	|
#		+===============+
#		| user stack	|
#
fpsavr:
	PUSHW	%r0		# create register save frame
	PUSHW	%r1

	# the SAVE instruction puts %fp where %r2 belongs, modifies %fp
	SAVE	%r3
	# put the old value of %fp back in the register itself
	MOVW	-7*4(%sp),%fp
	# and in the correct slot in the register save area
	PUSHW	%fp
	# and then put register %r2 in the slot where %fp used to be
	MOVW	%r2,-8*4(%sp)

	PUSHW	%ap
	PUSHW	-13*4(%sp)	# copy up the GATE psw
	PUSHAW	-15*4(%sp)	# sp pointing to top of user stack
	ADDW2	&8,%sp		# create "garbage" holes for pcbp, isp
	PUSHW	-18*4(%sp)	# copy up the GATE pc

	PUSHAW	-16*4(%sp)	# arg 1 to _getflt
	MOVW	&SIGFPE*4,%r3	# offset into d.vect[] table
	PUSHW	d.vect(%r3)	# arg 2 to _getflt
_m4_ifdef_(`SHLIB',
`	CALL	-2*4(%sp),*_libc__getflthw
',
`	CALL	-2*4(%sp),_getflthw
')

	ARSW3	&2,%r3,%r3
	TSTB	setflg(%r3)	# check to see whether it is
				# sigset or signal.
				# if sigset, release held signal
	je	fnorelse
	PUSHW	%r3
	CALL	-4(%sp), sigrelse
fnorelse:

	# restore the user environment
	# the CALL/RET of _getflt restored r3 - r8, ap and fp
	# of the other registers only PC and PSW may have changed
	POPW	-19*4(%sp)	# copy PC back to GATE slot
	SUBW2	&3*4,%sp	# skip isp, pcbp, sp

	# to ensure proper work-around of RETG retartability problem,
	# copy only the OE,NZCV bits back to GATE PSW
	EXTFW	&(5-1),&18,-4(%sp),%r0
	INSFW	&(5-1),&18,%r0,-14*4(%sp)

	SUBW2	&9*4,%sp	# skip psw, fp, ap, r8 - r3
	POPW	%r2
	POPW	%r1
	POPW	%r0
	SUBW2	&1*4,%sp	#skip BSBH return address

	# return to exception point (possibly updated by _getflt)
	RETG
#
# **********
# Definition of signal entry point from the kernel
# **********
#
# SAVR: common entry point for all but SIGFPE
# The stack upon entry to savr is:
#
#	%sp ->	|		|
#		+---------------+
#		| tvect ret.	| <- address of tvect[n+1]
#		+===============+
#		| trapped PSW	| : GATE
#		+---------------+
#		| trapped PC	|
#		+===============+
#		| user stack	|
#
# Before doing anything we must save r0 - r8, ap, and fp,
# check the floating-point hardware status, and save fp hardware state.
# If the user handler is in assembly, it may not do a "SAVE" on entry.
# We then use the tvect return address to calculate our signal number,
# call the user handler, restore the registers, and RETG to the user.
savr:
	PUSHW	%r0			# save user environment
	PUSHW	%r1	# this sequence saves r0 - r8, and fp. The ap
	PUSHW	%r2	# is saved/restored by the CALL/RET sequence.
	SAVE	%r3

	# calculate the signal number from the tvect return point
	# BEFORE saving the floating-point state.
	# %r3 := the number of bytes between tvect[1] and tvect[n+1]
	# (where n == the current signal number).
	SUBW3	&.entry2,-11*4(%sp),%r3

	# determine floating-point hardware equippage
	TSTW	_fp_hw
	je	.sav_done		# jump if no hardware

.sav_mau:	# MAU hardware (_fp_hw == +1)
	# save all MAU floating-point registers (DR, f0-f3)
	ADDW2	&(5*3 + 1)*4,%sp	# regs, plus temp slot for ASR

	# if one of the regs has a denormalized number in it,
	# trying to read it can cause an underflow exception,
	# unless we temporarily turn off exceptions.
	.data
	.align	4
.asr_csc:
	.word	ASR_CSC
	.text
	mmovfa	     -1*4(%sp)
	mmovfd	    -16*4(%sp)		# must save DR before movta
	mmovta	.asr_csc

	mmovxx	%x0,-13*4(%sp)
	mmovxx	%x1,-10*4(%sp)
	mmovxx	%x2, -7*4(%sp)
	mmovxx	%x3, -4*4(%sp)

	orw2	&(ASR_CSC),-1*4(%sp)
	mmovta	     -1*4(%sp)
	subw2	&(1*4),%sp

.sav_done:	# floating-point state save complete
	DIVW2	&TSIZE,%r3		# convert tvect offset to index
	PUSHW	%r3			# argument to user handler
	ALSW3	&2,%r3,%r3		# convert index to d.vect offset


# the user handler is passed one argument: the signal number
	CALL	-1*4(%sp),*d.vect(%r3)	# user handler
	ARSW3	&2,%r3,%r3
	TSTB	setflg(%r3)		# check to see whether it is
					# sigset or signal	
					# if sigset, release held signal	
	je	norelse

	PUSHW	%r3
	CALL	-4(%sp),sigrelse
norelse:
	# unwind the floating-point data from the stack
	TSTW	_fp_hw
	je	.res_done		# jump if no hardware

.res_mau:	# MAU hardware (_fp_hw == +1)
	addw2	&(1*4),%sp
	mmovfa	-1*4(%sp)
	mmovta	.asr_csc

	mmovxx	 -4*4(%sp),%x3
	mmovxx	 -7*4(%sp),%x2
	mmovxx	-10*4(%sp),%x1
	mmovxx	-13*4(%sp),%x0

	orw2	&(ASR_CSC),-1*4(%sp)
	mmovta	-1*4(%sp)
	mmovtd	-16*4(%sp)		# must restore DR after ASR

	SUBW2	&(5*3 + 1)*4,%sp

.res_done:	# floating-point state restore complete

# The user handler may have changed the setting of OE, which is
# a global property.  Copy the current value back into the saved PSW.
# To avoid the RETG restartability problem, we must be sure NOT to
# touch the <CM> field of the saved PSW.
	EXTFW	&(1-1),&22,%psw,%r0
	INSFW	&(1-1),&22,%r0,-12*4(%sp)

	RESTORE	%r3			# restore user environment
	POPW	%r2
	POPW	%r1
	POPW	%r0

	SUBW2	&4,%sp			# skip BSBH return PC

	RETG				# return to point of trap
