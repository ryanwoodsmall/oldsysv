/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
	/* @(#) optab.c: 1.7 7/18/83 */

#include	"head.h"

/*
 *  	Instruction printing data.
 *		MACHINE DEPENDENT
 *
 */

/* The registers in regname must be in the correct order, because the numeric
 * value of the register is used (in dis/bits.c) to index into this table.
 * The null final entry is a sentinel, used in prassym (opset.c).
 *
 * The registers in reglist must be in correct order, because the
 * number of the register is used to index into this table, to get roffs,
 * which gives the offset in the user area from register 0.
 */

#if u3b

STRING	regname[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
			"ap", "fp", "sp", "pc", "psbr", "ssbr", (char *)0};

REGLIST reglist [NUMREGLS] = {
		"r0", R0,
		"r1", R1,
		"r2", R2,
		"r3", R3,
		"r4", R4,
		"r5", R5,
		"r6", R6,
		"r7", R7,
		"r8", R8,
		"fp", FP,
		"ap", AP,
		"sp", SP,
		"pc", PC,
		"psw", PS,
		"psbr", PSBR,
		"ssbr", SSBR,
/*		"u_rval1", VAL1,	not register; removed */
/*		"u_rval2", VAL2,	not register; removed */
};
#endif

#if vax
STRING	regname[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
					"r8", "r9", "r10","r11","ap", "fp", "sp", "pc", (char *)0};

REGLIST reglist [NUMREGLS] = {
		"r0", R0,
		"r1", R1,
		"r2", R2,
		"r3", R3,
		"r4", R4,
		"r5", R5,
		"r6", R6,
		"r7", R7,
		"r8", R8,
		"r9", R9,
		"r10", R10,
		"r11", R11,
		"ap", AP,
		"fp", FP,
		"sp", SP,
		"pc", PC,
		"ps", PS,
};
#endif

#if ns32000
STRING	regname[] = { "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
					"fp", "sp", "pc", (char *)0};

REGLIST reglist [NUMREGLS] = {
		"r0", R0,
		"r1", R1,
		"r2", R2,
		"r3", R3,
		"r4", R4,
		"r5", R5,
		"r6", R6,
		"r7", R7,
		"fp", FP,
		"sp", SP,
		"pc", PC,
		"psr_mod", PSR_MOD,
};
#endif

#if u3b5
STRING regname[] = {"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7", "r8",
		    "fp", "ap", "psw", "sp", "pcbp", "isp", "pc"};

REGLIST reglist [NUMREGLS] = {
		"r0",	R0,
		"r1",	R1,
		"r2",	R2,
		"r3",	R3,
		"r4",	R4,
		"r5",	R5,
		"r6",	R6,
		"r7",	R7,
		"r8",	R8,
		"fp",	FP,
		"ap",	AP,
		"psw",	PS,
		"sp",	SP,
		"pcbp",	-1,	/* pcbp not available through ptrace */
		"isp",	-1,	/* isp not available  through ptrace */
		"pc",	PC,
};
#endif

STRING	fltimm[] = {
"0.5", "0.5625", "0.625", "0.6875", "0.75", "0.8125", "0.875", "0.9375",
"1.0", "1.125", "1.25", "1.375", "1.5", "1.625", "1.75", "1.875",
"2.0", "2.25", "2.5", "2.75", "3.0", "3.25", "3.5", "3.75",
"4.0", "4.5", "5.0", "5.5", "6.0", "6.5", "7.0", "7.5",
"8.0", "9.0", "10.0", "11.0", "12.0", "13.0", "14.0", "15.0",
"16.0", "18.0", "20.0", "22.0", "24.0", "26.0", "28.0", "30.0",
"32.0", "36.0", "40.0", "44.0", "48.0", "52.0", "56.0", "60.0",
"64.0", "72.0", "80.0", "88.0", "96.0", "104.0", "112.0", "120.0"
};

char *fmtr = {"%r"};
char *fmtR = {"%R"};
