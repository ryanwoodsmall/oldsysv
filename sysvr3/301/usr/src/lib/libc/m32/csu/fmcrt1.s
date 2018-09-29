#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"fmcrt1.s"
	.ident	"@(#)libc-m32:csu/fmcrt1.s	1.4"

#
#	C language startup routine with compatibility
#	floating point startup, and profileing
#	This startup is used when objects from 2.0 CCS or F77 1.1
#	is linked with objects produced by the 2.0p CCC
#
	.set	_exit,1*8
	.set	_signal,48*8
	.set	SIGILL,4
	.set	SIG_IGN,1
	.set	SIG_DFL,0

	.set	CBUFS,600
	.set	WORDSIZE,4


# exit() is system call #1, _exit == (call #)*(sizeof(gatevector))
	.set	_exit,1*8

# global entities defined in this file
	.globl	_start		# entry point of this file
	.globl	_istart
	.globl	countbase
	.globl	environ
	.globl	exit

# global entities defined elsewhere, but used here
	.globl	___Argv		# libc: port/gen/mon.c defines this
	.globl	__fpstart	# libc: floating-point startup
	.globl	_cleanup	# libc: I/O cleanup
	.globl	fptrap		# floating point emulation for 2.0
	.globl	etext		# set by 'ld' to end of text
	.globl	main		# user's entry point
	.globl	monitor		# libc: monitor(3C)
	.globl	sbrk		# libc: brk(2) system call
	.globl	write		# libc: write(2) system call


	.data
	.globl	d.vect		# in file signal.s
				# global flag tells signal.s that
	.comm  _old.fp,4

				# old floating point emulation
				# is being used, and the process
				# is tracing, which renders
				# special meaning to SIGILL
	        		# if tracing is not used,
				# signal SIGILL is NOT special
				# _old.fp is defined in m32/gen/m32_data.s
########################################################################
	.text

_start:
	PUSHW	0(%ap)		# argc

	MOVAW	4(%ap),%r0
	PUSHW	%r0		# argv
	MOVW	%r0,___Argv	# make ___Argv == argv for profiling
.L1:
	TSTW	0(%r0)		# null args term ?
	je	.L2
	ADDW2	&4,%r0
	jmp	.L1
.L2:
	MOVAW	4(%r0),%r0
	MOVW	%r0,environ	# indir is 0 if no env ; not 0 if env

	PUSHW	%r0		# envp

	CALL    0(%sp),_istart  # execute initialization code

# MUST initialize floating-point state before calling profile code,
# since profiling uses floating-point

	CALL	0(%sp),__fpstart

#	intercept floating point illegal op-codes using either undocumented
#	fast interface through sys3b system call or through normal signal
#	system call
#
	PUSHW	&6
	PUSHAW	.fastiop
	CALL	-8(%sp),.setiop	# sys3b( S3BIOP, .fastiop )
	TSTW	%r0
	je	.L3		# fast interface is now set; otherwise use signal

	PUSHW	&SIGILL
	PUSHAW	.sigill
	CALL	-8(%sp),.catch	# signal( SIGILL, .sigill )
	MOVW	&1,_old.fp	# next time SIGILL has special meaning
.L3:

#	setup for monitoring
#
	SUBW3	&.eprol,&(etext+7),%r8		# get text size in double words
	LRSW3	&3,%r8,%r8			#    (tally area == %r8 shorts)

	LLSW3	&1,%r8,%r8			# tally area size in bytes
	ADDW2	&8*CBUFS+12+WORDSIZE-1,%r8	# add in entry counts and header and
	ANDW2	&-WORDSIZE,%r8			#	round to word boundary
	PUSHW	%r8				# get space
	CALL	-1*4(%sp),sbrk
	CMPW	&-1,%r0
	je	.nospace

	ADDW3	&12,%r0,countbase		# set up count base for mcount
	PUSHAW	.eprol				# start profiling
	PUSHAW	etext
	PUSHW	%r0
	LRSW3	&1,%r8,%r8			# monitor wants # of shorts in buffer
	PUSHW	%r8
	PUSHW	&CBUFS
	CALL	-5*4(%sp),monitor		# monitor(lowpc,highpc,buffer,bufsiz,CBUFS)

	CALL	-3*4(%sp),main	# main(argc,argv,environ)

	PUSHW	%r0
	CALL	-1*4(%sp),exit

#	exit, but first call monitor() to write profile buffer
#
exit:
	PUSHW	&0
	PUSHW	&0
	PUSHW	&0
	PUSHW	&0
	PUSHW	&0
	CALL	-5*4(%sp),monitor		# terminate monitoring

	CALL	0(%sp),_cleanup			# clean up I/O buffers

	MOVW	&4,%r0				# exit
	MOVW	&_exit,%r1
	GATE

#	not enough memory for profiling buffer
#
.nospace:
	PUSHW	&2				# write error message and exit
	PUSHAW	.emesg
	PUSHW	&MESSL
	CALL	-3*4(%sp),write			# write(2,.emesg,MESSL)

	PUSHW	&-1
	CALL	-1*4(%sp),.nospacex		# exit(-1)

.nospacex:
	MOVW	&4,%r0				# exit
	MOVW	&_exit,%r1
	GATE

	.data
	.align	4
environ:
	.word	0

countbase:
	.word	0

.emesg:		# "No space for monitor buffer\n"
	.byte	78,111,32,115,112,97,99,101,32,102,111,114
	.byte	32,109,111,110,105,116,111,114
	.byte	32,98,117,102,102,101,114,10
	.set	MESSL,.-.emesg
	.byte	0

	.text
	.align	4	# generate padding NOP's before .eprol
.eprol:			# beginning of user text

#
#	issue the sys3b system call
#
	.set	_sys3b,50*8

	.text
.setiop:
	MOVW	&4,%r0
	MOVW	&_sys3b,%r1
	GATE
	jlu	.setiopbad
	CLRW	%r0
	RET
.setiopbad:
	MOVW	&-1,%r0		# didn't work; must be tracing with ptrace
	RET

#
#	The following routine issues the signal system call to catch SIGILL
#	interrupts in order to intercept floating point instructions
#
.catch:
	MOVW	&4,%r0			# issue system call
	MOVW	&_signal,%r1
	GATE
	RET

#
#	The following is the register parameter area for the
#	fptrap floating point simulator
#
	.data
	.align	4
.regs:
	.zero	16*4		# room for 16 registers

	.set	R0,0*4		# offsets into .regs
	.set	R1,1*4
	.set	R2,2*4
	.set	R3,3*4
	.set	R4,4*4
	.set	R5,5*4
	.set	R6,6*4
	.set	R7,7*4
	.set	R8,8*4
	.set	FP,9*4
	.set	AP,10*4
	.set	PSW,11*4
	.set	SP,12*4
	.set	PCBP,13*4
	.set	ISP,14*4
	.set	PC,15*4

#
#	The following routines recieves control for a SIGILL signal
#	or from the sys3b fast interface for illegal op-codes.  It
#	determines if the instruction is a floating point instruction or
#	not.  If it is a floating point instruction, then the floating
#	point simulator (fptrap) is called; otherwise, this routine
#	plays kernel and delivers the SIGILL signal according to the
#	option set by the user with a previous signal(SIGILL,?) call.
#
	.text

	.globl	fptrap

#	sys3b fast interface for illegal op-codes
#
.fastiop:
	MOVW	%r0,.regs+R0	# save environment
	MOVW	%r1,.regs+R1
	MOVW	%r2,.regs+R2
	MOVW	%r3,.regs+R3
	MOVW	%r4,.regs+R4
	MOVW	%r5,.regs+R5
	MOVW	%r6,.regs+R6
	MOVW	%r7,.regs+R7
	MOVW	%r8,.regs+R8
	MOVW	%fp,.regs+FP
	MOVW	%ap,.regs+AP
	MOVW	-4(%sp),.regs+PSW
	MOVAW	-8(%sp),.regs+SP
	CLRW	.regs+PCBP
	CLRW	.regs+ISP
	MOVW	-8(%sp),.regs+PC

	MOVB	*.regs+PC,%r0	# op-code of illegal instruction
	TSTB	_fpopcode(%r0)
	jne	.isfp

#	not a floating point op-code

	CMPW	d.vect+(4*SIGILL),&SIG_DFL
	je	.notfp4			# default action

	CMPW	d.vect+(4*SIGILL),&SIG_IGN
	jne	.notfp3			# call user signal handler
	jmp	.notfp1			# ignore signal; MAC32 will loop!


