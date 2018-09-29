	.file	"stat.s"
#	@(#)stat.s	1.5
# C library -- stat

# error = stat(string, statbuf);
# char statbuf[36]

	.set	stat,18
.globl	_stat
.globl  cerror

_stat:
	.word	0x0000
	MCOUNT
	chmk	$stat
	bcc 	.noerror
	jmp 	cerror
.noerror:
	clrl	r0
	ret
