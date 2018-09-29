	.file	"sbrk.s"
#	@(#)sbrk.s	1.5
#  C-library sbrk

#  oldend = sbrk(increment);

#  sbrk gets increment more core, and returns a pointer
#	to the beginning of the new core area

	.set	break,17
.globl	_sbrk
.globl	_end
.globl	cerror

	.align	2
_sbrk:
	.word	0x0000
	MCOUNT
	movl	.nd,r0
	tstl	4(ap)
	jeql	.zero
	addl3	r0,4(ap),-(sp)
	pushl	$1
	movl	ap,r3
	movl	sp,ap
	chmk	$break
	bcc 	.noerr1
	jmp 	cerror
.noerr1:
	movl	.nd,r0
	addl2	4(r3),.nd
.zero:
	ret

#  brk(value)
#  as described in man2.
#  returns 0 for ok, -1 for error.

.globl	_brk

_brk:
	.word	0x0000
	MCOUNT
	chmk	$break
	bcc 	.noerr2
	jmp 	cerror
.noerr2:
	movl	4(ap),.nd
	clrl	r0
	ret

	.data
.nd:	.long	_end
