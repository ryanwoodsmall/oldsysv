#	Copyright (c) 1984 AT&T
#	  All Rights Reserved

#	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T
#	The copyright notice above does not evidence any
#	actual or intended publication of such source code.

	.ident	"@(#)kern-port:ml/misc.s	10.9"

#	The following set's are used in iocde.

	.set	u_pcb, 0xc	# Offset to u_pcb in u-block.
	.set	u_pcbp, 0x6c	# Offset to u_pcbp in u-block.
	.set	slb, 0xc	# Offset to slb in pcb.
	.set	u_stack, 0x530	# Offset to u_stack in u-block.
	.set	u_ofile, 0x6e0	# Offset to u_ofile in u-block.
	.set	u_pofile, 0x534	# Offset to u_pofile in u-block.
	.set	u_caddrflt,0x130 # Offset to u_caddrflt in u-block.
	.set	v_nofiles, 0x50	# Offset to v_nofiles in v (var.h).
	.set	u_procp, 0x4e4	# Offset to u_procp in u-block.
	.set	p_sysid, 0x86	# Offset to p_sysid in proc table.

	.text
#
#	addupc( pc, &u.u_prof, tics )
#

	.globl	addupc

#
#   NOTE -- CHANGES TO USER.H MAY REQUIRE CORRESPONDING CHANGES TO THE
#		FOLLOWING .SET's.
#
	.set	pr_base,0	# buffer base, short *
	.set	pr_size,4	# buffer size, unsigned int
	.set	pr_off,8	# pc offset, unsigned int
	.set	pr_scale,12	# pc scaling, unsigned int

addupc:
	MOVW	4(%ap),%r2		# &u.u_prof
	SUBW3	pr_off(%r2),0(%ap),%r0	# corrected pc
	jlu	addret2
	LRSW3	&1,%r0,%r0		# PC >> 1
	LRSW3	&1,pr_scale(%r2),%r1	# scale >> 1
	MULW2	{uword}%r1,%r0
	LRSW3	&14,%r0,%r0
	INCW	%r0
	ANDW2	&0XFFFFFFFE,%r0
	CMPW	%r0,pr_size(%r2)	# length
	jleu	addret2
	ADDW2	pr_base(%r2),%r0	# base
	MOVAW	adderr,u+u_caddrflt
	ADDW2	8(%ap),{shalf}0(%r0)
addret1:
	CLRW	u+u_caddrflt
addret2:
	RET
adderr:
	CLRW	pr_scale(%r2)
	jmp	addret1


#
#	SPL - Set interrupt priority level routines
#

	.globl	spl1
spl1:
	MOVW	%psw,%r0
	INSFW	&3,&13,&8,%psw
	RET

	.globl	spl4
spl4:
	MOVW	%psw,%r0
	INSFW	&3,&13,&10,%psw
	RET

	.globl	spl5
spl5:
	MOVW	%psw,%r0
	INSFW	&3,&13,&10,%psw		# device interrupts
	RET

	.globl	spl6
spl6:
	MOVW	%psw,%r0
	INSFW	&3,&13,&12,%psw
	RET

	.globl	splpp
splpp:
	MOVW	%psw,%r0
	INSFW	&3,&13,&10,%psw
	RET

	.globl	splni
splni:
	MOVW	%psw,%r0
	INSFW	&3,&13,&12,%psw
	RET

	.globl	spltty
spltty:
	MOVW	%psw,%r0
	INSFW	&3,&13,&13,%psw
	RET

	.globl	spl7
spl7:
	MOVW	%psw,%r0
	ORW2	&0x1e000,%psw
	RET

	.globl	splhi
splhi:
	MOVW	%psw,%r0
	ORW2	&0x1e000,%psw
	RET

	.globl	spl0
spl0:
	MOVW	%psw,%r0
	ANDW2	&0xfffe1fff,%psw
	RET

	.globl	splx
splx:
	MOVW	%psw,%r0
	MOVW	0(%ap),%psw
	RET


#
# This routine establishes the call frames for the virtual portion
# of the UNIX boot and calls mlsetup(), passing it memory managemnet
# information as parameters.
#
# Register 8 contains the first free click,set by pstart.
# Register 7 contains the autoconfig flag passed by lboot, 
# 	that is used to determine when auto configuration was done
#	by lboot.

	.globl	vstart_s
	.globl	mlsetup
	.globl	cmn_err
	.globl	u
	.globl	v
	.globl	u400
	.globl  sys3bautoconfig
	.globl	nofiles_min
	.globl	nofiles_max
	.globl	nofiles_cfg

vstart_s:
	MOVW	%r7,sys3bautoconfig
	MOVW	v+v_nofiles,%r0		# Get max nbr of open files.
	MOVW	%r0,nofiles_cfg		# Save it for later.
	CMPW	%r0,nofiles_min		# Is configured value too small?
	BLEB	vstart_s_1		# No.  Go check max.
	MOVW	nofiles_min,%r0		# Yes.  Use min value.
	BRB	vstart_s_2		# Merge with good value.

vstart_s_1:
	CMPW	%r0,nofiles_max		# Is configured value too big?
	BGEB	vstart_s_2		# No.  Merge with good value.
	MOVW	nofiles_max,%r0		# Yes.  Use max value.

vstart_s_2:
	MOVW	%r0,v+v_nofiles		# Save actual value we used.
	LLSW3	&2,%r0,%r1		# Each entry is ptr to file tbl.
	ADDW2	&u+u_ofile,%r1		# Get ptr to end of u_ofiles.
	MOVW	%r1,u+u_pofile		# This is where u_pofile starts.
	ADDW2	%r0,%r1			# One byte per u_pofile entry.
	ADDW2	&3,%r1			# Round up to work boundary to
	ANDW2	&-4,%r1			# get ptr to start of stack.
	MOVW	%r1,u+u_stack		# Save start of kernel stack.
	MOVW	%r1,%sp			# Load the stack pointer.
	MOVW	%sp,%fp			# Set frame pointer.
	MOVW	%sp,%ap			# Set argument pointer.
	MOVW	%sp,u_slb(%pcbp)	# Set current stack lower bound.
	PUSHW	%r8			# Physclick.
	CALL	-4(%sp),mlsetup
	TSTB	u400
	BEB	L1_300
	ANDW2	&0x20104,%psw
	BRB	L1_400

