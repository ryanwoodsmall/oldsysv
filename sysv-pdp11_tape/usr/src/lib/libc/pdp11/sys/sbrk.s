/	@(#)sbrk.s	1.3
/	3.0 SID #	1.2
/old = sbrk(increment);

/sbrk gets increment more core, and returns a pointer
/	to the beginning of the new core area
/	if error, sets errno and returns -1 .

.globl	_sbrk, _brk, _end, cerror
.break = 17.

_sbrk:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	beq	1f
	add	nd,0f
	sys	0; 9f
	bcc	1f
	jmp	cerror
1:
	mov	nd,r0
	add	4(r5),nd
	mov	(sp)+,r5
	rts	pc

/ brk(value)
/ as described in man2.
/ returns 0 for ok, -1 for error.

_brk:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),0f
	sys	0; 9f
	bcc	1f
	jmp	cerror
1:
	mov	4(r5),nd
	clr	r0
	mov	(sp)+,r5
	rts	pc

.data
9:
	sys	.break; 0:..
nd:	_end
