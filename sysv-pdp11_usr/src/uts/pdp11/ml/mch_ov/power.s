/ @(#)power.s	1.1
.text
powtrap:
	inc	_pwr_cnt
	tst	power
	jne	powbad
	mov	$-1,power
	mov	$-1,_pwr_act
	mov	$PRUSER+HIPRI,PS
	mov	sp,pfksp
	mov	(sp),pfksp+2
	mov	2(sp),pfksp+4
	mov	r0,pfreg
	mov	$pfreg+2,r0
	mov	r1,(r0)+
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mfpd	sp
	mov	(sp)+,(r0)+

	mov	$PRSUPR+ALTREG+HIPRI,PS
	mov	r0,pfareg
	mov	$pfareg+2,r0
	mov	r1,(r0)+
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mfpd	sp
	mov	(sp)+,(r0)+

	mov	$HIPRI,PS
	mov	$pffreg,-(sp)
	jsr	pc,_savfp
	tst	(sp)+
	mov	$pfmreg,r0
	mov	$SISD0,r1
	mov	$64.,r2
1:
	mov	(r1)+,(r0)+
	sob	r2,1b
	mov	$UISD0,r1
	mov	$32.,r2
1:
	mov	(r1)+,(r0)+
	sob	r2,1b
	cmp	$45.,_cputype
	beq	2f
	mov	$pfsreg,r0
	mov	MSCR,(r0)+
	mov	$UBMAP,r1
	mov	$31.,r2
1:
	mov	(r1)+,(r0)+
	mov	(r1)+,(r0)+
	sob	r2,1b
2:
	mov	$pfcreg,r0
	mov	PIR,(r0)+
	mov	SL,(r0)+
	mov	SSR3,(r0)+
	mov	SSR0+[2*2],(r0)+
	mov	SSR0+[1*2],(r0)+
	mov	SSR0,(r0)+
	neg	power
	clr	pfloop
	mov	$powup,24	/ to come back up
.globl	_time
	mov	_time,pftime
	mov	_time+2,pftime+2
1:
	inc	pfloop
	bne	1b

/ false power down
	mov	$-4,power
	halt

powbad:
	halt

.data
powup:
	mov	$-2,power
	mov	$pfmreg,r0
	mov	$SISD0,r1
	mov	$64.,r2
1:
	mov	(r0)+,(r1)+
	sob	r2,1b
	mov	$UISD0,r1
	mov	$32.,r2
1:
	mov	(r0)+,(r1)+
	sob	r2,1b
	cmp	$45.,_cputype
	beq	2f
	mov	$pfsreg,r0
	mov	(r0)+,MSCR
	mov	$UBMAP,r1
	mov	$31.,r2
1:
	mov	(r0)+,(r1)+
	mov	(r0)+,(r1)+
	sob	r2,1b
2:
	mov	pfksp,sp
	mov	$pfcreg,r0
	mov	(r0)+,PIR
	mov	(r0)+,SL
	mov	(r0)+,SSR3
	cmp	(r0)+,(r0)+
	bic	$1,(r0)		/ Clear enable bit
	mov	(r0)+,SSR0	/ Restore status bits
.globl	powmm, powback
	jmp	powmm		/ Go turn on enable bit and return here
.text
powback:
	mov	$trap24,24
	mov	$pffreg,-(sp)
	jsr	pc,_restfp
	tst	(sp)+
	mov	$PRSUPR+ALTREG+HIPRI,PS
	mov	$pfareg+14.,r0
	mov	-(r0),-(sp)
	mtpd	sp
	mov	-(r0),r5
	mov	-(r0),r4
	mov	-(r0),r3
	mov	-(r0),r2
	mov	-(r0),r1
	mov	-(r0),r0
	mov	$PRUSER+HIPRI,PS
	mov	$pfreg+14.,r0
	mov	-(r0),-(sp)
	mtpd	sp
	mov	-(r0),r5
	mov	-(r0),r4
	mov	-(r0),r3
	mov	-(r0),r2
	mov	-(r0),r1
	mov	-(r0),r0
	tst	pflock
	beq	1f

	mov	$pflock,r0
	clr	(r0)+
	mov	(r0)+,r2
	mov	(r0)+,r3
	mov	(r0)+,r4
	mov	(r0)+,r5
	mov	(r0)+,sp
	mov	(r0)+,2(sp)
	mov	(r0),(sp)
1:
/ Priority 1 request - power fail
	bis	$1000,PIR
	clr	power
	rtt

.globl	_pwrlock, _pwrunlock
_pwrlock:
	mov	PS,-(sp)
	mov	$HIPRI,PS
	mov	$pflock,r0
	tst	(r0)+
	bne	1f
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	(sp)+,(r0)+
	mov	(sp),(r0)
1:
	inc	pflock
	clr	r0
	rts	pc

_pwrunlock:
	mov	$pflock,r0
	dec	(r0)
	bne	1f
	mov	12.(r0),PS
1:
	rts	pc
