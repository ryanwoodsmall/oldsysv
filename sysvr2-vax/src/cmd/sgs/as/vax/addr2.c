/*	@(#)addr2.c	2.3	*/

#include <stdio.h>
#include "systems.h"
#include "symbols.h"
#include "instab.h"
#include "gendefs.h"
#include "codeout.h"
#include "reloc.h"


extern unsigned short
		relent; /* number of relocation entries written to fdrel */

extern long 
		newdot; /* up to date value of "." */

extern symbol
		*dot; /* current location counter */



extern BYTE
		*longsdi;

extern FILE
	*fdrel; /* file of relocation table information */

static short sdijbr = 0;  /* flag to consider the special case jbr sdi */
static short pcrel = 0;   /* flag to indicate pc relative relocation */

extern
	genreloc(),
	genpcrel(),
	sdidst(),
	sdiinst(),
	newstmt(),
	setfile(),
	lineno(),
	linenum(),
	lineval(),
	define(),
	endef(),
	setval(),
	setscl(),
	settyp(),
	settag(),
	setlno(),
	setsiz(),
	setdim1(),
	setdim2(),
	gendisp(),
	genabssym();


int (*(modes[NACTION+2]))() = {

/*0*/	0, 		/* GENVALUE */
/*1*/	genreloc,	/* GENRELOC */
/*1*/	genpcrel,	/* GENPCREL */
/*3*/	sdidst,		/* SDIDST */
/*4*/	sdiinst,	/* SDIINST + 0 */
/*5*/	sdiinst,	/* SDIINST + 1 */
/*6*/	sdiinst,	/* SDIINST + 2 */
/*7*/	sdiinst,	/* SDIINST + 3 */
/*8*/	sdiinst,	/* SDIINST + 4 */
/*9*/	sdiinst,	/* SDIINST + 5 */
/*10*/	sdiinst,	/* SDIINST + 6 */
/*11*/	sdiinst,	/* SDIINST + 7 */
/*12*/	sdiinst,	/* SDIINST + 8 */
/*13*/	sdiinst,	/* SDIINST + 9 */
/*14*/	sdiinst,	/* SDIINST + 10 */
/*15*/	sdiinst,	/* SDIINST + 11 */
/*16*/	sdiinst,	/* SDIINST + 12 */
/*17*/	sdiinst,	/* SDIINST + 13 */
/*18*/	sdiinst,	/* SDIINST + 14 */
/*19*/	sdiinst,	/* SDIINST + 15 */
/*20*/	sdiinst,	/* SDIINST + 16 */
/*21*/	sdiinst,	/* SDIINST + 17 */
/*22*/	sdiinst,	/* SDIINST + 18 */
/*23*/	sdiinst,	/* SDIINST + 19 */
/*24*/	sdiinst,	/* SDIINST + 20 */
/*25*/	sdiinst,	/* SDIINST + 21 */
/*26*/	sdiinst,	/* SDIINST + 22 */
/*27*/	sdiinst,	/* SDIINST + 23 */
/*28*/ 	sdiinst,	/* SDIINST + 24 */
/*29*/	newstmt,	/* NEWSTMT */
/*30*/	setfile,	/* SETFILE */
/*31*/	lineno,		/* LINENBR */
/*32*/	linenum,	/* LINENUM */
/*33*/	lineval,	/* LINEVAL */
/*34*/	define,		/* DEFINE */
/*35*/	endef,		/* ENDEF */
/*36*/	setval,		/* SETVAL */
/*37*/	setscl,		/* SETSCL */
/*38*/	settyp,		/* SETTYP */
/*39*/	settag,		/* SETTAG */
/*40*/	setlno,		/* SETLNO */
/*41*/	setsiz,		/* SETSIZ */
/*42*/	setdim1,	/* SETDIM1 */
/*43*/	setdim2,	/* SETDIM2 */
/*44*/	gendisp,	/* GENDISP */
/*45*/	genabssym,	/* GENABSSYM */
	0 };


/* code generation action routines */

genreloc(sym,code) /* simple relocation with no frills */

register symbol *sym;
register codebuf *code;

	{ char *rsym;
	  unsigned short stype, rtype;
	  prelent trelent;

	  if (sym == NULLSYM)
		stype = ABS;
	  else {
		code->cvalue += ((code->cindex > 0) ?
				sym->value : (-sym->value));
		stype = (sym->styp & TYPE);
		};
	  switch (stype) {

		case ABS: if (pcrel)
				rsym = "=";
			  else
				return; /* no relocation is needed */
			  break;

		case TXT: rsym = ".text";
			  break;

		case DAT: rsym = ".data";
			  break;

		case BSS: rsym = ".bss";
			  break;
		
		case UNDEF: sym->styp = sym->styp | EXTERN;
			    rsym = sym->_name.name;
			    /* in case the symbol is common (it has a value) */
			    code->cvalue = code->cvalue - ((code->cindex > 0) ?
				 sym->value : (-sym->value));
			    break;

		default: aerror("attempt to relocate weird symbol");
		};
	switch (code->cnbits) { /* determine kind of relocation */
		 case 8: rtype = pcrel ? R_PCRBYTE : R_RELBYTE;
			 break;
		case 16: rtype = pcrel ? R_PCRWORD : R_RELWORD;
			 break;
		case 32: rtype = pcrel ? R_PCRLONG : R_RELLONG;
			 break;
		default: werror("attempt to relocate weird sized symbol");
		};
	/* generate relocation table entry */
	trelent.relval = newdot;
	trelent.relname = rsym;
	trelent.reltype = rtype;
	fwrite(&trelent,sizeof(prelent),1,fdrel);
	relent++;
	}



