/	@(#)rk11i.s	1.1
read	= 5
daddr	= 177412
sct	= 12.

/ set cyloff to select unit
/ cyloff is not useful as offset for rk
	ash	$9.,r0
	mov	r0,cyloff

