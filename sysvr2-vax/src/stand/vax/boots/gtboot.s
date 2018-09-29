#	@(#)gtboot.s	1.5
# Tape boot program to load and transfer
# to a program on a 9-tr mag tape 
# This program is the boot block on the mag tape.
	.set	RELOC,0x50000
	.set	HDRSIZ,168	# size of file header for VAX
	.set	MAGIC,20	# location of file type id in header
	.set	A_MAGIC2,0410	# file type id in header
	.set	A_MAGIC3,0413	# file type id in header
	.set	BLKSIZ,512	# tape block size, bytes
	.set	TSIZ,24		# text size
	.set	DSIZ,28		# data size
	.set	BSIZ,32		# bss size
	.set	TENT,36		# task header entry loc
	.set	SID,0x3e	# system id register
	.set	STAR,1		# id of vax 11/780
#
# MBA registers
	.set	MBASTAR,0x20012000	# MBA nexus 9 registers
	.set	MBACOMET,0xf2a000	# MBA 1 on 11/750
	.set	M_cfg,0		# MBA config reg
	.set	M_cr,4		# MBA control reg
	.set	M_stat,8	# MBA status reg
	.set	M_var,12	# MBA virt addr reg
	.set	M_bc,16		# MBA byte count reg
	.set	M_map,0x800	# start of MBA map reg's
	.set	MBAinit,1	# MBA init bit in MBA control reg
#
	.set	TMSTAR,MBASTAR+0x400	# start of TM reg's
	.set	TMCOMET,MBACOMET+0x400	# start of TM reg's
	.set	TM_cs1,0	# TM control 1 reg
	.set	TM_ds,4		# status reg
	.set	TM_dtc,4	# data transfer code
	.set	TM_fm,8		# TU78 control
	.set	TM_er,8		# error reg
	.set	TM_as,16	# attention summary
	.set	TM_fc,20	# frame count
	.set	TM_dt,24	# drive type
	.set	TM_ds78,28	# drive status
	.set	TM_tc,36	# TM tape control
	.set	TM_ndtc,44	# TU78 ndt
	.set	TM_ndt,48	# TU78 ndt
	.set	TM_id,0x44	# TU78 internal data
	.set	GO,1	# GO bit
	.set	RWND,06	# rewind, on-line
	.set	SENSE,010	# get TM78 status
	.set	DCLR,010	# drive clear
	.set	SFWD,030	# space forward
	.set	SREV,032	# space reverse
	.set	READ,070	# read forward

	.set	ERR,040000	# composite error bit in status reg
	.set	TCHAR,01700
	.set	DRDY,0200	# TM/drive ready in status reg
	.set	RDY,0x8000	# TM78/drive ready in status reg
	.set	REW,0x1000	# TM78/drive ready in status reg
	.set	DTC,13
	.set	TM_pe,14
	define(rMBA,r10)
	define(rTM,r11)
# system initialization
start:
	movl	$RELOC,sp
	moval	start,r6
	movc3	$end-start,(r6),(sp)
	jmp	*$RELOC+main
main:
	moval	msginit,r0
	bsbw	puts
	mfpr	$SID,rMBA		# get system id
	ashl	$-24,rMBA,rMBA
	cmpl	rMBA,$STAR
	beql	star
# set comet MBA address
	movab	*$MBACOMET,rMBA
	movab	*$TMCOMET,rTM
	brb	all
star:
	movab	*$MBASTAR,rMBA
	movab	*$TMSTAR,rTM
all:
	movl	$MBAinit,M_cr(rMBA)
	movl	TM_dt(rTM),r8
	bbs	$6,r8,waitMB
	movl	$TCHAR,TM_tc(rTM)	# drive no., etc.
	movl	$DCLR+GO,TM_cs1(rTM)	# drive clear
	brb	c0
waitMB:
	movl	TM_id(rTM),r2
	bbc	$15,r2,waitMB		# wait for TM78 ready
c0:
	bsbw	rew
	clrl	r7
	bsbb	taper
	bsbb	taper
c1:
	bsbb	taper
	bneq	c2
	addl2	$512,r7
	brb	c1
c2:
	clrl	r0
	cmpl	$A_MAGIC2,MAGIC(r0)
	beql	header
	cmpl	$A_MAGIC3,MAGIC(r0)
	bneq	clrcor
header:
# move start of exec image down to 0
	movq	TSIZ(r0),r1	# text size and data size
	movq	BSIZ(r0),r3	# bss size, symbol table size
	movl	TENT(r0),r5	# entry loc
	movl	r7,r6	# restore file size
movtxt:
	movb	HDRSIZ(r0),(r0)+
	sobgtr	r6,movtxt
# move data up to page boundary
	extzv	$0,$9,r1,r0		# byte-in-page of last text byte
	beql	clrcor
	subl3	r0,$512,r0		# room needed
	addl2	r2,r1			# current end+1 of data
	addl2	r1,r0			# new end+1 of data
	movl	r0,r7			# start here on bssz clearing
movdat:
	movb	-(r1),-(r0)
	sobgtr	r2,movdat
#
# clear core up to stack
clrcor:
	subl3	r7,$RELOC-4,r0	# no. bytes to clear
clrit:
	clrb	(r7)+
	sobgtr	r0,clrit
#
	calls	$0,(r5)
	brw	start	# returned from execution - start all over
taper:
	movl	$-BLKSIZ,M_bc(rMBA)	# byte count
	ashl	$-9,r7,r0
	bisl3	$0x80000000,r0,M_map(rMBA)	# MBA map entry
	clrl	M_var(rMBA)	# MBA virt addr reg
	bbc	$6,r8,t0
	movl	$4,TM_fm(rTM)	# TU78
t0:
	movl	$READ+GO,TM_cs1(rTM)	# read forward
tmrdy:
	movl	M_stat(rMBA),r2
	bbc	$DTC,r2,tmrdy	# device ready ?
	bbs	$6,r8,t1
	movl	TM_ds(rTM),r2
	bitl	$4,r2		# check for tape mark
	bneq	eot
	movl	TM_er(rTM),r2
	bbc	$TM_pe,r2,donred	# any read errors ?
	clrl	TM_ds(rTM)	# clear status - try to recover
	mnegl	$1,TM_fc(rTM)	# frame count for backspace
	movl	$SREV+GO,TM_cs1(rTM)	# space reverse
	brb	taper
donred:
	bispsw	$4		# set z-bit in condition code
eot:
	rsb
t1:	# TU78
	movl	TM_dtc(rTM),r2
	bicl2	$0xffffffe0,r2	# get DT interrupt code
	movl	TM_as(rTM),TM_as(rTM)
	movl	M_stat(rMBA),M_stat(rMBA)
	cmpl	$1,r2		# test for OK (done interrupt code)
	beql	donred
	cmpl	$0x12,r2	# retry?
	beql	taper
	brb	eot
# subr to rewind mag tape
rew:
	bbs	$6,r8,r78
	movl	$RWND+GO,TM_cs1(rTM)	# rewind
	rsb
r78:
	movl	$RWND+GO,TM_ndt(rTM)
rwait:
	movl	TM_as(rTM),r2
	bbc	$0,r2,rwait
	bicl3	$0xffffffc0,TM_ndtc(rTM),r2
	cmpl	$7,r2
	beql	cle
	movl	TM_as(rTM),TM_as(rTM)
	rsb
cle:
	movl	TM_as(rTM),TM_as(rTM)
	brb	rwait

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
end:
