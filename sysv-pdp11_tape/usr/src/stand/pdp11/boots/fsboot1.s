/	@(#)fsboot1.s	1.1
/ disk boot program to load and transfer
/ to a unix file system entry

/ entry is made by jsr pc,*$0
/ so return can be rts pc
/ jsr pc,(r5) is putc
/ jsr pc,2(r5) is getc

core = 28.
.. = [core*2048.]-512.
reset	= 5
nop	= 240

start:
	nop
	mov	$..,sp
	mov	sp,r1
	cmp	pc,sp
	bhis	main
move:
	clr	r0
moveunix:			/ entered from fsboot2.s to move unix over 407
	mov	r1,-(sp)
1:
	mov	(r0)+,(r1)+
	cmp	r1,buf
	blo	1b
	rts	pc
main:
	reset
	mov	$tvec,r5
	mov	$'#,r0
	jsr	pc,(r5)
	jsr	pc,2(r5)
	bic	$!7,r0
