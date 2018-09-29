	.file	"fstat.s"
#	@(#)fstat.s	1.5
# C library -- fstat

# error = fstat(file, statbuf);

# char statbuf[34]

	.set	fstat,28
.globl	_fstat
.globl	cerror

_fstat:
	.word	0x0000
	MCOUNT
	chmk	$fstat
	bcc 	.noerror
	jmp 	cerror
.noerror:
	clrl	r0
	ret
