/	@(#)mmmch.s	1.1
.globl	start
.globl	_main

start:
	mov	$8192.,sp
	jsr	pc,_main
	br	.
.globl	csav
csav:
	mov	r5,(sp)
	mov	sp,r5
	mov	r4,-(sp)
	mov	r3,-(sp)
	mov	r2,-(sp)
	tst	-(sp)
	mov	r0,pc

.globl cret
cret:
	mov	r5,r2
	mov	-(r2),r4
	mov	-(r2),r3
	mov	-(r2),r2
	mov	r5,sp
	mov	(sp)+,r5
	rts	pc

.globl _ldiv,_lrem

_ldiv:
	clr	r0
	mov	2(sp),r1
	div	4(sp),r0
	rts	pc

_lrem:
	clr	r0
	mov	2(sp),r1
	div	4(sp),r0
	mov	r1,r0
	rts	pc

.globl _runsuper
.globl _rtest
.globl _init

KISDR0 = 172300
KISAR0 = 172340

_runsuper:
	jsr	r0,csav
	mov	sp,r0
	mov	$40000,*$177776		/supervisor mode
	mov	r0,sp
	mov	4(r5),(sp)		/pass arg to rtest
	jsr	pc,*$_rtest
	mov	$KISDR0,(sp)		/reset kernel regs
	mov	$KISAR0,-(sp)
	jsr	pc,*$_init
	tst	(sp)+
	mov	$0,*$177776		/back to kernel mode
	jmp	cret
