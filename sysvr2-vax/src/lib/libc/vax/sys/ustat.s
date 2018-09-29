	.file	"ustat.s"
#	@(#)ustat.s	1.5
# C library -- ustat

	.set	utssys,57
	.set	ustat,2
.globl	_ustat
.globl	cerror

_ustat:
	.word	0x0000
	MCOUNT
	pushl	$ustat
	pushl	4(ap)
	pushl	8(ap)
	calls	$3,.sys
	ret

.sys:
	.word	0x0000
	chmk	$utssys
	bcc	.noerror
	jmp	cerror
.noerror:
	clrl	r0
	ret
