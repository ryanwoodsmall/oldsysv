/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)dis:m32/bits.c	1.16"
/*
 */
#include	<stdio.h>
#include	"dis.h"
#include	"filehdr.h"
#include	"scnhdr.h"
#include	"ldfcn.h"
#include	"sgs.h"


#define		MAXERRS	10	/* maximum # of errors allowed before	*/
				/* abandoning this disassembly as a	*/
				/* hopeless case			*/

static short errlev = 0;	/* to keep track of errors encountered during	*/
			/* the disassembly, probably due to being out	*/
			/* of sync.					*/

#define		OPLEN	35	/* maximum length of a single operand	*/
				/* (will be used for printing)		*/

static	char	operand[4][OPLEN];	/* to store operands as they	*/
					/* are encountered		*/
static	char	symarr[4][OPLEN];

static	long	start;			/* start of each instruction	*/
					/* used with jumps		*/

static	int	fpflag;		/* will indicate floating point instruction	*/
				/* (0=NOT FLOATING POINT, 1=SINGLE, 2=DOUBLE) 	*/
				/* so that immediates will be printed in	*/
				/* decimal floating point.			*/
static	int	bytesleft = 0;	/* will indicate how many unread bytes are in buffer */

#define	TWO_8	256
#define	TWO_16	65536

/*
 *	dis_text ()
 *
 *	disassemble a text section
 */

dis_text()

