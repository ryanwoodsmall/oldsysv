/	@(#)rk11r.s	1.1
rblk:
	div	$sct,r0
	add	cyloff,r0
	ash	$4,r0
	bis	r1,r0
	mov	$daddr,r1
	mov	r0,(r1)
	mov	buf,-(r1)
	mov	$-256.,-(r1)
	mov	$read,-(r1)
