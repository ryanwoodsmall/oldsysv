/	@(#)copy.s	1.1
/ copy mag tape to disk

/ entry is made by jsr pc,*$0
/ so return can be rts pc
/ jsr pc,(r5) is putc
/ jsr pc,2(r5) is getc
/ jsr pc,4(r5) is mesg

2:
	jsr	pc,4(r5)
		<'p' for rp03;'f' for rf; 'k' for rk;'4' for rp04\n\0>
		.even
	jsr	pc,2(r5)
	cmp	r0,$'k
	bne	3f
	mov	$rkblk,r0
	br	1f
3:
	cmp	r0,$'p
	bne	3f
	mov	$rpblk,r0
	br	1f
3:
	cmp	r0,$'f
	bne	3f
	mov	$rfblk,r0
	br	1f
3:
	cmp	r0,$'s
	bne	3f
	jsr	pc,4(r5)
		<rjs04 not available\n\0>
		.even
	br	2b
	jsr	pc,hsinit
	mov	$hsblk,r0
	br	1f
3:
	cmp	r0,$'4
	bne	2b
hpcs1 = 176700
hpcs2 = 176710
hpof  = 176732

fmt22  = 010000
preset = 021
clear = 040
hpinit:
	mov	$hpcs1,r3
	mov	$clear,8.(r3)
	mov	$preset,(r3)
	mov	$fmt22,26.(r3)
	mov	$hpblk,r0
1:
	mov	r0,wxblk
	mov	$'\n,r0
	jsr	pc,(r5)
2:
	jsr	pc,4(r5)
		<'m' for tm11; 'u' for tu16;'c' for tc11\n\0>
		.even
	jsr	pc,2(r5)
	cmp	r0,$'m
	bne	3f
	mov	$tmblk,rblk
	mov	$tmrew,rew
	br	1f
3:
	cmp	r0,$'u
	bne	3f
	mov	$tublk,rblk
	mov	$turew,rew
	br	1f
3:
	cmp	r0,$'c
	bne	2b
	mov	$tcblk,rblk
	mov	$tcrew,rew
1:
	mov	$'\n,r0
	jsr	pc,(r5)
	jsr	pc,4(r5)
		<disk offset\n\0>
		.even
	jsr	pc,numb
	mov	r0,bno
	jsr	pc,4(r5)
		<tape offset\n\0>
		.even
	jsr	pc,*rew
	jsr	pc,numb
	mov	r0,r1
	beq	2f
	mov	r0,tapa
	cmp	rblk,$tcblk
	beq	2f
1:
	jsr	pc,*rblk
	dec	r1
	bne	1b
2:

	jsr	pc,4(r5)
		<count\n\0>
		.even
	jsr	pc,numb
	mov	r0,r1
1:
	jsr	pc,*rblk
	jsr	pc,wblk
	inc	tapa
	dec	r1
	bne	1b
	jsr	pc,*rew
	rts	pc

numb:
	clr	r1
1:
	jsr	pc,2(r5)
	cmp	r0,$'\n
	beq	1f
	sub	$'0,r0
	cmp	r0,$9
	bhi	2f
	mul	$10.,r1
	add	r0,r1
	br	1b
1:
	mov	r1,r0
	rts	pc
2:
	jsr	pc,4(r5)
		<illegal digit\n\0>
		.even
	tst	(sp)+
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
	mov	r1,sav
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
	mov	sav,r1
	rts	pc

tcrew:
	mov	$4003,tccm
	rts	pc
wblk:
	mov	bno,r0
	inc	bno
	mov	r1,-(sp)
	mov	r0,r1
	clr	r0
	jmp	*wxblk

rpda = 176724
rpblk:
	div	$10.,r0
	mov	r1,-(sp)
	mov	r0,r1
	clr	r0
	div	$20.,r0
	bisb	r1,1(sp)
	mov	$rpda,r1
	mov	(sp)+,(r1)
	mov	r0,-(r1)
	br	1f

rfda = 177466

rfblk:
	div	$8.,r0
	mov	r0,sav
	ash	$-5.,r0
	mov	r0,*$rfda+2
	mov	sav,r0
	ash	$11.,r0
	mov	r0,-(sp)
	mul	$256.,r1
	bis	r1,(sp)
	mov	$rfda,r1
	mov	(sp)+,(r1)
	br	1f
rkda = 177412
rwrite = 03
rkblk:
	div	$12.,r0
	ash	$4.,r0
	bis	r1,r0
	mov	$rkda,r1
	mov	r0,(r1)

1:
	mov	$buf,-(r1)
	mov	$-256.,-(r1)
	mov	$3,-(r1)
	br	intr

hpda = 176724
hpdc = 176734
hpblk:
	div	$22.,r0
	mov	r1,-(sp)
	mov	r0,r1
	clr	r0
	div	$19.,r0
	bisb	r1,1(sp)
	mov	$hpcs1+8.,r1
	mov	r0,20.(r1)
	mov	(sp)+,r0
	br	1f
hscs1 = 172040
hsdt  = 172066
clear = 040
hsinit:
	mov	$clear,$hscs1
	mov	$4.,sav
	tst	$hsdt
	bne	2f
	mov	$8.,sav
2:
	rts	pc
hsda = 172046
hwrite = 061
hsblk:
	div	$sav,r0
	ash	$6.,r0
	bis	r1,r0
	mov	$hsda+2,r1
1:
	mov	r0,-(r1)
	mov	$buf,-(r1)
	mov	$-256.,-(r1)
	mov	$hwrite,-(r1)
1:
intr:
	tstb	(r1)
	bge	1b
	tst	(r1)
	blt	1f
	mov	(sp)+,r1
	rts	pc
1:
	jsr	pc,4(r5)
		<disk error\n\0>
		.even
	mov	(sp)+,r1
	dec	bno
	br	wblk

.bss
buf:	.=.+512.
rblk:	.=.+2
wxblk:	.=.+2
rew:	.=.+2
tapa:	.=.+2
bno:	.=.+2
sav:	.=.+2