L1_300:
	ANDW2	&0x2820104,%psw
	ORW2	&0x2800000,%psw

L1_400:
	CALL	0(%sp),main

#
#	if bit 31 is set, go to user process,
#	otherwise, call a kernel function.
#
	BITW	&0x80000000,%r0
	BEB	krnlmode
	PUSHW	%r0
	TSTB	u400
	BEB	L2_300
	PUSHW	&0x0700
	CFLUSH
	RETG

L2_300:
	PUSHW	&0x2800700
	RETG

krnlmode:
	CALL	0(%sp),0(%r0)
	PUSHW	&3
	PUSHAW	message
	CALL	-(2*4)(%sp),cmn_err
	jmp	.

	.data
message:	#Illegal return from main()
	.byte	0x49,0x6c,0x6c,0x65,0x67,0x61,0x6c,0x20,0x72,0x65
	.byte	0x74,0x75,0x72,0x6e,0x20,0x66,0x72,0x6f,0x6d,0x20
	.byte	0x6d,0x61,0x69,0x6e,0x28,0x29,0
	.text


#
# This is the system wait routine
#
	.globl	idle
idle:
	INSFW	&3,&13,&8,%psw	# Enable all but swtch() interrupts.
	WAIT
_waitloc:
	RET

	.data

	.globl	waitloc
	.align 4
waitloc:
	.word	_waitloc

	.text


#
#	min() and max() routines
#

	.globl	min
min:
	MOVW	0(%ap),%r0
	CMPW	%r0,4(%ap)
	jgeu	minxit
	MOVW	4(%ap),%r0
minxit:
	RET


	.globl	max
max:
	MOVW	0(%ap),%r0
	CMPW	%r0,4(%ap)
	jleu	maxit
	MOVW	4(%ap),%r0
maxit:
	RET

#
#	rtrue() and rfalse() routines supported by self-config
#

	.globl rtrue
rtrue:
	MOVW	&1,%r0
	RET

	.globl rfalse
rfalse:
	CLRW	%r0
	RET

# bzeroab(addr, bytes) - used when faulting in the last page
# to clear non-word aligned addresses/sizes. Assumes that after
# clearing to word size, address will be word aligned.

	.globl	bzeroba
bzeroba:
	ANDW3	&0x3,4(%ap),%r0		# count of bytes to next word boundry
	BEB	bzero
	MOVW	0(%ap),%r1		# address of bytes to clear
	MOVAW	bzeroflt,u+u_caddrflt
bzclrb:
	CLRB	0(%r1)			# clear bytes
	INCW	%r1			# until at next word
	DECW	%r0
	BNEB	bzclrb
	ANDW2	&0xfffffffc, 4(%ap)	# set count to word boundry
	MOVW	%r1,0(%ap)		# reset address


#
#	This is the block zero routine.  It requires an even-word
#	address and an even-word size.
#
#		bzero(addr, bytes)
#		caddr_t addr;
#		{
#			bytes >>= 2;
#			while( bytes-- > 0 )
#				*addr++ = 0;
#		}
#

	.globl	bzero
	.globl	cmn_err

bzero:
	PUSHW	%r3
	MOVW	0(%ap),%r0		# address to %r0
	MOVW	4(%ap),%r2		# get size in bytes
#ifdef	DEBUG
	BITB	&0x03,%r0
	BNEH	bzpanic			# ... address not word aligned
	BITB	&0x03,7(%ap)
	BNEH	bzpanic			# ... size not word aligned
#endif
	MOVAW	bzeroflt,u+u_caddrflt
	CMPW	&0x80,%r2		# If size < 128 just zero with
	BLH	bzsmall			# just 	zero, loop no helpee
	ANDW3	%r0,&0x7f,%r1		# how much of the current block
	ARSW3	&2,%r1,%r1		# bytes to words
	MULW2	&3,%r1			# words to loop offset
	PUSHW	%psw
	ORW2	&0x1e000,%psw	# block all interrupts whilst in loop
	NOP
	MOVAW	fbzlopflt,u+u_caddrflt	# psw has been pushed
	CLRW	0(%r0)			# in case page is not present
	ADDW2	%r0,%r2			# addr + size
	ANDW2	&0xffffff80,%r0		# set base for loop
	SUBW3	%r0,%r2,%r3
	ARSW3	&7,%r3,%r3		# no. of 128 byte blocks
	ANDW2	&0x7f,%r2		# number of bytes "left over"
	JMP	bzlpstrt(%r1)		# loop and addr 128 aligned!

fbzlop:
	PUSHW	%psw
	ORW2	&0x1e000,%psw	# block all interrupts whilst in loop
	NOP

	# The following move instructions are coded as text
	# to avoid NOP generation from SGS.  NOPs are not
	# necessary since interrupts are blocked.
	# If a page-fault is taken, it will happen on the first
	# CLRW, so no other NOPs are needed

