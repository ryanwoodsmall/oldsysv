/	@(#)cret.s	1.2
/ C register restore -- version 7/75
/
/ calling procedure:
/ prog:	jsr	r0,csav
/	...
/	jmp	cret
/
/ appearance of stack during execution of prog:
/	arg_n
/	...
/	arg_0
/	return address
/	old r5		(r5 -> old r5)
/	old r4
/	old r3
/	old r2
/

.globl	cret

cret:
	mov	r5,r2
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc
