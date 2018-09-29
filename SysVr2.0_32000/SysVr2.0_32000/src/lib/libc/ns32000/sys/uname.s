	.file	"uname.s"
#	@(#)uname.s	1.3
# C library -- uname

	.set	utssys,57
	.set	uname,0

	.globl	_uname
	.globl	cerror

_uname:
	MCOUNT
	addr	uname,tos
	movqd	0,tos
	movd	12(sp),tos
	jsr	sys
	adjspb	$-12
	ret	0

sys:
	addr	utssys,r0
	addr	4(sp),r1
	svc
	bfc	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
