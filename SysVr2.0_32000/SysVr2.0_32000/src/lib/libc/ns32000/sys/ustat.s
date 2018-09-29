	.file	"ustat.s"
#	@(#)ustat.s	1.3
# C library -- ustat

	.set	utssys,57
	.set	ustat,2
	.globl	_ustat
	.globl	cerror

_ustat:
	MCOUNT
	addr	0(sp),r0
	addr	ustat,tos
	movd	4(r0),tos
	movd	8(r0),tos
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
