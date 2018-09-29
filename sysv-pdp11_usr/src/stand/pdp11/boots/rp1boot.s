/	@(#)rp1boot.s	1.1
/ first disk boot program to load and transfer
/ to a unix file system entry

/ entry is made by jsr pc,*$0
/ so return can be rts pc

core = 28.
.. = [core*2048.]-512.
reset	= 5
nop	= 240

start:
	mov	$..,sp
	mov	sp,r2
	mov	r2,-(sp)
	cmp	pc,sp
	bhis	main
move:
	clr	r3
	mov	r2,-(sp)
	cmp	(r3),$407
	bne	1f
	mov	$20,r3
1:
	cmp	r3,r2
	beq	2f
1:
	mov	(r3)+,(r2)+
	cmp	r2,buf
	blo	1b
2:
	rts	pc
main:
	reset
	clr	r3
1:
	clr	(r3)+
	cmp	r3,sp
	blo	1b
/	boot rom put drive # in r0
read	= 5
daddr	= 176724
sct	= 10.
trk	= 20.

/ set read command to select unit
	movb	r0,drive+3


	mov	$bname,r2
	mov	$rootino,r0
1:
	jsr	pc,iget
		nop
	clr	buf
	clr	bno
2:
	tstb	(r2)+
	beq	2b
	bmi	1f
	dec	r2
2:
	jsr	pc,rmblk
		br	start
	clr	r1
3:
	mov	r2,r3
	mov	r1,r4
	add	$16.,r1
	tst	(r4)+
	beq	5f
4:
	cmpb	(r3)+,(r4)
	bne	5f
	tstb	(r4)+
	beq	4f
	cmp	r4,r1
	blo	4b
6:
	tstb	(r3)+
	bne	6b
4:
	mov	-16.(r1),r0
	mov	r3,r2
	br	1b
5:
	cmp	r1,$512.
	blo	3b
	br	2b
1:
	mov	mode,r0
	bic	$!170000,r0
	cmp	$100000,r0
	bne	start
1:
	jsr	pc,rmblk
		br	1f
	add	$512.,buf
	br	1b
1:
	clr	r2
	br	move
iget:
	add	$15.,r0
	mov	r0,r1
	ash	$-3.,r1
	bic	$!17777,r1
	mov	$inod,buf
	bic	$!7,r0
	ash	$6,r0
	sub	r0,buf
	clr	r0
	jsr	pc,rblk
	mov	$faddr,r0
	mov	$addr,r1
	mov	$bpi,r3
1:
	movb	(r0)+,(r1)+
	clrb	(r1)+
	movb	(r0)+,(r1)+
	movb	(r0)+,(r1)+
	sob	r3,1b
	mov	$10.,bno
	mov	$addr+40.,buf
rmblk:
	mov	bno,r0
	asl	r0
	asl	r0
	mov	addr+2(r0),r1
	mov	addr(r0),r0
	bne	1f
	tst	r1
	beq	2f
1:
	add	$2,(sp)
	inc	bno
rblk:
	div	$sct*trk,r0
	add	cyloff,r0
	mov	r0,-(sp)
	clr	r0
	div	$sct,r0
	swab	r0
	bis	r1,r0
	mov	$daddr,r1
	mov	r0,(r1)
	mov	(sp)+,-(r1)
	mov	buf,-(r1)
	mov	$-256.,-(r1)
drive:
	mov	$read,-(r1)
1:
	tstb	(r1)
	bge	1b
2:
	rts	pc

buf:	end
bname:	<stand\0rpboot2\0>
	.byte	-1
. = start+510.
cyloff:	0
end:
inod	= ..-1024.
mode	= inod
faddr	= inod+12.
addr	= inod+64.
bno	= addr+40.+512.
rootino	= 2
bpi	= 13.
