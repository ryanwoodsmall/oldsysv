/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)sdb:u3b2/bits.c	1.3"
#include	<stdio.h>
#include	"dis.h"
#include	"sgs.h"

int errlev = 0;	/* to keep track of errors encountered during	*/
			/* the disassembly, probably due to being out	*/
			/* of sync.					*/

#define		OPLEN	35	/* maximum length of a single operand	*/
				/* (will be used for printing)		*/

static	char	operand[4][OPLEN];	/* to store operands as they	*/
					/* are encountered		*/

static	long	start;			/* start of each instruction	*/
					/* used with jumps		*/

static	int	fpflag;		/* will indicate floating point instruction	*/
				/* (0=NOT FLOATING POINT, 1=SINGLE, 2=DOUBLE) 	*/
				/* so that immediates will be printed in	*/
				/* decimal floating point.			*/
static	int	bytesleft = 0;	/* will indicate how many unread bytes are in buffer */

/*
 *	dis_text ()
 *
 *	disassemble a text section
 */

long
dis_dot(ldot,idsp,fmt)
	long	ldot;	/* current address */
	int	idsp;	/* space: instruction/data */
	char	fmt;	/* format (never used) */
{
	/* the following arrays are contained in tables.c	*/
	extern	struct	instable	opcodetbl[256];
	extern	struct	formtable	adrmodtbl[256];

	/* the following entries are from _extn.c	*/
	extern	int	debug;
	extern	int	trace;
	extern	long	 loc;
	extern	char	mneu[];
	extern	unsigned short	cur1byte;

	/* the following routines are in _utls.c	*/
	extern	int	convert();
	extern	int	get1byte();

	/* the following routines are in this file	*/
	long		getword();
	unsigned short	gethalfword();

	long		lngtmp;
	struct instable	*cp;
	unsigned	key;
	char		ctemp[OPLEN];	/* to store EXTOP operand   */
	unsigned short	wdtmp;

	/* initialization for each beginning of text disassembly	*/

		trace = debug;
		errlev = 0;
		loc = ldot;
		start = ldot;
		sprintf(operand[0],"");
		sprintf(operand[1],"");
		sprintf(operand[2],"");
		sprintf(operand[3],"");
		fpflag = NOTFLOAT;	/* assume immediates are not floating
					 * point , unless floating point 
					 * operand is disassembled.
					 */

		/* key is the one byte op code		*/
		/* cp is the op code Class Pointer	*/

		get1byte(idsp);
		key = cur1byte;
		cp = &opcodetbl[key];

		/* print the mnemonic */
		sprintf(mneu,"%-8s",cp -> name);
		if (trace > 2) printf("\ninst class = %d  ",cp->class);
		/*
		 * Each instruction has been assigned a classby the author.
		 * Individual classes are explained as they are encountered in 
		 * the following switch construct.
		 */

		switch(cp -> class){

		/* This class handles instructions that wish to ignore	*/
		/* 8 bits of insignificant data				*/
		case	NOOP8:
			get1byte(idsp);
			break;

		/* This class handles instructions that wish to ignore	*/
		/* 16 bits of insignificant data			*/
		case	NOOP16:
			gethalfword(idsp);
			break;

		/* This case handles the EXTOP instruction for the	*/
		/* m32a simulator i/o routines.				*/
		case	EXT:
			get1byte(idsp);
			convert(cur1byte,ctemp,LEAD);
			sprintf(mneu,"%s%s",mneu,ctemp);
			break;

		/* This class handles instructions with no operands	*/
		case	OPRNDS0:
			break;

		/* This class handles instructions with 1 operand	*/
		/* that is described by a preceeding discriptor		*/
		case	OPRNDS1:
		case	JUMP:
			get_operand(operand[0],idsp);
			sprintf(mneu,"%s%s",mneu,operand[0]);
			break;

		/* This class handles instructions with 2 operands	*/
		/* that are each described by a preceeding discriptor	*/
		case	OPRNDS2:
			get_operand(operand[0],idsp);
			get_operand(operand[1],idsp);
			sprintf(mneu,"%s%s,%s",mneu,operand[0],operand[1]);
			break;

		/* This class handles instructions with 3 operands	*/
		/* that are each described by a preceeding discriptor	*/
		case	OPRNDS3:
			get_operand(operand[0],idsp);
			get_operand(operand[1],idsp);
			get_operand(operand[2],idsp);
			sprintf(mneu,"%s%s,%s,%s",mneu,operand[0],
				operand[1],operand[2]);
			break;

		/* This class handles instructions with 4 operands	*/
		/* that are each described by a preceeding discriptor	*/
		case	OPRNDS4:
			get_operand(operand[0],idsp);
			get_operand(operand[1],idsp);
			get_operand(operand[2],idsp);
			get_operand(operand[3],idsp);
			sprintf(mneu,"%s%s,%s,%s,%s",mneu,operand[0],
				operand[1],operand[2],operand[3]);
			break;
 
		/* This class handles jump instructions that have a one	*/
		/* byte offset (not an ordinary operand)		*/
		case	JUMP1:
			get_bjmp_oprnd(operand[0],idsp);
			sprintf(mneu,"%s%s",mneu,operand[0]);
			break;

		/* This class handles jump instructions that have a two	*/
		/* byte offset (not an ordinary operand)		*/
		case	JUMP2:
			get_hjmp_oprnd(operand[0],idsp);
			sprintf(mneu,"%s%s",mneu,operand[0]);
			break;

		/* This class handles single precision floating point 	*/
		/* instructions with two operands.			*/
		case	SFPOPS2:
			fpflag = FPSINGLE;
			get_operand(operand[0],idsp);
			get_operand(operand[1],idsp);
			sprintf(mneu,"%s%s,%s",mneu,operand[0],operand[1]);
			break;

		/* This class handles single precision floating point	*/
		/* instructions with three operands.			*/
		case	SFPOPS3:
			fpflag = FPSINGLE;
			get_operand(operand[0],idsp);
			get_operand(operand[1],idsp);
			get_operand(operand[2],idsp);
			sprintf(mneu,"%s%s,%s,%s",mneu,operand[0],
				operand[1],operand[2]);
			break;

		/* This class handles double precision floating point 	*/
		/* instructions with two operands.			*/
		case	DFPOPS2:
			fpflag = FPDOUBLE;
			get_operand(operand[0],idsp);
			get_operand(operand[1],idsp);
			sprintf(mneu,"%s%s,%s",mneu,operand[0],operand[1]);
			break;

		/* This class handles double precision floating point	*/
		/* instructions with three operands.			*/
		case	DFPOPS3:
			fpflag = FPDOUBLE;
			get_operand(operand[0],idsp);
			get_operand(operand[1],idsp);
			get_operand(operand[2],idsp);
			sprintf(mneu,"%s%s,%s,%s",mneu,operand[0],
				operand[1],operand[2]);
			break;
		/* an invalid op code (there aren't too many)	*/
		case UNKNOWN:
			sprintf(mneu,"***ERROR--unknown op code***");
			errlev++;		/* stop eventually.  */
			break;

		/* This special case handles the macro rom instructions */
		case MACRO:
			get_macro(operand[0],idsp);
			sprintf(mneu,"%s",operand[0]);
			break;

		/* Support Processor Instruction  "SPOP";  Opcode is 	*/
		/* followed by 32 bit "id" similar to an immediate word */
		/* but has no byte descriptor.				*/
		case SPRTOP0:
			lngtmp = getword(idsp);
			sprintf(mneu,"%s&",mneu);
/*
			if (sflag)  {
				sprintf(symrep,"%s&",symrep);
				longprint(symrep,symrep,lngtmp);
			}
*/
			longprint(mneu,mneu,lngtmp);
			break;
		/* Support Processor Instructions "SPOPRS", "SPOPRD",  	*/
		/* "SPOPRT", "SPOPWS", "SPOPWD", and "SPOPWT".  The	*/
		/* opcode is followed by a 32 bit support processor "id"*/
		/* similar to an immediate word by it has no byte 	*/
		/* descriptor. Following the "id" is one normal operand	*/
		/* which has a byte descriptor.				*/
		case SPRTOP1:
			lngtmp = getword(idsp);
			sprintf(mneu,"%s&",mneu);
			longprint(mneu,mneu,lngtmp);
			get_operand(operand[0],idsp);
			sprintf(mneu,"%s, %s",mneu,operand[0]);
/*
			if (sflag) {
				sprintf(symrep,"%s&",symrep);
				longprint(symrep,symrep,lngtmp);
				sprintf(symrep,"%s, %s",symrep,idsp);
			}
*/
			break;
		/* Support Processor Instructions "SPOPS2", "SPOPD2",  	*/
		/* "SPOPT2".  These are two operand instructions.  The	*/
		/* opcode is followed by a 32 bit support processor "id"*/
		/* similar to an immediate word by it has no byte 	*/
		/* descriptor. Following the "id" is one normal operand	*/
		/* which has a byte descriptor.				*/
		case SPRTOP2:
			lngtmp = getword(idsp);
			sprintf(mneu,"%s &",mneu);
			longprint(mneu,mneu,lngtmp);
			get_operand(operand[0],idsp);
			get_operand(operand[1],idsp);
			sprintf(mneu,"%s, %s, %s",mneu,operand[0],operand[1]);
/*
			if (sflag) {
				sprintf(symrep,"%s &",symrep);
				longprint(symrep,symrep,lngtmp);
				sprintf(symrep,"%s, %s, %s",symrep,idsp,idsp);
			}
*/
			break;
		default:
			printf("\n%sdis bug: notify implementor:",SGS);
			printf(" case from instruction table not found\n");
			errlev++;
			break;
		} /* end switch */

	return(loc);
}
/*
 *	get_operand ()
 *
 *	Determine the type of address mode for this operand and
 *	place the resultant operand text to be printed in the array
 *	whose address was passed to this function
 */

