/	@(#)rl11r.s	1.1
/ r0 and r1 have block number

seek	= 6
rdhdr	= 10
read	= 14
sec	= 20.

rblk:
	mov	$rladdr,r3
headit:
	mov	$rdhdr,(r3)
1:
	tstb	(r3)
	bpl	1b
	div	$sec,r0
	ash	$6,r0		/ cylinder << 7
	asl	r1		/ sector
	bis	r1,r0
	mov	r0,r4
	bic	$177,r4
	mov	r3,r1
	mov	6(r1),r3
	bic	$177,r3
	sub	r4,r3
	bcc	1f
	neg	r3
	bis	$4,r3		/ go to larger cylinder number
1:
	inc	r3
	bit	$100,r0
	beq	1f
	bis	$20,r3
1:
	mov	r3,4(r1)
seekit:
	mov	$seek,(r1)
1:
	tstb	(r1)
	bpl	1b
	mov	$rladdr+8.,r1
	mov	$-256.,-(r1)
	mov	r0,-(r1)
	mov	buf,-(r1)
readit:
	mov	$read,-(r1)
