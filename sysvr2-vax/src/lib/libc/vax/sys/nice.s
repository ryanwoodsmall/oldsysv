	.file	"nice.s"
#	@(#)nice.s	1.5
# C library-- nice

# error = nice(hownice)

	.set	nice,34
.globl	_nice
.globl  cerror

_nice:
	.word	0x0000
	MCOUNT
	chmk	$nice
	bcc 	.noerror
	jmp 	cerror
.noerror:
	ret
