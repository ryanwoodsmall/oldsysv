	.file	"fstat.s"
#	@(#)fstat.s	1.3
# C library -- fstat

# error = fstat(file, statbuf);

# char statbuf[34]

	.set	fstat,28
	.globl	_fstat
	.globl	cerror

_fstat:
	MCOUNT
	addr	fstat,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
