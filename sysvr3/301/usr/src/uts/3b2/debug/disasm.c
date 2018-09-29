/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)kern-port:debug/disasm.c	10.8"
/*
 */

#include	"sys/types.h"
#include	"sys/inline.h"
#include	"sys/cmn_err.h"
#include	"sys/sys3b.h"
#include	"sys/disasm.h"
#include	"sys/sgs.h"


opnd_t	dis_opnd[4];		/* to store operands as they	*/
				/* are encountered		*/

long	dis_start;		/* start of each instruction	*/
				/* used with jumps		*/

int	dis_fpflag;		/* Indicates floating point instruction	*/
				/* (0=NOT FLOATING POINT, 1=SINGLE,	*/
				/* 2=DOUBLE)				*/
				/* so that immediates will be printed	*/
				/* in decimal floating point.		*/
long	dis_loc;		/* Byte location in instruction being	*/
				/* disassembled.  Incremented by the	*/
				/* get1byte routine.			*/
char	dis_line[NLINE];	/* Array to store printable version of	*/
				/* line as it is disassembled.		*/
ushort	cur1byte;		/* The last byte fetched by get1byte.	*/
char	dis_relopnd;		/* Count of the number of operands for	*/
				/* which a relative form of the value	*/
				/* should be printed.			*/

char	blanks[] = "                                                    ";

int	disaddr;
int	discnt;
int	disabsflg;
long	disasmi();

disasm()
{
	asm("	PUSHW  %r0");
	asm("	PUSHW  %r1");
	asm("	PUSHW  %r2");

	ndisasm(disaddr, discnt, disabsflg);

	asm("	POPW  %r2");
	asm("	POPW  %r1");
	asm("	POPW  %r0");
}

ndisasm(addr, count, absflg)
register uint	addr;
register uint	count;
uint		absflg;
{
	register int	ii;
	register uint	nxtaddr;
	register char	*s3bsp;
	int		delay;
	int		oldpri;
	char		addrbuf[NLINE];
	char		*s3blookup();
	extern int	dmd_delay;

	oldpri = splhi();
	while (count--) {
		nxtaddr = disasmi(addr);
		if (absflg) {
			sprintf(addrbuf, "%.8x", addr);
			cmn_err(CE_CONT, "^%s:  ", addrbuf);
			addrbuf[0] = '\0';
			dis_loc = addr;
			for (ii = 0  ;
			    ii < 10  ||  addr + ii < nxtaddr  ;  ii++) {
				if (addr + ii < nxtaddr) {
					get1byte();
					sprintf(addrbuf, "%s%.2x ",
						addrbuf, cur1byte);
				} else {
					sprintf(addrbuf, "%s   ",
						addrbuf);
				}
			}
			cmn_err(CE_CONT, "^%s ", addrbuf);
		} else {
			s3bsp = s3blookup(addr);
			sprintf(addrbuf, "%s+%#x", s3bsp,
				addr - S3BSVAL(s3bsp));
			cmn_err(CE_CONT, "^%s", addrbuf);
			ii = 20 - strlen(addrbuf);
			sprintf(addrbuf, "%.*s", ii, blanks);
			cmn_err(CE_CONT, "^%s", addrbuf);
		}

		cmn_err(CE_CONT, "^%s", dis_line);

		if (!absflg && dis_relopnd) {
			int	val;

			ii = strlen(dis_line);
			if (ii < 30)
				sprintf(addrbuf, "%.*s[", 30 - ii,
					blanks);
			else
				strcpy(addrbuf, " [");
			for (ii = 0  ;  dis_relopnd  ;  ii++) {
				if (val = dis_opnd[ii].opnd_val) {
					s3bsp = s3blookup(val);
					sprintf(addrbuf, "%s%s+%#x",
						addrbuf, s3bsp,
						val - S3BSVAL(s3bsp));
					if (--dis_relopnd)
						strcat(addrbuf, ", ");
				}
			}

			cmn_err(CE_CONT, "^%s]", addrbuf);
		}

		cmn_err(CE_CONT, "^\n");
		addr = nxtaddr;
		if (dmd_delay) {
			delay = dmd_delay;
			while (delay--) ;
		}
	}
	splx(oldpri);
}

/*
 *	disasm ()
 *
 *	disassemble one instruction.
 */

long
disasmi(addr)
uint	addr;	/* current address */
{
	/* the following arrays are contained in tables.c	*/

	extern	struct	instable	opcodetbl[256];
	extern	struct	formtable	adrmodtbl[256];


	/* the following routines are in utils.c	*/

	extern	int	convert();
	extern	int	get1byte();

	/* the following routines are in this file	*/

	long		getword();
	unsigned short	gethalfword();

	register int			ii;
	register struct instable	*cp;
	register unsigned		key;
	register long			lngtmp;

	/* to store EXTOP operand   */

	opnd_t				ctemp;

	/* initialization for each beginning of text disassembly	*/

	dis_loc = addr;
	dis_start = addr;
	dis_relopnd = 0;

	for (ii = 0  ;  ii < 4  ;  ii++) {
		dis_opnd[ii].opnd_sym[0] = '\n';
		dis_opnd[ii].opnd_val = 0;
	}

	dis_fpflag = NOTFLOAT;	/* assume immediates are not floating
				 * point , unless floating point 
				 * operand is disassembled.
				 */

	/* key is the one byte op code		*/
	/* cp is the op code Class Pointer	*/

	get1byte();
	key = cur1byte;
	cp = &opcodetbl[key];

	/* print the mnemonic */

	sprintf(dis_line, "%-8s", cp -> name);

	/*
	 * Each instruction has been assigned a classby the author.
	 * Individual classes are explained as they are encountered
	 * in the following switch construct.
	 */

	switch (cp -> class) {

		/* This class handles instructions that wish to ignore	*/
		/* 8 bits of insignificant data				*/

		case	NOOP8:
			get1byte();
			break;

		/* This class handles instructions that wish to ignore	*/
		/* 16 bits of insignificant data			*/

		case	NOOP16:
			gethalfword();
			break;

		/* This case handles the EXTOP instruction for the	*/
		/* m32a simulator i/o routines.				*/

		case	EXT:
			get1byte();
			convert(cur1byte, ctemp.opnd_sym, LEAD);
			sprintf(dis_line, "%s%s", dis_line,
				ctemp.opnd_sym);
			break;

		/* This class handles instructions with no operands	*/

		case	OPRNDS0:
			break;

		/* This class handles instructions with 1 operand	*/
		/* that is described by a preceeding descriptor		*/

		case	OPRNDS1:
		case	JUMP:
			
			get_operand(&dis_opnd[0]);
			sprintf(dis_line, "%s%s", dis_line,
				dis_opnd[0].opnd_sym);
			break;

		/* This class handles instructions with 2 operands	*/
		/* that are each described by a preceeding descriptor	*/

		case	OPRNDS2:
			get_operand(&dis_opnd[0]);
			get_operand(&dis_opnd[1]);
			sprintf(dis_line, "%s%s,%s", dis_line,
				dis_opnd[0].opnd_sym,
				dis_opnd[1].opnd_sym);
			break;

		/* This class handles instructions with 3 operands	*/
		/* that are each described by a preceeding descriptor	*/

		case	OPRNDS3:
			get_operand(&dis_opnd[0]);
			get_operand(&dis_opnd[1]);
			get_operand(&dis_opnd[2]);
			sprintf(dis_line, "%s%s,%s,%s",
				dis_line, dis_opnd[0].opnd_sym,
				dis_opnd[1].opnd_sym,
				dis_opnd[2].opnd_sym);
			break;

		/* This class handles instructions with 4 operands	*/
		/* that are each described by a preceeding descriptor	*/

		case	OPRNDS4:
			get_operand(&dis_opnd[0]);
			get_operand(&dis_opnd[1]);
			get_operand(&dis_opnd[2]);
			get_operand(&dis_opnd[3]);
			sprintf(dis_line, "%s%s,%s,%s,%s", dis_line,
				dis_opnd[0].opnd_sym,
				dis_opnd[1].opnd_sym,
				dis_opnd[2].opnd_sym,
				dis_opnd[3].opnd_sym);
			break;
 
		/* This class handles jump instructions that have a one	*/
		/* byte offset (not an ordinary operand)		*/

		case	JUMP1:
			get_bjmp_oprnd(&dis_opnd[0]);
			sprintf(dis_line, "%s%s", dis_line,
				dis_opnd[0].opnd_sym);
			break;

		/* This class handles jump instructions that have a two	*/
		/* byte offset (not an ordinary operand)		*/

		case	JUMP2:
			get_hjmp_oprnd(&dis_opnd[0]);
			sprintf(dis_line, "%s%s", dis_line,
				dis_opnd[0].opnd_sym);
			break;

		/* This class handles single precision floating point 	*/
		/* instructions with two operands.			*/
		case	SFPOPS2:

			dis_fpflag = FPSINGLE;
			get_operand(&dis_opnd[0]);
			get_operand(&dis_opnd[1]);
			sprintf(dis_line, "%s%s,%s", dis_line,
				dis_opnd[0].opnd_sym,
				dis_opnd[1].opnd_sym);
			break;

		/* This class handles single precision floating point	*/
		/* instructions with three operands.			*/

		case	SFPOPS3:
			dis_fpflag = FPSINGLE;
			get_operand(&dis_opnd[0]);
			get_operand(&dis_opnd[1]);
			get_operand(&dis_opnd[2]);
			sprintf(dis_line, "%s%s,%s,%s", dis_line,
				dis_opnd[0].opnd_sym,
				dis_opnd[1].opnd_sym,
				dis_opnd[2].opnd_sym);
			break;

		/* This class handles double precision floating point 	*/
		/* instructions with two operands.			*/
		case	DFPOPS2:

			dis_fpflag = FPDOUBLE;
			get_operand(&dis_opnd[0]);
			get_operand(&dis_opnd[1]);
			sprintf(dis_line, "%s%s,%s", dis_line,
				dis_opnd[0].opnd_sym,
				dis_opnd[1].opnd_sym);
			break;

		/* This class handles double precision floating point	*/
		/* instructions with three operands.			*/

		case	DFPOPS3:
			dis_fpflag = FPDOUBLE;
			get_operand(&dis_opnd[0]);
			get_operand(&dis_opnd[1]);
			get_operand(&dis_opnd[2]);
			sprintf(dis_line, "%s%s,%s,%s", dis_line,
				dis_opnd[0].opnd_sym,
				dis_opnd[1].opnd_sym,
				dis_opnd[2].opnd_sym);
			break;

		/* Support Processor Instruction  "SPOP";  Opcode is 	*/
		/* followed by 32 bit "id" similar to an immediate word */
		/* but has no byte descriptor.				*/

		case SPRTOP0:
			lngtmp = getword();
			strcat(dis_line, "&");
			longprint(dis_line, dis_line, lngtmp);
			break;

		/* Support Processor Instructions "SPOPRS", "SPOPRD",  	*/
		/* "SPOPRT", "SPOPWS", "SPOPWD", and "SPOPWT".  The	*/
		/* opcode is followed by a 32 bit support processor "id"*/
		/* similar to an immediate word by it has no byte 	*/
		/* descriptor. Following the "id" is one normal operand	*/
		/* which has a byte descriptor.				*/

		case SPRTOP1:
			lngtmp = getword();
			strcat(dis_line, "&");
			longprint(dis_line, dis_line, lngtmp);
			get_operand(&dis_opnd[0]);
			sprintf(dis_line, "%s, %s", dis_line,
				dis_opnd[0].opnd_sym);
			break;

		/* Support Processor Instructions "SPOPS2", "SPOPD2",  	*/
		/* "SPOPT2".  These are two operand instructions.  The	*/
		/* opcode is followed by a 32 bit support processor "id"*/
		/* similar to an immediate word by it has no byte 	*/
		/* descriptor. Following the "id" is one normal operand	*/
		/* which has a byte descriptor.				*/

		case SPRTOP2:
			lngtmp = getword();
			strcat(dis_line, "&");
			longprint(dis_line, dis_line, lngtmp);
			get_operand(&dis_opnd[0]);
			get_operand(&dis_opnd[1]);
			sprintf(dis_line, "%s, %s, %s", dis_line,
				dis_opnd[0].opnd_sym,
				dis_opnd[1].opnd_sym);
			break;

		/* an invalid op code (there aren't too many)	*/

		case UNKNOWN:
			sprintf(dis_line, "***ERROR--unknown op code***");
			break;

		/* This special case handles the macro rom instructions */

		case MACRO:
			get_macro(&dis_opnd[0]);
			sprintf(dis_line,"%s", dis_opnd[0].opnd_sym);
			break;

		default:
			cmn_err(CE_NOTE,
				"Disasm error - instruction not found.\n");
			break;

	} /* end switch */

	return(dis_loc);
}