{
	/* the following arrays are contained in tables.c	*/
	extern	struct	instable	opcodetbl[256];
	extern	struct	formtable	adrmodtbl[256];

	/* the following entries are from _extn.c	*/
	extern	SCNHDR	scnhdr;
	extern	char	*sname;
	extern	char	*fname;
	extern	LDFILE	*t_ptr;
	extern	int	Sflag;
	extern	int	Lflag;
	extern	int	sflag;
	extern	int	trace;
	extern	long	 loc;
	extern	char	mneu[];
	extern	char	object[];
	extern	char	symrep[];
	extern	unsigned short	cur1byte;

	/* the following routines are in _utls.c	*/
	extern	int	printline();
	extern	int	looklabel();
	extern	int	line_nums();
	extern	int	prt_offset();
	extern	int	convert();
	extern	short	sect_name();
	extern	int	compoff();

	/* the following routines are in this file	*/
	long		getword();
	unsigned short	gethalfword();

	SCNHDR		*sech;
	struct instable	*cp;
	unsigned	key;
	char		temp[NCPS+1];
	short		sect_num;
	long 		lngtmp;
	char		ctemp[OPLEN];	/* to store EXTOP operand   */

	/* initialization for each beginning of text disassembly	*/

	sech = &scnhdr;

	if (Lflag > 0)
		sect_num = sect_name();
	/*
	 * An instruction is disassembled with each iteration of the
	 * following loop.  The loop is terminated upon completion of the
	 * section (loc minus the section's physical address becomes equal
	 * to the section size) or if the number of bad op codes encountered
	 * would indicate this disassembly is hopeless.
	 */

	for (loc = sech->s_paddr; ((loc-sech->s_paddr) < sech->s_size) && (errlev < MAXERRS); printline()){
		start = loc;
		sprintf(operand[0],"");
		sprintf(operand[1],"");
		sprintf(operand[2],"");
		sprintf(operand[3],"");
		sprintf(symarr[0],"");
		sprintf(symarr[1],"");
		sprintf(symarr[2],"");
		sprintf(symarr[3],"");
		fpflag = NOTFLOAT;	/* assume immediates are not floating
					 * point , unless floating point 
					 * operand is disassembled.
					 */
		if (Lflag > 0)
			looklabel(loc,sect_num);/* look for C source labels */
		line_nums(sech->s_nlnno);	/* print breakpoint line # */
		prt_offset();			/* print offset		   */

		/* key is the one byte op code		*/
		/* cp is the op code Class Pointer	*/

		get1byte(t_ptr);
		key = cur1byte;
		cp = &opcodetbl[key];

		/* print the mnemonic */
		sprintf(mneu,"%-8s",cp -> name);
		sprintf(symrep,"");
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
			get1byte(t_ptr);
			continue;

		/* This class handles instructions that wish to ignore	*/
		/* 16 bits of insignificant data			*/
		case	NOOP16:
			gethalfword(t_ptr);
			continue;

		/* This case handles the EXTOP instruction for the	*/
		/* m32a simulator i/o routines.				*/
		case	EXT:
			get1byte(t_ptr);
			convert(cur1byte,ctemp,LEAD);
			sprintf(mneu,"%s%s",mneu,ctemp);
			if (sflag)
				sprintf(symrep,"%s%s",symrep,ctemp);
			continue;

		/* This class handles instructions with no operands	*/
		case	OPRNDS0:
			continue;

		/* This class handles instructions with 1 operand	*/
		/* that is described by a preceeding discriptor		*/
		case	OPRNDS1:
		case	JUMP:
			get_operand(operand[0],symarr[0]);
			sprintf(mneu,"%s%s",mneu,operand[0]);
			if (sflag)
				sprintf(symrep,"%s%s",symrep,symarr[0]);
			continue;

		/* This class handles instructions with 2 operands	*/
		/* that are each described by a preceeding discriptor	*/
		case	OPRNDS2:
			get_operand(operand[0],symarr[0]);
			get_operand(operand[1],symarr[1]);
			sprintf(mneu,"%s%s,%s",mneu,operand[0],operand[1]);
			if (sflag)
				sprintf(symrep,"%s%s,%s",symrep,symarr[0],symarr[1]);
			continue;

		/* This class handles instructions with 3 operands	*/
		/* that are each described by a preceeding discriptor	*/
		case	OPRNDS3:
			get_operand(operand[0],symarr[0]);
			get_operand(operand[1],symarr[1]);
			get_operand(operand[2],symarr[2]);
			sprintf(mneu,"%s%s,%s,%s",mneu,operand[0],
				operand[1],operand[2]);
			if (sflag)
				sprintf(symrep,"%s%s,%s,%s",symrep,symarr[0],
							symarr[1],symarr[2]);
			continue;

		/* This class handles instructions with 4 operands	*/
		/* that are each described by a preceeding discriptor	*/
		case	OPRNDS4:
			get_operand(operand[0],symarr[0]);
			get_operand(operand[1],symarr[1]);
			get_operand(operand[2],symarr[2]);
			get_operand(operand[3],symarr[3]);
			sprintf(mneu,"%s%s,%s,%s,%s",mneu,operand[0],
				operand[1],operand[2],operand[3]);
			if (sflag)
				sprintf(symrep,"%s%s,%s,%s,%s",symrep,symarr[0],
						symarr[1],symarr[2],symarr[3]);
			continue;
 
		/* This class handles jump instructions that have a one	*/
		/* byte offset (not an ordinary operand)		*/
		case	JUMP1:
			get_bjmp_oprnd(operand[0],symarr[0]);
			sprintf(mneu,"%s%s",mneu,operand[0]);
			if (sflag)
				sprintf(symrep,"%s%s",symrep,symarr[0]);
			continue;

		/* This class handles jump instructions that have a two	*/
		/* byte offset (not an ordinary operand)		*/
		case	JUMP2:
			get_hjmp_oprnd(operand[0],symarr[0]);
			sprintf(mneu,"%s%s",mneu,operand[0]);
			if (sflag)
				sprintf(symrep,"%s%s",symrep,symarr[0]);
			continue;

		/* This class handles single precision floating point 	*/
		/* instructions with two operands.			*/
		case	SFPOPS2:
			fpflag = FPSINGLE;
			get_operand(operand[0],symarr[0]);
			get_operand(operand[1],symarr[1]);
			sprintf(mneu,"%s%s,%s",mneu,operand[0],operand[1]);
			if (sflag)
				sprintf(symrep,"%s%s,%s",symrep,symarr[0],symarr[1]);
			continue;

		/* This class handles single precision floating point	*/
		/* instructions with three operands.			*/
		case	SFPOPS3:
			fpflag = FPSINGLE;
			get_operand(operand[0],symarr[0]);
			get_operand(operand[1],symarr[1]);
			get_operand(operand[2],symarr[2]);
			sprintf(mneu,"%s%s,%s,%s",mneu,operand[0],
				operand[1],operand[2]);
			if (sflag)
				sprintf(symrep,"%s%s,%s,%s",symrep,symarr[0],
							symarr[1],symarr[2]);
			continue;

		/* This class handles double precision floating point 	*/
		/* instructions with two operands.			*/
		case	DFPOPS2:
			fpflag = FPDOUBLE;
			get_operand(operand[0],symarr[0]);
			get_operand(operand[1],symarr[1]);
			sprintf(mneu,"%s%s,%s",mneu,operand[0],operand[1]);
			if (sflag)
				sprintf(symrep,"%s%s,%s",symrep,symarr[0],symarr[1]);
			continue;

		/* This class handles double precision floating point	*/
		/* instructions with three operands.			*/
		case	DFPOPS3:
			fpflag = FPDOUBLE;
			get_operand(operand[0],symarr[0]);
			get_operand(operand[1],symarr[1]);
			get_operand(operand[2],symarr[2]);
			sprintf(mneu,"%s%s,%s,%s",mneu,operand[0],
				operand[1],operand[2]);
			if (sflag)
				sprintf(symrep,"%s%s,%s,%s",symrep,symarr[0],
							symarr[1],symarr[2]);
			continue;
		/* Support Processor Instruction  "SPOP";  Opcode is 	*/
		/* followed by 32 bit "id" similar to an immediate word */
		/* but has no byte descriptor.				*/
		case SPRTOP0:
			lngtmp = getword(t_ptr);
			if (!Sflag) {
					/* support processor id */
				switch ((lngtmp >> 24) & 0xff) {
				case MAU_ID:
					mau_dis(lngtmp,'\0');
					continue;
				default:
					break;
				}
			}
			sprintf(mneu,"%s&",mneu);
			if (sflag)  {
				sprintf(symrep,"%s&",symrep);
				longprint(symrep,symrep,lngtmp);
			}
			longprint(mneu,mneu,lngtmp);
			continue;
		/* Support Processor Instructions "SPOPRS", "SPOPRD",  	*/
		/* "SPOPRT", "SPOPWS", "SPOPWD", and "SPOPWT".  The	*/
		/* opcode is followed by a 32 bit support processor "id"*/
		/* similar to an immediate word by it has no byte 	*/
		/* descriptor. Following the "id" is one normal operand	*/
		/* which has a byte descriptor.				*/
		case SPRTOP1:
			lngtmp = getword(t_ptr);
			if (!Sflag) {
					/* support processor id */
				switch ((lngtmp >> 24) & 0xff) {
				case MAU_ID:
					mau_dis(lngtmp,mneu[5]);
					continue;
				default:
					break;
				}
			}
			sprintf(mneu,"%s&",mneu);
			longprint(mneu,mneu,lngtmp);
			get_operand(operand[0],symarr[0]);
			sprintf(mneu,"%s, %s",mneu,operand[0]);
			if (sflag) {
				sprintf(symrep,"%s&",symrep);
				longprint(symrep,symrep,lngtmp);
				sprintf(symrep,"%s, %s",symrep,symarr[0]);
			}
			continue;
		/* Support Processor Instructions "SPOPS2", "SPOPD2",  	*/
		/* "SPOPT2".  These are two operand instructions.  The	*/
		/* opcode is followed by a 32 bit support processor "id"*/
		/* similar to an immediate word by it has no byte 	*/
		/* descriptor. Following the "id" is one normal operand	*/
		/* which has a byte descriptor.				*/
		case SPRTOP2:
			lngtmp = getword(t_ptr);
			if (!Sflag) {
					/* support processor id */
				switch ((lngtmp >> 24) & 0xff) {
				case MAU_ID:
					mau_dis(lngtmp,mneu[4]);
					continue;
				default:
					break;
				}
			}
			sprintf(mneu,"%s &",mneu);
			longprint(mneu,mneu,lngtmp);
			get_operand(operand[0],symarr[0]);
			get_operand(operand[1],symarr[1]);
			sprintf(mneu,"%s, %s, %s",mneu,operand[0],operand[1]);
			if (sflag) {
				sprintf(symrep,"%s &",symrep);
				longprint(symrep,symrep,lngtmp);
				sprintf(symrep,"%s, %s, %s",symrep,symarr[0],symarr[1]);
			}
			continue;
		/* an invalid op code (there aren't too many)	*/
		case UNKNOWN:
			sprintf(mneu,"***ERROR--unknown op code***");
			printline();	/* to print the error message	*/
			/* attempt to resynchronize */
			if (resync() == FAILURE)	/* if cannot recover */
				errlev++;		/* stop eventually.  */
			sprintf(object,""); /* to prevent extraneous 	*/
			sprintf(mneu,"");   /* printing when continuing	*/
			if (sflag)
			      sprintf(symrep,""); /* to the for loop iteration*/
			continue;

		/* This special case handles the macro rom instructions */
		case MACRO:
			get_macro(operand[0]);
			sprintf(mneu,"%s",operand[0]);
			continue;

		default:
			printf("\n%sdis bug: notify implementor:",SGS);
			printf(" case from instruction table not found\n");
			exit(4);
			break;
		} /* end switch */
	}  /* end of for */

	if (errlev >= MAXERRS) {
		printf("%sdis: %s: %s: section probably not text section\n",
			SGS,fname, sname);
		printf("\tdisassembly terminated\n");
		exit(4);
	}
}
/*
 *	get_operand ()
 *
 *	Determine the type of address mode for this operand and
 *	place the resultant operand text to be printed in the array
 *	whose address was passed to this function (result). If sflag is ON then
 *	put symbolic disassembly information in the second array whose address
 *	was passed to this function (symresult).
 */

