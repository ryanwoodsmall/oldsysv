/	@(#)list.s	1.1
/ tape boot program to load and transfer
/ to a 'tp' entry

/ entry is made by jsr pc,*$0
/ so return can be rts pc
/ jsr pc,(r5) is putc
/ jsr pc,2(r5) is getc
/ jsr pc,4(r5) is mesg

tmsize = 62.
tcsize = 24.
start:
2:
	jsr	pc,4(r5)
		<'m' for tm11; 'c' for tc11 ; 'u' for tu16\n\0>
		.even
	jsr	pc,2(r5)
	mov	$tmsize,tpsize

	cmp	r0,$'m
	bne	3f
	mov	$tmblk,rblk
	mov	$tmrew,rew
	br	1f
3:
	cmp	r0,$'c
	bne	3f
	mov	$tcblk,rblk
	mov	$tcrew,rew
	mov	$tcsize,tpsize
	br	1f
3:
	cmp	r0,$'u
	bne	2b
	mov	$tublk,rblk
	mov	$turew,rew
1:
	jsr	pc,*rew
	jsr	pc,4(r5)
		<\nTape entries\n\0>
		.even
	jsr	pc,*rblk
	clr	tapa
tread:
	inc	tapa
	cmp	tapa,tpsize
	bhi	4f
	jsr	pc,*rblk
	mov	$buf,r1
1:
	mov	r1,r2
	tstb	(r1)
	beq	2f
	jsr	pc,string
2:
	cmp	r2,r1
	beq	2f
3:
	mov	r1,-(sp)
	clr	r0
	mov	38.(r2),r1
	div	$512.,r0
	tst	r1
	beq	3f
	inc	r0
3:
	add	r0,wc
	mov	(sp)+,r1
2:
	mov	r2,r1
	add	$64.,r1
	cmp	r1,$buf+512.
	blo	1b
	br	tread
4:
	jsr	pc,psize
	rts	pc
psize:
	jsr	pc,*rew
	jsr	pc,4(r5)
		<\nTape size = \0>
		.even
	mov	$bsize,r2
	mov	wc,r1
	add	tpsize,r1
	add	$1,r1
2:
	clr	r0
	div	$10.,r0
	add	$'0,r1
	movb	r1,(r2)+
	mov	r0,r1
	tst	r0
	bne	2b
2:
	mov	$size,r0
3:
	movb	-(r2),(r0)+
	cmp	$bsize,r2
	blo	3b
	movb	$'\n,(r0)+
	clrb	(r0)
	mov	$size,r1
	jsr	pc,string
	jsr	pc,4(r5)
		<(decimal)\n\0>
		.even
	rts	pc

mts = 172520
mtc = 172522
mtbrc = 172524
mtcma = 172526

tmblk:
	bit	$2,*$mts
	bne	tmblk
	tstb	*$mtc
	bge	tmblk
	mov	$-512.,*$mtbrc
	mov	$buf,*$mtcma
	mov	$60003,*$mtc
1:
	tstb	*$mtc
	bge	1b
	tst	*$mtc
	bge	1f
	jsr	pc,4(r5)
		<tape error\n\0>
		.even
	mov	$-1,*$mtbrc
	mov	$60013,*$mtc
	br	tmblk
1:
	rts	pc

tmrew:
	mov	$60017,*$mtc
	rts	pc

tucs1 = 172440
tuwc = 172442
tuba = 172444
tufc = 172446
tucs2 = 172450
tuds = 172452
tutc = 172472

tublk:
	bit	$020000,*$tuds
	bne	tublk
	tstb	*$tucs1
	bge	tublk
	mov	$-256.,*$tuwc
	mov	$buf,*$tuba
	mov	$071,*$tucs1
1:
	tstb	*$tucs1
	bge	1b
	bit	$040000,*$tucs1
	beq	1f
	jsr	pc,4(r5)
		<tape error\n\0>
		.even
	mov	$040,*$tucs2
	mov	$-1,*$tufc
	mov	$033,*$tucs1
	br	tublk
1:
	rts	pc

turew:
	mov	$040,*$tucs2
	mov	$07,*$tucs1
	rts	pc
tcdt = 177350
tccm = 177342
tcblk:
	mov	$tcdt,r0
	mov	$tccm,r1
for:
	mov	$3,(r1)			/ rbn for
1:
	tstb	(r1)
	bge	1b
	tst	(r1)
	blt	rev
	cmp	tapa,(r0)
	beq	rd
	bgt	for

rev:
	mov	$4003,(r1)		/ rbn bac
1:
	tstb	(r1)
	bge	1b
	tst	(r1)
	blt	for
	mov	(r0),r2
	add	$5,r2
	cmp	tapa,r2
	blt	rev
	br	for

rd:
	mov	$buf,-(r0)			/ bus addr
	mov	$-256.,-(r0)			/ wc
	mov	$5,-(r0)			/ read
1:
	tstb	(r1)
	bge	1b
	tst	(r1)
	blt	tcblk
	rts	pc

tcrew:
	mov	$4003,tccm
	rts	pc


string:
	clr	r4
2:
	tstb	(r1)
	beq	2f
	cmpb	(r1),$'!
	blo	3f
	cmpb	(r1),$0176
	bhi	3f
	movb	(r1)+,r0
	jsr	pc,(r5)
	inc	r4
	cmp	$32.,r4
	bhi	2b
2:
	mov	$'\n,r0
	jsr	pc,(r5)
3:
	rts	pc
.bss
buf: .=.+512.
tapa: .=.+2
rblk: .=.+2
rew:	.=.+2
tpsize:	.=.+2
wc:	.=.+2
bsize:	.=.+32.
size:   .=.+32.