/*
 *	get_operand ()
 *
 *	Determine the type of address mode for this operand and
 *	place the resultant operand text to be printed in the array
 *	whose address was passed to this function
 */

get_operand(result)
opnd_t	*result;
{
	extern struct formtable adrmodtbl[256];	/* from tables.c	*/
	extern	int	compoff();		/* in utils.c	*/
	extern	int	sfpconv();		/* in utils.c	*/
	extern	int	dfpconv();		/* in utils.c	*/

	/* the following routines are in this file	*/

	long	getword();
	unsigned short	gethalfword();

	struct	formtable	*tp;
	opnd_t			temp;
	char			*temp2;
	long			lngtmp;	/* temporary variable	*/
	long			ltemp;	/* temporary variable	*/
	ushort			wdtmp;	/* temporary variable	*/
	double			fpnum;	/* used to hold decimal	*/
					/* decimal		*/
					/* representation of	*/
					/* floating point	*/
					/* immediate operand.	*/
	int		fptype;
	short		fpbigex;	/* used to hold		*/
					/* exponent of floating	*/
					/* point number that is	*/
					/* too large for	*/
					/* pdp11 or vax.	*/

	/* "tp" is the Type Pointer, pointing to the entry	*/
	/* in the address modes table.				*/
	/* "temp2" holds the pointer to the string from the	*/
	/* address modes table that will be printed		*/
	/* as part or all of the current operand.		*/

	get1byte();
	tp = &adrmodtbl[cur1byte];
	temp.opnd_sym[0] = '\0';
	temp.opnd_val = 0;
	temp2 = tp -> name;

	switch (tp -> typ) {

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
		sprintf(result->opnd_sym, "%s", temp2);
		return;

	/* type = immediate operand in following byte (IMB)	*/
	/* For this type the next byte is read, converted for	*/
	/* printing and preceded by an "&".			*/

	case	IMB:
		get1byte();
		convert(cur1byte, temp.opnd_sym, LEAD);
		sprintf(result->opnd_sym,"&%s", temp.opnd_sym);
		return;

	/* type = register + byte displacement (BDB)		*/
	/* For this type the next byte is read and converted	*/
	/* and prepended to the register name taken from the	*/
	/* address modes teable.				*/

	case	BDB:
		get1byte();
		convert(cur1byte, temp.opnd_sym, LEAD);
		sprintf(result->opnd_sym, "%s%s",
			temp.opnd_sym, temp2);
		return;

	/* type = register + byte displacement deferred (BDBDF)	*/
	/* Same as BDB except an additional "*" is prepended.	*/

	case	BDBDF:
		get1byte();
		convert(cur1byte, temp.opnd_sym, LEAD);
		sprintf(result->opnd_sym, "*%s%s",
			temp.opnd_sym, temp2);
		return;

	/* type = immediate operand in following half word (IMH)	*/
	/* For this type the next half word is read, converted for	*/
	/* printing and preceded by an "&".				*/

	case	IMH:
		wdtmp = gethalfword();
		convert(wdtmp, temp.opnd_sym, LEAD);
		sprintf(result->opnd_sym, "&%s", temp.opnd_sym);
		return;

	/* type = register + half word displacement (BDH)		*/
	/* For this type the next half word is read and converted	*/
	/* and prepended to the register name taken from the		*/
	/* address modes table.						*/

	case	BDH:
		wdtmp = gethalfword();
		convert(wdtmp, temp.opnd_sym, LEAD);
		sprintf(result->opnd_sym, "%s%s", temp.opnd_sym, temp2);
		return;

	/* type = register + half word displacement deferred (BDHDF)	*/
	/* Same as BDH except an additional "*" is prepended.		*/

	case	BDHDF:
		wdtmp = gethalfword();
		convert(wdtmp, temp.opnd_sym, LEAD);
		sprintf(result->opnd_sym, "*%s%s",
			temp.opnd_sym, temp2);
		return;

	/* type = immediate operand in following word (IMW)		*/
	/* type = immediate operand in following doubleword (IMD)	*/
	/* NOTE: immediate words may be floating point or nonfloating	*/
	/*       point. Doubleword immediates are floating point.	*/
	/* For this type the next word is read, converted for		*/
	/* printing and preceded by an "&".				*/

	case	IMD:
	case	IMW:
		lngtmp = getword();
		switch (dis_fpflag) {

		case	NOTFLOAT:
			longprint(result->opnd_sym, "&", lngtmp);
			result->opnd_val = lngtmp;
			dis_relopnd++;
			return;

		case	FPSINGLE:
			/* nofloat
			fptype = sfpconv(lngtmp, &fpnum);
			*/
			longprint(result->opnd_sym, "&", lngtmp);
			break;

		case	FPDOUBLE:
			/* nofloat
			ltemp = getword();
			fptype = dfpconv(lngtmp, ltemp, &fpnum, &fpbigex);
			*/
			longprint(result->opnd_sym, "&", lngtmp);
			lngtmp = getword();
			sprintf(result->opnd_sym, "%s%lx",
				result->opnd_sym, lngtmp);
			break;

		} /* switch (dis_fpflag)	*/

		/* fptype is the return value of the floating point 
		 * conversion routines: sfpconv() and dfpconv().
		 */

		switch (fptype) {

		case	NOTANUM:
			sprintf (result->opnd_sym, "INVALID FP NUMBER");
			break;
		case	NEGINF:
			sprintf (result->opnd_sym, "NEGATIVE INFINITY");
			break;
		case	INFINITY:
			sprintf (result->opnd_sym, "INFINITY");
			break;
		case	ZERO:
			sprintf (result->opnd_sym, "&0.0");
			break;
		case	NEGZERO:
			sprintf (result->opnd_sym, "&-0.0");
			break;
		case	FPNUM:
			sprintf (result->opnd_sym, "&%.15e", fpnum);
			break;
		case	FPBIGNUM:
			sprintf(result->opnd_sym, "&%.13fE%d{RADIX 2}", 
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
		lngtmp = getword();
		longprint(result->opnd_sym, "", lngtmp);
		sprintf(result->opnd_sym, "%s%s", result->opnd_sym, temp2);
		result->opnd_val = lngtmp;
		dis_relopnd++;
		return;

	/* type = register + word displacement deferred (BDWDF)	*/
	/* type = absolute address deferred (ABADDF)		*/
	/* Same as BDW and ABAD except an additional "*" is 	*/
	/* prepended.						*/
	
	case	BDWDF:
	case	ABADDF:
		lngtmp = getword();
		longprint(result->opnd_sym, "*", lngtmp);
		sprintf(result->opnd_sym, "%s%s", result->opnd_sym, temp2);
		result->opnd_val = lngtmp;
		dis_relopnd++;
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
		get_operand(&temp);
		sprintf(result->opnd_sym, "%s%s", temp2,
			temp.opnd_sym);
		result->opnd_val = temp.opnd_val;
		return;

	default:
		cmn_err(CE_NOTE,
			"Disasm - invalid general addressing mode.\n");

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

get_bjmp_oprnd(result)
opnd_t	*result;
{
	int	tmpint;
	long	tmplng;

	get1byte();
	convert(cur1byte, result->opnd_sym, LEAD);
	tmpint = (cur1byte < 0x80) ? (int) cur1byte : (int) cur1byte | ~0xff;
	tmplng = dis_start - dis_loc + (long) tmpint;
	compoff(tmplng, result->opnd_sym);
	result->opnd_val = dis_start + tmpint;
	dis_relopnd++;
	return(tmplng);
}

/*
 *	get_macro (result)
 *
 *	Get the next byte and use that as the index of the
 *	macro rom instruction.
 *
 */

get_macro(result)
opnd_t	*result;
{
	extern	unsigned  short  cur1byte;
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

	get1byte();
	for (rpt = romlist; rpt->romcode != 0; rpt++) {
		if (cur1byte == rpt->romcode) {
			sprintf(result->opnd_sym, "%s", rpt->romstring);
			return;
		}
	}
	sprintf(result->opnd_sym, "MACRO %X", cur1byte);
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

get_hjmp_oprnd(result)
opnd_t	*result;
{
	unsigned short	gethalfword();
	int		tmpint;
	long		tmplng;

	tmpint = (int) gethalfword();
	if (tmpint & 0x8000)
		tmpint |= 0xffff0000;
	convert(tmpint, result->opnd_sym, LEAD);
	tmplng = dis_start - dis_loc + (long) tmpint;
	compoff(tmplng, result->opnd_sym);
	result->opnd_val = dis_start + tmpint;
	dis_relopnd++;
	return(tmplng);
}

/*
 *	gethalfword()
 *
 *	This routine will read the next 2 bytes in the object file from
 *	the buffer (filling the 4 byte buffer if necessary).
 */

unsigned short
gethalfword()
{
	union {
		unsigned short 	half;
		char		bytes[2];
	} curhalf;

	curhalf.half = 0;
	get1byte();
	curhalf.bytes[1] = cur1byte;
	get1byte();
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
getword()
{
	union {
		char	bytes[4];
		long	word;
	} curword;

	curword.word = 0;
	get1byte();
	curword.bytes[3] = cur1byte;
	get1byte();
	curword.bytes[2] = cur1byte;
	get1byte();
	curword.bytes[1] = cur1byte;
	get1byte();
	curword.bytes[0] = cur1byte;
	return(curword.word);
}

/*
 * 	longprint
 *	simply a routine to print a long constant with an optional
 *	prefix string such as "*" or "$" for operand descriptors
 */

longprint(result, prefix, value)
char	*result;
char	*prefix;
long	value;
{
	sprintf(result, "%s0x%lx", prefix, value);
	return;
}
