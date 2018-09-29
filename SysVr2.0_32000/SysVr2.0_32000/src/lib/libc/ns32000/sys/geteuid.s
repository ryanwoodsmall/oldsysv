	.file	"geteuid.s"
#	@(#)geteuid.s	1.3
# C library -- geteuid

# uid = geteuid();
#  returns effective uid

	.set	getuid,24
	.globl	_geteuid

_geteuid:
	MCOUNT
	addr	getuid,r0
	addr	4(sp),r1
	svc
	movd	r1,r0
	ret	0
