/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)head:ieeefp.h	1.2"

/*
 * Copyright (c) by AT&T, 1984.  All rights reserved.
 *
 * The 3B family interface for the exception environment for
 * the IEEE Proposed Standard for Floating-Point Arithmetic.
 * IEEE Task P754 -- Draft 10.
 *
 * This header defines four interfaces:
 *	1) Rounding Control
 *	2) Exception Control
 *	3) Exception Handling
 *	4) Utility Macros
 *	5) Full Exception Environment Control
 */

/* ROUNDING CONTROL ******************************************
 *
 * At all times, floating-point math is done using one of four
 * mutually-exclusive rounding modes.
 *
 * NOTE: the values given are chosen to match
 * the values needed by the FPA and MAU rounding-control fields.
 */
typedef	enum	fp_rnd {
    FP_RN = 0,	/* round to nearest representable number, tie -> even */
    FP_RP = 1,	/* round toward plus infinity			      */
    FP_RM = 2,	/* round toward minus infinity			      */
    FP_RZ = 3	/* round toward zero (truncate)			      */
    } fp_rnd;

extern fp_rnd	fpsetround();	/* set rounding mode, return previous */
extern fp_rnd	fpgetround();	/* return current rounding mode       */

/* EXCEPTION CONTROL *****************************************
 *
 * There are five floating-point exceptions, which can be individually
 * ENABLED (== 1) or DISABLED (== 0).  When an exception occurs
 * (ENABLED or not), the fact is noted by changing an associated
 * "sticky bit" from CLEAR (==0) to SET (==1).
 *
 * NOTE: the bit positions in an fp_except are chosen to match that in
 * the MAU and the FPA hardware mask bits.
 */
#define	fp_except	int

/* an fp_except can have the following (not exclusive) values:  */
#define	FP_X_INV	0x10	/* invalid operation exception	*/
#define	FP_X_OFL	0x08	/* overflow exception		*/
#define	FP_X_UFL	0x04	/* underflow exception		*/
#define	FP_X_DZ		0x02	/* divide-by-zero exception	*/
#define	FP_X_IMP	0x01	/* imprecise (loss of precision)*/

#define	FP_DISABLE	0	/* exception will be ignored	*/
#define	FP_ENABLE	1	/* exception will cause SIGFPE	*/
#define	FP_CLEAR	0	/* exception has not occurred	*/
#define	FP_SET		1	/* exception has occurred	*/

extern fp_except fpgetmask();	/* current exception mask       */
extern fp_except fpsetmask();	/* set mask, return previous    */
extern fp_except fpgetsticky();	/* return logged exceptions     */
extern fp_except fpsetsticky();	/* change logged exceptions     */


/* EXCEPTION HANDLING ****************************************
 * When a floating-point exception handler is entered, the variables
 * _fpftype -- floating-point fault type, and
 * _fpfault -- pointer to floating-point exception structure
 * will be established. _fpftype identifies the primary exception
 * (if both underflow and inexact are generated, _fpftype is underflow)
 * and _fpfault points to all the other information needed.
 * Two integer faults also use this mechanism: "integer overflow" and
 * "integer divide by zero". If either of these faults is indicated,
 * or if _fpftype == UNKNOWN, _fpfault will be NULL.
 */
typedef	enum	fp_ftype {
    UNKNOWN	= 0,	/* library code could not determine type      */
    INT_DIVZ	= 1,	/* integer divide by zero		      */
    INT_OVFLW	= 2,	/* integer overflow, including float->int conv*/
    FP_OVFLW	= 3,	/* floating-point overflow		      */
    FP_UFLW	= 4,	/* floating-point underflow		      */
    FP_INXACT	= 5,	/* floating-point loss of precision	      */
    FP_DIVZ	= 6,	/* floating-point divide by zero	      */
    FP_INVLD	= 7,	/* floating-point invalid operation	      */
    FP_IN_OVFLW = 8,	/* inexact AND overflow                       */
    FP_IN_UFLW  = 9,	/* inexact AND underflow                      */
    } fp_ftype;

extern fp_ftype _fpftype;

/* reporting a floating-point exception requires a discriminated union
 * of all relevant data-types. The following defines the discriminant.
 * The particular values are arbitrary.
 */
