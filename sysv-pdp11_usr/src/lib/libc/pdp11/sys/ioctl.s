/	@(#)ioctl.s	1.3
/ C library -- ioctl

/ error = ioctl(file, request, arg);

.globl	_ioctl, cerror
.ioctl	= 54.

_ioctl:
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
	sys	.ioctl; 0:..; ..
