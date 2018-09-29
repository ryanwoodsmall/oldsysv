	.file	"sbrk.s"
#	@(#)sbrk.s	1.3
#  C-library sbrk

#  oldend = sbrk(increment);

#  sbrk gets increment more core, and returns a pointer
#	to the beginning of the new core area

	.set	break,17
	.globl	_sbrk
	.globl	cerror

	.align	2
_sbrk:
	MCOUNT
	cmpqd	0,nd
	bne	gotbrk
	addr	_end,nd
gotbrk:
	addd	nd,4(sp)
	addr	break,r0
	addr	4(sp),r1
	svc
	bfc	noerr1
	jump	cerror
noerr1:
	movd	nd,r0
	movd	4(sp),nd
	ret	0

#  brk(value)
#  as described in man2.
#  returns 0 for ok, -1 for error.

	.globl	_brk

_brk:
	addr	break,r0
	addr	4(sp),r1
	svc
	bfc	noerr2
	jump	cerror
noerr2:
	movd	4(sp),nd
	movqd	0,r0
	ret	0

	.data
	.globl	_end		#ejobcode
nd:
	.double	0
