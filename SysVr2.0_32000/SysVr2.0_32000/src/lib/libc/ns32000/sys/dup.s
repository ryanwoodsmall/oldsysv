	.file	"dup.s"
#	@(#)dup.s	1.3
# C library -- dup

#	f = dup(of [ ,nf])
#	f == -1 for error

	.set	dup,41
	.globl	_dup
	.globl	cerror

_dup:
	MCOUNT
	addr	dup,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	ret	0
