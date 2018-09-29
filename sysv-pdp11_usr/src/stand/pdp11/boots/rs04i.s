/	@(#)rs04i.s	1.1
read	= 71
daddr	= 172050

/ select unit, detect rs03 or rs04
	mov	r0,*$daddr
	bit	$2,*$daddr+16
	beq	1f
	mov	$17.,shift+2
1:

