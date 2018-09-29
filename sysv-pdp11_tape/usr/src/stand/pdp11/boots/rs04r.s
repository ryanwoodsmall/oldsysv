/	@(#)rs04r.s	1.1
rblk:
shift:
	ashc	$18.,r0
	add	cyloff,r0
	mov	$daddr,r1
	mov	r0,-(r1)
	mov	buf,-(r1)
	mov	$-256.,-(r1)
	mov	$read,-(r1)
