	.file	"dup.s"
#	@(#)dup.s	1.5
# C library -- dup

#	f = dup(of [ ,nf])
#	f == -1 for error

	.set	dup,41
.globl	_dup
.globl	cerror

_dup:
	.word	0x0000
	MCOUNT
	chmk	$dup
	bcc 	.noerror
	jmp 	cerror
.noerror:
	ret
