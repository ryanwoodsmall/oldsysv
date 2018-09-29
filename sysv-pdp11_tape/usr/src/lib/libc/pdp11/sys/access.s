/	@(#)access.s	1.3
/  return = access(file, request)
/  test ability to access file in all indicated ways
/  1 - read
/  2 - write
/  4 - execute

.globl	_access, cerror
.access = 33.

_access:
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
	sys	.access; 0: ..; ..
