/	@(#)umask.s	1.3
/ C library -- umask

/ omask = umask(mode);

.globl	_umask, cerror
.umask = 60.

_umask:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	sys	0; 9f
	bec	1f
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	.umask; 0:..
