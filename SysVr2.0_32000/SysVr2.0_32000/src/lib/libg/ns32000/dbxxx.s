	.file	"dbxxx.s"
#	@(#)dbxxx.s	1.1
        .data
        .comm   __dbargs,512
        .text
        .align  1
        .globl  __dbsubc
__dbsubc:
	movd	__dbargs+4,r0	#number of args
	ashd	$2,r0		#number of arg bytes
	adjspd	r0		#make room for args
	ashd	$-2,r0		#number of args
	addr	__dbargs+8,r1	#from
	addr	0(sp),r2	#to
	movsd
	movd	__dbargs,r0
	jsr	r0
	movd	__dbargs+4,r2
	ashd	$2,r2
	negd	r2,r2
	adjspd	r2
        .globl  __dbsubn
__dbsubn:
        bpt
        .data
