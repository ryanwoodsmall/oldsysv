/	@(#)rp03i.s	1.1
read	= 5
daddr	= 176724
sct	= 10.
trk	= 20.

/ set read command to select unit
	movb	r0,drive+3

