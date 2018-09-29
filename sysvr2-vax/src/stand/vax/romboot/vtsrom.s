#	@(#)vtsrom.s	1.1
.set 	RELOC,0x20000
.set	SSR,0x80
.set	READ,0xc001
.set	BUFSIZ,512
start:
	.space	0xc
	movl	$RELOC,sp
	moval	start,r6
	movc3	$end-start,(r6),(sp)
	jmp	*$RELOC+main-start
main:
	movl	$0xfff550,r0
	movl	$0xf30800,r1
	clrw	2(r0)
	bsbb	wait
rd:
	movl	$0x80000000,(r1)
	movl	$0x80000000+RELOC/0x200,4(r1)
	moval	cmd,r3
	bicl2	$0xfffffe00,r3
	bisl2	$0x200,r3
	movw	r3,(r0)
	bsbb	wait
	movw	$BUFSIZ,count
	clrw	cmd+2
	movw	$READ,cmd
	movw	r3,(r0)
	sobgeq	indx,rd
	halt
wait:
	bitw	$SSR,2(r0)
	beql	wait
	rsb
.align	2
cmd:	.word	0xc004
	.word	0x200+cad
	.word	0
count:	.word	8
sts:	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
	.word	0
cad:	.word	0x200+sts
	.word	0
	.word	cad-sts
	.word	0
indx:
	.long	1
end:
