	.file	"umask.s"
#	@(#)umask.s	1.5
#  C library -- umask
 
#  omask = umask(mode);
 
	.set	umask,60
.globl	_umask
.globl	cerror

_umask:
	.word	0x0000
	MCOUNT
	chmk	$umask
	bcc	.noerror
	jmp	cerror
.noerror:
	ret