get_operand(result,symresult)
char	*result;
char	*symresult;
{
	extern struct formtable adrmodtbl[256];	/* from tables.c	*/
	extern	int	compoff();		/* in _utls.c	*/
	extern	int	sfpconv();		/* in _utls.c	*/
	extern	int	dfpconv();		/* in _utls.c	*/
	extern	LDFILE	*t_ptr;			/* in _extn.c	*/
	extern	int	oflag;			/* in _extn.c	*/
	extern	int	sflag;			/* in _extn.c	*/
	extern	int	trace;			/* in _extn.c	*/
	extern	int	loc;			/* in _extn.c	*/
	extern	unsigned short	cur1byte;	/* in _extn.c	*/

	/* the following routines are in this file	*/
	long	getword();
	unsigned short	gethalfword();

	struct	formtable  *tp;
	char	temp[OPLEN], stemp[OPLEN];
	char	*temp1;
	char	*temp2;
	char	*tstr;
	int	regno;			/* used to hold register number */
	long	addr;			/* used to hold operand address or offset */
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

	get1byte(t_ptr);
	tp = &adrmodtbl[cur1byte];
	temp2 = tp -> name;

	if (trace > 0)
		printf("\nadr mod typ = %x\n",tp->typ);

	tstr= symresult;
	switch (tp -> typ){

	/* type = immediate numbers 0 to 63 (IM6)		*/
	/* type = immediate numbers -16 to -1 (IM4)		*/
	/* For this type the address modes table holds		*/
	/* the entire text for this operand.			*/
	case	IM4:
	case	IM6:
		sprintf(result,"%s",temp2);
		if (sflag)
			sprintf(symresult,"%s",temp2);
		return;

	/* type = register (R) 					*/
	/* For this type the address modes table holds		*/
	/* the entire text for this operand.			*/
	case	R:
		sprintf(result,"%s",temp2);
		if (sflag) {
			regno= cur1byte & 0xf;
			if (cur1byte >> 4 == 5) {
				addr= 0;
				locsympr(addr,regno,&tstr);
			} else 
				locsympr(regno,-1,&tstr);
		}
		return;

	/* type = register and displacement of 0 to 14 (R4)	*/
	/* For this type the address modes table holds		*/
	/* the entire text for this operand.			*/
	case	R4:
		sprintf(result,"%s",temp2);
		if (cur1byte >> 4 == 6)
			regno= 9;
		else
			regno= 10;
		addr= cur1byte & 0xf;
		if (sflag)
			locsympr(bext(addr),regno,&tstr);
		return;

	/* type = immediate operand in following byte (IMB)	*/
	/* For this type the next byte is read, converted for	*/
	/* printing and preceded by an "&".			*/
	case	IMB:
		get1byte(t_ptr);
		convert(cur1byte,temp,LEAD);
		sprintf(result,"&%s",temp);
		if (sflag)
			sprintf(symresult,"&%s",temp);
		return;

	/* type = register + byte displacement (BDB)		*/
	/* For this type the next byte is read and converted	*/
	/* and prepended to the register name taken from the	*/
	/* address modes teable.				*/
	case	BDB:
		regno= cur1byte & 0xf;
		get1byte(t_ptr);
		convert(cur1byte,temp,LEAD);
		sprintf(result,"%s%s",temp,temp2);
		if (sflag) {
			if (regno == PCNO) {
				ltemp= start + bext(cur1byte);
				extsympr(ltemp,&tstr);
			} else
				locsympr(bext(cur1byte),regno,&tstr);
		}
		return;

	/* type = register + byte displacement deferred (BDBDF)	*/
	/* Same as BDB except an additional "*" is prepended.	*/
	case	BDBDF:
		regno= cur1byte & 0xf;
		get1byte(t_ptr);
		convert(cur1byte,temp,LEAD);
		sprintf(result,"*%s%s",temp,temp2);
		if (sflag) {
			sprintf(tstr++,"*");
			if (regno == PCNO) {
				ltemp= start + bext(cur1byte);
				extsympr(ltemp,&tstr);
			} else
				locsympr(bext(cur1byte),regno,&tstr);
		}
		return;

	/* type = immediate operand in following half word (IMH)	*/
	/* For this type the next half word is read, converted for	*/
	/* printing and preceded by an "&".				*/
	case	IMH:
		wdtmp = gethalfword(t_ptr);
		convert(wdtmp,temp,LEAD);
		sprintf(result,"&%s",temp);
		if (sflag)
			sprintf(symresult,"&%s",temp);
		return;

	/* type = register + half word displacement (BDH)		*/
	/* For this type the next half word is read and converted	*/
	/* and prepended to the register name taken from the		*/
	/* address modes table.						*/
	case	BDH:
		regno= cur1byte & 0xf;
		wdtmp = gethalfword(t_ptr);
		convert(wdtmp,temp,LEAD);
		sprintf(result,"%s%s",temp,temp2);
		if (sflag) {
			if (regno == PCNO) {
				ltemp= start + hext(wdtmp);
				extsympr(ltemp,&tstr);
			} else
				locsympr(hext(wdtmp),regno,&tstr);
		}
		return;

	/* type = register + half word displacement deferred (BDHDF)	*/
	/* Same as BDH except an additional "*" is prepended.		*/
	case	BDHDF:
		regno= cur1byte & 0xf;
		wdtmp = gethalfword(t_ptr);
		convert(wdtmp,temp,LEAD);
		sprintf(result,"*%s%s",temp,temp2);
		if (sflag) {
			sprintf(tstr++,"*");
			if (regno == PCNO) {
				ltemp= start + hext(wdtmp);
				extsympr(ltemp,&tstr);
			} else
				locsympr(hext(wdtmp),regno,&tstr);
		}
		return;

	/* type = immediate operand in following word (IMW)		*/
	/* NOTE: immediate words may be floating point or nonfloating	*/
	/*       point. Doubleword immediates are floating point.	*/
	/* For this type the next word is read, converted for		*/
	/* printing and preceded by an "&".				*/
	case	IMW:
		lngtmp = getword(t_ptr);
		switch (fpflag) {

		case	NOTFLOAT:
			longprint(result,"&",lngtmp);
			if (sflag) {
				sprintf(tstr++,"&");
				extsympr(lngtmp,&tstr);
			}
			return;
		case	FPSINGLE:
			fptype = sfpconv(lngtmp,  &fpnum);
			break;
		case	FPDOUBLE:
			ltemp = getword(t_ptr);
			fptype = dfpconv(lngtmp, ltemp, &fpnum, &fpbigex);
			break;

		} /* switch (fpflag)	*/
		/* fptype is the return value of the floating point 
		 * conversion routines: sfpconv() and dfpconv().
		 */
		switch (fptype) {

		case	NOTANUM:
			sprintf (result, "INVALID FP NUMBER");
			if (sflag)
				sprintf(symresult,"-");
			break;
		case	NEGINF:
			sprintf (result, "NEGATIVE INFINITY");
			if (sflag)
				sprintf(symresult,"-");
			break;
		case	INFINITY:
			sprintf (result, "INFINITY");
			if (sflag)
				sprintf(symresult,"-");
			break;
		case	ZERO:
			sprintf (result, "&0.0");
			if (sflag)
				sprintf(symresult,"&0.0");
			break;
		case	NEGZERO:
			sprintf (result, "&-0.0");
			if (sflag)
				sprintf(symresult,"&-0.0");
			break;
		case	FPNUM:
			sprintf(result,"&%.15e", fpnum);
			if (sflag)
				sprintf(symresult,"&%.15e", fpnum);
			break;
		case	FPBIGNUM:
			sprintf(result, "&%.13fE%d{RADIX 2}", 
				fpnum, fpbigex);
			if (sflag)
				sprintf(symresult, "&%.13fE%d{RADIX 2}", 
							fpnum, fpbigex);
			break;

		} /* switch (fptype)	*/
		return;

	/* type = register + word displacement (BDW)		*/
	/* For this type the next word is read and converted	*/
	/* and prepended to the register name (for BDW) or	*/
	/* the null string (for ABAD) from the address modes	*/
	/* table.						*/
	case	BDW:
		regno= cur1byte & 0xf;
		lngtmp = getword(t_ptr);
		longprint(result,"",lngtmp);
		sprintf(result,"%s%s",result,temp2);
		if (sflag) {
			if (regno == PCNO) {
				ltemp= start + lngtmp;
				extsympr(ltemp,&tstr);
			} else
				locsympr(lngtmp,regno,&tstr);
		}
		return;

	/* type = absolute address (ABAD)			*/
	case	ABAD:
		lngtmp = getword(t_ptr);
		longprint(result,"$",lngtmp);
		sprintf(result,"%s%s",result,temp2);
		if (sflag)
			extsympr(lngtmp, &tstr);
		return;

	/* type = register + word displacement deferred (BDWDF)	*/
	/* Same as BDW and ABAD except an additional "*" is 	*/
	/* prepended.						*/
	case	BDWDF:
		regno= cur1byte & 0xf;
		lngtmp = getword(t_ptr);
		longprint(result,"*",lngtmp);
		sprintf(result,"%s%s",result,temp2);
		if (sflag) {
			sprintf(tstr++,"*");
			if (regno == PCNO) {
				ltemp= start + lngtmp;
				extsympr(ltemp,&tstr);
			} else
				locsympr(lngtmp,regno,&tstr);
		}
		return;

	/* type = absolute address deferred (ABADDF)		*/
	case	ABADDF:
		lngtmp = getword(t_ptr);
		longprint(result,"*$",lngtmp);
		sprintf(result,"%s%s",result,temp2);
		if (sflag) {
			tstr+=sprintf(tstr,"*");
			extsympr(lngtmp, &tstr);
		}
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
		get_operand(temp,stemp);
		sprintf(result,"%s%s",temp2,temp);
		if (sflag)
			sprintf(symresult,"%s%s",temp2,stemp);
		return;

	default:
		printf("\n%sdis: invalid general addressing mode (%x)\n",
			SGS, cur1byte);
		printf("\tnotify disassembler implementor\n");
	}	/* end switch */
}
/*
 *	mau_dis(cmd,size)
 *
 *	Disassemble mau instruction symbolically
 */
