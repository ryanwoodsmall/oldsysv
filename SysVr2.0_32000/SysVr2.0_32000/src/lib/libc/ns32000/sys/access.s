	.file	"access.s"
#	@(#)access.s	1.3
# access(file, request)
#  test ability to access file in all indicated ways
#  1 - read
#  2 - write
#  4 - execute

	.set	access,33
	.globl	_access
	.globl	cerror

_access:
	MCOUNT
	addr	access,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	ret	0
