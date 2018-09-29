/ @(#)cswitch.s	1.1
/
/ WARNING _u and these routines must not lie in address range 20000 - 40000
/
.globl	_save
_save:
	mov	(sp)+,r1
	mov	(sp),r0
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	r1,(r0)+
	mov	PS,-(sp)
	bis	$HIPRI,PS
	mov	_u+U_PROCP,r0
	bitb	$SLOAD,P_FLAG(r0)
	bne	1f
	halt
1:
	mov	KISA0+[1*2],-(sp)
	mov	P_ADDR(r0),KISA0+[1*2]
	mov	KISD0+[1*2],-(sp)
	mov	$77406,KISD0+[1*2]
	mov	$20000,r0
	mov	r2,-(sp)
	mov	$_u,r2
	mov	r3,-(sp)
	mov	$usize*32.,r3
1:
	mov	(r2)+,(r0)+
	sob	r3,1b
	mov	(sp)+,r3
	mov	(sp)+,r2
	mov	(sp)+,KISD0+[1*2]
	mov	(sp)+,KISA0+[1*2]
	mov	(sp)+,PS
	clr	r0
	jmp	(r1)

.globl	_resume
_resume:
	bis	$HIPRI,PS
	mov	KISA0+[1*2],r0
	mov	2(sp),KISA0+[1*2]
	mov	KISD0+[1*2],r1
	mov	$77406,KISD0+[1*2]
	mov	$20000,r2
	mov	$_u,r3
	mov	$usize*32.,r4
	mov	4(sp),r5
1:
	mov	(r2)+,(r3)+
	sob	r4,1b
	mov	r1,KISD0+[1*2]
	mov	r0,KISA0+[1*2]
	mov	r5,r1			/ new stack
	mov	(r1)+,r2
	mov	(r1)+,r3
	mov	(r1)+,r4
	mov	(r1)+,r5
	mov	(r1)+,sp
	mov	$1,r0
	bic	$HIPRI,PS
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
	mov	$1,r0
	jmp	*(r1)+
