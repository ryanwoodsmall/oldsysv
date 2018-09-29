/	@(#)csv.s	1.2
/ C register save -- version 7/75
/
/ calling procedure:
/ prog:	jsr	r5,csv
/	...
/	jmp	cret
/
/ appearance of stack during execution of prog:
/	arg_n
/	...
/	arg_0
/	return address
/	old r5		(r5 -> old r5)
/	old r4
/	old r3
/	old r2
/

.globl	csv

csv:
	mov	r5,r0
	mov	sp,r5
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	jsr	pc,(r0)		/ jsr part is sub $2,sp