mau_dis(cmd,size)
long cmd;
char size;
{
	int opcode, op1, op2, op3;
	int numop = 0,isize,dsize;
	int choose = 0;
	int i;
	static char *szarr = "sdx";
	static char *mau_regs[] = {
		"%f0","%f1","%f2","%f3",
		"%s0","%s1","%s2","%s3",
		"%d0","%d1","%d2","%d3",
		"%x0","%x1","%x2","%x3"
	};
	static char *mau_mnem[] = {
/* 0x00 */	0,      0,      "fadd",  "fsub",  "fdiv", "frem","fmul","mov",
/* 0x08 */	"movfa","movta","fcmp",  "fcmpt", "fabs", "fsqr","frnd","movtw",
/* 0x10 */	"mov",  "mov",  "movt10","nop",   "movfd",0,     0,     "fneg",
/* 0x18 */	"movtd",0,      "fcmps", "fcmpst",0,      0,     0,     0
	};

	opcode = ((cmd >> 10) & 0x1f);
	op1 = ((cmd >> 7) & 0x07);
	op2 = ((cmd >> 4) & 0x07);
	op3 = (cmd & 0x0f);
	if (op1 != 0x7) ++numop;
	if (op2 != 0x7) ++numop;
	if (op3 != 0xf) ++numop;

	dsize = (((op3 < 0x0c) ? (op3>>2) : op3) & 0x3);
	switch(size) {
	case '\0':	isize = dsize;	break;
	case 'S':	isize = 0;	break;
	case 'D':	isize = 1;	break;
	case 'T':	isize = 2;	break;
	}

	switch (opcode) {
	case 0x02:	/* mfadd[sdx][23] */
	case 0x03:	/* mfsub[sdx][23] */
	case 0x04:	/* mfdiv[sdx][23] */
	case 0x05:	/* mfrem[sdx][23] */
	case 0x06:	/* mfmul[sdx][23] */
	case 0x0c:	/* mfabs[sdx][12] */
	case 0x0d:	/* mfsqr[sdx][12] */
	case 0x0e:	/* mfrnd[sdx][12] */
	case 0x17:	/* mfneg[sdx][12] */
		choose = numop;
		sprintf(mneu,"m%s%c",mau_mnem[opcode],szarr[isize]);
		break;
	case 0x07:	/* mmov[sdx][sdx] */
		sprintf(mneu,"mmov%c   ",szarr[dsize]);
		break;
	case 0x08:	/* mmovfa */
	case 0x09:	/* mmovta */
	case 0x0f:	/* mmov[sdx]w */
	case 0x12:	/* mmov[sdx]10 */
	case 0x13:	/* mnop */
	case 0x14:	/* mmovfd */
	case 0x18:	/* mmovtd */
		sprintf(mneu,"m%-8s",mau_mnem[opcode]);
		break;
	case 0x0a:	/* mfcmp[sdx] */
	case 0x0b:	/* mfcmpt[sdx] */
	case 0x1a:	/* mfcmps[sdx] */
	case 0x1b:	/* mfcmpts[sdx] */
		sprintf(mneu,"m%s%c  ",mau_mnem[opcode],szarr[isize]);
		break;
	case 0x10:	/* mmovw[sdx] */
		sprintf(mneu,"mmovw%c  ",szarr[dsize]);
		break;
	case 0x11:	/* mmov10[sdx] */
		sprintf(mneu,"mmov10%c ",szarr[dsize]);
		break;
	default:
		goto bad_spop;
	}

	numop = 0;
	if (op1 <= 0x03)	/* register */
		strcpy(operand[numop++],mau_regs[op1]);
	else if (op1 <= 0x06) {	/* memory */
		if ((op1&0x03) != isize)
			goto bad_spop;
		get_operand(operand[numop],symarr[numop]);
		++numop;
	}

	if (op2 <= 0x03)	/* register */
		strcpy(operand[numop++],mau_regs[op2]);
	else if (op2 <= 0x06) {	/* memory */
		if ((op2&0x03) != isize)
			goto bad_spop;
		get_operand(operand[numop],symarr[numop]);
		++numop;
	}

	if (op3 <= 0x0b)	/* register */
		strcpy(operand[numop++],mau_regs[op3+4]);
	else if (op3 <= 0x0e) {	/* memory */
		if ((op3&0x03) != isize)
			goto bad_spop;
		get_operand(operand[numop],symarr[numop]);
		++numop;
	}

	if (choose) {
		if (choose == 2 && !strcmp(operand[0],operand[1]))
			--choose;
		else if (choose == 3 && !strcmp(operand[1],operand[2]))
			--choose;
		sprintf(mneu,"%s%d ",mneu,choose);
	}

	for (i = 0; i<numop; i++)
		if (symarr[i][0] == '\0')
			(void) strcpy(symarr[i],operand[i]);

	switch (numop) {
	case 1:
		sprintf(mneu,"%s%s",mneu,operand[0]);
		if (sflag)
			sprintf(symrep,"%s",symarr[0]);
		break;
	case 2:
		sprintf(mneu,"%s%s, %s",mneu,operand[0],operand[1]);
		if (sflag)
			sprintf(symrep,"%s, %s",symarr[0],symarr[1]);
		break;
	case 3:
		sprintf(mneu,"%s%s, %s, %s",mneu,operand[0],operand[1],operand[2]);
		if (sflag)
			sprintf(symrep,"%s, %s, %s",symarr[0],symarr[1],symarr[2]);
		break;
	}
	return;

bad_spop:
		sprintf(mneu,"***ERROR--illegal mau operation***");
		printline();
		if (resync() == FAILURE)
			++errlev;
		sprintf(object,"");
		sprintf(mneu,"");
		if (sflag)
			sprintf(symrep,"");
		return;
}
/*
 *	get_bjmp_oprnd (result, symresult)
 *
 *	Get the next byte as a jump displacement operand.
 *	Call pr_bjmp_oprnd() to convert the characters, calculate the 
 *	jump address and prepare for printing.
 *
 */

get_bjmp_oprnd(result, symresult)
char	*result;
char	*symresult;
{
	extern	LDFILE	*t_ptr;
	extern	unsigned  short  cur1byte;
	long	tlng;

	get1byte(t_ptr);
	tlng = ((long)cur1byte) & 0x000000ffL;
	pr_bjmp_oprnd(result, symresult, tlng);
}
/*
 *	get_macro (result)
 *
 *	Get the next byte and use that as the index of the
 *	macro rom instruction.
 *
 */

get_macro(result)
char	*result;
{
	extern	LDFILE	*t_ptr;
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
		0x2f,"INTACK",
		0x35,"STRCPY",
		0x3c,"SLFTST",
		0x45,"RETG",
		0x61,"GATE",
		0xac,"CALLPS",
		0xc8,"RETPS",
		0,""
	};
	register struct romlist *rpt;

	get1byte(t_ptr);
	cur1byte = cur1byte & 0x00ff;
	for( rpt = romlist; rpt->romcode != 0; rpt++) {
		if( cur1byte == rpt->romcode) {
			strcpy(result, rpt->romstring);
			return;
		}
	}
	(oflag > 0)?	sprintf(result,"MACRO %O",cur1byte):
			sprintf(result,"MACRO %X",cur1byte);
	return;
}

