# sash0.s : assembly code for the stand alone shell (NS32000)

	.file	"sash0.s"
	.globl	_main
	.globl	_xcom
	.globl	_entry
	.globl	_end
	.globl	_edata
	.set	sashbase,0x70000

	.text

	.double	0,0,0,0,0,0,0,0		# space for module table (32 bytes)

# 'ld.file' sets entry point to abs. addr 32, which is L0 (before reloc.)

L0:

	movd	$sashbase,r2	# relocate code to 'sashbase'
	lprd	sp,r2
	movd	$0,r1
	movd	$_end-sashbase,r0
	movsb
	jump	main0

# running in the relocated code now

main0:
	jsr	_main

# _exit ()
	.globl	__exit
__exit:
	lprd	sp,$sashbase
	jsr	_xcom
	movd	$_argv,tos
	movd	_argc,tos
	movd	$0x12345678,tos
	movmb	0xe00010,16,12	# restore mod tbl for monitor interrupt stuff
	movd	_entry,r0
	jsr	r0
	br	__exit

# movc3 (length, from, to) -- Do not use to shift a segment higher in memory
#		to an address overlapping the original segment... This will
#		destroy the data.  Use "movc3up" for this (non-destructive).

	.globl	_movc3
_movc3:
	movd	4(sp),r0	# length
	movd	8(sp),r1	# source addr
	movd	12(sp),r2	# dest addr
	movsb
	ret	0

# movc3up (length, from, to) -- Uses a backward move to preserve data
#		when it is moved upward in memory to an area overlapping
#		the original data.

	.globl	_movc3up
_movc3up:
	movd	4(sp),r0	# length of segment
	movd	8(sp),r1	# starting addr of source segment
	addd	r0,r1		#  convert to ending addr of source
	addqd	-1,r1		#  (end addr = beginning addr + size - 1)
	movd	12(sp),r2	# starting addr of dest. segment
	addd	r0,r2		#  convert to ending addr of dest.
	addqd	-1,r2
	movsb	b		# backwards string move to preserve data
	ret	0

# clrseg (buf addr, length)
	.globl	_clrseg
_clrseg:
	movd	8(sp),r0	# length of section to clear
	cmpqd	0,r0		# are we trying to clear 0 bytes?
	beq	_clrsegret	#   yes, so quit
	movd	4(sp),r1	# get buffer addr.
	addr	1(r1),r2	# shift up 1 to zero buffer
	addqd	-1,r0		#  sub. 1 from length because of zero move
	movqb	0,0(r1)		# zero first byte
	movsb
_clrsegret:
	ret	0

	.data
	.comm	_argc,4
