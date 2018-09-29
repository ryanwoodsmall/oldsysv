/*	@(#)gencode.c	2.4	*/

#include <stdio.h>
#include <string.h>
#include "symbols.h"
#include "instab.h"
#include "parse.h"
#include "gendefs.h"
#include "expand.h"
#include "expand2.h"

extern long
	newdot;		/* up-to-date value of "." */
extern symbol
	*dot;		/* current location counter */
extern short
	transvec,	/* indicates active transfer vector option */
	opt;	/* no optimize flag */



/* flags */

int	defdisp = 4; /* default number of bytes displacement for externals 
		        as ... -l ...  change by -[bwl] flags              */

short	readonly = 0;/* convert .data segment into .text for readonly and
   			shareable access				    */


static int warnbad = 0;
static char noshort = 0; /* flag set to prohibit generation of short literals
			    within the processing of a span dependent instr.
			    the generation of the short literals royally
			    screws up the sdi logic of the common assembler */

flags(flag)
char flag;
{
	char errmsg[28];
	switch(flag) {


	case 'r':	readonly = 1; /* put .data stuff into .text segment */
			break;


	case 'l':	defdisp = 4; /* displace externals with 4 bytes=long */
			break;

	case 'w':	defdisp = 2; /* displace externals with 2 bytes=word */
			break;

	case 'b':	defdisp = 1; /* displace externals with 1 byte */
			break;


	default:
		sprintf(errmsg,"Illegal flag (%c) - ignored",flag);
		werror(errmsg);
		break;
	} /* switch */
} /* flags */


typedef struct { BYTE mode[6]; } contab;

contab context[47] = {
{INSTB,INSTB,INSTB,INSTJW,0,0}, 	/* ILU00 */
{INSTD,INSTD,INSTD,INSTJW,0,0},		/* ILU01 */
{INSTF,INSTF,INSTF,INSTJW,0,0},		/* ILU02 */
{INSTL,INSTL,INSTL,INSTJW,0,0},		/* ILU03 */
{INSTW,INSTW,INSTW,INSTJW,0,0},		/* ILU04 */
{INSTW,INSTL,INSTW,INSTL,INSTW,INSTL},	/* ILU05 */
{INSTL,INSTL,INSTJB,0,0,0}, 		/* ILU06 */
{INSTB,INSTL,INSTL,0,0,0}, 		/* ILU07 */
{INSTB,INSTW,INSTL,INSTB,INSTW,INSTL},	/* ILU08 */
{INSTB,INSTQ,INSTQ,0,0,0}, 		/* ILU09 */
{INSTL,INSTB,INSTJB,0,0,0}, 		/* ILU10 */
{INSTL,INSTJB,0,0,0,0},			/* ILU11 */
{INSTW,INSTL,INSTL,INSTL,0,0},		/* ILU12 */
{INSTW,INSTL,INSTB,INSTW,INSTL,0},	/* ILU13 */
{INSTL,INSTB,INSTB,INSTL,0,0}, 		/* ILU14 */
{INSTL,INSTL,INSTW,INSTL,0,0}, 		/* ILU15 */
{INSTB,INSTD,0,0,0,0},	 		/* ILU16 */
{INSTD,INSTB,0,0,0,0}, 			/* ILU17 */
{INSTD,INSTF,0,0,0,0},	 		/* ILU18 */
{INSTD,INSTL,0,0,0,0}, 			/* ILU19 */
{INSTD,INSTW,0,0,0,0}, 			/* ILU20 */
{INSTF,INSTB,0,0,0,0}, 			/* ILU21 */
{INSTF,INSTD,0,0,0,0}, 			/* ILU22 */
{INSTF,INSTL,0,0,0,0},	 		/* ILU23 */
{INSTF,INSTW,0,0,0,0},			/* ILU24 */
{INSTL,INSTD,0,0,0,0},	 		/* ILU25 */
{INSTL,INSTF,0,0,0,0},	 		/* ILU26 */
{INSTL,INSTW,INSTL,0,0,0}, 		/* ILU27 */
{INSTW,INSTL,INSTL,0,0,0}, 		/* ILU28 */
{INSTW,INSTL,INSTL,INSTW,INSTL,0},	/* ILU29 */
{INSTW,INSTB,0,0,0,0},	 		/* ILU30 */
{INSTW,INSTD,0,0,0,0}, 			/* ILU31 */
{INSTW,INSTF,0,0,0,0}, 			/* ILU32 */
{INSTL,INSTQ,INSTL,INSTL,0,0}, 		/* ILU33 */
{INSTD,INSTB,INSTD,INSTL,INSTD,0},	/* ILU34 */
{INSTF,INSTB,INSTF,INSTL,INSTF,0}, 	/* ILU35 */
{INSTL,INSTL,INSTL,INSTQ,0,0}, 		/* ILU36 */
{INSTL,INSTB,INSTB,INSTL,0,0}, 		/* ILU37 */
{INSTL,INSTL,INSTB,INSTB,0,0}, 		/* ILU38 */
{INSTB,INSTW,INSTL,0,0,0}, 		/* ILU39 */
{INSTW,INSTL,INSTB,INSTL,INSTW,INSTL},	/* ILU40 */
{INSTD,INSTW,INSTL,0,0,0}, 		/* ILU41 */
{INSTF,INSTW,INSTL,0,0,0}, 		/* ILU42 */
{INSTB,INSTW,INSTL,0,0,0}, 		/* ILU43 */
{INSTB,INSTL,INSTL,0,0,0}, 		/* ILU44 */
{INSTW,INSTL,INSTL,INSTB,0,0}, 		/* ILU45 */
{INSTL,INSTJB,0,0,0,0} 			/* ILU46 */
};

