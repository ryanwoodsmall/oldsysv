/	@(#)rp2boot.s	1.2
/ disk boot program to load and transfer
/ to a unix file system entry

/ entry is made by jsr pc,*$0
/ so return can be rts pc
/ jsr pc,(r5) is putc
/ jsr pc,2(r5) is getc
/ jsr pc,4(r5) is mesg

halt	= 0
reset	= 5
nop	= 240

start:
	mov	$77406,KISD0
	mov	$0,KISA0
	mov	$77016,KISD0+[6*2]
	mov	$HMEM+4-200,KISA0+[6*2]
	mov	$77406,KISD0+[7*2]
	mov	$177600,KISA0+[7*2]
	mov	$77406,KISD0+[1*2]
	mov	$HMEM+4,KISA0+[1*2]
	mov	sp,r4
	mov	$160000,sp
	inc	SSR0
	clr	-(sp)
	mov	r5,-(sp)
	mov	r4,-(sp)
	mov	$0,r0
	mov	$20000,r1
	mov	$end-start,r2
	asr	r2
1:
	mov	(r0)+,(r1)+
	sob	r2,1b
	mov	KISA0+[1*2],KISA0
main:
	mov	$tvec,r5
	mov	$'#,r0
	jsr	pc,(r5)
	jsr	pc,2(r5)
	bic	$!7,r0
read	= 5|[[HMEM\/64.]&60]
daddr	= 176724
sct	= 10.
trk	= 20.

/ set read command to select unit
	movb	r0,drive+3

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
	mov	$temp,buf
	clr	bno
2:
	tstb	(r2)+
	beq	2b
	bmi	1f
	dec	r2
2:
	jsr	pc,rmblk
		br	error1
	mov	$temp,r1
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
	cmp	r1,$temp+512.
	blo	3b
	br	2b
1:
	mov	mode,r0
	bic	$!170000,r0
	cmp	$100000,r0
	bne	error2
	mov	$temp,buf
	jsr	pc,rmblk
		br	error3
	mov	$temp,r0
	clr	r2
	clr	r3
	cmp	(r0),$407
	bne	1f

	tst	(r0)+
	mov	(r0)+,r3
	add	(r0)+,r3
	ror	r3
	add	$10.,r0
	br	2f
1:
	cmp	(r0),$437	/ overlayed a.out ??
	bne	2f
	movb	13.(r0),r2	/ hi-text
	mov	2(r0),r3	/ text
	add	4(r0),r3	/ data
	adc	r2
	ashc	$-1,r2		/ get word count
	add	$16.,r0		/ skip 437 header
2:
	mov	$77406,KISD0+[2*2]
	clr	KISA0+[2*2]
	mov	$40000,r4
1:
	mov	(r0)+,(r4)+
	sub	$1,r3
	sbc	r2
	bne	3f
	tst	r3
	beq	1f
3:
	cmp	r4,$60000
	blo	2f
	add	$200,KISA0+[2*2]
	mov	$40000,r4
2:
	cmp	r0,$temp+512.
	blo	1b
	jsr	pc,rmblk
		br	1f
	mov	$temp,r0
	br	1b
1:
	mov	(sp)+,r4
	mov	(sp)+,r5
	mov	r4,sp
	mov	KISA0+[6*2],KISA0+[7*2]
	mov	$reset,177776
	jmp	177776

error1:
	jsr	pc,4(r5)
	<\nerror1\n\0>
	.even
	halt
error2:
	jsr	pc,4(r5)
	<\nerror2\n\0>
	.even
	halt
error3:
	jsr	pc,4(r5)
	<\nerror3\n\0>
	.even
	halt
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
	mov	$11.,bno
	mov	$addr+44.,buf
	jsr	pc,rmblk
		nop
	mov	$12.,bno
	mov	$addr+40.+512.+512.,buf
	jsr	pc,rmblk
		nop
	mov	$11.,bno
	mov	$addr+40.+512.,buf
	jsr	pc,rmblk
		nop
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
	mov	buf,r0
	add	reloc,r0
	mov	r0,-(r1)
	mov	$-256.,-(r1)
drive:
	mov	$read,-(r1)
1:
	tstb	(r1)
	bge	1b
2:
	rts	pc

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
	add	$'a-'A,r0
1:
	cmp	r0,$'\r
	bne	putc
	mov	$'\n,r0
tps = 177564
tpb = 177566
putc:
	tstb	*$tps
	bge	putc
	mov	r0,*$tpb
	cmp	r0,$'\n
	bne	1f
	jsr	pc,4(r5)
	.byte	'\r, 177
	177
	mov	$'\n,r0
1:
	rts	pc

1:
	inc	(sp)
	jsr	pc,(r5)
mesg:
	movb	*(sp),r0
	bne	1b
	asr	(sp)
	inc	(sp)
	asl	(sp)
	rts	pc

buf:	end
cyloff:	0
reloc:	400
bno:	.=.+2
names:	.=.+128.
temp:	.=.+512.
inod:	.=.+64.+40.+512.+512.+512.
mode	= inod
faddr	= inod+12.
addr	= inod+64.
rootino	= 2
bpi	= 13.
end:

KISA0	= 172340
KISD0	= 172300
SSR0	= 177572
HMEM	= 6000