bzlpstrt:
	.text
	.byte	0x80,0x50	# CLRW	0(%r0)
	NOP
	.byte	0x80,0xc0,0x04	# CLRW	4(%r0)
	.byte	0x80,0xc0,0x08	# CLRW	8(%r0)
	.byte	0x80,0xc0,0x0c	# CLRW	12(%r0)
	.byte	0x80,0xc0,0x10	# CLRW	16(%r0)
	.byte	0x80,0xc0,0x14	# CLRW	20(%r0)
	.byte	0x80,0xc0,0x18	# CLRW	24(%r0)
	.byte	0x80,0xc0,0x1c	# CLRW	28(%r0)
	.byte	0x80,0xc0,0x20	# CLRW	32(%r0)
	.byte	0x80,0xc0,0x24	# CLRW	36(%r0)
	.byte	0x80,0xc0,0x28	# CLRW	40(%r0)
	.byte	0x80,0xc0,0x2c	# CLRW	44(%r0)
	.byte	0x80,0xc0,0x30	# CLRW	48(%r0)
	.byte	0x80,0xc0,0x34	# CLRW	52(%r0)
	.byte	0x80,0xc0,0x38	# CLRW	56(%r0)
	.byte	0x80,0xc0,0x3c	# CLRW	60(%r0)
	.byte	0x80,0xc0,0x40	# CLRW	64(%r0)
	.byte	0x80,0xc0,0x44	# CLRW	68(%r0)
	.byte	0x80,0xc0,0x48	# CLRW	72(%r0)
	.byte	0x80,0xc0,0x4c	# CLRW	76(%r0)
	.byte	0x80,0xc0,0x50	# CLRW	80(%r0)
	.byte	0x80,0xc0,0x54	# CLRW	84(%r0)
	.byte	0x80,0xc0,0x58	# CLRW	88(%r0)
	.byte	0x80,0xc0,0x5c	# CLRW	92(%r0)
	.byte	0x80,0xc0,0x60	# CLRW	96(%r0)
	.byte	0x80,0xc0,0x64	# CLRW	100(%r0)
	.byte	0x80,0xc0,0x68	# CLRW	104(%r0)
	.byte	0x80,0xc0,0x6c	# CLRW	108(%r0)
	.byte	0x80,0xc0,0x70	# CLRW	112(%r0)
	.byte	0x80,0xc0,0x74	# CLRW	116(%r0)
	.byte	0x80,0xc0,0x78	# CLRW	120(%r0)
	.byte	0x80,0xc0,0x7c	# CLRW	124(%r0)
	ADDW2	&128,%r0
	POPW	%psw
	DECW	%r3
	BGB	fbzlop

	# still have to clear whatever is left over
bzsmall:
	TSTW	%r2
	BLEB	bzdone
	ARSW3	&2,%r2,%r2		# bytes to words
	CLRW	0(%r0)			# zero first word
	DECW	%r2
	BLEB	bzdone
	MOVAW	4(%r0),%r1		# zero remainder to 128 byte mult.
	MOVBLW	
bzdone:
	CLRW	u+u_caddrflt
	CLRW	%r0
	POPW	%r3
	RET

# if the psw has been pushed, it needs to be restored before returning
fbzlopflt:
	POPW	%psw
# return an error
bzeroflt:
	CLRW	u+u_caddrflt
	POPW	%r3
	MOVW	&-1,%r0
	RET

#ifdef	DEBUG
bzpanic:
	PUSHW	&3
	PUSHAW	bzmsg			# panic( "alignment error message" )
	CALL	-(2*4)(%sp),cmn_err
	jmp	.


	.data
bzmsg:		#bzero() called with non-word aligned address/size
	.byte	0x62,0x7a,0x65,0x72,0x6f,0x28,0x29,0x20
	.byte	0x63,0x61,0x6c,0x6c,0x65,0x64,0x20,0x77
	.byte	0x69,0x74,0x68,0x20,0x6e,0x6f,0x6e,0x2d
	.byte	0x77,0x6f,0x72,0x64,0x20,0x61,0x6c,0x69
	.byte	0x67,0x6e,0x65,0x64,0x20,0x61,0x64,0x64
	.byte	0x72,0x65,0x73,0x73,0x2f,0x73,0x69,0x7a
	.byte	0x65,0x0

	.text
#endif


#
#	This is the block copy routine.
#
#		bcopy(from, to, bytes)
#		caddr_t from, to;
#		{
#			while( bytes-- > 0 )
#				*to++ = *from++;
#		}
#

	.globl	bcopy

bcopy:
	SAVE	%r6

	MOVW	0(%ap),%r0		# from-address to %r0
	MOVW	4(%ap),%r1		# to-address to %r1
	MOVW	8(%ap),%r6		# byte count to %r6
 	BEH	bcret

	ANDW3	%r0,&0x03,%r7		# get alignment of addresses
	ANDW3	%r1,&0x03,%r8
	BITB	&3,%r6			# check word aligned count
	BNEB	bcopygen
	ORW3	%r7,%r8,%r2		# check both addresses word aligned
	BEH	bcopyblk
bcopygen:
	SUBW3	%r7,%r8,%r2
	BEB	bcalw			# word aligned addresses
	BITW	&0x01,%r2
	BEB	bcalh			# half-word aligned addresses

#
#	byte-aligned addresses; just copy a byte at a time
#
bcalb:
	MOVB	0(%r0),0(%r1)		# copy bytes
	INCW	%r0
	INCW	%r1
	DECW	%r6
	BNEB	bcalb
	jmp	bcret

#
#	half-word aligned addresses; copy a half-word at a time
#
bcalh:
	BITW	&0x01,%r0		# copy any initial odd-aligned byte
	BEB	bcalh1

	MOVB	0(%r0),0(%r1)
	INCW	%r0
	INCW	%r1
	DECW	%r6
	BEB	bcret

bcalh1:
	LRSW3	&1,%r6,%r2		# number of half-words
	BEB	bcalh2

bcalhmv:
	MOVH	0(%r0),0(%r1)		# copy half-words
	ADDW2	&2,%r0
	ADDW2	&2,%r1
	DECW	%r2
	BNEB	bcalhmv

	BITW	&0x01,%r6		# copy any trailing odd-aligned byte
	BEB	bcret

bcalh2:
	MOVB	0(%r0),0(%r1)
	jmp	bcret

