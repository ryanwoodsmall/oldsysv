/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:common/symbols.h	1.7.1.4"
/*
 */

#include	"systems.h"
#include	"symbols2.h"
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

typedef struct	symlstag
		{
		long	index;
		struct symlstag	*nextelmt;
		short	sclass;
	} symlist;

#define SYMSIZ	sizeof(symlist)

typedef	struct symtag
	{
		name_union	_name;
		short	styp;
		long	value;
		short	maxval;
		short	sectnum;
		symlist *symlst;
		struct symtag	*next;
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
#define USRNAME	1
#define MNEMON	0

typedef	union
	{
		symbol	*stp;
		instr	*itp;
	} upsymins;

/* macros to convert between symbol table indices and symbol pointers */
#define GETSYMPTR(indx,sym) \
	{ extern symbol *symtab[];\
	extern unsigned long tablesize;\
	sym = &symtab[(indx)/tablesize][(indx)%tablesize]; }


#define GETSYMINDX(sym,index) 	\
	{ extern symbol *symtab[];\
	extern unsigned long tablesize;\
	extern short tabletop;\
	register short i;\
	for (i=0;i<tabletop;i++) \
		if ((&symtab[i][0] <= (sym)) && ((sym) < &symtab[i][tablesize]))\
			break; \
	if (i>=tabletop) \
		aerror("symbol not in symbol table"); \
	index = i*tablesize + (unsigned short)((sym)-&symtab[i][0]);\
	}

