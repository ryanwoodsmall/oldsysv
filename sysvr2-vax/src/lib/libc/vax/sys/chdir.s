	.file	"chdir.s"
#	@(#)chdir.s	1.5
# C library -- chdir

# error = chdir(string);

	.set	chdir,12
.globl	_chdir
.globl	cerror

_chdir:
	.word	0x0000
	MCOUNT
	chmk	$chdir
	bcc 	.noerror
	jmp 	cerror
.noerror:
	clrl	r0
	ret
