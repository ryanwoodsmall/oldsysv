/	@(#)umount.s	1.3
/ C library -- umount

.globl	_umount, cerror
.umount = 22.

_umount:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(sp),0f
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	clr	r0
	mov	(sp)+,r5
	rts	pc

.data
9:
	sys	.umount; 0:..
