/	@(#)mboot.s	1.1
/
/ tape boot program to load and transfer
/ to a 'tp' entry

/ entry is made by jsr pc,*$0
/ so return can be rts pc
/ jsr pc,(r5) is putc
/ jsr pc,2(r5) is getc
/ jsr pc,4(r5) is mesg

core = 28.
.tc. = 0
.. = [core*2048.]-512.
.if .tc.
toc = 24.*256.
.endif
.if .tc.-1
toc = 62.*256.
.endif
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

main:
.if .tc.-1
	mov	pc,tu16
	mov	$load,*$4
	clr	*$6
	bit	$16,*$mtc
	beq	load
	clr	tu16
.endif
load:
	jsr	pc,rew
	mov	$tvec,r5
	mov	$'=,r0
	jsr	pc,(r5)
	mov	$name,r1
2:
	mov	r1,r2
1:
	jsr	pc,2(r5)
	cmp	r0,$'\n
	beq	1f
	cmp	r0,$'@
	beq	2b
	cmp	r0,$'#
	bne	3f
	dec	r2
	cmp	r2,r1
	bhis	1b
3:
	movb	r0,(r2)+
	br	1b
1:
	clrb	(r2)
	cmp	r2,r1
	blos	load
	mov	$1,r4
	mov	$-toc,r3
	jsr	pc,taper
	clr	r2
1:
	mov	r2,r3
	mov	r1,r0
2:
	cmpb	(r0)+,(r3)
	bne	2f
	tstb	(r3)+
	bne	2b
	br	1f
2:
	add	$64.,r2
	cmp	r2,$2*toc
	blo	1b
	br	load
1:
	mov	44.(r2),r4
	mov	38.(r2),r3
	inc	r3
	clc
	ror	r3
	neg	r3
	clr	r0
1:
	clr	(r0)+
	cmp	r0,sp
	blo	1b
	jsr	pc,taper
	jsr	pc,rew
	clr	r0
	cmp	(r0),$407		/ unix a.out?
	bne	2f
1:
	mov	20(r0),(r0)+
	cmp	r0,sp
	blo	1b
2:
	jsr	pc,*$0
	br	start

.if .tc.-1
mtc = 172522

2:
	jsr	pc,rew
taper:
/				r2 = mtma, r3 = wc, r4 = tapa
	clr	r2
	cmp	mtapa,r4
	beq	1f
	bhi	2b
	jsr	pc,rrec
	br	taper
1:
	jsr	pc,rrec
	add	$256.,r3
	bmi	1b
	rts	pc

rrec:
	tst	tu16
	bne	rrec16
	bit	$2,*$mtc-2
	bne	rrec
	tstb	*$mtc
	bge	rrec
	mov	$mtc+6,r0
	mov	r2,-(r0)
	mov	$-512.,-(r0)
	mov	$60003,-(r0)
1:
	tstb	(r0)
	bge	1b
	tst	(r0)+
	bge	2f
	mov	$-1,(r0)
	mov	$60013,-(r0)
	br	rrec
2:
	add	$512.,r2
	inc	mtapa
	rts	pc

rew:
	tst	tu16
	bne	rew16
	mov	$60017,*$mtc
3:
	clr	mtapa
	rts	pc

mtcs1 = 172440

rrec16:
	bit	$020000,*$mtcs1+12
	bne	rrec16
	tstb	*$mtcs1
	bge	rrec16
	mov	$mtcs1+6,r0
	mov	r2,-(r0)
	mov	$-256.,-(r0)
	mov	$071,-(r0)
1:
	tstb	(r0)
	bge	1b
	tst	(r0)
	bge	2b
	mov	$040,10(r0)
	mov	$-1,6(r0)
	mov	$033,(r0)
	br	rrec16

rew16:
	mov	$040,*$mtcs1+10
	mov	$07,*$mtcs1
	br	3b
.endif

.if .tc.
tccm = 177342
taper:
/				r2 = mtma, r3 = wc, r4 = tapa
	mov	$tccm,r0
for:
	mov	$3,(r0)			/ rbn for
1:
	tstb	(r0)
	bge	1b
	tst	(r0)
	blt	rev
	cmp	r4,6(r0)
	beq	rd
	bgt	for

rev:
	mov	$4003,(r0)		/ rbn bac
1:
	tstb	(r0)
	bge	1b
	tst	(r0)
	blt	for
	mov	6(r0),r2
	add	$5,r2
	cmp	r4,r2
	blt	rev
	br	for

rd:
	add	$6,r0
	clr	-(r0)			/ bus addr
	mov	r3,-(r0)		/ wc
	mov	$5,-(r0)		/ read
1:
	tstb	(r0)
	bge	1b
	tst	(r0)
	blt	taper
	rts	pc

rew:
	mov	$4003,*$tccm
	rts	pc
.endif

tvec:
	br	putc
	br	getc
	br	mesg

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
	add	$40,r0
1:
	cmp	r0,$'\r
	bne	putc
	mov	$'\n,r0

tps = 177564
tpb = 177566
putc:
	tstb	*$tps
	bge	putc
	cmp	r0,$'\n
	bne	1f
	jsr	pc,4(r5)
	.byte	'\r, '\n+200, 0
	.even
	mov	$'\n,r0
	rts	pc
1:
	mov	r0,*$tpb
	rts	pc

mesg:
	movb	*(sp),r0
	inc	(sp)
	jsr	pc,(r5)
	bne	mesg
	inc	(sp)
	bic	$1,(sp)
	rts	pc

tu16:	.=.+2
mtapa:	.=.+2
end:
name	= ..-64.
