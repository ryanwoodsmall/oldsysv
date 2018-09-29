	.file	"getuid.s"
#	@(#)getuid.s	1.3
# C library -- getuid

# uid = getuid();
#  returns real uid

	.set	getuid,24
	.globl	_getuid

_getuid:
	MCOUNT
	addr	getuid,r0
	addr	4(sp),r1
	svc
	ret	0
