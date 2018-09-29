	.file	"stime.s"
#	@(#)stime.s	1.3
#  C - library stime

	.set	stime,25
	.globl	_stime
	.globl	cerror

_stime:
	MCOUNT
	movd	0(4(sp)),4(sp)	# copy time to set
	addr	stime,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