#	SIGILL illegal op-code processing
#
.sigill:
	MOVW	%r0,.regs+R0	# save environment
	MOVW	%r1,.regs+R1
	MOVW	%r2,.regs+R2
	MOVW	%r3,.regs+R3
	MOVW	%r4,.regs+R4
	MOVW	%r5,.regs+R5
	MOVW	%r6,.regs+R6
	MOVW	%r7,.regs+R7
	MOVW	%r8,.regs+R8
	MOVW	%fp,.regs+FP
	MOVW	%ap,.regs+AP
	MOVW	-4(%sp),.regs+PSW
	MOVAW	-8(%sp),.regs+SP
	CLRW	.regs+PCBP
	CLRW	.regs+ISP
	MOVW	-8(%sp),.regs+PC

	MOVB	*.regs+PC,%r0	# op-code of illegal instruction
	TSTB	_fpopcode(%r0)
	je	.notfp
.isfp:
	PUSHAW	.regs		# call floating point simulator
	CALL	-4(%sp),fptrap

	MOVW	.regs+PC,-8(%sp)	# restore environment and return
	EXTFW	&(4-1),&18,.regs+PSW,%r0	# PSW flag bits
	INSFW	&(4-1),&18,%r0,-4(%sp)
	MOVW	.regs+AP,%ap
	MOVW	.regs+FP,%fp
	MOVW	.regs+R8,%r8
	MOVW	.regs+R7,%r7
	MOVW	.regs+R6,%r6
	MOVW	.regs+R5,%r5
	MOVW	.regs+R4,%r4
	MOVW	.regs+R3,%r3
	MOVW	.regs+R2,%r2
	MOVW	.regs+R1,%r1
	MOVW	.regs+R0,%r0
	RETG

.notfp:					# SIGILL was not caused by a
					# floating point instruction

	CMPW	d.vect+(4*SIGILL),&SIG_DFL
	je	.notfp2			# default action

	CMPW	d.vect+(4*SIGILL),&SIG_IGN
	je	.notfp1			# ignore signal; MAC32 will loop!
.notfp3:
	PUSHW	&SIGILL			# call user signal handler
	CALL	-4(%sp),*d.vect+(4*SIGILL)

.notfp1:
	MOVW	.regs+AP,%ap		# restore environment and return
	MOVW	.regs+FP,%fp
	MOVW	.regs+R8,%r8
	MOVW	.regs+R7,%r7
	MOVW	.regs+R6,%r6
	MOVW	.regs+R5,%r5
	MOVW	.regs+R4,%r4
	MOVW	.regs+R3,%r3
	MOVW	.regs+R2,%r2
	MOVW	.regs+R1,%r1
	MOVW	.regs+R0,%r0
	RETG

.notfp2:
	PUSHW	&SIGILL			# reset SIGILL catching
	PUSHW	&SIG_DFL
	CALL	-8(%sp),.catch
	jmp	.notfp1

.notfp4:
	PUSHW	&6			# reset sys3b fast interface
	PUSHAW	0
	CALL	-8(%sp),.setiop
	jmp	.notfp1
	

#
#	The following is the table of floating point op-codes; zero means
#	not floating point and non-zero means floating point.
#
	.data
	.globl	_fpopcode
_fpopcode:
	.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0		# 00-0F
	.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0		# 10-1F
	.byte	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0		# 20-2F
	.byte	0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0		# 30-3F
	.byte	0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0		# 40-4F
	.byte	0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0		# 50-5F
	.byte	0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0		# 60-6F
	.byte	0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0		# 70-7F
	.byte	0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0		# 80-8F
	.byte	0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0		# 90-9F
	.byte	0,1,0,0,0,1,0,0,0,1,0,0,0,0,0,0		# A0-AF
	.byte	0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0		# B0-BF
	.byte	0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0		# C0-CF
	.byte	0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0		# D0-DF
	.byte	0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0		# E0-EF
	.byte	0,1,0,0,0,1,0,0,0,0,0,0,0,0,0,0		# F0-FF

	.section	.init,"x"
_istart:
	SAVE	%fp