#
#	word aligned addresses; use block move instruction
#
bcalw:
	BITW	&0x03,%r0		# copy any initial unaligned bytes
	BEB	bcalw1

	MOVB	0(%r0),0(%r1)
	INCW	%r0
	INCW	%r1
	DECW	%r6
	BNEB	bcalw
	jmp	bcret

bcalw1:
	LRSW3	&2,%r6,%r2		# number of words
	BEB	bcalw2

	MOVBLW				# copy words

	ANDW2	&0x03,%r6		# bytes remaining to be copied
	BEB	bcret

bcalw2:
	MOVB	0(%r0),0(%r1)		# copy any trailing unaligned bytes
	INCW	%r0
	INCW	%r1
	DECW	%r6
	BNEB	bcalw2
	jmp	bcret

#
#	word aligned addresses and count; copy 128 byte blocks
#
bcopyblk:
	ARSW3	&2,%r6,%r2		# get size in words
	ANDW2	&0x1f,%r2		# move enough to get to 128 byte mult.
	BEB	bcybmain
	MOVBLW
bcybmain:
	ARSW3	&7,%r6,%r2		# no. of 128 byte blocks
	BEB	bcret
	JSB	fbclop			# copy remainder
bcret:
	CLRW	%r0			# return success flag
					# can fail for rfs
	RESTORE	%r6
	RET


#
#	fbcopy(from, to, n)
#
#	Fast memory to memory copy of n clicks.  Both from
#	and to addresses assumed to be word aligned.
#

	.globl	fbcopy

fbcopy:
	MOVW	0(%ap),%r0	# from address
	MOVW	4(%ap),%r1	# to address
	ALSW3	&4,8(%ap),%r2	# loop count=clicks * 16
	JSB	fbclop
	RET


#
#	Fast block copy subroutine.
#
#	%r0 = from address
#	%r1 = to address
#	%r2 = no. 128 byte blocks to be copied
#

	.globl	fbclop

fbclop:
	PUSHW	%psw
	ORW2	&0x1e000,%psw

	# The following move instructions are coded as text
	# to avoid NOP generation from SGS.  NOPs are not
	# necessary since interrupts are blocked.

	.text
fbclopstrt:
	.byte	0x84,0x50,0x51		# MOVW	0(%r0),0(%r1)
	NOP				# in case a fault is taken
	NOP				# (padding)
	.byte	0x84,0xc0,0x04,0xc1,0x04	# MOVW	4(%r0),4(%r1)
	.byte	0x84,0xc0,0x08,0xc1,0x08	# MOVW	8(%r0),8(%r1)
	.byte	0x84,0xc0,0x0c,0xc1,0x0c	# MOVW	12(%r0),12(%r1)
	.byte	0x84,0xc0,0x10,0xc1,0x10	# MOVW	16(%r0),16(%r1)
	.byte	0x84,0xc0,0x14,0xc1,0x14	# MOVW	20(%r0),20(%r1)
	.byte	0x84,0xc0,0x18,0xc1,0x18	# MOVW	24(%r0),24(%r1)
	.byte	0x84,0xc0,0x1c,0xc1,0x1c	# MOVW	28(%r0),28(%r1)
	.byte	0x84,0xc0,0x20,0xc1,0x20	# MOVW	32(%r0),32(%r1)
	.byte	0x84,0xc0,0x24,0xc1,0x24	# MOVW	36(%r0),36(%r1)
	.byte	0x84,0xc0,0x28,0xc1,0x28	# MOVW	40(%r0),40(%r1)
	.byte	0x84,0xc0,0x2c,0xc1,0x2c	# MOVW	44(%r0),44(%r1)
	.byte	0x84,0xc0,0x30,0xc1,0x30	# MOVW	48(%r0),48(%r1)
	.byte	0x84,0xc0,0x34,0xc1,0x34	# MOVW	52(%r0),52(%r1)
	.byte	0x84,0xc0,0x38,0xc1,0x38	# MOVW	56(%r0),56(%r1)
	.byte	0x84,0xc0,0x3c,0xc1,0x3c	# MOVW	60(%r0),60(%r1)
	.byte	0x84,0xc0,0x40,0xc1,0x40	# MOVW	64(%r0),64(%r1)
	.byte	0x84,0xc0,0x44,0xc1,0x44	# MOVW	68(%r0),68(%r1)
	.byte	0x84,0xc0,0x48,0xc1,0x48	# MOVW	72(%r0),72(%r1)
	.byte	0x84,0xc0,0x4c,0xc1,0x4c	# MOVW	76(%r0),76(%r1)
	.byte	0x84,0xc0,0x50,0xc1,0x50	# MOVW	80(%r0),80(%r1)
	.byte	0x84,0xc0,0x54,0xc1,0x54	# MOVW	84(%r0),84(%r1)
	.byte	0x84,0xc0,0x58,0xc1,0x58	# MOVW	88(%r0),88(%r1)
	.byte	0x84,0xc0,0x5c,0xc1,0x5c	# MOVW	92(%r0),92(%r1)
	.byte	0x84,0xc0,0x60,0xc1,0x60	# MOVW	96(%r0),96(%r1)
	.byte	0x84,0xc0,0x64,0xc1,0x64	# MOVW	100(%r0),100(%r1)
	.byte	0x84,0xc0,0x68,0xc1,0x68	# MOVW	104(%r0),104(%r1)
	.byte	0x84,0xc0,0x6c,0xc1,0x6c	# MOVW	108(%r0),108(%r1)
	.byte	0x84,0xc0,0x70,0xc1,0x70	# MOVW	112(%r0),112(%r1)
	.byte	0x84,0xc0,0x74,0xc1,0x74	# MOVW	116(%r0),116(%r1)
	.byte	0x84,0xc0,0x78,0xc1,0x78	# MOVW	120(%r0),120(%r1)
	.byte	0x84,0xc0,0x7c,0xc1,0x7c	# MOVW	124(%r0),124(%r1)
	ADDW2	&128,%r0
	ADDW2	&128,%r1
	POPW	%psw
	DECW	%r2
	BLEB	fbcdon
	jmp	fbclop
fbcdon:
	RSB



#
#	This is the copyin and copyout routine.
#
#	copyin(from,to,count)
#	caddr_t from, to;
#	int count;
#	{
#
#		if
#		(
#			from < MINUVTXT
#			||
#			UVUBLK < from+count  &&  from < UVSTACK
#		)
#			return(-1);
#
#	label:
#		if (caddrflt) {
#			caddrflt = 0;
#			return(-1);
#		}
#
#		caddrflt = label;
#		bcopy( from, to, count );
#		caddrflt = 0;
#		return(0);
#	}
#
#	copyout(from,to,count)
#	caddr_t from, to;
#	int count;
#	{
#
#		if
#		(
#			to < MINUVTXT
#			||
#			UVUBLK < to+count  &&  to < UVSTACK
#		)
#			return(-1);
#
#	label:
#		if (caddrflt) {
#			caddrflt = 0;
#			return(-1);
#		}
#
#		caddrflt = label;
#		bcopy( from, to, count );
#		caddrflt = 0;
#		return(0);
#	}
#

	.globl	copyin
	.globl	copyout
	.globl	u

#
#   NOTE -- CHANGES TO USER.H MAY REQUIRE CORRESPONDING CHANGES TO THE
#		FOLLOWING .SET's.
#
	.set	UVTEXT,0x80800000	# main store beginning virtual address
	.set	UVUBLK,0xC0000000	# ublock virtual address
	.set	UVSTACK,0xC0020000	# stack start virtual address
	.set	MINUVTXT,0x80000000	# Minimum user virtual text.
	
copyin:
	SAVE	%r5
	MOVW	&0,%r5			# r0 contains the user address
	MOVW	u+u_procp,%r0		# check if the process is a server
	CMPH	p_sysid(%r0),&0		# true if u.u_procp->p_sysid != 0
	je 	ci_kern
	PUSHW	0(%ap)
	PUSHW	4(%ap)
	PUSHW	8(%ap)
	call	&3,rcopyin
	RESTORE	%r5
	RET
					# check end-points of copy
ci_kern:				# kernel mem
	CMPW	&MINUVTXT,0(%ap)
	BLUH	copybad
ci_ublk:				# ublock
	ADDW3	{uword}0(%ap),{uword}8(%ap),%r0
	CMPW	%r0,&UVUBLK
	BGUH	copycom
	CMPW	&UVSTACK,0(%ap)
	BLUB	copybad
	JMP	copycom

copyout:
	SAVE	%r5
	MOVW	&1,%r5			# r1 contains the user address
	MOVW	u+u_procp,%r0		# check if the process is a server
	CMPH	p_sysid(%r0),&0		# true if u.u_procp->p_sysid != 0
	je 	co_kern
	PUSHW	0(%ap)
	PUSHW	4(%ap)
	PUSHW	8(%ap)
	call	&3,rcopyout
	RESTORE	%r5
	RET
					# check end-points of copy
co_kern:				# kernel memory
	CMPW	&MINUVTXT,4(%ap)
	BLUB	copybad
co_ublk:				# ublock
	ADDW3	{uword}4(%ap),{uword}8(%ap),%r0	
	CMPW	%r0,&UVUBLK
	BGUB	copycom
	CMPW	&UVSTACK,4(%ap)
	BLUB	copybad
	JMP	copycom

#
#	If a paging fault happens in fbclop, control is returned here.
#
fbclopflt:
	POPW	%psw		# restore interrupts
	POPW	%r0		# get the saved pc off the stack
#
#	If a paging fault happens in copycom, control is returned here.
#
copyfault:
	CLRW	u+u_caddrflt
copybad:
	MOVW	&-1,%r0			# return failure flag
	RESTORE	%r5
	RET

#
#	If a paging fault happens in rcopyin or rcopyout, 
#	control is returned here.
#
	.globl 	rcopyfault
rcopyfault:
	CLRW	u+u_caddrflt
	MOVW	&-1,%r0			# return failure flag
	RESTORE	%r6			# restore bcopy register
	RET

copycom:
	MOVW	0(%ap),%r0		# from-address to %r0
	MOVW	4(%ap),%r1		# to-address to %r1
	MOVW	8(%ap),%r2		# byte count to %r6
 	BEH	copygood

	MOVAW	copyfault,u+u_caddrflt

	ANDW3	%r0,&0x03,%r7		# get alignment of addresses
	ANDW3	%r1,&0x03,%r8
	BITB	&0x3,%r2		# check word aligned count
	BNEB	copygen
	ORW3	%r7,%r8,%r6		# check both addresses word aligned
	BEH	copyblk

copygen:
	SUBW3	%r7,%r8,%r6
	BEB	copyalw			# word aligned addresses
	BITW	&0x01,%r6
	BEB	copyalh			# half-word aligned addresses

#
#	byte-aligned addresses; just copy a byte at a time
#
copyalb:
	MOVB	0(%r0),0(%r1)		# copy bytes
	INCW	%r0
	INCW	%r1
	DECW	%r2
	BNEB	copyalb
	jmp	copygood

#
#	half-word aligned addresses; copy a half-word at a time
#
copyalh:
	BITW	&0x01,%r0		# copy any initial odd-aligned byte
	BEB	copyalh1

	MOVB	0(%r0),0(%r1)
	INCW	%r0
	INCW	%r1
	DECW	%r2
	BEH	copygood

copyalh1:
	LRSW3	&1,%r2,%r6		# number of half-words
	BEB	copyalh2

copyalhmv:
	MOVH	0(%r0),0(%r1)		# copy half-words
	ADDW2	&2,%r0
	ADDW2	&2,%r1
	DECW	%r6
	BNEB	copyalhmv

	BITW	&0x01,%r2		# copy any trailing odd-aligned byte
	BEH	copygood

copyalh2:
	MOVB	0(%r0),0(%r1)
	jmp	copygood

