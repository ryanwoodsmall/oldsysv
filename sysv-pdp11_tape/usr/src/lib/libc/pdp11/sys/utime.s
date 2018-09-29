/	@(#)utime.s	1.3
/ C library -- utime

/ error = utime(string, timev);

.globl	_utime, cerror
.utime = 30.

_utime:
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
	sys	.utime; 0:..; ..
