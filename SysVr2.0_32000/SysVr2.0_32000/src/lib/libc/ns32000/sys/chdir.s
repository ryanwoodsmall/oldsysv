	.file	"chdir.s"
#	@(#)chdir.s	1.3
# C library -- chdir

# error = chdir(string);

	.set	chdir,12
	.globl	_chdir
	.globl	cerror

_chdir:
	MCOUNT
	addr	chdir,r0
	addr	4(sp),r1
	svc
	bfc 	noerror
	jump	cerror
noerror:
	movqd	0,r0
	ret	0
