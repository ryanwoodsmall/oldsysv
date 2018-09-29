	.file	"mcrt0.s"
#	@(#)mcrt0.s	1.10
#	3.0 SID #	1.2
# C runtime startoff including monitoring

	.set	exit,1
	.set	cbufs,600
	.set	hdr,12
	.set	cnt,8

.globl	start
.globl	_monitor
.globl	_sbrk
.globl	_main
.globl	_exit
.globl	_IEH3exit
.globl	_etext
.globl	_environ
.globl	__cleanup
.globl	____Argv
.comm	countbase,4


start:
	.word	0x0000
	subl2	$8,sp
	movl	8(sp),(sp)  #  argc
	movab	12(sp),r0
	movl	r0,4(sp)  #  argv
	movl	r0,____Argv  #  prog name for profiling
.L1:
	tstl	(r0)+  #  null args term ?
	bneq	.L1
	cmpl	r0,*4(sp)  #  end of 'env' or 'argv' ?
	blss	.L2
	tstl	-(r0)  # envp's are in list
.L2:
	movl	r0,8(sp)  #  env
	movl	r0,_environ  #  indir is 0 if no env ; not 0 if env

	pushl	$cbufs		# entrance counters (arg5 for monitor)
	subl3	$.eprol,$_etext,r1	# text size
	addl2	$1,r1
	bicl2	$1,r1			# 
	addl2	$hdr+cnt*cbufs,r1	# add entrance count plus header
	ashl	$-1,r1,r1	# cvt byte length to `short' count
	pushl	r1		# bufsiz (arg4 for monitor)
	ashl	$1,r1,r1	# cvt `short' count back to byte count
	pushl	r1		# for sbrk
	calls	$1,_sbrk
	cmpl	r0,$-1
	beql	.nospace
	pushl	r0		# buf ptr (arg3 for monitor)
	addl3	$12,r0,countbase	# initialize countbase
	pushab	_etext
	pushab	.eprol
	calls	$5,_monitor
	calls	$3,_main
	pushl	r0
	calls	$1,_exit

	.data
_environ:	.space	4
.emsg:
	.byte	'N,'o,' ,'s,'p,'a,'c,'e,' ,'f,'o,'r,' 
	.byte	'm,'o,'n,'i,'t,'o,'r,' ,'b,'u,'f,'f,'e,'r,0xa,0x0
.em1:
	.text

.nospace:
	pushl	$.em1-.emsg
	pushab	.emsg
	pushl	$2
	calls	$3,_write
	chmk	$exit

_exit:
_IEH3exit:
	.word	0x0000
	pushl	$0
	calls	$1,_monitor
	calls	$0,__cleanup
	chmk	$exit
.eprol:
