/	@(#)getpid.s	1.3
/ getpid -- get process ID

.globl	_getpid
getpid	= 20.

_getpid:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	sys	getpid
	mov	(sp)+,r5
	rts	pc
