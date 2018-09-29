	.file	"stat.s"
#	@(#)stat.s	1.3
# C library -- stat

# error = stat(string, statbuf);
# char statbuf[36]

	.set	stat,18
	.globl	_stat
	.globl	cerror

_stat:
	MCOUNT
	addr	stat,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
