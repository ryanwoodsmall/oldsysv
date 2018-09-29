	.file	"drand48.s"
#	@(#)drand48.s	2.4
#    *** Version for VAX-11/780 & VAX-11/750 ***
# Modulo-48 linear congruential pseudo-random number generator
#    X(i+1) = [a*X(i) + c] mod(48)
# See TM82-11353-1 by C. S. Roberts (which obsoletes TM79-1353-5)
# "Implementing and Testing New Versions of a Good 48-bit
#    Pseudo-random Number Generator"
	.set	l1,-4
	.set	l0,-8
	.set	k1,-12
	.set	k0,-16
	.data
	.align	1
.lastx:	.space	8
.x:	.word	0x330e,0xabcd,0x1234
.xa:	.word	0163155,0157354,5
.xc:	.word	013,0
.a:	.word	0163155,0157354,5	# the 48-bit number 0273673163155 = 0x5DEECE66D
.c:	.word	013
	.text
	.align	1
# Version to return a double floating-point fraction
# Example usage from C:
#	double fnext,drand48();
#	fnext=drand48();
# the value range for fnext is: 0.0<= fnext <1.0
	.globl	_drand48
_drand48:	.word	0100
	MCOUNT
	subl2	$16,sp
	bsbb	.clc48x
.L0:	movzwl	$0x4400,r2
	clrl	r3
	rotl	$16,(r6),r1
	ashl	$16,4(r6),r0
	bisl2	r2,r0
	subd2	r2,r0
	ret
# Call analogous to drand48 but capable of generating independent streams of
# pseudo-random numbers.  Storage for the 48-bit X(i) provided by the caller.
# Example usage from C:
#	double fnext, erand48();
#	short xsubi[3];
#	fnext = erand48(xsubi);
	.globl	_erand48
_erand48:	.word 0100
	MCOUNT
	subl2	$16,sp
	movl	4(ap),r6
	pushab	.L0
	brb	.clc48
#<*><*><*><*>#
.clc48x:	movaw	.x,r6
.clc48:	moval	k0(fp),r1
	movl	(r6)+,(r1)+
	clrb	-1(r1)
	decl	r6
	movl	(r6)+,(r1)+
	clrb	-1(r1)
	movl	.xa,(r1)+
	clrb	-1(r1)
	movl	.xa+3,(r1)+
	clrb	-1(r1)

	emul	k0(fp),l0(fp),.xc,r0
	movl	r0,-7(r6)
	ashq	$-24,r0,r2
	emul	k0(fp),l1(fp),r2,r0
	emul	k1(fp),l0(fp),r0,r0
	movw	r0,-4(r6)
	ashl	$-16,r0,r0
	movb	r0,-2(r6)
	subl2	$7,r6		# restore r6 to its value at .clc48:
	rsb
# Version to return a 31-bit positive integer
# Example usage from C:
#	long int lnext,lrand48();
#	lnext=lrand48();
# the value range for lnext is: 0<= lnext <2^31
	.globl	_lrand48
_lrand48:	.word	0100
	MCOUNT
	subl2	$16,sp
	bsbb	.clc48x
.L2:	ashl	$-1,2(r6),r0
	bbcc	$31,r0,.lrret
.lrret:	ret
# Version to return a 32-bit positive or negative integer
# Example usage from C:
#	long int mnext,mrand48();
#	mnext=mrand48();
# the value range for mnext is: -2^31<= mnext <2^31
	.globl	_mrand48
_mrand48:	.word	0100
	MCOUNT
	subl2	$16,sp
	bsbw	.clc48x
.L3:	movl	2(r6),r0
	ret
# Calls analogous to lrand48 & mrand48 but capable of generating independent
#  streams of pseudo-random numbers.
# Storage for the 48-bit X(i) provided by the caller
# Example usage from C:
#	long int lnext,nrand48();
#	long int mnext,jrand48();
#	short xsubi[3];
#	lnext = nrand48(xsubi);
#	mnext = jrand48(xsubi);
	.globl	_nrand48
_nrand48:	.word	0100
	MCOUNT
	subl2	$16,sp
	movl	4(ap),r6	# pick up pointer to current 48-bit X(i)
	pushab	.L2
	brw	.clc48
#<*><*><*><*>#
	.globl	_jrand48
_jrand48:	.word	0100
	MCOUNT
	subl2	$16,sp
	movl	4(ap),r6
	pushab	.L3
	brw	.clc48

# Routine to seed with a string of 32 bits contained in a long int
# Useage from C:
#	long int iiseed;
#	srand48(iiseed);
	.globl	_srand48
_srand48:	.word	0
	MCOUNT
	movl	4(ap),.x+2
	movw	$031416,.x	# pattern for low-order bits = 0x330E
	movq	.a,.xa
	ret
# Routine to seed with an arbitrary string of 48 bits
# Example usage from C:
#	short rseed[3], *shp, *seed48();
#	seed48(rseed);	/* one mode of use -- returned pointer value ignored */
#	shp=seed48(rseed);  /* other mode -- acquire pointer to previous X(i) */
	.globl	_seed48
_seed48:	.word	0
	MCOUNT
	movq	.x,.lastx		# save last value of the 48-bit X(i)
	movl	4(ap),r0
	movl	(r0)+,.x
	movw	(r0),.x+4
	movq	.a,.xa
	moval	.lastx,r0	# return pointer to last 48-bit X(i)
	ret
# Routine to create a modulo 48, linear congruential pseudo-random number
#   generator with arbitrary multiplier, a, and addend, c.
# Example usage from C:
#	short param48[7];
#	lcong48(param48);
# 48-bit seed value contained in param48[0] - param48[2]
# 48-bit multiplier value, a, contained in param48[3] - param48[5]
# 16-bit addend value, c, contained in param48[6]
# After the call to lcong48, use the normal calls to drand48(),
#   lrand48(), or mrand48() to generate the next psuedo-random
#   number in the format desired
	.globl	_lcong48
_lcong48:	.word	0
	MCOUNT
	movl	4(ap),r0
	movaw	.x,r1
	movq	(r0)+,(r1)+
	movl	(r0)+,(r1)+
	movw	(r0),(r1)
	ret
