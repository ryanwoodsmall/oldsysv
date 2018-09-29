/	@(#)sash0.s	1.1
/ Startup code for two-stage bootstrap

/ non-UNIX instructions
mfpi	= 6500^tst
mtpi	= 6600^tst
rtt	= 6
reset	= 5
trap	= 104400

.globl	_end
.globl	_main
.globl	_argc
.globl	__exit
.globl	_xcom

	reset
	mov	$340,PS
	mov	$140100,sp

	mov	$77406,r2

/ set kernel I+D to physical 0 and IO page
	clr	r1
	mov	$KISA0,r3
	mov	$KISD0,r4
	jsr	pc,setseg
	mov	$IO,-(r3)
	clr	r1
	mov	$KDSA0,r3
	mov	$KDSD0,r4
	jsr	pc,setseg
	mov	$IO,-(r3)

/ set user I to physical 64K (words) and IO page
	mov	$4000,r1
	mov	$UISA0,r3
	mov	$UISD0,r4
	jsr	pc,setseg
	mov	$IO,-(r3)

/ enable map
	mov	$64,SSR3	/ 22-bit map
	bit	$20,SSR3
	beq	1f
	mov	$3,MSCR
1:
	mov	$30340,PS
	inc	SSR0

/ copy program to user I space
	mov	$_end,r0
	asr	r0
	clr	r1
1:
	mov	(r1),-(sp)
	mtpi	(r1)+
	sob	r0,1b

/ continue execution in user space copy
	mov	$140340,-(sp)
	mov	$main0,-(sp)
	rtt

main0:
	mov	$STACK-10,sp
	jsr	pc,_main
	jsr	pc,__exit

__exit:
	mov	$STACK-10,sp
	jsr	pc,_xcom
1:
	mov	$STACK,r2
	mov	$157300,r0
	sub	r2,r0
	asr	r0
	mov	$stack,r1
1:
	mov	(r1)+,-(sp)
	mtpi	(r2)+
	sob	r0,1b

	mov	$STACK+4,-(sp)	/ initial value of program sp
	mtpi	sp

	mov	$STACK-6,r2
	mov	$3,r0
	mov	$chmk,r1
1:
	mov	(r1)+,(r2)+
	sob	r0,1b

	jmp	*$STACK-6	/ enter KERNEL mode; continue at STACK

chmk:
	mov	$340,*$PS

stack:
	jmp	*$STACK+12	/ 157250=STACK
_argc:	0			/ 157254 (argc)
	157300			/ 157256 (argv)
	123456			/ 157260 (magic number)
	jsr	pc,*$0		/ 157262
	mov	$157274,sp	/ 157266
	rtt			/ 157272
	__exit			/ 157274 (PC)
	140340			/ 157276 (PS)

setseg:
	mov	$8,r0
1:
	mov	r1,(r3)+
	add	$200,r1
	mov	r2,(r4)+
	sob	r0,1b
	rts	pc

/ setseg (base)
.globl	_setseg
_setseg:
	mov	2(sp),r1
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	r4,-(sp)
	mov	$77406,r2
	mov	$KISA0,r3
	mov	$KISD0,r4
	jsr	pc,setseg
	mov	(sp)+,r4
	mov	(sp)+,r3
	mov	(sp)+,r2
	rts	pc

/ movc3 (length, from, to, KERNEL or USER)
.globl	_movc3
_movc3:
	mov	r2,-(sp)
	mov	4(sp),r0
	beq	2f

	clc
	ror	r0
	mov	6(sp),r1
	mov	10(sp),r2
	tst	12(sp)
	beq	1f
0:
	mov	(r1)+,-(sp)
	mtpi	(r2)+
	sob	r0,0b
	br	2f
1:
	mfpi	(r1)+
	mtpi	(r2)+
	sob	r0,1b

2:
	mov	(sp)+,r2
	rts	pc

/ clrseg (buf, length, KERNEL or USER)
.globl	_clrseg
_clrseg:
	mov	4(sp),r0
	beq	2f

	clc
	ror	r0
	mov	2(sp),r1
	tst	6(sp)
	beq	1f
0:
	clr	(r1)+
	sob	r0,0b
	br	2f
1:
	clr	-(sp)
	mtpi	(r1)+
	sob	r0,1b
2:
	rts	pc

PS	= 177776
SSR0	= 177572
SSR3	= 172516
KISA0	= 172340
KISD0	= 172300
KDSA0	= 172360
KDSD0	= 172320
UISA0	= 177640
UISD0	= 177600
MSCR	= 177746	/ 11/70 memory control register
IO	= 177600
STACK	= 157250
