/	@(#)getegid.s	1.3
/ C library -- getegid

/ gid = getegid();
/ returns effective gid

.globl	_getegid
.getgid	= 47.

_getegid:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	sys	.getgid
	mov	r1,r0
	mov	(sp)+,r5
	rts	pc
