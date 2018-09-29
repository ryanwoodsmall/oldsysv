	.file	"creat.s"
#	@(#)creat.s	1.3
# C library -- creat

# file = creat(string, mode);
# file == -1 if error

	.set	creat,8
	.globl	_creat
	.globl	cerror

_creat:
	MCOUNT
	addr	creat,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	ret	0
