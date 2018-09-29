/	@(#)signal.s	1.3
/	3.0 SID #	1.2
/ C library -- signal

/ signal(n, 0); /* default action on signal(n) */
/ signal(n, odd); /* ignore signal(n) */
/ signal(n, label); /* goto label on signal(n) */
/ returns old label, only one level.

rtt	= 6
.signal	= 48.
.globl	_signal, cerror

_signal:
	MCOUNT
	mov	r5,-(sp)
	mov	sp,r5
	mov	4(r5),r1
	cmp	r1,$NSIG
	bhis	2f		/ skip if arg1 too big
	mov	6(r5),r0
	mov	r1,0f
	asl	r1
	mov	dvect(r1),-(sp)
	mov	r0,dvect(r1)
	mov	r0,0f+2
	beq	1f
	bit	$1,r0
	bne	1f
	asl	r1
	add	$tvect,r1
	mov	r1,0f+2
1:
	sys	0; 9f
	bes	3f
	bit	$1,r0
	beq	1f
	mov	r0,(sp)
1:
	mov	(sp)+,r0
	mov	(sp)+,r5
	rts	pc
2:
	mov	$22.,r0		/ EINVAL for errno
3:
	jmp	cerror

NSIG = 0
tvect:
	jsr	r0,1f; NSIG=NSIG+1	/ 1
	jsr	r0,1f; NSIG=NSIG+1	/ 2
	jsr	r0,1f; NSIG=NSIG+1	/ 3
	jsr	r0,1f; NSIG=NSIG+1	/ 4
	jsr	r0,1f; NSIG=NSIG+1	/ 5
	jsr	r0,1f; NSIG=NSIG+1	/ 6
	jsr	r0,1f; NSIG=NSIG+1	/ 7
	jsr	r0,1f; NSIG=NSIG+1	/ 8
	jsr	r0,1f; NSIG=NSIG+1	/ 9
	jsr	r0,1f; NSIG=NSIG+1	/ 10
	jsr	r0,1f; NSIG=NSIG+1	/ 11
	jsr	r0,1f; NSIG=NSIG+1	/ 12
	jsr	r0,1f; NSIG=NSIG+1	/ 13
	jsr	r0,1f; NSIG=NSIG+1	/ 14
	jsr	r0,1f; NSIG=NSIG+1	/ 15
	jsr	r0,1f; NSIG=NSIG+1	/ 16
	jsr	r0,1f; NSIG=NSIG+1	/ 17
	jsr	r0,1f; NSIG=NSIG+1	/ 18
	jsr	r0,1f; NSIG=NSIG+1	/ 19
	jsr	r0,1f; NSIG=NSIG+1	/ 20

1:
	mov	r1,-(sp)
	mov	r2,-(sp)
	mov	r3,-(sp)
	mov	r4,-(sp)
	sub	$tvect+4,r0
	asr	r0
	mov	r0,-(sp)
	asr	(sp)
	jsr	pc,*dvect(r0)
	tst	(sp)+
	mov	(sp)+,r4
	mov	(sp)+,r3
	mov	(sp)+,r2
	mov	(sp)+,r1
	mov	(sp)+,r0
	rtt
.data
9:
	sys	.signal; 0:..; ..
.bss
dvect:	.=.+[NSIG*2]
