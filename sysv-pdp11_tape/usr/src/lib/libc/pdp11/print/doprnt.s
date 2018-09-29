/	@(#)doprnt.s	1.7
/ C library -- conversions
/
/ ** modified 12 Aug 80 to handle %0<width><conversion code>
/
/ ++ modified 2 Dec 80 to print null string for %#.0o with 0 value
/
/ -- modified 20 Dec 81 to ignore irrelevant "l" in format

width=-8.
formp=-10.
flags=-12.
ndfnd=-14.
ndigit=-16.
ostart=-18.
code=-20.
ptr=-22.
tlen=-24.
PLUS=1
MINUS=2
SHARP=4
BLANK=10
ISNEG=20
IZZERO=40	/**
UCASE=2000
LONG=4000
.globl	__doprnt

.globl	fltcvt

.globl	__strout
.globl	csav
.globl	cret

__doprnt:
	MCOUNT
	jsr	r0,csav
	sub	$128.+20.,sp
	clr	tlen(r5)		/ # chars generated
	mov	4(r5),formp(r5)		/ format
	mov	6(r5),r4
loop:
	mov	formp(r5),r3
	mov	r3,r2
	clr	flags(r5)
	clr	width(r5)
	clr	ndigit(r5)
2:
	movb	(r3)+,r0
	beq	2f
	cmp	r0,$'%
	bne	2b
2:
	dec	r3
	cmp	r3,r2
	beq	2f
	mov	r3,formp(r5)
	jbr	prstr
2:
	tst	r0
	bne	2f
	mov	tlen(r5),r0
	jmp	cret
2:
	inc	r3
	mov	r3,formp(r5)
	mov	sp,r3
	add	$4,r3
	mov	r3,ostart(r5)
2:
	movb	*formp(r5),r0
	cmpb	r0,$'-
	bne	4f
	bis	$MINUS,flags(r5)
	br	3f
4:
	cmp	r0,$'+
	bne	4f
	bis	$PLUS,flags(r5)
	br	3f
4:
	cmp	r0,$'   /space
	bne	4f
	bis	$BLANK,flags(r5)
	br	3f
4:
	cmp	r0,$'#
	bne	4f			/**
	bis	$SHARP,flags(r5)
	br	3f			/**
4:
	cmp	r0,$'0			/**
	bne	2f			/**
	bis	$IZZERO,flags(r5)	/**
3:
	inc	formp(r5)
	br	2b
2:
	jsr	pc,gnum
	mov	r1,width(r5)
	bpl	1f
	neg	width(r5)
	bis	$MINUS,flags(r5)
1:
	clr	ndfnd(r5)
	cmp	r0,$'.
	bne	1f
	inc	formp(r5)
	jsr	pc,gnum
	mov	r1,ndigit(r5)
1:
	bit	$IZZERO,flags(r5)	/**
	beq	1f			/**
	cmp	ndigit(r5),width(r5)	/**
	bge	1f			/**
	mov	width(r5),ndigit(r5)	/**
	inc	ndfnd(r5)		/** fake out compute
	br	1f
morecode:
	movb	*formp(r5),r0
1:
	inc	formp(r5)
	cmp	r0,$'A
	blt	1f
	cmp	r0,$'Z
	bge	1f
	bis	$UCASE,flags(r5)
	sub	$'A-'a,r0
1:
	mov	r0,code(r5)
/	bit	$LONG,flags(r5)		/ -- deleted
/	beq	1f			/ -- deleted
/	bis	$LONG,r0		/ -- deleted
/1:					/ -- deleted
	mov	$swtab,r1
1:
	mov	(r1)+,r2
	bne	2f
	movb	r0,(r3)+
	jmp	prbuf
2:
	cmp	r0,(r1)+
	bne	1b
	jmp	(r2)
	.data
swtab:
	decimal;	'd
	octal;		'o
	hex;		'x
	float;		'f
	scien;		'e
	general;	'g
	charac;		'c
	string;		's
	makelong;	'l
	unsigned;	'u
	remote;		'r
/	long;		'd+LONG		/ -- deleted
/	loct;		'o+LONG		/ -- deleted
/	lhex;		'x+LONG		/ -- deleted
/	lunsigned;	'u+LONG		/ -- deleted
	morecode;	'h
	0
	.text

makelong:
	bis	$LONG,flags(r5)
	br	morecode

octal:
	bit	$LONG+UCASE,flags(r5)	/ -- changed
	bne	loct
	clr	r0
	br	1f
loct:
	mov	(r4)+,r0
1:
	mov	$8.,r2
	br	2f

hex:
	bit	$LONG,flags(r5)		/ -- added
	bne	lhex			/ -- added
	clr	r0
	br	1f

lhex:
	mov	(r4)+,r0
1:
	mov	$16.,r2
2:
	mov	(r4)+,r1
	br	compute

decimal:
	bit	$LONG+UCASE,flags(r5)	/ -- changed
	bne	long
	mov	(r4)+,r1
	sxt	r0
	bmi	3f
	br	2f

unsigned:
	bit	$LONG,flags(r5)		/ -- added
	bne	lunsigned		/ -- added
	clr	r0
	br	1f

long:
	mov	(r4)+,r0
	bge	1f
	mov	(r4)+,r1
3:
	neg	r0
	neg	r1
	sbc	r0
	bis	$ISNEG,flags(r5)
	br	2f

lunsigned:
	mov	(r4)+,r0
1:
	mov	(r4)+,r1
2:
	mov	$10.,r2

/
/
compute:
	mov	r4,-(sp)
	tst	ndfnd(r5)
	bne	1f
	mov	$1,ndigit(r5)
1:
	clrb	(r3)
	mov	r3,ptr(r5)
	mov	r2,r4
	mov	r0,r2
	mov	r1,r3
	jsr	pc,convertit
	mov	(sp)+,r4
	mov	ptr(r5),r3
	mov	ostart(r5),r2
	bit	$SHARP,flags(r5)
	jeq	prsign
	cmp	code(r5),$'o
	bne	2f
	cmpb	(r2),$'0		/ leading 0 on octal
	jeq	prsign
	tstb	(r2)			/ ++ no output?
	jeq	prsign			/ ++ skip zero printing
	jbr	przero
2:
	cmp	code(r5),$'x
	jne	prsign
	tstb	(r2)
	beq	prsign
	movb	$'x,-(r2)
	bit	$UCASE,flags(r5)
	beq	przero
	movb	$'X,(r2)
przero:
	movb	$'0,-(r2)
prsign:
	bit	$ISNEG,flags(r5)
	jeq	1f
	movb	$'-,-(r2)
	jbr	prstr
1:
	cmp	code(r5),$'d
	jne	prstr
	bit	$PLUS,flags(r5)
	beq	1f
	movb	$'+,-(r2)
	jbr	prstr
1:
	bit	$BLANK,flags(r5)
	jeq	prstr
	movb	$' ,-(r2)
	jbr	prstr

convertit:
	dec	ndigit(r5)
	bge	1f
	ashc	$0,r2
	bne	1f
	rts	pc
1:
	clr	r0
	mov	r2,r1
	beq	2f
	div	r4,r0
	mov	r0,r2
	mov	r1,r0
2:
	mov	r3,r1
	asl	r4
	div	r4,r0
	asr	r4
	asl	r0
	cmp	r4,r1
	bgt	2f
	sub	r4,r1
	inc	r0
2:
	mov	r1,-(sp)
	mov	r0,r3
	jsr	pc,convertit
1:
	mov	(sp)+,r0
	add	$'0,r0
	cmp	r0,$'9
	ble	1f
	add	$'a-'0-10.,r0
	bit	$UCASE,flags(r5)
	beq	1f
	add	$'A-'a,r0
1:
	movb	r0,*ptr(r5)
	inc	ptr(r5)
	rts	pc
	
charac:
	mov	(r4)+,r0
	movb	r0,(r3)+
	br	prbuf


string:
	mov	ndigit(r5),r1
	mov	(r4)+,r2
	bne	1f
	mov	$nulstr,r2
1:
	mov	r2,r3
1:
	tstb	(r3)+
	beq	1f
	sob	r1,1b
	inc	r3
1:
	dec	r3
	br	prstr

float:
scien:
general:
	mov	ndigit(r5),r0
	tst	ndfnd(r5)
	bne	1f
	mov	$6,r0
1:
	mov	flags(r5),r1
	mov	code(r5),r2
	jsr	pc,fltcvt
	mov	r0,r3
	br	prbuf

remote:
	mov	(r4)+,r4
	mov	(r4)+,formp(r5)
	jmp	loop

prbuf:
	mov	ostart(r5),r2
prstr:
	sub	r2,r3
	mov	width(r5),r1
	sub	r3,r1
	bge	1f
	clr	r1
1:
	bit	$MINUS,flags(r5)
	beq	1f
	add	r1,tlen(r5)
	add	r1,tlen(r5)
	neg	r1
1:
	add	r1,tlen(r5)
	mov	8(r5),-(sp)
	mov	r1,-(sp)
	mov	r3,-(sp)
	add	r3,tlen(r5)
	mov	r2,-(sp)
	jsr	pc,__strout
	add	$8.,sp
	jmp	loop

gnum:
	clr	ndfnd(r5)
	clr	r1
1:
	movb	*formp(r5),r0
	sub	$'0,r0
	cmp	r0,$'*-'0
	bne	2f
	mov	(r4)+,r0
	br	3f
2:
	cmp	r0,$9.
	bhi	1f
3:
	inc	ndfnd(r5)
	mul	$10.,r1
	add	r0,r1
	inc	formp(r5)
	br	1b
1:
	add	$'0,r0
	rts	pc

.data
nulstr:
	<(null)\0>
