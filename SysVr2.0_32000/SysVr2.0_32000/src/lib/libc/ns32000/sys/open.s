	.file	"open.s"
#	@(#)open.s	1.3
# C library -- open

# file = open(string, mode)

# file == -1 means error

	.set	open,5
	.globl	_open
	.globl	cerror

_open:
	MCOUNT
	addr	open,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	ret	0
