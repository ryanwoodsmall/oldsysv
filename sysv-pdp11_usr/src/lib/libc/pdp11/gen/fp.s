/	@(#)fp.s	1.2
/ fp1 -- floating point simulator

rti	= 2
bpt	= 3

m.ext = 200		/ long mode bit
m.lngi = 100		/ long integer mode

.globl	fptrap
.globl	ac0, ac1, ac2, ac3

fptrap:
	dec	reenter
	bge	1f
	4		/ reentered!
1:
	mov	(sp)+,spc
	mov	(sp)+,sps
	mov	r0,sr0
	mov	$sr1,r0
	mov	r1,(r0)+
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	r5,(r0)+
	mov	sp,(r0)+
	mov	(r0),r5		/ pc
	mov	-(r5),r5	/ trapped instruction

again:
	sub	$8,sp		/ room for double push
	clr	trapins
	mov	r5,r4
	bic	$7777,r4
	cmp	r4,$170000
	beq	1f
	jmp	badins
1:
	bic	$100000,fpsr	/ clear fp error
	bic	$170000,r5
	mov	r5,r4
	bit	$7000,r4
	bne	class3
	bit	$700,r4
	bne	class2
	cmp	r4,$12
	blos	1f
	jmp	badins
1:
	asl	r4
	jmp	*1f(r4)

	.data
1:
	i.cfcc		/ 170000
	i.setf		/ 170001
	i.seti		/ 170002
	badins
	badins
	badins
	badins
	badins
	badins
	i.setd		/ 170011
	i.setl		/ 170012
	.text

class2:
	cmp	r5,$400
	bge	1f
	jsr	r1,fsrc; mod0rx; mod242
	br	2f
1:
	jsr	r1,fsrc; mod0f; mod24f
2:
	mov	r3,r5
	asl	r4
	asl	r4
	clrb	r4
	swab	r4
	asl	r4
	jsr	pc,*1f(r4)
	jmp	sret

	.data
1:
	badins		/ 1700xx
	i.ldfps		/ 1701xx
	i.stfps		/ 1702xx
	badins		/ 1703xx - stst
	i.clrx		/ 1704xx
	i.tstx		/ 1705xx
	i.absx		/ 1706xx
	i.negx		/ 1707xx
	.text

class3:
	cmp	r5,$5000
	blt	1f
	mov	r5,r2
	clrb	r2
	cmp	r2,$6400
	blt	2f
	sub	$1400,r2
2:
	cmp	r2,$5000
	bne	2f
	jsr	r1,fsrc; mod0rx; mod242
	br	3f
2:
	cmp	r2,$5400
	bne	2f
	jsr	r1,fsrc; mod0ra; mod24i
	br	3f
2:
	jsr	r1,fsrc; mod0f; mod24d
	br	3f
1:
	jsr	r1,fsrc; mod0f; mod24f
3:
	jsr	pc,freg
	mov	r2,r5
	clrb	r4
	swab	r4
	asl	r4
	jsr	pc,*1f(r4)
	br	sret

	.data
1:
	badins		/ 1700xx
	badins		/ 1704xx
	i.mulx		/ 1710xx
	i.modx		/ 1714xx
	i.addx		/ 1720xx
	i.ldx		/ 1724xx
	i.subx		/ 1730xx
	i.cmpx		/ 1734xx
	i.stx		/ 1740xx
	i.divx		/ 1744xx
	i.stexp		/ 1750xx
	i.stcxj		/ 1754xx
	i.stcxy		/ 1760xx
	i.ldexp		/ 1764xx
	i.ldcjx		/ 1770xx
	i.ldcyx		/ 1774xx
	.text

i.cfcc:
	mov	fpsr,r0
	bic	$!17,r0
	mov	r0,sps
	br	ret

i.setf:
	bic	$m.ext,fpsr
	br	ret

i.setd:
	bis	$m.ext,fpsr
	br	ret

i.seti:
	bic	$m.lngi,fpsr
	br	ret

i.setl:
	bis	$m.lngi,fpsr
	br	ret

badins:
	inc	trapins
	br	ret1

sret:
	mov	$fpsr,r0
	bic	$17,(r0)
	tstb	1(r5)
	bpl	1f
	bis	$10,(r0)
	br	ret
1:
	bne	ret
	bis	$4,(r0)

ret:
	mov	ssp,sp
	mov	*spc,r5
	cmp	r5,$170000
	blo	ret1
	add	$2,spc
	jbr	again			/ if another fp, save trap

ret1:
	mov	$sr1,r0
	mov	(r0)+,r1
	mov	(r0)+,r2
	mov	(r0)+,r3
	mov	(r0)+,r4
	mov	(r0)+,r5
	mov	(r0)+,sp
	mov	sr0,r0
	mov	sps,-(sp)
	mov	spc,-(sp)
	tst	trapins
	bne	1f
	inc	reenter
	rti
1:
	bpt

freg:
	mov	r5,r2
	bic	$!300,r2
	asr	r2
	asr	r2
	asr	r2
	add	$ac0,r2
	rts	pc

fsrc:
	mov	r5,r3
	bic	$!7,r3			/ register
	asl	r3
	add	$sr0,r3
	mov	r5,r0
	bic	$!70,r0			/ mode
	asr	r0
	asr	r0
	jmp	*1f(r0)

	.data
1:
	mod0
	mod1
	mod2
	mod3
	mod4
	mod5
	mod6
	mod7
	.text

mod24f:
	mov	$4,r0
	bit	$m.ext,fpsr
	beq	1f
	add	$4,r0
1:
	rts	pc

mod24d:
	mov	$8,r0
	bit	$m.ext,fpsr
	beq	1f
	sub	$4,r0
1:
	rts	pc

mod242:
	mov	$2,r0
	rts	pc

mod24i:
	mov	$2,r0
	bit	$m.lngi,fpsr
	beq	1f
	add	$2,r0
1:
	rts	pc

mod0:
	jmp	*(r1)+

mod0f:
	sub	$sr0,r3			/ get fp ac
	cmp	r3,$6*2
	bhis	badi1
	asl	r3
	asl	r3
	add	$ac0,r3
	tst	(r1)+
	rts	r1

mod0ra:
	bit	$m.lngi,fpsr
	bne	badi1

mod0r:
	cmp	r3,$ssp
	bhis	badi1
mod0rx:
	tst	(r1)+
	rts	r1

mod1:
	cmp	r3,$spc
	beq	badi1
	mov	(r3),r3
	br	check

mod2:
	mov	(r3),-(sp)
	jsr	pc,*2(r1)
	cmp	r3,$spc
	bne	1f
	mov	$2,r0
	mov	*(r3),pctmp
	mov	$pctmp,(sp)
1:
	add	r0,(r3)
	mov	(sp)+,r3
	br	check

mod3:
	mov	*(r3),-(sp)
	add	$2,(r3)
	mov	(sp)+,r3
	br	check

mod4:
	cmp	r3,$spc		/ test pc
	beq	badi1
	jsr	pc,*2(r1)
	sub	r0,(r3)
	mov	(r3),r3
	br	check

mod5:
	cmp	r3,$spc
	beq	badi1
	sub	$2,(r3)
	mov	*(r3),r3
	br	check

mod6:
	mov	*spc,-(sp)
	add	$2,spc
	add	(r3),(sp)
	mov	(sp)+,r3
	br	check

mod7:
	jsr	r1,mod6; ..; ..
	mov	(r3),r3
	br	check

badi1:
	jmp	badins

check:
	bit	$1,r3
	bne	badi1
	cmp	(r1)+,(r1)+
	rts	r1

setab:
	mov	$asign,r0
	jsr	pc,seta
	mov	r3,r2
	mov	$bsign,r0

seta:
	clr	(r0)
	mov	(r2)+,r1
	mov	r1,-(sp)
	beq	1f
	blt	2f
	inc	(r0)+
	br	3f
2:
	dec	(r0)+
3:
	bic	$!177,r1
	bis	$200,r1
	br	2f
1:
	clr	(r0)+
2:
	mov	r1,(r0)+
	mov	(r2)+,(r0)+
	bit	$m.ext,fpsr
	beq	2f
	mov	(r2)+,(r0)+
	mov	(r2)+,(r0)+
	br	3f
2:
	clr	(r0)+
	clr	(r0)+
3:
	mov	(sp)+,r1
	asl	r1
	clrb	r1
	swab	r1
	sub	$200,r1
	mov	r1,(r0)+	/ exp
	rts	pc

norm:
	mov	$areg,r0
	mov	(r0)+,r1
	mov	r1,-(sp)
	mov	(r0)+,r2
	bis	r2,(sp)
	mov	(r0)+,r3
	bis	r3,(sp)
	mov	(r0)+,r4
	bis	r4,(sp)+
	bne	1f
	clr	asign
	rts	pc
1:
	bit	$!377,r1
	beq	1f
	clc
	ror	r1
	ror	r2
	ror	r3
	ror	r4
	inc	(r0)
	br	1b
1:
	bit	$200,r1
	bne	1f
	asl	r4
	rol	r3
	rol	r2
	rol	r1
	dec	(r0)
	br	1b
1:
	mov	r4,-(r0)
	mov	r3,-(r0)
	mov	r2,-(r0)
	mov	r1,-(r0)
	rts	pc

/ 
/ fp2 -- floating point simulation

i.ldx:
	mov	(r3)+,(r2)+
	mov	(r3)+,(r2)+
	bit	$m.ext,fpsr
	beq	1f
	mov	(r3)+,(r2)+
	mov	(r3)+,(r2)+
	rts	pc
1:
	clr	(r2)+
	clr	(r2)+
	rts	pc

i.stx:
	mov	(r2)+,(r3)+
	mov	(r2)+,(r3)+
	bit	$m.ext,fpsr
	beq	1f
	mov	(r2)+,(r3)+
	mov	(r2)+,(r3)+
1:
	jmp	ret			/ does not set cc's

i.clrx:
	clr	(r3)+
	clr	(r3)+
	bit	$m.ext,fpsr
	beq	1f
	clr	(r3)+
	clr	(r3)+
1:
	rts	pc

i.negx:
	tst	(r3)
	beq	1f
	add	$100000,(r3)
1:
	rts	pc

i.absx:
	bic	$!77777,(r3)
	rts	pc

i.tstx:
	rts	pc

i.cmpx:
	mov	$areg,r5
	tst	(r2)
	bge	1f
	tst	(r3)
	bge	1f
	cmp	(r2),(r3)
	bgt	4f
	blt	3f
1:
	cmp	(r2)+,(r3)+
	bgt	3f
	blt	4f
	cmp	(r2)+,(r3)+
	bne	1f
	bit	$m.ext,fpsr
	beq	2f
	cmp	(r2)+,(r3)+
	bne	1f
	cmp	(r2)+,(r3)+
	beq	2f
1:
	bhi	3f
4:
	movb	$1,1(r5)
	rts	pc
3:
	mov	$-1,(r5)
	rts	pc
2:
	clr	(r5)
	rts	pc

i.ldcyx:
	mov	(r3)+,(r2)+
	mov	(r3)+,(r2)+
	bit	$m.ext,fpsr
	bne	1f
	mov	(r3)+,(r2)+
	mov	(r3)+,(r2)+
	rts	pc
1:
	clr	(r2)+
	clr	(r2)+
	rts	pc

i.stcxy:
	mov	(r2)+,(r3)+
	mov	(r2)+,(r3)+
	bit	$m.ext,fpsr
	bne	1f
	clr	(r3)+
	clr	(r3)+
1:
	rts	pc

i.ldcjx:
	mov	$asign,r0
	mov	$1,(r0)+
	mov	(r3)+,(r0)+
	bit	$m.lngi,fpsr
	beq	1f
	mov	(r3)+,(r0)+
	clr	(r0)+
	clr	(r0)+
	mov	$32.-8,(r0)+
	jmp	saret
1:
	clr	(r0)+
	clr	(r0)+
	clr	(r0)+
	mov	$16.-8,(r0)
	jmp	saret

i.stcxj:
	mov	r3,r5
	mov	$asign,r0
	jsr	pc,seta
	clr	r4
	mov	$areg,r0
	mov	(r0)+,r1
	mov	(r0)+,r2
	mov	(r0)+,r3
	mov	aexp,r0
1:
	cmp	r0,$48.-8
	bge	1f
	clc
	ror	r1
	ror	r2
	ror	r3
	inc	r0
	br	1b
1:
	bgt	7f
	tst	r1
	beq	1f
7:
	bis	$1,r4			/ C-bit
1:
	bit	$m.lngi,fpsr
	beq	1f
	tst	asign
	bge	2f
	neg	r3
	adc	r2
	bcs	2f
	neg	r2
	bis	$10,r4			/ N-bit
2:
	mov	r2,(r5)
	mov	r3,2(r5)
	bis	r2,r3
	br	8f
1:
	tst	r2
	beq	1f
	bis	$1,r4			/ C-bit
1:
	tst	asign
	bge	2f
	neg	r3
	bis	$10,r4			/ N-bit
2:
	mov	r3,(r5)
8:
	bne	1f
	bis	$4,r4			/ Z-bit
1:
	bic	$17,sps
	bic	$17,fpsr
	bis	r4,sps
	bis	r4,fpsr
	jmp	ret

xoflo:
	bis	$1,fpsr			/ set fixed overflow (carry)
	jmp	ret

i.ldexp:
	mov	$asign,r0
	jsr	pc,seta
	mov	(r3),aexp
	jsr	pc,reta
	jmp	sret

i.stexp:
	mov	$asign,r0
	jsr	pc,seta
	mov	aexp,(r3)
	mov	r3,r5
	bic	$17,sps
	tst	(r3)
	bmi	1f
	bne	2f
	bis	$4,sps			/ Z-bit
	br	2f
1:
	bis	$10,sps			/ N-bit
2:
	jmp	sret

i.ldfps:
	mov	(r3),fpsr
	jmp	ret

i.stfps:
	mov	fpsr,(r3)
	jmp	ret

/ 
/ fp3 -- floating simulation

i.addx:
	jsr	pc,setab
	br	1f

i.subx:
	jsr	pc,setab
	neg	bsign
1:
	tst	bsign
	beq	reta
	tst	asign
	beq	retb
	mov	areg+8,r1
	sub	breg+8,r1
	blt	1f
	beq	2f
	cmp	r1,$56.
	bge	reta
	mov	$breg,r0
	br	4f
1:
	neg	r1
	cmp	r1,$56.
	bge	retb
	mov	$areg,r0
4:
	mov	r1,-(sp)
	mov	(r0)+,r1
	mov	(r0)+,r2
	mov	(r0)+,r3
	mov	(r0)+,r4
	add	(sp),(r0)
1:
	clc
	ror	r1
	ror	r2
	ror	r3
	ror	r4
	dec	(sp)
	bgt	1b
	mov	r4,-(r0)
	mov	r3,-(r0)
	mov	r2,-(r0)
	mov	r1,-(r0)
	tst	(sp)+
2:
	mov	$areg+8,r1
	mov	$breg+8,r2
	mov	$4,r0
	cmp	asign,bsign
	bne	4f
	clc
1:
	adc	-(r1)
	bcs	3f
	add	-(r2),(r1)
2:
	dec	r0
	bne	1b
	br	5f
3:
	add	-(r2),(r1)
	sec
	br	2b
	br	5f
4:
	clc
1:
	sbc	-(r1)
	bcs	3f
	sub	-(r2),(r1)
2:
	dec	r0
	bne	1b
	br	5f
3:
	sub	-(r2),(r1)
	sec
	br	2b

saret:
	mov	$areg,r1
5:
	tst	(r1)
	bge	3f
	mov	$areg+8,r1
	mov	$4,r0
	clc
1:
	adc	-(r1)
	bcs	2f
	neg	(r1)
2:
	dec	r0
	bne	1b
	neg	-(r1)
3:
	jsr	pc,norm
	br	reta

retb:
	mov	$bsign,r1
	mov	$asign,r2
	mov	$6,r0
1:
	mov	(r1)+,(r2)+
	dec	r0
	bne	1b

reta:
	mov	r5,r2
	mov	$asign,r0
	tst	(r0)
	beq	unflo
	mov	aexp,r1
	cmp	r1,$177
	bgt	ovflo
	cmp	r1,$-177
	blt	unflo
	add	$200,r1
	swab	r1
	clc
	ror	r1
	tst	(r0)+
	bge	1f
	bis	$100000,r1
1:
	bic	$!177,(r0)
	bis	(r0)+,r1
	mov	r1,(r2)+
	mov	(r0)+,(r2)+
	bit	$m.ext,fpsr
	beq	1f
	mov	(r0)+,(r2)+
	mov	(r0)+,(r2)+
1:
	rts	pc

unflo:
	clr	(r2)+
	clr	(r2)+
	bit	$m.ext,fpsr
	beq	1f
	clr	(r2)+
	clr	(r2)+
1:
	rts	pc

ovflo:
	bis	$2,fpsr			/ set v-bit (overflow)
	jmp	ret

i.mulx:
	jsr	pc,i.mul
	br	saret

i.modx:
	jsr	pc,i.mul
	jsr	pc,norm
	mov	$asign,r0
	mov	$bsign,r1
	mov	$6,r2
1:
	mov	(r0)+,(r1)+
	dec	r2
	bne	1b
	clr	r0		/ count
	mov	$200,r1		/ bit
	clr	r2		/ reg offset
1:
	cmp	r0,aexp
	bge	2f		/ in fraction
	bic	r1,areg(r2)
	br	3f
2:
	bic	r1,breg(r2)
3:
	inc	r0
	clc
	ror	r1
	bne	1b
	mov	$100000,r1
	add	$2,r2
	cmp	r2,$8
	blt	1b
	jsr	pc,norm
	jsr	pc,reta
	cmp	r5,$ac1
	beq	1f
	cmp	r5,$ac3
	beq	1f
	bit	$200,breg
	bne	2f
	clr	bsign
2:
	add	$8,r5
	jsr	pc,retb
	sub	$8,r5
1:
	rts	pc

i.divx:
	jsr	pc,setab
	tst	bsign
	beq	ovflo
	sub	bexp,aexp
	jsr	pc,xorsign
	mov	r5,-(sp)
	mov	$areg,r0
	mov	(r0),r1
	clr	(r0)+
	mov	(r0),r2
	clr	(r0)+
	mov	(r0),r3
	clr	(r0)+
	mov	(r0),r4
	clr	(r0)+
	mov	$areg,r5
	mov	$400,-(sp)
1:
	mov	$breg,r0
	cmp	(r0)+,r1
	blt	2f
	bgt	3f
	cmp	(r0)+,r2
	blo	2f
	bhi	3f
	cmp	(r0)+,r3
	blo	2f
	bhi	3f
	cmp	(r0)+,r4
	bhi	3f
2:
	mov	$breg,r0
	sub	(r0)+,r1
	clr	-(sp)
	sub	(r0)+,r2
	adc	(sp)
	clr	-(sp)
	sub	(r0)+,r3
	adc	(sp)
	sub	(r0)+,r4
	sbc	r3
	adc	(sp)
	sub	(sp)+,r2
	adc	(sp)
	sub	(sp)+,r1
	bis	(sp),(r5)
3:
	asl	r4
	rol	r3
	rol	r2
	rol	r1
	clc
	ror	(sp)
	bne	1b
	mov	$100000,(sp)
	add	$2,r5
	cmp	r5,$areg+8
	blo	1b
	tst	(sp)+
	mov	(sp)+,r5
	jmp	saret


i.mul:
	jsr	pc,setab
	add	bexp,aexp
	dec	aexp
	jsr	pc,xorsign
	mov	r5,-(sp)
	mov	$breg+4,r5
	bit	$m.ext,fpsr
	beq	1f
	add	$4,r5
1:
	clr	r0
	clr	r1
	clr	r2
	clr	r3
	clr	r4
1:
	asl	r0
	bne	2f
	inc	r0
	tst	-(r5)
2:
	cmp	r0,$400
	bne	2f
	cmp	r5,$breg
	bhi	2f
	mov	$areg,r0
	mov	r1,(r0)+
	mov	r2,(r0)+
	mov	r3,(r0)+
	mov	r4,(r0)+
	mov	(sp)+,r5
	rts	pc
2:
	clc
	ror	r1
	ror	r2
	ror	r3
	ror	r4
	bit	r0,(r5)
	beq	1b
	mov	r0,-(sp)
	mov	$areg,r0
	add	(r0)+,r1
	clr	-(sp)
	add	(r0)+,r2
	adc	(sp)
	clr	-(sp)
	add	(r0)+,r3
	adc	(sp)
	add	(r0)+,r4
	adc	r3
	adc	(sp)
	add	(sp)+,r2
	adc	(sp)
	add	(sp)+,r1
	mov	(sp)+,r0
	br	1b

xorsign:
	cmp	asign,bsign
	beq	1f
	mov	$-1,asign
	rts	pc
1:
	mov	$1,asign
	rts	pc

/ 
/ fpx -- floating point simulation

.data
reenter: 1

.bss
asign:	.=.+2
areg:	.=.+8
aexp:	.=.+2
bsign:	.=.+2
breg:	.=.+8
bexp:	.=.+2

fpsr:	.=.+2
trapins: .=.+2

ac0:	.=.+8.
ac1:	.=.+8.
ac2:	.=.+8.
ac3:	.=.+8.
ac4:	.=.+8.
ac5:	.=.+8.

sr0:	.=.+2
sr1:	.=.+2
	.=.+2
	.=.+2
	.=.+2
	.=.+2
ssp:	.=.+2
spc:	.=.+2
sps:	.=.+2
pctmp:	.=.+8