genopr(op,oppos,type)  /* generate preliminary code for an instr operand */

addrmode op;
BYTE oppos, type;

	{ register int field;
	  register BYTE opmode;	
	  BYTE usage;

	  opmode = op.admode;

	  if (opmode & AINDX) {			/* ...[%r] */
		field = op.adreg2 | 0x40;
		generate(BITSPBY,GENVALUE,field,NULLSYM);
		opmode = opmode & ~AINDX;
		};

	  if (opmode & ASTAR) {			/* * ... */
		op.adreg1 = op.adreg1 | 0x10;
		opmode = opmode & ~ASTAR;
		};

	  
	  switch (opmode) {
		
		case AREG: field = op.adreg1 | 0x50;	/* %r */
			   generate(BITSPBY,GENVALUE,field,NULLSYM);
			   break;

		case ABASE: field = op.adreg1 | 0x60;	/* (%r) */
			    generate(BITSPBY,GENVALUE,field,NULLSYM);
			    break;

		case ADECR: field = op.adreg1 | 0x70;	/* -(%r) */
			    generate(BITSPBY,GENVALUE,field,NULLSYM);
			    break;

		case AINCR: field = op.adreg1 | 0x80;	/* (%r)+ */
			    generate(BITSPBY,GENVALUE,field,NULLSYM);
			    break;

		case AEXP:  if (type < ILU00)		/* <expr> */
				usage = type;
			    else
				usage = context[type-ILU00].mode[oppos-1];
			    if (usage == 0)
				aerror("cannot determine operand context");
			    if (usage == INSTJB) {
				field = op.adexpr.expval.lngval - newdot - 1;
			        if ((field < -128 || field > 127) &&
				    (op.adexpr.exptype != TSYM))
					yyerror("branch error (byte disp)");
				generate(BITSPBY,GENDISP,
				      op.adexpr.expval.lngval,op.adexpr.symptr);
				break;
				};
			    if (usage == INSTJW) {
				field = op.adexpr.expval.lngval - newdot - 2;
				if ((field < -32768 || field > 32767) &&
				    (op.adexpr.exptype != TSYM))
					yyerror("branch error (word disp)");
				generate((2*BITSPBY),GENDISP,
				      op.adexpr.expval.lngval,op.adexpr.symptr);
				break;
				};
			    /* assume expr(pc) mode */
			    if (defdisp == 1)      /* byte disp */	
				field = op.adreg1 | 0xaf;
			    else if (defdisp == 2) /* word disp */
				field = op.adreg1 | 0xcf;
			    else if (defdisp == 4) /* long disp */
				field = op.adreg1 | 0xef;
			    else /* uh oh! */
				aerror("cannot determine operand displacement");
			    generate(BITSPBY,GENVALUE,field,NULLSYM);
			    generate((defdisp*BITSPBY),GENPCREL,
				     op.adexpr.expval.lngval,op.adexpr.symptr);
			    break;

		case ADISP: if (op.adexpr.exptype==TSYM) {/* <expr> (%r) */
				if (defdisp == 1)      /* byte disp */
					field = op.adreg1 | 0xa0;
				else if (defdisp == 2) /* word disp */
					field = op.adreg1 | 0xc0;
				else if (defdisp == 4) /* long disp */
					field = op.adreg1 | 0xe0;
				else /* uh oh! */
					aerror("cannot determine operand displacement");
				generate(BITSPBY,GENVALUE,field,NULLSYM);
				generate((defdisp*BITSPBY),GENRELOC,
				    op.adexpr.expval.lngval,op.adexpr.symptr);
				break;
				};
			    if (op.adexpr.expval.lngval==0 && !(op.admode&ASTAR)) {
				field = op.adreg1 | 0x60; /* (%r) equivalent */
				generate(BITSPBY,GENVALUE,field,NULLSYM);
				break;
				};
			    if (op.adexpr.expval.lngval>=-128&&op.adexpr.expval.lngval<=127) {
				field = op.adreg1 | 0xa0; /* byte disp */
				generate(BITSPBY,GENVALUE,field,NULLSYM);
				generate(BITSPBY,GENVALUE,op.adexpr.expval.lngval,
							NULLSYM);
				break;
				};
			    if (op.adexpr.expval.lngval >= -32768 &&
				op.adexpr.expval.lngval <= 32767) {
				field = op.adreg1 | 0xc0; /* word disp */
				generate(BITSPBY,GENVALUE,field,NULLSYM);
				generate((2*BITSPBY),GENVALUE,op.adexpr.expval.lngval,
							NULLSYM);
				break;
				};
			    /* assume long displacement */
			    field = op.adreg1 | 0xe0;
			    generate(BITSPBY,GENVALUE,field,NULLSYM);
			    generate((4*BITSPBY),GENVALUE,op.adexpr.expval.lngval,
							NULLSYM);
			    break;

		case AIMM:  if (type < ILU00) 			/* $ <expr> */
				usage = type;
			    else
				usage = context[type-ILU00].mode[oppos-1];
			    if (usage == 0)
				aerror("cannot determine operand context");
			    if (op.admode&ASTAR)
				usage = INSTL;
			    /* see if we have a short literal */
			    if (!(op.admode&ASTAR) &&
				(op.adexpr.exptype!=TSYM) &&
				(!noshort) &&
				((op.adexpr.exptype&TINT)!=0) &&
				(op.adexpr.expval.lngval >= 0) &&
				(op.adexpr.expval.lngval <= 63)) {
				generate(BITSPBY,GENVALUE,op.adexpr.expval.lngval,
							NULLSYM);
				break;
				};
			    if (warnbad)
				aerror("displacement overflow, try cc -Wa,-j option to invoke ljas instead of as");
			    field = op.adreg1 | 0x8f;
			    switch (usage) {
				case INSTD:  /* double precision */
				case INSTF:  /* single precision */ {
					register int *ptrl, *ptrr;
					if ((op.adexpr.exptype&TFLT)==0)
						yyerror("illegal operand");
					ptrl = (int *)&op.adexpr.expval.dblval;
					ptrr = ptrl + 1;
					if ((*ptrr == 0) && /* look for 
						     short floating literal */
					     (*ptrl&0x00004000) &&
					    ((*ptrl&0x000043f0)==*ptrl)) {
						field = *ptrl>>4;
						generate(BITSPBY,GENVALUE,
							  field,NULLSYM);
						break;
						};
					generate(BITSPBY,GENVALUE,field,
								NULLSYM);
					if (usage == INSTF)
					     generate((4*BITSPBY),GENVALUE,
					      op.adexpr.expval.fltval,NULLSYM);
					else {
					     generate((4*BITSPBY),GENVALUE,
					      *ptrl,NULLSYM);
					     generate((4*BITSPBY),GENVALUE,
					      *ptrr,NULLSYM);
					};

					break;
					};
				case INSTJB:
				case INSTB: {
					generate(BITSPBY,GENVALUE,field,NULLSYM);
					if (op.adexpr.exptype&TDIFF)
					generate(BITSPBY,GENABSSYM,
						op.adexpr.expval.lngval,
						op.adexpr.symptr);
					else
					generate(BITSPBY,GENRELOC,
						op.adexpr.expval.lngval,
						op.adexpr.symptr);
					break;
					};
				case INSTJW:
				case INSTW: {
					generate(BITSPBY,GENVALUE,field,NULLSYM);
					if (op.adexpr.exptype&TDIFF)
					generate((2*BITSPBY),GENABSSYM,
						op.adexpr.expval.lngval,
						op.adexpr.symptr);
					else
					generate((2*BITSPBY),GENRELOC,
						op.adexpr.expval.lngval,
						op.adexpr.symptr);
					break;
					};
				case INSTL: {
					generate(BITSPBY,GENVALUE,field,NULLSYM);
					if (op.adexpr.exptype&TDIFF)
					generate((4*BITSPBY),GENABSSYM,
						op.adexpr.expval.lngval,
						op.adexpr.symptr);
					else
					generate((4*BITSPBY),GENRELOC,
						op.adexpr.expval.lngval,
						op.adexpr.symptr);
					break;
					};
				case INSTQ: {
					generate(BITSPBY,GENVALUE,field,NULLSYM);
					generate((4*BITSPBY),GENVALUE,0L,NULLSYM);
					generate((4*BITSPBY),GENRELOC,
						op.adexpr.expval.lngval,
						op.adexpr.symptr);
					break;
					};
				default:
					aerror("operand context confusion");
				};
			    break;
		default: /* uh oh! */
			aerror("attempt to assemble weird operand");
			break;
			};
	}



typedef struct { char name[9];
		 unsigned short nopcode; } sditab;

sditab nsdiop[25] = {

/* ops.out contains the information needed to generate the short form of these
   instructions.  this table contains the information needed to generate the
   long form of these instructions.  this table is searched linearly. */

/*      opcode of negated instruction */

{"jcc",		0x1f}, /* bcc dest / bcs .+3; brw dest */
{"jcs",		0x1e}, /* bcs dest / bcc .+3; brw dest */
{"jeql",	0x12}, /* beql dest / bneq .+3; brw dest */
{"jeqlu",	0x12}, /* beqlu dest / bnequ .+3; brw dest */
{"jgeq",	0x19}, /* bgeq dest / blss .+3; brw dest */
{"jgequ",	0x1f}, /* bgequ dest / blssu .+3; brw dest */
{"jgtr",	0x15}, /* bgtr dest / bleq .+3; brw dest */
{"jgtru",	0x1b}, /* bgtru dest / blequ .+3; brw dest */
{"jleq",	0x14}, /* bleq dest / bgtr .+3; brw dest */
{"jlequ",	0x1a}, /* blequ dest / bgtru .+3; brw dest */
{"jlss",	0x18}, /* blss dest / bgeq .+3; brw dest */
{"jlssu",	0x1e}, /* blssu dest / bgequ .+3; brw dest */
{"jneq",	0x13}, /* bneq dest / beql .+3; brw dest */
{"jnequ",	0x13}, /* bnequ dest / beqlu .+3; brw dest */
{"jvc",		0x1d}, /* bvc dest / bvs .+3; brw dest */
{"jvs",		0x1c}, /* bvs dest / bvc .+3; brw dest */
{"jbr",		0x31}, /* brb dest / brw dest */
{"jbc",		0xe0}, /* bbc dest / bbs .+3; brw dest */
{"jbs",		0xe1}, /* bbs dest / bbc .+3; brw dest */
{"jbcc",	0xe4}, /* bbcc dest / bbsc .+3; brw dest */
{"jbsc",	0xe5}, /* bbsc dest / bbcc .+3; brw dest */
{"jbcs",	0xe2}, /* bbcs dest / bbss .+3; brw dest */
{"jbss",	0xe3}, /* bbss dest / bbcs .+3; brw dest */
{"jlbc",	0xe8}, /* blbc dest / blbs .+3; brw dest */
{"jlbs",	0xe9}  /* blbs dest / blbc .+3; brw dest */
};


