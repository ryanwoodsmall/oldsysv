	.file	"getgid.s"
#	@(#)getgid.s	1.3
# C library -- getgid

# gid = getgid();
# returns real gid

	.set	getgid,47
	.globl	_getgid

_getgid:
	MCOUNT
	addr	getgid,r0
	addr	4(sp),r1
	svc
	ret	0
