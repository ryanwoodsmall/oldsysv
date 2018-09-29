	.file	"time.s"
#	@(#)time.s	1.3
# C library -- time

# tvec = time(tvec);

	.set	time,13
	.globl	_time

_time:
	MCOUNT
	addr	time,r0
	addr	4(sp),r1
	svc
	movd	4(sp),r1
	cmpqd	0,r1
	beq	nostore
	movd	r0,0(r1)
nostore:
	ret	0