/*
 *	get_hjmp_oprnd (result, symresult)
 *
 *	Get the next half word as a jump displacement operand.
 *	Call pr_hjmp_oprnd() to convert the characters, calculate the
 *	jump address and prepare for printing.
 *
 */

get_hjmp_oprnd(result, symresult)
char	*result;
char	*symresult;
{
	extern	LDFILE	*t_ptr;
	unsigned short	gethalfword();
	long	tlng;

	tlng = ((long)gethalfword(t_ptr)) & 0x0000ffffL;
	pr_hjmp_oprnd(result, symresult, tlng);
}

/*
 *	pr_hjmp_oprnd()
 *
 *	Calculates and prints jump address of branch instructions.
 *
 */
pr_hjmp_oprnd(result, symresult, tmplng)
char	*result;
char	*symresult;
long	tmplng;
{
	long	neglng;
	extern	long	loc;
	extern	int	oflag;
	extern	int	sflag;

	if (tmplng & 0x00008000L) {
		neglng = TWO_16 - tmplng;
		tmplng = start - neglng;
		sprintf(result++,"-");
	} else {
		neglng = tmplng;
		tmplng = start + tmplng;
	}
	if (oflag > 0) {
		if (sflag) {
			sprintf(result,"0%lo",neglng);
			sprintf(symresult,"<0%lo>",tmplng);			
		} else
			sprintf(result,"0%lo <0%lo>",neglng, tmplng);
	} else {
		if (sflag) {
			sprintf(result,"0x%lx",neglng);
			sprintf(symresult,"<0x%lx>",tmplng);
		} else
			sprintf(result,"0x%lx <0x%lx>",neglng, tmplng);
	}
}
/*
 *	pr_bjmp_oprnd()
 *
 *	Calculates and prints jump address of branch instructions.
 *
 */
