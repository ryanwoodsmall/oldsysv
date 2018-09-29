/	@(#)ulimit.s	1.3
/ C library -- ulimit

/ value = ulimit(request, argument);
/	long ulimit(), argument;
/	int request;

.globl	_ulimit, cerror
.ulimit = 63.

_ulimit:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	mov	6(r5),0f+2
	mov	10(r5),0f+4
	sys	0; 9f
	bec	1f
	mov	$-1,r1
	jmp	cerror
1:
	mov	(sp)+,r5
	rts	pc
.data
9:
	sys	.ulimit; 0:.. ;.. ;..
