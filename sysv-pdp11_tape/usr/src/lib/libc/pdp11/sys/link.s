/	@(#)link.s	1.3
/ C library -- link

/ error = link(old-file, new-file);

.globl	_link, cerror
.link = 9.

_link:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	mov	6(r5),0f+2
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	.link; 0:..; ..
