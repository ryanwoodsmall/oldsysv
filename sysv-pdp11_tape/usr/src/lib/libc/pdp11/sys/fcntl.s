/	@(#)fcntl.s	1.3
/ C library -- fcntl

/ error = fcntl (file, request, arg);

.globl	_fcntl, cerror
.fcntl	= 62.

_fcntl:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r0
	mov	6(r5),0f
	mov	8(r5),0f+2
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	.fcntl; 0:..; ..
