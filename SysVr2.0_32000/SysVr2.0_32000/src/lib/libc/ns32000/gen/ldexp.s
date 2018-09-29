	.file	"ldexp.s"
#
#	double ldexp (value, exp)
#		double value#
#		int exp#
#
#	Ldexp returns value*2**exp, if that result is in range.
#	If underflow occurs, it returns zero.  If overflow occurs,
#	it returns a value of appropriate sign and largest
#	possible magnitude.  In case of either overflow or underflow,
#	the external int "errno" is set to ERANGE.  Note that errno is
#	not modified if no error occurs, so if you intend to test it
#	after you use ldexp, you had better set it to something
#	other than ERANGE first (zero is a reasonable value to use).
#
#	Constants
#
	.set	erange,34		# error number for range error

	.globl	_errno		# error flag

#
#	Entry point
#
	.globl	_ldexp
_ldexp:
#	MCOUNT
	movbl	0,f0		# f0 = 0.0
	cmpl	f0,4(sp)
	beq	ld1		# If it's zero, we're done

	extsd	10(sp),r0,4,11	# r0 := biased exponent
	addd	12(sp),r0	# r0 := new biased exponent
	cmpqd	0,r0
	bge	under		# if it's <= 0, we have an underflow
	cmpd	0x7ff,r0	# Otherwise check if it's too big
	ble	over		# jump if overflow
#
#	Construct the result and return
#
	inssd	r0,10(sp),4,11	# Put the exponent back in the result
	movl	4(sp),f0
ld1:	ret	0
#
#	Underflow
#
under:				# f0 is alread zero
	br	err		# Join general error code
#
#	Overflow
#
over:	movl	huge,f0		# Largest possible floating magnitude
	cmpqb	0,11(sp)
	ble	err		# Jump if argument was positive
	negl	f0,f0		# If arg < 0, make result negative

err:	movd	erange,_errno	# Indicate range error
	ret	0

huge:	.double	0xffffffff		# The largest number that can
	.double	0x7fefffff		#   be represented in a long floating
