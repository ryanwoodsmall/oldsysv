/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/symbols.h	1.7"
/*
 */

#include	"systems.h"
typedef char BYTE;

#define UNDEF	000
#define ABS	001
#define TXT	002
#define DAT	003
#define BSS	004
#define TYPE	(UNDEF|ABS|TXT|DAT|BSS)
#define	SECTION	010
#define TVDEF	020
#define EXTERN	040

typedef	union
	{
		char	name[9];
		struct
		{
			long	zeroes;
			long	offset;
		} tabentry;
	} name_union;

typedef	struct
	{
		name_union	_name;
		BYTE	tag;
		short	styp;
		long	value;
		short	maxval;
		short	sectnum;
	} symbol;

#define SYMBOLL	sizeof(symbol)

typedef	struct
	{
		char	name[sizeof(name_union)];
		BYTE	tag;
		BYTE	val;
		BYTE	nbits;
		long	opcode;
		symbol	*snext;
	} instr;

#define INSTR sizeof(instr);

#define INSTALL	1
#define N_INSTALL	0
#define USRNAME	0
#define MNEMON	1

typedef	union
	{
		symbol	*stp;
		instr	*itp;
	} upsymins;
