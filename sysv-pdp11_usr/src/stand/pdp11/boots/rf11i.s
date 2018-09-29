/	@(#)rf11i.s	1.1
read	= 5
daddr	= 177470

/ set cyloff to select unit
/ cyloff is not useful as offset for rf
	ash	$2.,r0
	mov	r0,cyloff

