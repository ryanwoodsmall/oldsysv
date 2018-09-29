/	@(#)getuid.s	1.3
/ C library -- getuid

/ uid = getuid();

.globl	_getuid
.getuid = 24.

_getuid:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	sys	.getuid
	mov	(sp)+,r5
	rts	pc
