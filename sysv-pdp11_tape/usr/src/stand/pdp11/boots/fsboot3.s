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
