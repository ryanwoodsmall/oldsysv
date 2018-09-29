/	@(#)rm05i.s	1.1
read	= 71
daddr	= 176710
sct	= 32.
trk	= 19.

/ select unit, read-in preset and set FMT22
	mov	r0,*$daddr
	mov	$21,*$daddr-10
	mov	$10000,*$daddr+22

