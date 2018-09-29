	.file	"cuexit.s"
#	@(#)cuexit.s	1.3
# C library -- exit

# exit(code)
# code is return in r0 to system

	.set	sysexit,1
	.globl	_exit
	.globl	__cleanup

	.align	2
_exit:
	MCOUNT
	jsr	__cleanup
	movqd	sysexit,r0
	addr	4(sp),r1
	svc
	bpt