get_operand(result,idsp)
char	*result;
int	idsp;
{
	extern struct formtable adrmodtbl[256];	/* from tables.c	*/
	extern	int	compoff();		/* in _utls.c	*/
	extern	int	sfpconv();		/* in _utls.c	*/
	extern	int	dfpconv();		/* in _utls.c	*/
	extern	int	oflag;			/* in _extn.c	*/
	extern	int	trace;			/* in _extn.c	*/
	extern	unsigned short	cur1byte;	/* in _extn.c	*/

	/* the following routines are in this file	*/
	long	getword();
	unsigned short	gethalfword();

	struct	formtable  *tp;
	char	temp[OPLEN];
	char	*temp1;
	char	*temp2;
	long	lngtmp, ltemp;		/* temporary variables */
	unsigned short 	wdtmp;		/* a temporary variable */
	double		fpnum;		/* used to hold decimal representation	*/
					/* of floating point immediate operand.	*/
	int		fptype;
	short		fpbigex;	/* used to hold exponent of floating	*/
					/* point number that is too large for	*/
					/* pdp11 or vax.			*/

	/* "tp" is the Type Pointer, pointing to the entry	*/
	/* in the address modes table.				*/
	/* "temp2" holds the pointer to the string from the	*/
	/* address modes table that will be printed		*/
	/* as part or all of the current operand.		*/

	get1byte(idsp);
	tp = &adrmodtbl[cur1byte];
	temp2 = tp -> name;

	if (trace > 0)
		printf("\nadr mod typ = %x\n",tp->typ);

	switch (tp -> typ){

	/* type = immediate numbers 0 to 63 (IM6)		*/
	/* type = immediate numbers -16 to -1 (IM4)		*/
	/* type = register (R) and register deferred (RDF)	*/
	/* type = register and displacement of 0 to 14 (R4)	*/
	/* For this type the address modes table holds		*/
	/* the entire text for this operand.			*/
	case	IM4:
	case	IM6:
	case	R:
	case	RDF:
	case	R4:
		sprintf(result,"%s",temp2);
		return;

	/* type = immediate operand in following byte (IMB)	*/
	/* For this type the next byte is read, converted for	*/
	/* printing and preceded by an "&".			*/
	case	IMB:
		get1byte(idsp);
		convert(cur1byte,temp,LEAD);
		sprintf(result,"&%s",temp);
		return;

	/* type = register + byte displacement (BDB)		*/
	/* For this type the next byte is read and converted	*/
	/* and prepended to the register name taken from the	*/
	/* address modes teable.				*/
	case	BDB:
		get1byte(idsp);
		convert(cur1byte,temp,LEAD);
		sprintf(result,"%s%s",temp,temp2);
		return;

	/* type = register + byte displacement deferred (BDBDF)	*/
	/* Same as BDB except an additional "*" is prepended.	*/
	case	BDBDF:
		get1byte(idsp);
		convert(cur1byte,temp,LEAD);
		sprintf(result,"*%s%s",temp,temp2);
		return;

	/* type = immediate operand in following half word (IMH)	*/
	/* For this type the next half word is read, converted for	*/
	/* printing and preceded by an "&".				*/
	case	IMH:
		wdtmp = gethalfword(idsp);
		convert(wdtmp,temp,LEAD);
		sprintf(result,"&%s",temp);
		return;

	/* type = register + half word displacement (BDH)		*/
	/* For this type the next half word is read and converted	*/
	/* and prepended to the register name taken from the		*/
	/* address modes table.						*/
	case	BDH:
		wdtmp = gethalfword(idsp);
		convert(wdtmp,temp,LEAD);
		sprintf(result,"%s%s",temp,temp2);
		return;

	/* type = register + half word displacement deferred (BDHDF)	*/
	/* Same as BDH except an additional "*" is prepended.		*/
	case	BDHDF:
		wdtmp = gethalfword(idsp);
		convert(wdtmp,temp,LEAD);
		sprintf(result,"*%s%s",temp,temp2);
		return;

	/* type = immediate operand in following word (IMW)		*/
	/* type = immediate operand in following doubleword (IMD)	*/
	/* NOTE: immediate words may be floating point or nonfloating	*/
	/*       point. Doubleword immediates are floating point.	*/
	/* For this type the next word is read, converted for		*/
	/* printing and preceded by an "&".				*/
	case	IMD:
	case	IMW:
		lngtmp = getword(idsp);
		switch (fpflag) {

		case	NOTFLOAT:
			longprint(result,"&",lngtmp);
			return;
		case	FPSINGLE:
			fptype = sfpconv(lngtmp,  &fpnum);
			break;
		case	FPDOUBLE:
			ltemp = getword(idsp);
			fptype = dfpconv(lngtmp, ltemp, &fpnum, &fpbigex);
			break;

		} /* switch (fpflag)	*/
		/* fptype is the return value of the floating point 
		 * conversion routines: sfpconv() and dfpconv().
		 */
		switch (fptype) {

		case	NOTANUM:
			sprintf (result, "INVALID FP NUMBER");
			break;
		case	NEGINF:
			sprintf (result, "NEGATIVE INFINITY");
			break;
		case	INFINITY:
			sprintf (result, "INFINITY");
			break;
		case	ZERO:
			sprintf (result, "&0.0");
			break;
		case	NEGZERO:
			sprintf (result, "&-0.0");
			break;
		case	FPNUM:
			sprintf (result,"&%.15e", fpnum);
			break;
		case	FPBIGNUM:
			sprintf(result, "&%.13fE%d{RADIX 2}", 
				fpnum, fpbigex);
			break;

		} /* switch (fptype)	*/
		return;

	/* type = register + word displacement (BDW)		*/
	/* type = absolute address (ABAD)			*/
	/* For this type the next word is read and converted	*/
	/* and prepended to the register name (for BDW) or	*/
	/* the null string (for ABAD) from the address modes	*/
	/* table.						*/
	case	BDW:
	case	ABAD:
		lngtmp = getword(idsp);
		longprint(result,"",lngtmp);
		sprintf(result,"%s%s",result,temp2);
		return;

	/* type = register + word displacement deferred (BDWDF)	*/
	/* type = absolute address deferred (ABADDF)		*/
	/* Same as BDW and ABAD except an additional "*" is 	*/
	/* prepended.						*/
	case	BDWDF:
	case	ABADDF:
		lngtmp = getword(idsp);
		longprint(result,"*",lngtmp);
		sprintf(result,"%s%s",result,temp2);
		return;

	/* type = expanded operand			*/
	/* Data from the address modes table indicates	*/
	/* {Signed/Unsigned BYTE/HALFword/WORD}		*/
	/* followed by the operand.			*/
	case	EXPSB:
	case	EXPUB:
	case	EXPSH:
	case	EXPUH:
	case	EXPSW:
	case	EXPUW:
		get_operand(temp);
		sprintf(result,"%s%s",temp2,temp);
		return;

	default:
		printf("\n%sdis: invalid general addressing mode (%x)\n",
			SGS, cur1byte);
		printf("\tnotify disassembler implementor\n");
	}	/* end switch */
}
/*
 *	get_bjmp_oprnd (result)
 *
 *	Get the next byte as a jump displacement operand.
 *	Call pr_bjmp_oprnd() to convert the characters, calculate the 
 *	jump address and prepare for printing.
 *
 */

