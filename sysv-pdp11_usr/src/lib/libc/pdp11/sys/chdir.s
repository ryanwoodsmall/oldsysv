/	@(#)chdir.s	1.3
/ C library -- chdir

/ error = chdir(string);

.globl	_chdir, cerror
.chdir = 12.

_chdir:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	.chdir; 0:..
