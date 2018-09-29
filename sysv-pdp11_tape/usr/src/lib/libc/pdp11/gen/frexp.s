/	@(#)frexp.s	1.4
/ double frexp(x, ip)
/ double x; int *ip;
/ returns a fractional part 1/2 <= |value| < 1
/ and stores an exponent so x = value * 2^(*ip)
/ If value is zero, x and *ip are both returned as zero.

.globl	_frexp
.globl	csav, cret

_frexp:
	MCOUNT
	jsr	r0,csav
	movf	4(r5),fr0
	movei	fr0,*12.(r5)
	clr	r0
	movie	r0,fr0
	jmp	cret