#
#	word aligned addresses; use block move instruction
#
copyalw:
	BITW	&0x03,%r0		# copy any initial unaligned bytes
	BEB	copyalw1

	MOVB	0(%r0),0(%r1)
	INCW	%r0
	INCW	%r1
	DECW	%r2
	BNEB	copyalw
	jmp	copygood

copyalw1:
	MOVW	%r2,%r6
	LRSW3	&2,%r2,%r2		# number of words
	BEB	copyalw2

	MOVBLW				# copy words

	ANDW2	&0x03,%r6		# bytes remaining to be copied
	BEH	copygood

copyalw2:
	MOVB	0(%r0),0(%r1)		# copy any trailing unaligned bytes
	INCW	%r0
	INCW	%r1
	DECW	%r6
	BNEB	copyalw2
	jmp	copygood

#
#	word aligned addresses and count; copy 128 byte blocks
#
copyblk:
	CMPW	&0x80,%r2
	BLH	copysmall
	TSTW	%r5		# which r[0-1] contains the user address?
	BNEB	cpalgr1		#align on r1
#
#	align on %r0 (the user address being copied to)
#
	ANDW3	%r0,&0x7f,%r6		# how much of the current block copied
	SUBW2	%r6,%r1			# set offset for the loop
	ARSW3	&2,%r6,%r6		# bytes to words
	MULW2	&5,%r6			# words to loop offset
	PUSHW	&copylast		# where we return to
	PUSHW	%psw
	ORW2	&0x1e000,%psw		# block all interrupts in loop
	NOP
	MOVAW	fbclopflt,u+u_caddrflt	# psw has been pushed
	TSTW	0(%r0)			# bring the page in
	ADDW2	%r0,%r2			# addr + size
	ANDW2	&0xffffff80,%r0		# set bases for loop
	SUBW3	%r0,%r2,%r7
	ARSW3	&7,%r7,%r2		# no of 128 byte blocks
	ANDW2	&0x7f,%r7		# number of bytes left over.
	JMP	fbclopstrt(%r6)		# do the copies
#
#	align on %r1 (the user address being copied from)
#
cpalgr1:
	ANDW3	%r1,&0x7f,%r6		# how much of the current block copied.
	SUBW2	%r6,%r0			# set offset for the loop
	ARSW3	&2,%r6,%r6		# bytes to words
	MULW2	&5,%r6			# words to loop offset
	PUSHW	&copylast		# where we return to
	PUSHW	%psw
	ORW2	&0x1e000,%psw		# block all interrupts in loop
	NOP
	MOVAW	fbclopflt,u+u_caddrflt	# psw has been pushed
	TSTW	0(%r1)			# bring the page in
	ADDW2	%r1,%r2			# addr + size
	ANDW2	&0xffffff80,%r1		# set base for loop
	SUBW3	%r1,%r2,%r7
	ARSW3	&7,%r7,%r2		# no of 128 byte blocks
	ANDW2	&0x7f,%r7		# number of bytes left over.
	JMP	fbclopstrt(%r6)		# do the copies
#
#	After bulk copies are done, come here to finish the copy
#
copylast:
	MOVW	%r7,%r2
copysmall:
	MOVAW	copyfault,u+u_caddrflt
	ARSW3	&2,%r2,%r2
	MOVBLW

copygood:
	CLRW	u+u_caddrflt
	CLRW	%r0		# return success flag
	RESTORE	%r5
	RET


# Code for fubyte, fuibyte, fuword, etc.
#
# functions to fetch and store bytes and words from user space.
#
#
#extern int caddrflt;
#
#fubyte(src)
#register unsigned char * src;
#{
#	register unsigned int tbyte;
#
#	if
#	(
#		(int) src < (unsigned)MINUVTXT
#		||
#		(unsigned)UVBLK <= (unsigned)src  &&  (unsigned)src < (unsigned)UVSTACK
#	)
#		return(-1);
#
#label:
#	if (caddrflt) {
#		caddrflt = 0;
#		return(-1);
#	}
#
#	caddrflt = (int) label;
#	tbyte = *src;
#	caddrflt = 0;
#	return(tbyte);
#}
#
#fuibyte(src)
#unsigned char * src;
#{
#	return(fubyte(src));
#}
#
#fuword(src)
#register unsigned int * src;
#{
#	register unsigned int tword;
#
#	if
#	(
#		(int) src < (unsigned)MINUVTXT
#		||
#		(unsigned)UVBLK <= (unsigned)src  &&  (unsigned)src < (unsigned)UVSTACK
#	)
#		return(-1);
#
#label:
#	if (caddrflt) {
#		caddrflt = 0;
#		return(-1);
#	}
#
#	caddrflt = (int) label;
#	tword = *src;
#	caddrflt = 0;
#	return(tword);
#}
#
#fuiword(src)
#unsigned int * src;
#{
#	return(fuword(src));
#}
#
#subyte(dst,tbyte)
#register unsigned char * dst;
#register unsigned char tbyte;
#{
#	if
#	(
#		(int) dst < (unsigned)MINUVTXT
#		||
#		(unsigned)UVBLK <= (unsigned)dst  &&  (unsigned)dst < (unsigned)UVSTACK
#	)
#		return(-1);
#
#label:
#	if (caddrflt) {
#		caddrflt = 0;
#		return(-1);
#	}
#
#	caddrflt = (int) label;
#	*dst = tbyte;
#	caddrflt = 0;
#	return(0);
#}
#
#suibyte(dst,tbyte)
#unsigned char * dst;
#unsigned char tbyte;
#{
#	subyte(dst,tbyte);
#}
#
#suword(dst,tword)
#register unsigned int * dst;
#register unsigned int tword;
#{
#	if
#	(
#		(int) dst < (unsigned)MINUVTXT
#		||
#		(unsigned)UVBLK <= (unsigned)dst  &&  (unsigned)dst < (unsigned)UVSTACK
#	)
#		return(-1);
#
#label:
#	if (caddrflt) {
#		caddrflt = 0;
#		return(-1);
#	}
#
#	caddrflt = (int) label;
#	*dst = tword;
#	caddrflt = 0;
#	return(0);
#}
#
#suiword(dst,tword)
#unsigned int * dst;
#unsigned int tword;
#{
#	return(suword(dst,tword));
#}
#

	.globl	fubyte
	.globl	fuibyte
