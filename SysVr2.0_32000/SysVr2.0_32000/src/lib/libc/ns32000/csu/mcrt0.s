	.file	"mcrt0.s"
#	@(#)mcrt0.s	1.5
#	3.0 SID #	1.2
# C runtime startoff including monitoring

	.set	sysexit,1
	.set	cbufs,600
	.set	hdr,12
	.set	cnt,8

	.globl	start
	.globl	_monitor
	.globl	_sbrk
	.globl	_main
	.globl	_write
	.globl	_exit
	.globl	_IEH3exit
	.globl	_etext
	.globl	_environ
	.globl	__cleanup
	.globl	____Argv
	.data
	.comm	countbase,4
	.text


	.double	0,0,0,0,0,0,0,0 # space for mod table
start:
	adjspb	$8
	movd	8(sp),0(sp)	#argc
	addr	12(sp),r0
	movd	r0,4(sp)	#argv
	movd	r0,____Argv	#prog name for profiling
L1:
	cmpqd	0,0(r0)		#null args term ?
	addqd	4,r0
	bne	L1
	cmpd	r0,0(4(sp))	#end of 'env' or 'argv' ?
	bls	L2
	addqd	-4,r0		#envp's are in list
L2:
	movd	r0,8(sp)	# env
	movd	r0,_environ	#  indir is 0 if no env ; not 0 if env

	addr	cbufs,tos	# entrance counters (arg5 for monitor)
	addr	_etext,r1
	addr	eprol,r0
	subd	r0,r1		# text size
	addqd	1,r1
	bicb	$1,r1
	addd	$hdr+cnt*cbufs,r1	# add entrance count plus header
	ashd	$-1,r1		# cvt byte length to 'short' count
	movd	r1,tos		# bufsiz (arg4 for monitor)
	ashd	$1,r1		# cvt `short' count back to byte count
	movd	r1,tos		# for sbrk
	jsr	_sbrk
	adjspb	$-4
	cmpqd	-1,r0
	beq	nospace
	movd	r0,tos		# buf ptr (arg3 for monitor)
	addd	$12,r0	# initialize countbase
	movd	r0,countbase
	addr	_etext,tos
	addr	eprol,tos
	jsr	_monitor
	adjspb	$-20
	jsr	_main
	movd	r0,tos
	jsr	_exit

	.data
_environ:	.double	0
emsg:
	.byte	"No space for "
	.byte	"monitor buffer",10,0
em1:
	.text

nospace:
	movd	$em1-emsg,tos
	addr	emsg,tos
	movqd	2,tos
	jsr	_write
	movqd	sysexit,r0
	svc

_exit:
_IEH3exit:
	movqd	0,tos
	jsr	_monitor
	adjspb	$-4
	jsr	__cleanup
	movqd	sysexit,r0
	svc
	.align	2
eprol:
