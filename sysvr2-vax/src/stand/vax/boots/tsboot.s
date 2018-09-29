#	@(#)tsboot.s	1.4
# Boot program for the TS11
# Tape boot program to load and transfer
# to a program on a 9-track mag tape.
# This program is the boot block on the mag tape.
#
	.set	CUBAIO,0xff0000		# VAX 750
	.set	CUBAMAP,0xf30800	# VAX 750
	.set	SUBAIO,0x20130000	# VAX 780
	.set	SUBAMAP,0x20006800	# VAX 780
	.set	RELOC,0x50000		# must be page-aligned
	.set	TSADDR,0xf550		# TS11 starting address
	.set	REWIND,0xc408		# Rewind command
	.set	SFR,0xc008		# Skip foward command
	.set	READ,0xc001		# Read command
	.set	BLKSIZ,512		# Tape block size, bytes
	.set	SREV,0xc108		# Reverse command
	.set	SETCHR,0xc004		# Write characteristics cmd
	.set	SSR,0x80		# Subsystem ready
	.set	HDRSIZ,168		# Size of file header
	.set	MAGIC,20		# Location of file type id in header
	.set	A_MAGIC2,0410		# File type id in header
	.set	A_MAGIC3,0413
	.set	TSIZ,24			# Text size
	.set	DSIZ,28			# Data size
	.set	BSIZ,32			# BSS size
	.set	TENT,36			# Task header entry loc
# system initialization
#
start:
	brb	init
	brb	star
	.space	8
comet:
	movl	$CUBAIO,r10
	movl	$CUBAMAP,r11
	brb	common
init:
	mfpr	$0x3e,r9
	extzv	$24,$8,r9,r12
	cmpb	$2,r12
	beql	comet
star:
	movl	$SUBAIO,r10
	movl	$SUBAMAP,r11
common:
	movl	$RELOC,sp	# Relocate to high memory
	moval	start,r6
	movc3	$end-start,(r6),(sp)
	jmp	*$RELOC+main
main:
	moval	msginit,r0
	bsbw	puts
	addl2	$TSADDR,r10
	clrw	2(r10)		# init TS
	bsbw	wait
	movl	$0x80000000,0(r11)	# Set the Unibus map
	movl	$0x80000000+RELOC/0x200,4(r11)
	moval	cmd,r13
	bicl2	$0xfffffe00,r13
	bisl2	$0x200,r13
	movw	r13,0(r10)	# Do write-characteristics
	bsbw	wait
 	movw	$REWIND,cmd	# Rewind
 	movw	r13,0(r10)
 	bsbw	wait
	movw	$SFR,cmd	# Skip 2 records
	movw	$2,loba
	movw	r13,0(r10)
	bsbw	wait
	clrl	r7
	movw	$BLKSIZ,count
	clrw	loba		# buffer starts at zero
taper:
	movw	$READ,cmd
	movw	r13,0(r10)
	bsbw	wait
	tstw	xs0		# encountered tape mark?
	bgeq	cont		# no
	brw	c1
cont:
	movw	2(r10),r9	# error?
	bgeq	cont2		# no
	extzv	$2,$2,r9,r12
	cmpb	$2,r12
error:
	bneq	error		# Can't correct error
	bitw	$2,r9		# tape not read, reread
	bneq	taper
	movw	$SREV,cmd	# backspace
	movw	$1,loba
	movw	r13,0(r10)
	bsbw	wait
	clrw	loba		# restore bus address
	brb	taper
cont2:
	addl2	$BLKSIZ/512,0(r11)
	addl2	$512,r7
	brb	taper
c1:
	clrl	r0
	cmpl	$A_MAGIC2,MAGIC(r0)	# Check a.out header number
	beql	header
	cmpl	$A_MAGIC3,MAGIC(r0)
	bneq	clrcor
header:
# move start of exec image down to 0
	movq	TSIZ(r0),r1	# Text size and data size
	movq	BSIZ(r0),r3	# BSS size and symbol table size
	movl	TENT(r0),r5	# Entry location
	movl	r7,r6	# restore file size
movtxt:
	movb	HDRSIZ(r0),(r0)+
	sobgtr	r6,movtxt
# Move data up to page boundary
	extzv	$0,$9,r1,r0	# Byte-in-page of last text byte
	beql	clrcor
	subl3	r0,$512,r0	# Room needed
	addl2	r2,r1		# Current end+1 of data
	addl2	r1,r0		# New end+1 of data
	movl	r0,r7		# Start here on BSS clearing
movdat:
	movb	-(r1),-(r0)
	sobgtr	r2,movdat
# Clear core up to stack
clrcor:
	subl3	r7,$RELOC-4,r0	# No. of bytes to clear
clrit:
	clrb	(r7)+
	sobgtr	r0,clrit
#
	calls	$0,(r5)
	brw	start		# Returned form execution - start all over
wait:
	bitw	$SSR,2(r10)
	beql	wait
	rsb
#
	.set	TXCS, 34
	.set	TXDB, 35
puts:
	mfpr	$TXCS,r1
	bbc	$7,r1,puts
	movzbl	(r0)+,r1
	beql	puts1
	mtpr	r1,$TXDB
	brb	puts
puts1:
	rsb
msginit:
	.byte	'U, 'N, 'I, 'X, ' , 't, 'a, 'p, 'e, ' , 'b, 'o, 'o, 't
 	.byte	' , 'l, 'o, 'a, 'd, 'e, 'r, 0xa, 0xd, 0
	.align	4
cmd:	.word	SETCHR
loba:	.word	0x200+caddr
	.word	0
count:	.word	end-caddr
sts:	.word	0
	.word	0
	.word	0
xs0:	.word	0
	.word	0
	.word	0
	.word	0
caddr:	.word	0x200+sts
	.word	0
	.word	caddr-sts
	.word	0
end:
