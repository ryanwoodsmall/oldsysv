/	@(#)drand48.s	2.2
/*    *** Version for PDP-11/45, PDP-11/70 ***
/* Modulo-48 linear congruential pseudo-random number generator
/*    X(i+1) = [a*X(i) + c] mod(48)
/* See TM82-11353-1 by C. S. Roberts (which obsoletes TM79-1353-5)
/* "Implementing and Testing New Versions of a Good 48-bit
/*    Pseudo-random Number Generator"
/* routines drand48(), lrand48(), and mrand48() generate the next pseudo-random number
/* Version to return a double floating-point fraction
/* Useage from C:
/*      double xnext,drand48();
/*      xnext=drand48();
/* the value range for xnext is: 0.0<= xnext <1.0
	.globl	csav,cret,fltused
	.text
	.globl	_drand48
_drand48:
	MCOUNT
	jsr	r0,csav
	mov	$x,r0
	jsr	pc,lcong48
Ld:	mov	$042000,temp
	movf	temp,fr0
	subf	$42000,fr0	/* xnext returned in fr0
	jmp	cret

/* version to return a 31-bit positive integer
/* Useage from C:
/*	long int lnext,lrand48();
/*	lnext=lrand48();
/* the value range for lnext is 0<= lnext <2^31
	.globl	_lrand48
_lrand48:
	MCOUNT
	jsr	r0,csav
	mov	$x,r0
	jsr	pc,lcong48
Ll:	mov	4(r3),r0
	mov	2(r3),r1
	ashc	$-1,r0
	bic	$100000,r0
	jmp	cret

/* Version to return a 32-bit positive or negative integer
/* Example useage from C:
/*	long int mnext,mrand48();
/*	mnext=mrand48();
/* the value range for mnext is: -2^31<= mnext <2^31
	.globl	_mrand48
_mrand48:
	MCOUNT
	jsr	r0,csav
	mov	$x,r0
	jsr	pc,lcong48
Lm:	mov	4(r3),r0
	mov	2(r3),r1
	jmp	cret

/* Call analogous to drand48 but capable of generating independent streams of
/* pseudo-random numbers.  Storage for the 48-bit X(i) provided by the caller.
/* Example usage from C:
/*	double fnext, erand48();
/*	short xsubi[3];
/*	fnext = erand48(xsubi);
	.globl	_erand48
_erand48:
	MCOUNT
	jsr	r0,csav
	mov	4(r5),r0	/* pick up pointer to the storage for X(i)
	mov	$Ld,-(sp)
	jmp	lcong48

/* Calls analogous to lrand48 & mrand48 but capable of generating independent
/*  streams of pseudo-random numbers.
/* Storage for the 48-bit X(i) provided by the caller.
/* Example usage from C:
/*	long int lnext, nrand48();
/*	long int mnext, jrand48();
/*	short xsubi[3];
/*	lnext = nrand48(xsubi);
/*	mnext = jrand48(xsubi);
	.globl	_nrand48,_jrand48
_nrand48:
	MCOUNT
	jsr	r0,csav
	mov	4(r5),r0	/* pick up pointer to the storage for X(i)
	mov	$Ll,-(sp)
	jmp	lcong48
_jrand48:
	MCOUNT
	jsr	r0,csav
	mov	4(r5),r0
	mov	$Lm,-(sp)
	jmp	lcong48

	.data
clong:	0;0
a:	163155;157354;5 /* the 48-bit number 273673163155 (octal) = 5DEECE66D (hex)
c:	13
x:	031416;125715;011064
xa:	163155;157354;5
xc:	13
/* Routine to seed with a string of 48 bits
/* Useage from C:
/*	short rseed[3], *shp, *seed48();
/*	seed48(rseed);	/* one mode of use -- returned pointer value ignored */
/*	shp=seed48(rseed);  /* other mode -- acquire pointer to previous X(i) */
	.text
	.globl	_seed48
_seed48:
	MCOUNT
	mov	$x,r0
	mov	$lastx,r1
	mov	(r0)+,(r1)+	/* Save last value of the 48-bit X(i)
	mov	(r0)+,(r1)+
	mov	(r0)+,(r1)+
	mov	2(sp),r0 /* pick up address of rseed
	mov	(r0)+,x
	mov	(r0)+,x+2
	mov	(r0)+,x+4
giz:	mov	$a,r0
	mov	(r0)+,xa
	mov	(r0)+,xa+2
	mov	(r0)+,xa+4
	mov	(r0)+,xc
	mov	$lastx,r0	/* return pointer to last 48-bit X(i)
	rts	pc
/* Routine to seed with a string of 32-bits contained in a long int
/* Useage from C:
/*	long int iiseed;
/*	srand48(iiseed);
	.globl	_srand48
_srand48:
	MCOUNT
	mov	$31416,x /* pattern for low-order 16 bits = 330E (hex)
	mov	4(sp),x+2
	mov	2(sp),x+4
	br	giz
/* Routine to create a modulo 48, linear congruential pseudo-random number
/*   generator with arbitrary multiplier, a, and addend, c.
/* Example useage from C:
/*	short param48[7];
/*	lcong48(param48);
/* 48-bit seed value contained in param48[0] - param48[2]
/* 48-bit multiplier value, a, contained in param48[3] - param48[5]
/* 16-bit addend value, c, contained in param48[6]
/* After the call to lcong48, use the normal calls to drand48(),
/*   lrand48(), or mrand48() to generate the next pseudo-random
/*   number in the format desired.
	.globl	_lcong48
_lcong48:	
	MCOUNT
	mov	r2,temp
	mov	2(sp),r0	/*pick up address of param48
	mov	$x,r1
	mov	$7,r2
4:	mov	(r0)+,(r1)+
	sob	r2,4b
	mov	temp,r2
	rts	pc

	.bss
lastx:	.=.+6
k0:	.=.+4
k1:	.=.+4
l0:	.=.+4
l1:	.=.+4
temp:	.=.+10
	.text
lcong48:
	mov	$k0,r1
	mov	$4,r2
	mov	r0,r3	/* pointer to X(i) preserved in r3
1:	movb	2(r0),(r1)+
	clrb	(r1)+
	movb	(r0)+,(r1)+
	movb	(r0)+,(r1)+
	inc	r0
	cmp	$3,r2
	bne	2f
	mov	$xa,r0
2:	sob	r2,1b
	mov	(r0),clong+2	/* value of c

	setl
	movif	k0,fr2
	movif	l1,fr3
	mulf	fr2,fr3
	movif	l0,fr1
	movif	k1,fr2
	mulf	fr1,fr2
	addf	fr2,fr3
	addf	$56000,fr3
	movf	fr3,temp

	bic	$177400,temp+4 /* mask off low-order 24 bits of result
	movif	temp+4,fr3
	mulf	$46200,fr3 /* 1 x 2^24
	movif	k0,fr1
	movif	l0,fr2
	mulf	fr1,fr2
	addf	fr2,fr3
	movif	clong,fr1	/* value of c to floating
	addf	fr1,fr3
	addf	$56000,fr3
	movf	fr3,temp
	seti

	mov	$temp+10,r1
	mov	r3,r0
	mov	-(r1),(r0)+
	mov	-(r1),(r0)+
	mov	-(r1),(r0)+
	rts	pc
