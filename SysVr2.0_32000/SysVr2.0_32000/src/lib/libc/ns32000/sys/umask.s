	.file	"umask.s"
#	@(#)umask.s	1.3
#  C library -- umask
 
#  omask = umask(mode);
 
	.set	umask,60
	.globl	_umask
	.globl	cerror

_umask:
	MCOUNT
	addr	umask,r0
	addr	4(sp),r1
	svc
	bfc	noerror
	jump	cerror
noerror:
	ret	0
