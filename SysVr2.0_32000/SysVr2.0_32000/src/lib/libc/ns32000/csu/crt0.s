	.file	"crt0.s"
#	@(#)crt0.s	1.3
# C runtime startoff

	.set	sysexit,1
	.globl	_exit
	.globl	start
	.globl	_main
	.globl	_environ

#
#	C language startup routine

	.double	0,0,0,0,0,0,0,0	# space for mod table
start:
	adjspb	$8
	movd	8(sp),0(sp)	#argc
	addr	12(sp),r0
	movd	r0,4(sp)	#argv
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
	jsr	_main
	adjspb	$-12
	movd	r0,tos
	jsr	_exit
	movqd	sysexit,r0
	addr	0(sp),r1
	svc
#
mcount:				# dummy in case an object module has been
	ret	0		# compiled with cc -p but not the load module
#
	.data
_environ:	.double	0
