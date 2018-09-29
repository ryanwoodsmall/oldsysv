/	@(#)getppid.s	1.3
/ getppid -- get parent process ID

.globl	_getppid
.getpid	= 20.

_getppid:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	sys	.getpid
	mov	r1,r0
	mov	(sp)+,r5
	rts	pc
