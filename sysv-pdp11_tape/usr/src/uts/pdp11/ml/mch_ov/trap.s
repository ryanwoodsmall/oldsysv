/ @(#)trap.s	1.1
.globl	trap4, trap10, trap14, trap24, trap240, trap250
.globl	_trap, trap, call, _stray
.globl	_pwr

RTSPC = rts pc

trap4:
	mov	PS,saveps
	tst	sp		/test for Red Zone Violation
	beq	9f
	br	1f

trap10:
	mov	PS,saveps
	bit	$20000,PS
	bne	2f
	cmp	(sp),$nofpp
	bne	2f
	mov	$PRSUPR,PS
	mov	$RTSPC,-(sp)
	mtpi	_savfp
	mov	$RTSPC,-(sp)
	mtpi	_restfp
	rtt
2:
	jsr	r0,call1; jmp _trap

trap14:
	mov	PS,saveps
	cmpb	$377,2(sp)	/ test if stray interrupt
	bne	2f
	mov	(sp)+,saveps	/ save pc of stray and fix stack
	tst	(sp)+
	jsr	r0,call1; jmp _stray

trap24:
	jbr	powtrap

trap240:
	mov	PS,saveps
	cmpb	$42,PIR
	bne	5f
	bic	$1000,PIR
	spl	1
	jsr	r0,call; jmp _pwr
5:
	br	2f

trap250:
	mov	PS,saveps
1:
	bit	$20000,PS
	bne	3f
	cmp	(sp),$nofulow
	blo	1f
	cmp	(sp),$nofuhigh
	bhis	1f
	mov	$nofuerr,(sp)
	br	4f
1:
	tst	nofault
	beq	1f
	mov	nofault,(sp)
4:
	mov	$1,SSR0
	rtt
1:
3:
	mov	SSR0,ssr
	mov	SSR0+[1*2],ssr+2
	mov	SSR0+[2*2],ssr+4
	mov	$1,SSR0
2:
	jsr	r0,call1; jmp _trap

trap:
	spl	0
	jsr	r0,call; jmp _trap

/ final death
9:
	reset
	mov	r0,stk
	mov	$0f,r0
1:
	tstb	*$177564
	bge	1b
	movb	(r0)+,*$177566
	tstb	(r0)
	bne	1b
	mov	stk,r0
1:
	halt
	br	1b
0:
	<\0death\0>
.text

.globl	_runrun
call1:
	mov	saveps,-(sp)
	spl	0
	br	1f

call:
	mov	PS,-(sp)
1:
	mov	r1,-(sp)
	mfpd	sp
	mov	4(sp),-(sp)
	bic	$!37,(sp)
	bit	$20000,PS
	beq	1f
	jsr	pc,(r0)+
	tstb	_runrun
	beq	2f
	mov	$12.,(sp)		/ trap 12 is give up cpu
	jsr	pc,_trap
2:
	tst	(sp)+
	mtpd	sp
	br	2f
1:
	bis	$PRUSER,PS
	jsr	pc,(r0)+
	cmp	(sp)+,(sp)+
2:
	mov	(sp)+,r1
	tst	(sp)+
	mov	(sp)+,r0
	rtt

.globl	_idle, _waitloc
_idle:
	mov	PS,-(sp)
	spl	0
	wait
waitloc:
	mov	(sp)+,PS
	rts	pc

	.data
_waitloc:
	waitloc
	.text
