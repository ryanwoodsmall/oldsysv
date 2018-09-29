/	@(#)tapeboot.s	1.1
core = 28.
.. = [core*2048.]-512.
start:
	mov	$..,sp
	mov	sp,r1
	cmp	pc,r1
	bhis	main
	clr	r0
	cmp	(r0),$407
	bne	1f
	mov	$20,r0
1:
	mov	(r0)+,(r1)+
	cmp	r1,$end
	blo	1b
	jmp	(sp)
tread:
	turead
rew:
	turew
main:
	mov	$load,*$4
	clr	*$6
	tst	*$mtc
/ check for existence only, will trap if not there
	mov	$tmrew,rew
	mov	$tmread,tread
load:
	jsr	pc,mesg
	<UNIX tape boot loader\r\n\0>
	.even
	jsr	pc,*rew
	clr	r2
	jsr	pc,*tread
	jsr	pc,*tread
	clr	r0
1:
	clr	(r0)+
	cmp	r0,sp
	blo	1b
1:
	jsr	pc,*tread
	bne	1f
	add	$512.,r2
	br	1b
1:
	clr	r0
	cmp	$407,(r0)
	bne	2f
1:
	mov	20(r0),(r0)+
	cmp	r0,sp
	blo	1b
2:
	jsr	pc,*$0
	br	load

mtc	= 172522
tmrew:
	mov	$60017,*$mtc
	rts	pc

tmread:
	bit	$2,*$mtc-2
	bne	tmread
	tstb	*$mtc
	bge	tmread
	mov	$mtc+6,r1
	mov	r2,-(r1)
	mov	$-512.,-(r1)
	mov	$60003,-(r1)
1:
	tstb	(r1)
	bge	1b
	bit	$100000,(r1)
	beq	1f
	bit	$40000,-2(r1)
	beq	2f
1:
	rts	pc
2:
	mov	$-1,2(r1)
	mov	$60013,(r1)
	br	error

mtcs1	= 172440
turew:
	mov	$40,*$mtcs1+10
	mov	$7,*$mtcs1
	rts	pc

turead:
	bit	$20000,*$mtcs1+12
	bne	turead
	tstb	*$mtcs1
	bge	turead
	mov	$mtcs1+6,r1
	mov	r2,-(r1)
	mov	$-256.,-(r1)
	mov	$71,-(r1)
1:
	tstb	(r1)
	bge	1b
	bit	$40000,(r1)
	beq	1f
	bit	$4,12(r1)
	beq	2f
1:
	rts	pc
2:
	mov	$40,10(r1)
	mov	$-1,6(r1)
	mov	$33,(r1)
error:
	jsr	pc,mesg
	<tape error\r\n\0>
	.even
	jmp	*tread

tps = 177564
tpb = 177566
putc:
	tstb	*$tps
	bge	putc
	mov	r0,*$tpb
	rts	pc

mesg:
	movb	*(sp),r0
	jsr	pc,putc
	beq	1f
	inc	(sp)
	br	mesg
1:
	add	$2,(sp)
	bic	$1,(sp)
	rts	pc
end:
