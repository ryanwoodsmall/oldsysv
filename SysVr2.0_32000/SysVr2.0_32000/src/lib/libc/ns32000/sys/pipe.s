	.file	"pipe.s"
#	@(#)pipe.s	1.3
# pipe -- C library

#	pipe(f)
#	int f[2];

	.set	pipe,42
	.globl	_pipe
	.globl	cerror

_pipe:
	MCOUNT
	addr	pipe,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	movd	4(sp),r2
	movd	r0,0(r2)
	movd	r1,4(r2)
	movqd	0,r0
	ret	0
