// _udvd(longdividend,intdivisor,adrintrem) --
//
// Perform unsigned divide of longdividend by intdivisor; return value
// is long quotient, int remainder is side-effected.
//
.globl	__udvd
__udvd: ld	r1,8(r15)	//get adr(intrem)
	clr	r2		//clr hi-order divisor
	ld	r3,6(r15)	//get lo-order divisor
	clr	r4		//clr hi-order dividend
	clr	r5		//clr hi-order dividend
	ldl	rr6,2(r15)	//pick up lo-order dividend
	divl	rq4,rr2		//quad-divide, quo->rr6
	ld	*r1,r5		//store int remainder
	ret			//and return
