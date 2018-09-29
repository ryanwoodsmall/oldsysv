/	@(#)gdboot.s	1.1
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
read	= 71
daddr	= 176700
d_typ	= 026		/drive type register

/ select unit, read-in preset and set FMT22
	mov	r0,*$daddr
	mov	$21,*$daddr-10
	mov	$10000,*$daddr+22
/		check device type for seek addr. calcs
	mov	*$daddr+d_typ,r2
	bic	$!70777,r2
	cmp	rp6_dt,r2
	beq	1f
	mov	$rm5_dt,d_lim	/ wasnt rp06, mus' be rm05
	br	.	/ invalid drive type??
1:

/	@(#)fsboot2.s	1.1
	mov	$'=,r0
	jsr	pc,(r5)
	mov	$names,r2
1:
	mov	r2,r1
2:
	jsr	pc,2(r5)
	cmp	r0,$'\n
	beq	1f
	cmp	r0,$'@
	beq	1b
	cmp	r0,$'#
	beq	3f
	cmp	r0,$'/
	bne	4f
	clr	r0
4:
	movb	r0,(r1)+
	br	2b
3:
	cmp	r2,r1
	beq	2b
	dec	r1
	br	2b
1:
	clrb	(r1)+
	movb	$-1,(r1)
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
	clr	r1
	mov	$20,r0
	br	moveunix
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
/	@(#)rp04r.s	1.1
rblk:
	mov	d_lim,r3
	mov	2(r3),r1
	mpy	4(r3)_,r1
	div	r1,r0
	add	cyloff,r0
	mov	r0,-(sp)
	clr	r0
	div	2(r3),r0
	swab	r0
	bis	r1,r0
	mov	$daddr,r1
	mov	(sp)+,24(r1)
	mov	r0,-(r1)
	mov	buf,-(r1)
	mov	$-256.,-(r1)
	mov	$read,-(r1)
/	@(#)fsboot3.s	1.1
1:
	tstb	(r1)
	bge	1b
2:
	rts	pc

tvec:
	br	putc
	br	getc

tks = 177560
tkb = 177562
getc:
	mov	$tks,r0
	inc	(r0)
1:
	tstb	(r0)
	bge	1b
	mov	*$tkb,r0
	bic	$!177,r0
	cmp	r0,$'A
	blo	1f
	cmp	r0,$'Z
	bhi	1f
	add	$'a-'A,r0
1:
	cmp	r0,$'\n
	bne	putc
	mov	$'\r,r0
tps = 177564
tpb = 177566
putc:
	tstb	*$tps
	bge	putc
	mov	r0,*$tpb
	cmp	r0,$'\r
	bne	1f
	mov	$'\n,r0
	br	putc
1:
	rts	pc

/ drive type table (dtyp,sectors,tracks)
rp6_dt:	2012,22.,19.
rm5_dt:	2017,32.,19.
d_lim:	rp6dt
buf:	end
. = start+510.
cyloff:	0
end:
inod	= ..-1024.
mode	= inod
faddr	= inod+12.
addr	= inod+64.
bno	= addr+40.+512.
names	= bno+2.
rootino	= 2
bpi	= 13.
