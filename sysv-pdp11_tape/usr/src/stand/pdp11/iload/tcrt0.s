/	@(#)tcrt0.s	1.1
/ C runtime startoff
/ for PWB tape boot programs

.globl	_exit
.globl	_main

start:
	br	1f
. = 4^.
	6; 0		/ trap catcher, halts at 6, pc = 10
	12; 0
	16; 0
	22; 0
	26; 0
	32; 0
	36; 0
1:
	mov	sp,save
	clr	-(sp)
	clr	-(sp)	/ argc = 0
	jsr	pc,_main
	mov	r0,2(sp)
_exit:
	tst	2(sp)
	beq	1f
	mov	save,sp
	rts	pc
1:
	0
	br	1b
save:
	. = .+2