fubyte:
fuibyte:
	MOVW	u+u_procp,%r0		# check if the process is a server
	CMPH	p_sysid(%r0),&0		# true if u.u_procp->p_sysid != 0
	je	lfubyte
	PUSHW	0(%ap)
	call	&1,rfubyte
	RET
lfubyte:				# local fubyte
	MOVW	0(%ap),%r1
	CMPW	&MINUVTXT,%r1
	jlu	sufubad
	CMPW	&UVUBLK,%r1
	jlu	cont2
	CMPW	&UVSTACK,%r1
	jlu	sufubad

cont2:	MOVAW	sf_fault,u+u_caddrflt
	MOVB	0(%r1),%r0
	CLRW	u+u_caddrflt
	RET

	.globl	fuword
	.globl	fuiword
fuword:
fuiword:
	MOVW	u+u_procp,%r0		# check if the process is a server
	CMPH	p_sysid(%r0),&0		# true if u.u_procp->p_sysid != 0
	je	lfuword
	PUSHW	0(%ap)
	call	&1,rfuword
	RET
lfuword:				# local fuword
	MOVW	0(%ap),%r1
	CMPW	&MINUVTXT,%r1
	jlu	sufubad
	CMPW	&UVUBLK,%r1
	jlu	cont4
	CMPW	&UVSTACK,%r1
	jlu	sufubad

cont4:	MOVAW	sf_fault,u+u_caddrflt
	MOVW	0(%r1),%r0
	CLRW	u+u_caddrflt
	RET

	.globl	subyte
	.globl	suibyte
subyte:
suibyte:
	MOVW	u+u_procp,%r0		# check if the process is a server
	CMPH	p_sysid(%r0),&0		# true if u.u_procp->p_sysid != 0
	je	lsubyte
	PUSHW	0(%ap)
	PUSHW	4(%ap)
	call	&2,rsubyte		# remote subyte
	RET
lsubyte:				# local lsubyte
	MOVW	0(%ap),%r1
	CMPW	&MINUVTXT,%r1
	jlu	sufubad
	CMPW	&UVUBLK,%r1
	jlu	cont6
	CMPW	&UVSTACK,%r1
	jlu	sufubad

cont6:	MOVAW	sf_fault,u+u_caddrflt
	MOVB	7(%ap),0(%r1)
	CLRW	u+u_caddrflt
	CLRW	%r0
	RET

	.globl	suword
	.globl	suiword
suword:
suiword:
	MOVW	u+u_procp,%r0		# check if the process is a server
	CMPH	p_sysid(%r0),&0		# true if u.u_procp->p_sysid != 0
	je	lsuword
	PUSHW	0(%ap)
	PUSHW	4(%ap)
	call	&2,rsuword		# remote suword
	RET
lsuword:				# local suword
	MOVW	0(%ap),%r1
	CMPW	&MINUVTXT,%r1
	jlu	sufubad
	CMPW	&UVUBLK,%r1
	jlu	cont8
	CMPW	&UVSTACK,%r1
	jlu	sufubad

cont8:	MOVAW	sf_fault,u+u_caddrflt
	MOVW	4(%ap),0(%r1)
	CLRW	u+u_caddrflt
	CLRW	%r0
	RET

sf_fault:
	CLRW	u+u_caddrflt
sufubad:
	MOVW	&-1,%r0			# return failure flag
	RET


	# Read in pathname from kernel space 
	# spath (from, to, maxbufsize);
	# Returns -2 if pathname is too long, otherwise returns
	#  the pathname length.
	.globl	spath
spath:
	MOVW	0(%ap), %r1		# calculate the pathname 
	MOVW	%r1, %r0		# length and leave 
	STREND				# the result in 
	SUBW2	%r1, %r0		# r0 and r2
	MOVW	%r0, %r2
	SUBW3	%r0, 8(%ap), %r0	# Compare the pathname length
					# with the maximum size of 
					# the buffer, difference is in 
					# r0
	jle	plenerr			# Error, if the length is 
					# > (maxbufsize-1), -1 for NULL
	MOVW	%r1, %r0		# r0 = from
	MOVW	4(%ap), %r1		# r1 = to
	STRCPY
	MOVW	%r2, %r0		# return the pathname length
	RET

	# Read in pathname from user space 
	# upath (from, to, maxbufsize);
	# Returns -2 if the pathname is too long, -1 if a bad user
	#  address is supplied, otherwise returns the pathname length.
	.globl	upath
upath:
	MOVW	u+u_procp,%r0		# check if the process is a server
	CMPH	p_sysid(%r0),&0		# true if u.u_procp->p_sysid != 0
	jne	spath			# if (server()) spath();
	MOVW	0(%ap),%r1
	CMPW	&MINUVTXT,%r1	# if from < (unsigned)MINUVTXT ||
	jlu	upbaddr
	CMPW	&UVUBLK,%r1	# if (unsigned) UVBLK <= from &&
	jlu	upcont
	CMPW	&UVSTACK,%r1	# if from < (unsigned)UVSTACK -> error
	jlu	upbaddr
upcont:
	MOVAW	sf_fault,u+u_caddrflt	# initialize fault code
	MOVW	%r1, %r0		# calculate the pathname 
	STREND				# length and leave 
	SUBW2	%r1, %r0		# the result in
	MOVW	%r0, %r2		# r0 and r2
	SUBW3	%r0, 8(%ap), %r0	# Compare the pathname length
					# with the maximum size of 
					# the buffer, difference is in 
					# r0
	jle	plenerr			# Error, if the length is 
					# > (maxbufsize-1), -1 for NULL
	MOVW	%r1, %r0		# r0 = from
	MOVW	4(%ap), %r1		# r1 = to
	STRCPY
	MOVW	%r2, %r0		# return the pathname length
	CLRW	u+u_caddrflt
	RET

upbaddr:
	MOVW	&-1, %r0		# return error (-1) on 
					# out of range address
	RET

plenerr:
	CLRW	u+u_caddrflt
	MOVW	&-2, %r0		# return error (-2) on pathname
					# length error 
	RET


#
# This code is the init process; it is copied to user text space
# and it then does exec( "/etc/init", "/etc/init", 0 );
#
	.data
	.align 4

	.globl	icode
	.globl	szicode

	.set	_exec,11*8

icode:
	MOVW	&UVTEXT,%r0	# this is the virtual address of `icode'

#	We are still running on the kernel pcb and kernel stack.
#	We must continue to run in kernel mode since we are
#	going to write %pcbp in order to switch to the user's
#	pcb and stack.

	MOVAW	u+u_pcb,%pcbp	# Point to new pcb.
	MOVAW	userstack,%sp	# Switch to user stack.
	MOVW	%sp,%ap		# Init arg pointer to empty stack.
	MOVW	%sp,%fp		# Init frame pointer to empty stack.
	MOVAW	u+u_pcb,u+u_pcbp # Set ptr to current pcb.

	PUSHAW	etc_off(%r0)	# address of etc_init
	PUSHAW	argv_off(%r0)	# argv[] array

	CALL	-8(%sp),icode1_off(%r0)	# call icode1
icode1:
	.set	icode1_off,.-icode
	MOVW	&4,%r0
	MOVW	&_exec,%r1
	GATE			# exec( "/etc/init", argv )
icode2:
	BRB	icode2

	.align 4
argv:		# argv area
	.set	argv_off,.-icode
	.word	UVTEXT+etc_off
	.word	0

etc_init:	# /etc/init
	.set	etc_off,.-icode
	.byte	0x2f,0x65,0x74,0x63,0x2f,0x69,0x6e,0x69,0x74,0
icode_end:

	.data
	.align	4
szicode:	# length of icode
	.word	icode_end-icode

	.globl	cputype
cputype:
	.half	0x3b2


#	searchdir(buf, n, target) - search a directory for target
#	returns offset of match, or empty slot, or -1

	.globl	searchdir

	.text
searchdir:
	save	&4
	MOVW	0(%ap), %r8			# pointer to directory
	MOVW	4(%ap), %r7			# directory length in bytes
	MOVW	&16, %r6			# sizeof(struct direct)
	MOVW	&0, %r5				# pointer to empty slot

	.align	4
s_top:
	CMPW	%r6, %r7			# length less than 16?
	jl	sdone				# jump if r7 < r6
	CMPH	0(%r8), &0			# directory entry empty?
	je	sempty				# jump if true
	MOVAW	2(%r8), %r0			# address of file name
	MOVW	8(%ap), %r2			# address of target name
	CMPB	0(%r0), 0(%r2)			# compare characters
	jne	scont				# jump if different
	CMPB	1(%r0), 1(%r2)
	jne	scont
	TSTB	1(%r0)				# after second character,
	je	smatch				# if equal and zero, match

	CMPB	2(%r0), 2(%r2)
	jne	scont
	TSTB	2(%r0)
	je	smatch

	CMPB	3(%r0), 3(%r2)
	jne	scont
	TSTB	3(%r0)
	je	smatch

	CMPB	4(%r0), 4(%r2)
	jne	scont
	TSTB	4(%r0)
	je	smatch

	CMPB	5(%r0), 5(%r2)
	jne	scont
	TSTB	5(%r0)
	je	smatch

	CMPB	6(%r0), 6(%r2)
	jne	scont
	TSTB	6(%r0)
	je	smatch

	CMPB	7(%r0), 7(%r2)
	jne	scont
	TSTB	7(%r0)
	je	smatch

	CMPB	8(%r0), 8(%r2)
	jne	scont
	TSTB	8(%r0)
	je	smatch

	CMPB	9(%r0), 9(%r2)
	jne	scont
	TSTB	9(%r0)
	je	smatch

	CMPB	10(%r0), 10(%r2)
	jne	scont
	TSTB	10(%r0)
	je	smatch

	CMPB	11(%r0), 11(%r2)
	jne	scont
	TSTB	11(%r0)
	je	smatch

	CMPB	12(%r0), 12(%r2)
	jne	scont
	TSTB	12(%r0)
	je	smatch

	CMPB	13(%r0), 13(%r2)
	je	smatch				# jump if match even if not zero
scont:
	ADDW2	%r6, %r8			# increment directory pointer
	SUBW2	%r6, %r7			# decrement size
	jmp	s_top				# keep looking

sempty:
	CMPW	%r5, &0				# do we need an empty slot?
	jne	scont				# jump if no
	MOVW	%r8, %r5			# save current offset
	jmp	scont				# and goto to next entry

smatch:
	SUBW2	0(%ap), %r8			# convert to offset
	MOVW	%r8, %r0			# return offset
	ret	&4

sdone:
	MCOMW	&0, %r0				# save failure return
	CMPW	%r5, &0				# empty slot found?
	je	sfail				# jump if false
	SUBW2	0(%ap), %r5			# convert to offset
	MOVW	%r5, %r0			# return empty slot
sfail:
	ret	&4

	.globl is32b

is32b:
	MVERNO			# puts version number in r0
	SUBW2	&0x19,%r0
	jle	WE32001		# ver. number <= 25 is a WE32001 - ret. 0
	MOVW	&0x1,%r0	# ver. number >  25 is a WE32100 - ret. 1
	RET
WE32001:
	MOVW	&0x0,%r0
	RET
