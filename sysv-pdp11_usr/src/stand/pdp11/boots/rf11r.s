/	@(#)rf11r.s	1.1
rblk:
	ashc	$8.,r0
	mov	r1,-(sp)
	add	cyloff,r0
	mov	$daddr,r1
	mov	r0,(r1)
	mov	(sp)+,-(r1)
	mov	buf,-(r1)
	mov	$-256.,-(r1)
	mov	$read,-(r1)
