/	@(#)setgid.s	1.3
/ C library -- setgid

/ error = setgid(uid);

.globl	_setgid, cerror
.setgid = 46.

_setgid:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	sys	.setgid
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
