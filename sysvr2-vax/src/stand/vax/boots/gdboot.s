#	@(#)gdboot.s	1.3
#	@(#)gdboot.s	1.3
	.globl	_gdboot
_gdboot:
# General Disk Boot
	.set	BLKSIZ,512	# disc block size
	.set	RELOC,0x50000
	.set	HDRSIZ,040	# size of file header for VAX
	.set	MAGIC,0410	# file type id in header
	.set	MAGIC3,0413	# file type id in header
	.set	TEXT,04
	.set	ENTRY,024
	.set	INOSIZ,64	# no. bytes/inode entry
	.set	INOBLK,BLKSIZ/INOSIZ	# no. inodes/disc block
	.set	INOMSK,0xfffffff8	# changes with inode size
	.set	NAMSIZ,14	# no. bytes in name field of dir entry
	.set	ENTADR,024	# offset to entry addr in task header
	.set	DIRSIZ,16	# size of directory entry, bytes
	.set	ROOTINO,2	# root dir inode no.
# MBA registers
	.set	M_cr,0x4	# MBA control reg
	.set	M_var,0xc	# MBA virt addr reg
	.set	M_bc,0x10	# MBA byte count reg
	.set	M_map,0x800	# start of MBA map reg's
	.set	M_dev,0x400	# start of drive 0 reg's
# Drive registers
	.set	D_cr,0x0	# control reg, drive 0
	.set	D_sr,0x4	# status reg
	.set	D_da,0x14	# desired track/sector reg
	.set	D_dt,0x18	# drive type
	.set	D_off,0x24	# Rx offset reg
	.set	D_cyl,0x28	# desired cyl reg
# function codes, status bits 
	.set	READ,071	# read data
	.set	RIP,021	# Read-In Preset code
	.set	FMT,0x1000	# format bit for offset reg
	.set	DRY,0200	# drive ready, staus reg
	.set	pDRY,7	# bit position of DRY

start:
	.word	0
	brb	star	# 780 only code
	.space	8
	movl	r1,r9	# physaddr of mba 
	ashl	$7,r3,r10
	brb	common
star:	movl	0x5c(r11),r9
	ashl	$7,0x64(r11),r10
common:	addl2	r9,r10
	addl2	$M_dev,r10
	cvtwl	$RIP,D_cr(r10)
	cvtwl	$FMT,D_off(r10)
#	move boot image up to higher core
move:
	movl	$RELOC,sp
	moval	start,r6
	movc3	$end-start,(r6),(sp)
	jmp	*$RELOC+main-start
main:
# search inodes for pathname specified in 'names'
	moval	names,r6
	movzbl	$ROOTINO,r0
nxti:
	bsbw	iget
	clrl	bno
	clrl	bufptr
	tstb	(r6)
	beql	getfil
get1b:
	bsbw	rmblk
	beql	main	# file not found
	clrl	r7
nxtent:
	tstw	(r7)	# null dir entry
	beql	dirchk
	cmpc3	$NAMSIZ,(r6),2(r7)
	bneq	dirchk
# found component in a dir entry
	movzwl	(r7),r0	
	addl2	$NAMSIZ,r6
	brb	nxti
dirchk:	
	acbl	$BLKSIZ-1,$DIRSIZ,r7,nxtent
	brb	get1b	# get another dir block into buffer
# here if inode for desired file has been read in
getfil:
	clrl	bufptr	# new buffer ptr is low core
getlop:
	bsbw	rmblk	# get a block in the file
	beql	clear	# branch if no more file blocks to read
	addl2	$BLKSIZ,bufptr	# next page in low core
	brb	getlop
# clear core
clear:
	movl	size,r6
	subl3	r6,sp,r5
	movc5	$0,0,$0,r5,(r6)	# clear memory down to stack
# go execute file
#	note that r0 has been cleared by the movc5 instr
	clrl	r7
	cmpw	(r0),$MAGIC
	beql	header
	cmpw	(r0), $MAGIC3
	bneq	exec	# no header, direct executable image
header:
	movq	TEXT(r0),r8
	movl	ENTRY(r0),r7
	movc3	r6,HDRSIZ(r0),(r0)	# move text to start at 0, over header
	extzv	$0,$9,r8,r0
	beql	exec
	subl3	r0,$512,r0
	addl2	r8,r0
	movc3	r9,(r8),(r0)	# move data segment up to page boundary
exec:
	calls	$0,(r5)
	brw	main
#
# subroutine to get the inode whose no. is in r0
iget:
	addl3	$(INOBLK*2)-1,r0,r8
	divl3	$INOBLK,r8,r4	# calculate inode block , put in r4
	moval	inode,bufptr
	bicl2	$INOMSK,r8
	mull2	$INOSIZ,r8
	subl2	r8,bufptr
	bsbb	rblk
	moval	faddr,r0
	moval	addr,r1
	movl	$13,r2
iget1:
	extzv	$0,$24,(r0),(r1)+
	addl2	$3,r0
	sobgtr	r2,iget1
	movl	$10,bno
	moval	addr+40,bufptr
# subr to read in 1 of blocks (bno) in 'addr[]'
rmblk:
	movl	bno,r0
	movl	addr[r0],r4
	bneq	rmblk1
	rsb
rmblk1:
	incl	bno
# subr to read disc block no. specified in r4 from drive 0
rblk:
	movl	D_dt(r10),r5
	movab	types-3,r1
tc1:
	addl2	$3,r1
	movb	(r1)+,r0
	beqlu	rperr
	cmpb	r5,r0
	bnequ	tc1
	clrl	r5
	movzwl	(r1)+,r0
	ediv	r0,r4,D_cyl(r10),r4
	movzbl	(r1)+,r0
	ediv	r0,r4,r4,r5
	insv	r4,$8,$5,r5	# move track no. into stk
	movl	r5,D_da(r10)	# track-sector
	mnegl	$BLKSIZ,M_bc(r9)
	ashl	$-9,bufptr,r0	# start page no. of buffer
	bisl2	$0x80000000,r0
	movl	r0,M_map(r9)
	addl3	$1,r0,M_map+4(r9)
	bicl3	$~0x1ff,bufptr,M_var(r9)
	cvtbl	$READ,D_cr(r10)
rprdy:
	movl	D_sr(r10),r0
	bbc	$pDRY,r0,rprdy	# loop unitl ready
# need some error check here
	rsb
rperr:	# disc i/o error
	halt		# halt on error
#
bufptr: .long 0
names:
	.byte	's,'t,'a,'n,'d,0,0,0,0,0,0,0,0,0
	.byte	's,'a,'s,'h,0,0,0,0,0,0,0,0,0,0
	.byte 0
types:
	.byte	0x17	# RM05
	.word	608
	.byte	32
	.byte	0x12	# RP06
	.word	418
	.byte	22
	.byte	0x22	# RP07
	.word	1600
	.byte	50
	.byte	0x16	# RM80
	.word	434
	.byte	31
	.byte	0
#
end:
	.set	inode,start-1024
	.set	mode,inode
	.set	nlink,mode+2
	.set	uid,nlink+2
	.set	gid,uid+2
	.set	size,gid+2
	.set	faddr,size+4
	.set	time,faddr+40
	.set	addr,time+12
	.set	bno,addr+40+512