genpcrel(sym,code)

register symbol *sym;
register codebuf *code;

	{
	code->cvalue -=  newdot + (code->cnbits/BITSPBY);
	pcrel = 1; /* toggle pc relative relocation flag on */
	genreloc(sym,code);
	pcrel = 0; /* toggle pc relative relocation flag off */
	}




unsigned short negops[25] = {

0x1f, /* bcc dest / bcs .+3; brw dest */
0x1e, /* bcs dest / bcc .+3; brw dest */
0x12, /* beql dest / bneq .+3; brw dest */
0x12, /* beqlu dest / bnequ .+3; brw dest */
0x19, /* bgeq dest / blss .+3; brw dest */
0x1f, /* bgequ dest / blssu .+3; brw dest */
0x15, /* bgtr dest / bleq .+3; brw dest */
0x1b, /* bgtru dest / blequ .+3; brw dest */
0x14, /* bleq dest / bgtr .+3; brw dest */
0x1a, /* blequ dest / bgtru .+3; brw dest */
0x18, /* blss dest / bgeq .+3; brw dest */
0x1e, /* blssu dest / bgequ .+3; brw dest */
0x13, /* bneq dest / beql .+3; brw dest */
0x13, /* bnequ dest / beqlu .+3; brw dest */
0x1d, /* bvc dest / bvs .+3; brw dest */
0x1c, /* bvs dest / bvc .+3; brw dest */
0x31, /* brb dest / brw dest */
0xe0, /* bbc dest / bbs .+3; brw dest */
0xe1, /* bbs dest / bbc .+3; brw dest */
0xe4, /* bbcc dest / bbsc .+3; brw dest */
0xe5, /* bbsc dest / bbcc .+3; brw dest */
0xe2, /* bbcs dest / bbss .+3; brw dest */
0xe3, /* bbss dest / bbcs .+3; brw dest */
0xe8, /* blbc dest / blbs .+3; brw dest */
0xe9  /* blbs dest / blbc .+3; brw dest */
};


sdidst(sym,code)

register symbol *sym;
register codebuf *code;

	{ if (*longsdi) { /* was previous instruction set as a long sdi
			     if so, make the operands follow suit */
		if (!sdijbr) /* generate the .+3 dest following
				the long instruction sequence */
			codgen(BITSPBY,3L); /* .+3 dest */
		if (!sdijbr) /* generate the brw sequence */
			codgen(BITSPBY,0x31L);
		code->cnbits = 2*BITSPBY; /* dest is now a word, the routine
					     which called sdidst will output
					     the destination operand */
		};
	  gendisp(sym,code); /* add the finishing touches */
	  if (sdijbr) /* toggle the special case flag */
		sdijbr = 0;
	}


sdiinst(sym,code)

register symbol *sym;
register codebuf *code;

	{ if (*++longsdi)
		code->cvalue = (long)negops[code->caction-SDIINST];
	  if (code->caction==SDIINST+16) /* jbr special case, set flag */
		sdijbr++;
	}


gendisp(sym,code)

register symbol *sym;
register codebuf *code;

	{
	  if (sym != NULLSYM)
		code->cvalue += ((code->cindex > 0) ? 
				sym->value : (-sym->value));
	  code->cvalue -= newdot + (code->cnbits/BITSPBY); /* adjust offset */

	  /* check to see that we do not have a jump displacement overflow */

	  if (((code->cnbits == BITSPBY) &&
	      (code->cvalue < -128 || code->cvalue > 127)) ||
	      ((code->cnbits == (2*BITSPBY)) &&
	      (code->cvalue < -32768 || code->cvalue > 32767)))
		aerror("displacement overflow, try cc -Wa,-j option to invoke ljas instead of as");
	}


genabssym(sym,code)

register symbol *sym;
register codebuf *code;

	{ /* handle symbol - symbol special case, the parser stuck the ptr
	     to the left symbol in the code->cvalue parameter, see parse.y */

	  symbol *dummy;

	  dummy = (symbol *)code->cvalue;
	  code->cvalue = dummy->value - ((code->cindex > 0) ?
			 sym->value : (-sym->value));
	}