typedef	enum	fp_type {
    FP_NULL	= 0,	/* ininitialized data			      */
    FP_C	= 1,	/* result of comparison, returned from trap   */
    FP_L	= 2,	/* long int data, being converted to/from flt.*/
    FP_F	= 3,	/* single-precision floating-point	      */
    FP_D	= 4,	/* double-precision floating-point	      */
    FP_X	= 5,	/* double-extended-precision floating-point   */
    FP_P	= 6,	/* packed-decimal floating-point	      */
    FP_U	= 7	/* unsigned int data, being converted 	      */
    }	fp_type;

/* floating-point data types					*/
typedef struct extended { /* double-extended floating point	*/
    unsigned long	w[3];
    } extended;

typedef struct packdec { /* packed-decimal floating point	*/
    unsigned long	w[3];
    } packdec;

typedef	enum	fp_cmp { /* result of comparison		*/
    FP_LT	= -1,	/* less than	*/
    FP_EQ	=  0,	/* equal	*/
    FP_GT	=  1,	/* greater than	*/
    FP_UO	=  2	/* unordered	*/
    } fp_cmp;

typedef union fp_union { /* union of all types			*/
    fp_cmp	c;
    long	l;
    unsigned	u;
    float	f;
    double	d;
    extended	x;
    packdec	p;
    } fp_union;

typedef struct fp_dunion { /* discriminated union		*/
    fp_type	type;
    fp_union	value;
    } fp_dunion;

/* the rest of the information pointed to by _fp_fault includes:
 *	the type of operation being performed,
 *	the types and values of the operands,
 *	the type of a trapped value (if any), and
 *	the desired type of the result.
 * the following defines and struct give the information layout.
 */

typedef	enum	fp_op {	/* floating-point operations		      */
    FP_ADD  = 0,	/* floating-point addition		      */
    FP_SUB  = 1,	/* floating-point subtraction		      */
    FP_MULT = 2,	/* floating-point multiplication	      */
    FP_DIV  = 3,	/* floating-point division		      */
    FP_SQRT = 4,	/* floating-point square root		      */
    FP_REM  = 5,	/* floating-point remainder		      */
    FP_CONV = 6,	/* data movement, including changing format   */
    FP_RNDI = 7,	/* round to integer (keep in floating format) */
    FP_CMPT = 8,	/* trapping floating-point compare	      */
    FP_CMP  = 9,	/* non-trapping floating-point compare	      */
    FP_NEG  = 10,	/* (MAU) negate operation		      */
    FP_ABS  = 11	/* (MAU) absolute value operation	      */
    }	fp_op;

struct fp_fault {
    fp_op	operation;
    fp_dunion	operand[2];
    fp_dunion	t_value;
    fp_dunion	result;
    };

extern struct fp_fault	* _fpfault;

/* UTILITY MACROS ********************************************
 *
 * It is impossible to provide, in a "C" environment, a function
 * which checks to see if a single-precision number is a NaN,
 * since the number will be converted to "double" before being
 * passed.  The following macro simulates a function which returns
 * 1 if applied to a NaN, 0 otherwise.
 */
#define isnanf(x)	(((*(int *)&(x) & 0x7f800000L)==0x7f800000L)&& \
			 ((*(int *)&(x) & 0x007fffffL)!=0x00000000L) )

/* FULL EXCEPTION ENVIRONMENT CONTROL ************************
 *
 * The complete exception environment defined above requires
 * a significant amount of code to be picked up from the libraries.
 * The "glue" routines _getfltsw() and _getflthw() are provided
 * in two versions, one with full capability and one without.
 * The version without full capability has the following behavior:
 *	floating-point faults will raise SIGFPE, but
 *	the user handler will have _fpftype == UNKNOWN, and
 *	_fpfault == NULL. If SIGFPE is set to SIG_DFL, a core dump
 *	will be produced.
 * If ANY module in the final a.out #includes ieeefp.h (except as noted
 * below), the code below will force in the FULL environment,
 * 
 * NOTES: 1) code in the library (e.g. for setround(), etc.) which
 *	needs the definitions above, but does not care which fault
 *	environment is picked up SHOULD #define P754_NOFAULT
 *	before #including ieeefp.h.
 *	2) If a user program does not wish to do any fault recovery,
 *	just produce a core dump, it MAY also #define P754_NOFAULT,
 *	and may gain a code size reduction. However, if it uses a
 *	routine	in the math library that does request the full fault
 *	environment then the full environment WILL be included.
 */
#ifndef P754_NOFAULT
#define	P754_NOFAULT	1
extern void	_getflthw();
extern fp_union	_getfltsw();
static void	(* _p754_1)() = _getflthw;
static fp_union	(* _p754_2)() =	_getfltsw;
#endif