pr_bjmp_oprnd(result, symresult, tmplng)
char	*result;
char	*symresult;
long	tmplng;
{
	long	neglng;
	extern	long	loc;
	extern	int	oflag;
	extern	int	sflag;

	if (tmplng & 0x00000080L) {
		neglng = TWO_8 - tmplng;
		tmplng = start - neglng;
		sprintf(result++,"-");
	} else {
		neglng = tmplng;
		tmplng = start + tmplng;
	}
	if (oflag > 0) {
		if (sflag) {
			sprintf(result,"0%lo",neglng);
			sprintf(symresult,"<0%lo>",tmplng);
		} else
			sprintf(result,"0%lo <0%lo>",neglng, tmplng);
	} else {
		if (sflag) {
			sprintf(result,"0x%lx",neglng);
			sprintf(symresult,"<0x%lx>",tmplng);
		} else
			sprintf(result,"0x%lx <0x%lx>",neglng, tmplng);
	}
}
/*
 *	ad1_operand (result)
 *
 *	convert the string operand to an integer, add 1, and
 *	convert the result back to a string.
 */

ad1_operand(result)
char	*result;
{
	long	lngtmp;

	sscanf(result,"&0x%lx",&lngtmp);
	++lngtmp;
	cnvrtlng(lngtmp,result,LEAD);
}

/*
 *	get1byte()
 *
 *	This routine will read the next byte in the object file from
 *	the buffer (filling the 4 byte buffer if necessary). 
 *
 */
get1byte(fptr)
LDFILE	*fptr;
{
	extern	long	loc;
	extern	int	oflag;
	extern	char	object[];
	extern	int	trace;
	extern	LDFILE	*t_ptr;
	extern	char	bytebuf[];
	extern	unsigned short	cur1byte;

	if (bytesleft == 0) {
		fillbuff(t_ptr);
		if (bytesleft == 0) {
			fprintf(stderr,"\n%sdis:  premature EOF\n",SGS);
			exit(4);
		}
	}
	cur1byte = ((unsigned short) bytebuf[4-bytesleft]) & 0x00ff;
	bytesleft--;
	(oflag > 0)?	sprintf(object, "%s%.3o ",object,cur1byte):
			sprintf(object, "%s%.2x ",object,cur1byte);
	loc++;
	if (trace > 1)
		printf("\nin get1byte object<%s> cur1byte<%.2x>\n",object,cur1byte);
}

/*
 *	gethalfword()
 *
 *	This routine will read the next 2 bytes in the object file from
 *	the buffer (filling the 4 byte buffer if necessary).
 */
unsigned short
gethalfword(fptr)
LDFILE *fptr;
{
	extern	unsigned short	cur1byte;
	extern	char	object[];
	extern	int	trace;
	union {
		unsigned short 	half;
		char		bytes[2];
	} curhalf;

	curhalf.half = 0;
#ifdef AR32W
	get1byte(fptr);
	curhalf.bytes[1] = cur1byte;
	get1byte(fptr);
	curhalf.bytes[0] = cur1byte;
#else
	get1byte(fptr);
	curhalf.bytes[0] = cur1byte;
	get1byte(fptr);
	curhalf.bytes[1] = cur1byte;
#endif
	if (trace > 1)
		printf("\nin gethalfword object<%s> halfword<%.4x>\n",object,curhalf.half);
	return(curhalf.half);
}

/*
 *	getword()
 *	This routine will read the next 4 bytes in the object file from
 *	the buffer (filling the 4 byte buffer if necessary).
 *
 */
