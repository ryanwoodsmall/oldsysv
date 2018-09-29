/	@(#)modf.s	1.3
/ double modf(x, *fp)
/ double x, *fp;
/ return fractional part
/ stash integer part (as double)

.globl	_modf
.globl	csav, cret
one	= 040200

_modf:
	MCOUNT
	jsr	r0,csav
	movf	4(r5),fr0
	modf	$one,fr0
	movf	fr1,*12.(r5)
	jmp	cret
