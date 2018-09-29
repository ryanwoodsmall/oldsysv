/ @(#)cswitch.s	1.1
.globl	_save
_save:
	mov	(sp)+,r1
	mov	(sp),r0
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	__ovno,(r0)+		/ save current overlay number
	mov	r1,(r0)+
	clr	r0
	jmp	(r1)

.globl	_resume
_resume:
	mov	2(sp),r0		/ new process
	mov	4(sp),r1		/ new stack
	spl	7
	mov	r0,KDSA0+[6*2]		/ In new process
	mov	(r1)+,r2
	mov	(r1)+,r3
	mov	(r1)+,r4
	mov	(r1)+,r5
	mov	(r1)+,sp

	mov	(r1)+,r0		/ get saved overlay info
.if OVDEBUG
	tst	r0
	blt	8f-2
	cmp	$14.,r0
	bge	8f
	0
8:
.endif
	mov	r0,__ovno
	mov	__ova(r0),KISA0+[7*2]
	mov	__ovd(r0),KISD0+[7*2]

	mov	$1,r0
	spl	0
	jmp	*(r1)+

.globl	_setjmp
_setjmp:
	mov	(sp)+,r1
	mov	(sp),r0
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	__ovno,(r0)+		/ save current overlay number
	mov	r1,(r0)+
	clr	r0
	jmp	(r1)

.globl	_longjmp
_longjmp:
	mov	2(sp),r1		/ new stack
	mov	(r1)+,r2
	mov	(r1)+,r3
	mov	(r1)+,r4
	mov	(r1)+,r5
	mov	(r1)+,sp

	mov	(r1)+,r0		/ get saved overlay info
.if OVDEBUG
	tst	r0
	blt	8f-2
	cmp	$14.,r0
	bge	8f
	0
8:
.endif
	mov	r0,__ovno
	mov	__ova(r0),KISA0+[7*2]
	mov	__ovd(r0),KISD0+[7*2]

	mov	$1,r0
	jmp	*(r1)+