get_bjmp_oprnd(result,idsp)
char	*result;
int	idsp;
{
	extern	unsigned  short  cur1byte;
	extern	long	loc;
	extern	long	start;

	int	tmpint;
	long	tmplng;

	get1byte(idsp);
	convert(cur1byte,result,LEAD);
	tmpint = (cur1byte < 0x80) ? (int) cur1byte : (int) cur1byte | ~0xff;
	tmplng = start - loc + (long) tmpint;
	compoff(tmplng,result);
	return(tmplng);
}
/*
 *	get_macro (result)
 *
 *	Get the next byte and use that as the index of the
 *	macro rom instruction.
 *
 */

get_macro(result,idsp)
char	*result;
int	idsp;
{
	extern	unsigned  short  cur1byte;
	extern	int	oflag;
	static struct romlist {
		short	romcode;
		char	*romstring;
	} romlist[] = {
		0x9,"MVERNO",
		0xd,"ENBVJMP",
		0x13,"DISVJMP",
		0x19,"MOVBLW",
		0x1f,"STREND",
		0x35,"STRCPY",
		0x3c,"SLFTST",
		0x45,"RETG",
		0x61,"GATE",
		0xac,"CALLPS",
		0xc8,"RETPS",
		0,""
	};
	register struct romlist *rpt;

	get1byte(idsp);
	for( rpt = romlist; rpt->romcode != 0; rpt++) {
		if( cur1byte == rpt->romcode) {
			strcpy(result, rpt->romstring);
			return;
		}
	}
	sprintf(result,"MACRO %X",cur1byte);
	return;
}

/*
 *	get_hjmp_oprnd (result)
 *
 *	Get the next half word as a jump displacement operand.
 *	Call pr_hjmp_oprnd() to convert the characters, calculate the
 *	jump address and prepare for printing.
 *
 */

get_hjmp_oprnd(result,idsp)
char	*result;
int	idsp;
{
	unsigned short	gethalfword();
	extern	long	loc;
	extern	long	start;

	int	tmpint;
	long	tmplng;

	tmpint = (int) gethalfword(idsp);
	convert(tmpint,result,LEAD);
	tmplng = start - loc + (long) tmpint;
	compoff(tmplng,result);
	return(tmplng);
}

/*
 *	gethalfword()
 *
 *	This routine will read the next 2 bytes in the object file from
 *	the buffer (filling the 4 byte buffer if necessary).
 */
unsigned short
gethalfword(idsp)
int	idsp;
{
	extern	unsigned short	cur1byte;
	extern	int	trace;
	union {
		unsigned short 	half;
		char		bytes[2];
	} curhalf;

	curhalf.half = 0;
	get1byte(idsp);
	curhalf.bytes[1] = cur1byte;
	get1byte(idsp);
	curhalf.bytes[0] = cur1byte;
	return(curhalf.half);
}

/*
 *	getword()
 *	This routine will read the next 4 bytes in the object file from
 *	the buffer (filling the 4 byte buffer if necessary).
 *
 */
long
getword(idsp)
int	idsp;
{
	extern unsigned short	cur1byte;
	union {
		char	bytes[4];
		long	word;
	} curword;

	curword.word = 0;
	get1byte(idsp);
	curword.bytes[3] = cur1byte;
	get1byte(idsp);
	curword.bytes[2] = cur1byte;
	get1byte(idsp);
	curword.bytes[1] = cur1byte;
	get1byte(idsp);
	curword.bytes[0] = cur1byte;
	return(curword.word);
}

/*
 * 	longprint
 *	simply a routine to print a long constant with an optional
 *	prefix string such as "*" or "$" for operand descriptors
 */
longprint(result,prefix,value)
char	*result;
char	*prefix;
long	value;
{
	extern	int	oflag;

	if(oflag){
		sprintf(result,"%s0%lo",prefix,value);
		}
	else{
		sprintf(result,"%s0x%lx",prefix,value);
		}
	return;
}