long
getword(fptr)
LDFILE *fptr;
{
	extern	long	loc;
	extern	int	oflag;
	extern	char	object[];
	extern	char	bytebuf[];
	extern	LDFILE	*t_ptr;
	extern	int	trace;
	char	temp1;
	short	byte0, byte1, byte2, byte3;
	int	i, j, bytesread;
	union {
		char	bytes[4];
		long	word;
	} curword;

	curword.word = 0;
	for(i = 0, j= 4 - bytesleft; i < bytesleft; i++, j++)
		curword.bytes[i] = bytebuf[j];
	if (bytesleft < 4) {
		bytesread = bytesleft;
		fillbuff(t_ptr);
		if ((bytesread + bytesleft) < 4) {
			fprintf(stderr,"\n%sdis:  premature EOF\n",SGS);
			exit(4);
		}
		for (i = bytesread, j= 0; i < 4; i++, j++) {
			bytesleft--;
			curword.bytes[i] = bytebuf[j];
		}
	}
	byte0 = ((short)curword.bytes[0]) & 0x00ff;
	byte1 = ((short)curword.bytes[1]) & 0x00ff;
	byte2 = ((short)curword.bytes[2]) & 0x00ff;
	byte3 = ((short)curword.bytes[3]) & 0x00ff;
	(oflag > 0)?	sprintf(object,"%s%.3o %.3o %.3o %.3o ",object,
					byte0, byte1, byte2, byte3):
			sprintf(object,"%s%.2x %.2x %.2x %.2x ",object,
					byte0, byte1, byte2, byte3);
#ifdef AR16WR
	temp1 = curword.bytes[0];
	curword.bytes[0] = curword.bytes[2];
	curword.bytes[2] = temp1;
	temp1 = curword.bytes[1];
	curword.bytes[1] = curword.bytes[3];
	curword.bytes[3] = temp1;
#endif
#ifdef AR32W
	temp1 = curword.bytes[0];
	curword.bytes[0] = curword.bytes[3];
	curword.bytes[3] = temp1;
	temp1 = curword.bytes[1];
	curword.bytes[1] = curword.bytes[2];
	curword.bytes[2] = temp1;
#endif
	loc += 4;
	if (trace > 1)
		printf("\nin getword object<%s>> word<%.8lx>\n",object,curword.word);
	return(curword.word);
}

/*
 *	cnvrtlng (num, temp, flag)
 *
 *	Convert the passed number to either hex or octal, depending on
 *	the oflag, leaving the result in the supplied string array.
 *	If  LEAD  is specified, preceed the number with '0' or '0x' to
 *	indicate the base (used for information going to the mnemonic
 *	printout).  NOLEAD  will be used for all other printing (for
 *	printing the offset, object code, and the second byte in two
 *	byte immediates, displacements, etc.) and will assure that
 *	there are leading zeros.
 */

cnvrtlng(num,temp,flag)
long	num;
char	temp[];
int	flag;

{
	extern	int	oflag;		/* in _extn.c */

	if (flag == LEAD)
		(oflag) ?	sprintf(temp,"&0%lo",num):
				sprintf(temp,"&0x%lx",num);
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

/*
 *	fillbuff()
 *
 *	This routine will read 4 bytes from the object file into the 
 *	4 byte buffer.
 *	The bytes will be stored in the buffer in the correct order
 *	for the disassembler to process them. This requires a knowledge
 *	of the type of host machine on which the disassembler is being
 *	executed (AR32WR = vax, AR32W = maxi or 3B, AR16WR = 11/70), as
 *	well as a knowledge of the target machine (FBO = forward byte
 *	ordered, RBO = reverse byte ordered).
 *
 */
fillbuff()
{
	extern	char	bytebuf[];
	extern	LDFILE	*t_ptr;
	long tlong;

bytesleft = FREAD( &tlong, 1,  4, t_ptr);
	switch (bytesleft) {
	case 0:
	case 4:
		break;
	case 1:
		bytebuf[1] = bytebuf[2] = bytebuf[3] = 0;
		break;
	case 2:
		bytebuf[2] = bytebuf[3] = 0;
		break;
	case 3:
		bytebuf[3] = 0;
		break;
	}
	/* NOTE		The bytes have been read in the correct order
	 *		if one of the following is true:
	 *
	 *		host = AR32WR  and  target = FBO
	 *			or
	 *		host = AR32W   and  target = RBO
	 *
	 */

#if (RBO && AR32WR) || (FBO && AR32W)
	bytebuf[0] = (char)((tlong >> 24) & 0x000000ffL);
	bytebuf[1] = (char)((tlong >> 16) & 0x000000ffL);
	bytebuf[2] = (char)((tlong >>  8) & 0x000000ffL);
	bytebuf[3] = (char)( tlong        & 0x000000ffL);
#endif

#if (FBO && AR32WR) || (RBO && AR32W)
	bytebuf[0] = (char)( tlong        & 0x000000ffL);
	bytebuf[1] = (char)((tlong >>  8) & 0x000000ffL);
	bytebuf[2] = (char)((tlong >> 16) & 0x000000ffL);
	bytebuf[3] = (char)((tlong >> 24) & 0x000000ffL);
#endif

#if RBO && AR16WR
	bytebuf[0] = (char)((tlong >>  8) & 0x000000ffL);
	bytebuf[1] = (char)( tlong        & 0x000000ffL);
	bytebuf[2] = (char)((tlong >> 24) & 0x000000ffL);
	bytebuf[3] = (char)((tlong >> 16) & 0x000000ffL);
#endif
#if FBO && AR16WR
	bytebuf[0] = (char)((tlong >> 16) & 0x000000ffL);
	bytebuf[1] = (char)((tlong >> 24) & 0x000000ffL);
	bytebuf[2] = (char)( tlong        & 0x000000ffL);
	bytebuf[3] = (char)((tlong >>  8) & 0x000000ffL);
#endif
}

