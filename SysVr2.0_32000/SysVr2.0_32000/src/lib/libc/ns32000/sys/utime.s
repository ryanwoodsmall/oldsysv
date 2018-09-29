	.file	"utime.s"
#	@(#)utime.s	1.3
# C library -- utime
 
#  error = utime(string,timev);
 
	.set	utime,30
	.globl	_utime
	.globl	cerror
 
_utime:
	MCOUNT
	addr	utime,r0
	addr	4(sp),r1
	svc
	bfc	noerror
	jump	cerror
noerror:
	ret	0
