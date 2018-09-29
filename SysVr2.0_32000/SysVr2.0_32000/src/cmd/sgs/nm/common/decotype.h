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
    /*  decotype.h contains format strings used to print out the type field of
     *  a symbol table entry
     *
     *  the minimum number of characters printed depends on the amount of room 
     *  left by the other fields in the output line (the type field may grow
     *  beyond this minimum) and differs for Basic-16 and New 3b as well as
     *  depending on the setting of eflag (-e) and numbase.
     */


#ifdef	B16
/* FORMAT STRINGS FOR BASIC-16 (and its kin) */

/*  if all symbols are printed (eflag is OFF) */
static char	*prtype[2] = {
			"|%18s",
			"|%18s"
};

/*  if only statics and externs are printed (eflag is ON) */
static char	*pretype[2] = {
			"|%16s",
			"|%14s"
};

#else
/* FORMAT STRINGS FOR NEW 3B (and its kin) */

static char	*prtype[3] = {
			"|%18s",
			"|%17s",
			"|%14s"
};

static char	*pretype[3] = {
			"|%16s",
			"|%13s",
			"|%14s"
};

#endif


/* TYPE NAMES */
static	char	*typelist[16] = {
			"",
			"arg",
			"char",
			"short",
			"int",
			"long",
			"float",
			"double",
			"struct",
			"union",
			"enum",
			"enmem",
			"Uchar",
			"Ushort",
			"Uint",
			"Ulong"
};

/*
*    static char	ID_deco[ ] = "@(#) decotype.h: 1.1 1/7/82";
*/
