/	@(#)toupper.s	1.3
/ C library -- toupper
/
/	This routine is a fast machine language version of the
/	following C routine:
/
/		int toupper (c)
/			register int c;
/		{
/			if (c >= 'a' && c <= 'z')
/				c += 'A' - 'a';
/			return c;
/		}
/
	.globl	_toupper

_toupper:
	MCOUNT
	mov	2(sp),r0	/ pick up the argument
	cmp	$'a,r0		/ Is it too low?
	jgt	1f		/  ...yes
	cmp	$'z,r0		/ Too high?
	jlt	1f		/  ...yes
	add	$'A-'a,r0	/ Make it upper case
1:
	rts	pc		/ Return
