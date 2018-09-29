#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.


	.ident	"@(#)kern-port:boot/lboot/misc.s	10.2"
#
#	LBOOT ENTRY POINT
#
#	Lboot is called by mboot (which is operating on the IAU
#	stack).  The IAU stack is only 1K (0x30100 - 0x30500) for
#	release 1.2 of the 3B5.  This code will switch the stack
#	to address &sstack (length estack-sstack); the stack
#	bounds in the current PCB will also be adjusted.
#

	.globl	sstack
	.globl	estack
	.globl	main
	.globl	start
start:
	SAVE	%r8
	MOVW	16(%pcbp),%r8
	MOVAW	estack-8,16(%pcbp)	# set stack upper bound

	MOVW	%sp,%r0
	MOVAW	sstack,%sp
	CALL	0(%r0),main		# use CALL so unwinding is easy

	MOVW	%r8,16(%pcbp)
	RESTORE	%r8
	RET




#
#	min() and max() routines for signed and unsigned arithmetic
#

#	signed min()
#
	.globl	min
min:
	MOVW	0(%ap),%r0
	CMPW	%r0,4(%ap)
	BGEB	minxit
	MOVW	4(%ap),%r0
minxit:
	RET


#	signed max()
#
	.globl	max
max:
	MOVW	0(%ap),%r0
	CMPW	%r0,4(%ap)
	BLEB	maxit
	MOVW	4(%ap),%r0
maxit:
	RET

#	unsigned min()
#
	.globl	umin
umin:
	MOVW	0(%ap),%r0
	CMPW	%r0,4(%ap)
	BGEUB	uminxit
	MOVW	4(%ap),%r0
uminxit:
	RET


#	unsigned max()
#
	.globl	umax
umax:
	MOVW	0(%ap),%r0
	CMPW	%r0,4(%ap)
	BLEUB	umaxit
	MOVW	4(%ap),%r0
umaxit:
	RET




#
#	This is the block copy routine.
#
#		bcopy(from, to, count)
#		caddr_t from, to;
#		{
#			while( count-- > 0 )
#				*to++ = *from++;
#		}
#

	.globl	bcopy

bcopy:
	SAVE	%r6

	MOVW	0(%ap),%r0		# from-address to %r0
	MOVW	4(%ap),%r1		# to-address to %r1
	MOVW	8(%ap),%r6		# byte count to %r6
 	je	bcret

	ANDW3	%r0,&0x03,%r7		# get alignment of addresses
	ANDW3	%r1,&0x03,%r8

	SUBW3	%r7,%r8,%r2
	je	bcalw			# word aligned addresses
	BITW	&0x01,%r2
	je	bcalh			# half-word aligned addresses

#
#	byte-aligned addresses; just copy a byte at a time
#
bcalb:
	MOVB	0(%r0),0(%r1)		# copy bytes
	INCW	%r0
	INCW	%r1
	DECW	%r6
	jne	bcalb
	jmp	bcret

#
#	half-word aligned addresses; copy a half-word at a time
#
bcalh:
	BITW	&0x01,%r0		# copy any initial odd-aligned byte
	je	bcalh1

	MOVB	0(%r0),0(%r1)
	INCW	%r0
	INCW	%r1
	DECW	%r6
	je	bcret

bcalh1:
	LRSW3	&1,%r6,%r2		# number of half-words
	je	bcalh2

bcalhmv:
	MOVH	0(%r0),0(%r1)		# copy half-words
	ADDW2	&2,%r0
	ADDW2	&2,%r1
	DECW	%r2
	jne	bcalhmv

	BITW	&0x01,%r6		# copy any trailing odd-aligned byte
	je	bcret

bcalh2:
	MOVB	0(%r0),0(%r1)
	jmp	bcret

#
#	word aligned addresses; use block move instruction
#
bcalw:
	BITW	&0x03,%r0		# copy any initial unaligned bytes
	je	bcalw1

	MOVB	0(%r0),0(%r1)
	INCW	%r0
	INCW	%r1
	DECW	%r6
	jne	bcalw
	jmp	bcret

bcalw1:
	LRSW3	&2,%r6,%r2		# number of words
	je	bcalw2

	MOVBLW				# copy words

	ANDW2	&0x03,%r6		# bytes remaining to be copied
	je	bcret

bcalw2:
	MOVB	0(%r0),0(%r1)		# copy any trailing unaligned bytes
	INCW	%r0
	INCW	%r1
	DECW	%r6
	jne	bcalw2
	
bcret:
	RESTORE	%r6
	RET



#
#	Setjmp() and Longjmp() -- these routines are used exclusively by the
#	routines which implement the ON-CONDITION feature
#

	.globl	setjmp
setjmp:
	MOVW	0(%ap),%r0
	MOVW	%r3,0(%r0)
	MOVW	%r4,4(%r0)
	MOVW	%r5,8(%r0)
	MOVW	%r6,0xc(%r0)
	MOVW	%r7,0x10(%r0)
	MOVW	%r8,0x14(%r0)
	MOVW	-4(%sp),0x18(%r0)	# old %ap
	MOVW	-8(%sp),0x1c(%r0)	# old %pc
	MOVW	%ap,0x20(%r0)		# old %sp
	MOVW	%fp,0x24(%r0)		# old %fp
	CLRW	%r0
	RET

	.globl	longjmp
longjmp:
	MOVW	0(%ap),%r0
	MOVW	0(%r0),%r3
	MOVW	4(%r0),%r4
	MOVW	8(%r0),%r5
	MOVW	0xc(%r0),%r6
	MOVW	0x10(%r0),%r7
	MOVW	0x14(%r0),%r8
	MOVW	0x18(%r0),%ap
	MOVW	0x1c(%r0),%r1
	MOVW	0x20(%r0),%sp
	MOVW	0x24(%r0),%fp
	MOVW	&1,%r0
	JMP	0(%r1)


#
#	ON-CONDITION assembler assists
#

	.globl	JSBONEXIT
	.data
	.align	4
JSBONEXIT:
	JSB	_on_exit

	.text
_on_exit:
	PUSHW	%r0		#save r0

	CALL	0(%sp),on_exit

	MOVW	%r0,-8(%sp)	#original return address
	POPW	%r0		#restore r0
	RSB			#return to original return address




#
#	strlen( string )
#

	.globl	strlen
strlen:
	MOVW	0(%ap),%r0
	BEB	strret
	STREND
	SUBW2	0(%ap),%r0
strret:	RET



#
#	strcpy( s1, s2 )
#

	.globl	strcpy
strcpy:
	MOVW	0(%ap),%r1	# destination
	MOVW	4(%ap),%r0	# source
	STRCPY
	MOVW	0(%ap),%r0	# return(s1)
	RET


#
#	SPL - Set interrupt priority level routine; used if TEST is #define'd
#

	.globl	splx
splx:
	EXTFW	&3,&13,%psw,%r0
	INSFW	&3,&13,0(%ap),%psw	# set interrupt priority level
	RET



#
#	getsp()
#
#	Return value of caller's %sp; used if TEST is #define'd
#

	.globl	getsp
getsp:
	MOVW	%ap,%r0
	RET
