/	@(#)rp04r.s	1.1
rblk:
	div	$sct*trk,r0
	add	cyloff,r0
	mov	r0,-(sp)
	clr	r0
	div	$sct,r0
	swab	r0
	bis	r1,r0
	mov	$daddr,r1
	mov	(sp)+,24(r1)
	mov	r0,-(r1)
	mov	buf,-(r1)
	mov	$-256.,-(r1)
	mov	$read,-(r1)
