/	@(#)rl11i.s	1.1
/ ro has unit number

rladdr	= 174400

/ set seek and read command with unit number

	movb	r0,seekit+3
	movb	r0,readit+3
	movb	r0,headit+3
