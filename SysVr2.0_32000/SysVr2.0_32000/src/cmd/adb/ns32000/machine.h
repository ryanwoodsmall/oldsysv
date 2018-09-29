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
/*	@(#)machine.h	1.3	*/
/* 
 * NS32000 UNIX parameters 
 */

/* User block in core file */

#define ADDR_U ((unsigned) 0x810000)
#define ADRREG ((unsigned) 0x1C4)
#define ADRFREG ((unsigned) 0x208)
union  Uuniontag {
    struct user strpart;
    char chrpart[ctob(USIZE)];
};

extern union Uuniontag Uunion;

/*  Note - the following define must appear after the above
 *  definition else the preprocessor will use the wrong u
 *  in the expansion of USIZE.
*/
#define u Uunion.strpart