gensdi(inst,op1,op2,op3) /* generate span dependent instruction */

register instr *inst;
addrmode op1,op2,op3;

	{ register short iptr;
	 
	  switch (inst->tag) {

	case EXTIN1: {  /* short = 2 bytes; long = 5 bytes */
			/* operands = dest */
			if (opt) {
				switch(shortsdi(op1.adexpr.symptr,
					op1.adexpr.expval.lngval,SDIBR1)) {
				case S_SDI: /* short form guaranteed to fit */
					generate(inst->nbits,GENVALUE,
						 inst->opcode,NULLSYM);
					genopr(op1,1,INSTJB);
					return;
				case U_SDI: /* don't know, put in short form
					       temporarily and check later */
					iptr = 0;
					while (strcmp(inst->name,
						  nsdiop[iptr].name) &&
						  iptr <= 15) iptr++;
					if (iptr == 16) /* uh oh! */
						aerror("cannot find sdi name");
					generate(inst->nbits,SDIINST+iptr,
						 inst->opcode,NULLSYM);
					generate(BITSPBY,SDIDST,
						 op1.adexpr.expval.lngval,
						 op1.adexpr.symptr);
					return;
				case L_SDI: /* fall thru to no optimize case */
					break;
				}
			}
			/* dont optimize, generate long form */
			iptr = 0;
			while (strcmp(inst->name, nsdiop[iptr].name) &&
					iptr <= 15) iptr++;
			if (iptr == 16) /* uh oh! */
				aerror("cannot find sdi name");
			/* generate logically negated instruction */
			generate(BITSPBY,GENVALUE,(long)nsdiop[iptr].nopcode,
								NULLSYM);
			generate(BITSPBY,GENVALUE,3L,NULLSYM); /* .+3 operand */
			generate(BITSPBY,GENVALUE,0x31L,NULLSYM); /* brw inst */
			genopr(op1,1,INSTJW); /* dest operand */
			break;
		     };

	case EXTIN2: {  /* short = 2 bytes; long = 3 bytes */
			/* operands = dest */
			if (opt) {
				switch(shortsdi(op1.adexpr.symptr,
					op1.adexpr.expval.lngval,SDIBR2)) {
				case S_SDI: /* short form guaranteed to fit */
					generate(inst->nbits,GENVALUE,
						 inst->opcode,NULLSYM);
					genopr(op1,1,INSTJB);
					return;
				case U_SDI: /* don't know, put in short form
					       temporarily and check later */
					iptr = 16;
					while (strcmp(inst->name,
						  nsdiop[iptr].name) &&
						  iptr <= 16) iptr++;
					if (iptr == 17) /* uh oh! */
						aerror("cannot find sdi name");
					generate(inst->nbits,SDIINST+iptr,
						 inst->opcode,NULLSYM);
					generate(BITSPBY,SDIDST,
						op1.adexpr.expval.lngval,
						op1.adexpr.symptr);
					return;
				case L_SDI: /* fall thru to no optimize case */
					break;
				}
			}
			/* dont optimize, generate long form */
			iptr = 16;
			while (strcmp(inst->name, nsdiop[iptr].name) &&
					iptr <= 16) iptr++;
			if (iptr == 17) /* uh oh! */
				aerror("cannot find sdi name");
			/* generate logically negated instruction */
			generate(BITSPBY,GENVALUE,(long)nsdiop[iptr].nopcode,
								NULLSYM);
			genopr(op1,1,INSTJW); /* dest operand */
			break;
		     };

	case EXTIN3: {  /* short = 4 bytes; long = 7 bytes */
			/* operands = byte, byte, dest */
			if (opt) {
				switch(shortsdi(op3.adexpr.symptr,
					op3.adexpr.expval.lngval,SDIBR3)) {
				case S_SDI: /* short form guaranteed to fit */
					generate(inst->nbits,GENVALUE,
						 inst->opcode,NULLSYM);
					warnbad++;
					genopr(op1,1,ILU10);
					warnbad = 0;
					noshort++;
					genopr(op2,2,ILU10);
					genopr(op3,3,ILU10);
					noshort = 0;
					return;
				case U_SDI: /* don't know, put in short form
					       temporarily and check later */
					iptr = 17;
					while (strcmp(inst->name,
						  nsdiop[iptr].name) &&
						  iptr <= 22) iptr++;
					if (iptr == 23) /* uh oh! */
						aerror("cannot find sdi name");
					generate(inst->nbits,SDIINST+iptr,
						 inst->opcode,NULLSYM);
					warnbad++;
					genopr(op1,1,ILU10);
					warnbad = 0;
					noshort++;
					genopr(op2,2,ILU10);
					noshort = 0;
					generate(BITSPBY,SDIDST,
						op3.adexpr.expval.lngval,
						op3.adexpr.symptr);
					return;
				case L_SDI: /* fall thru to no optimize case */
					break;
				}
			}
			/* dont optimize, generate long form */
			iptr = 17;
			while (strcmp(inst->name, nsdiop[iptr].name) &&
					iptr <= 22) iptr++;
			if (iptr == 23) /* uh oh! */
				aerror("cannot find sdi name");
			/* generate logically negated instruction */
			generate(BITSPBY,GENVALUE,(long)nsdiop[iptr].nopcode,
								NULLSYM);
			warnbad++;
			genopr(op1,1,ILU10);
			warnbad = 0;
			noshort++;
			genopr(op2,2,ILU10);
			noshort = 0;
			generate(BITSPBY,GENVALUE,3L,NULLSYM); /* .+3 operand */
			generate(BITSPBY,GENVALUE,0x31L,NULLSYM); /* brw inst */
			genopr(op3,3,INSTJW); /* dest operand */
			break;
		     };
	
	case EXTIN4: {  /* short = 3 bytes; long = 6 bytes */
			/* operands = byte, dest */
			if (opt) {
				switch(shortsdi(op2.adexpr.symptr,
					op2.adexpr.expval.lngval,SDIBR4)) {
				case S_SDI: /* short form guaranteed to fit */
					generate(inst->nbits,GENVALUE,
						 inst->opcode,NULLSYM);
					warnbad++;
					genopr(op1,1,ILU11);
					warnbad = 0;
					noshort++;
					genopr(op2,2,ILU11);
					noshort = 0;
					return;
				case U_SDI: /* don't know, put in short form
					       temporarily and check later */
					iptr = 23;
					while (strcmp(inst->name,
						  nsdiop[iptr].name) &&
						  iptr <= 24) iptr++;
					if (iptr == 25) /* uh oh! */
						aerror("cannot find sdi name");
					generate(inst->nbits,SDIINST+iptr,
						 inst->opcode,NULLSYM);
					warnbad++;
					genopr(op1,1,ILU11);
					warnbad = 0;
					generate(BITSPBY,SDIDST,
						op2.adexpr.expval.lngval,
						op2.adexpr.symptr);
					return;
				case L_SDI: /* fall thru to no optimize case */
					break;
				}
			}
			/* dont optimize, generate long form */
			iptr = 23;
			while (strcmp(inst->name, nsdiop[iptr].name) &&
					iptr <= 24) iptr++;
			if (iptr == 25) /* uh oh! */
				aerror("cannot find sdi name");
			/* generate logically negated instruction */
			generate(BITSPBY,GENVALUE,(long)nsdiop[iptr].nopcode,
								NULLSYM);
			warnbad++;
			genopr(op1,1,ILU11);
			warnbad = 0;
			generate(BITSPBY,GENVALUE,3L,NULLSYM); /* .+3 operand */
			generate(BITSPBY,GENVALUE,0x31L,NULLSYM); /* brw inst */
			genopr(op2,2,INSTJW); /* dest operand */
			break;
		     };

	default: aerror("cannot determine sdi type"); /* uh oh! */
		};
	}

