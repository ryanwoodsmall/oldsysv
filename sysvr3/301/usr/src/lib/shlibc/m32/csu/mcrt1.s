#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.file	"mcrt1.s"
	.ident	"@(#)libc-m32:csu/mcrt1.s	1.2"
#	C runtime startup and exit with profiling
#

	.set	CBUFS,600
	.set	WORDSIZE,4

# exit() is system call #1, _exit == (call #)*(sizeof(gatevector))
	.set	_exit,1*8

# global entities defined in this file
	.globl	_start
	.globl	_istart
	.globl	countbase
	.globl	environ
	.globl	exit

# global entities defined elsewhere, but used here
	.globl	___Argv		# libc: port/gen/mon.c defines this
	.globl	__fpstart	# libc: floating-point startup
	.globl	_cleanup	# libc: I/O cleanup
	.globl	etext		# set by 'ld' to end of text
	.globl	main		# user's entry point
	.globl	monitor		# libc: monitor(3C)
	.globl	sbrk		# libc: brk(2) system call
	.globl	write		# libc: write(2) system call

#
#	C language startup routine with profiling
#

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
# since profiling uses floating-point on the 3B15/3B5
	CALL	0(%sp),__fpstart

#	setup for monitoring
#
	SUBW3	&eprol,&(etext+7),%r8		# get text size in double words
	LRSW3	&3,%r8,%r8			#    (tally area == %r8 shorts)

	LLSW3	&1,%r8,%r8			# tally area size in bytes
	ADDW2	&8*CBUFS+12+WORDSIZE-1,%r8	# add in entry counts and header and
	ANDW2	&-WORDSIZE,%r8			#	round to word boundary
	PUSHW	%r8				# get space
	CALL	-1*4(%sp),sbrk
	CMPW	&-1,%r0
	je	nospace

	ADDW3	&12,%r0,countbase		# set up count base for mcount
	PUSHAW	eprol				# start profiling
	PUSHAW	etext
	PUSHW	%r0
	LRSW3	&1,%r8,%r8			# monitor wants # of shorts in buffer
	PUSHW	%r8
	PUSHW	&CBUFS
	CALL	-5*4(%sp),monitor		# monitor(lowpc,highpc,buffer,bufsiz,CBUFS)

	CALL	-3*4(%sp),main			# main(argc,argv,environ)

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
nospace:
	PUSHW	&2				# write error message and exit
	PUSHAW	emesg
	PUSHW	&MESSL
	CALL	-3*4(%sp),write			# write(2,emesg,MESSL)

	PUSHW	&-1
	CALL	-1*4(%sp),nospacex		# exit(-1)

nospacex:
	MOVW	&4,%r0				# exit
	MOVW	&_exit,%r1
	GATE

	.data
	.align	4
environ:
	.word	0

countbase:
	.word	0

emesg:		# "No space for monitor buffer\n"
	.byte	78,111,32,115,112,97,99,101,32,102,111,114
	.byte	32,109,111,110,105,116,111,114
	.byte	32,98,117,102,102,101,114,10
	.set	MESSL,.-emesg
	.byte	0

	.text
	.align	4	# generate padding NOP's before eprol
eprol:			# beginning of user text

	.section	.init,"x"
_istart:
	SAVE	%fp
