	.file	"getegid.s"
#	@(#)getegid.s	1.3
# C library -- getegid

# gid = getegid();
# returns effective gid

	.set	getgid,47
	.globl	_getegid

_getegid:
	MCOUNT
	addr	getgid,r0
	addr	4(sp),r1
	svc
	movd	r1,r0
	ret	0
