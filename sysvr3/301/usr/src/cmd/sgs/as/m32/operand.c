/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)as:m32/operand.c	1.7"
#include <stdio.h>
#include <ctype.h>
#include "as_ctype.h"
#include "symbols.h"
#include "program.h"
#include "instab.h"

extern char	*oplexptr;	/* set up for oplex() by yyparse() */

upsymins *lookup();

extern OPERAND *opertop;/* bottom of expression stack & target for current operand */
OPERAND	*tiptop;	/* top of the expression stack, always starts at opertop */

/* expr1() return codes */
#define ERROR	-1
#define FNDEXPR  0
#define FNDFLOAT 1

/* for expression parsing by routines other than operand() (which calls expr1())  */
expr()
{
	tiptop = opertop;
	switch (expr1()) {
	case FNDEXPR  : return(*oplexptr);
	case FNDFLOAT : yyerror("expecting expression not float constant");
	default	      : return(-1);
	}
}


/* for use by routines other than operand() to parse floating point expressions */
fp_expr()
{
	tiptop = opertop;
	switch (expr1()) {
	case FNDFLOAT: return(*oplexptr);
	case FNDEXPR : if (opertop->fasciip != NULL) return(*oplexptr);
		       yyerror("expecting float constant not expression");
	default	     : return(-1);
	}
}


struct exptype { char *name; int typno; }
	exptypes[] = {
		{"byte",	0x7L},
		{"half",	0x6L},
		{"sbyte",	0x7L},
		{"shalf",	0x6L},
		{"sword",	0x4L},
		{"ubyte",	0x3L},
		{"uhalf",	0x2L},
		{"uword",	0x0L},
		{"word",	0x4L},
		0
	};

static find_exptype(name)
char *name;
{
	register struct exptype *curtype;

	for (curtype = exptypes; curtype->name != NULL; curtype++)
		if (!strcmp(name, curtype->name)) return(curtype->typno);
	return(NOTYPE);	/* expanded type not found */
}

/* operand tokens - used by operand(), expr1(), and oplex() */
#define	ID	0200
#define REG	0201
#define NUM	0202
#define FPNUM	0203
#define FREG	0204
#define SREG	0205
#define DREG	0206
#define XREG	0207

/* operand parser - returns 0 on success */
operand()
{
	register mode = 0;

	tiptop = opertop;
	opertop->newtype = NOTYPE;
	while (*oplexptr == '{') {
		register char *aheadptr;

		for (aheadptr = ++oplexptr; IDBEGIN(*aheadptr); aheadptr++) ;
		if (*aheadptr != '}') {
			yyerror("expecting '}'");
			return(-1);
		} 
		*aheadptr = '\0';
		if ((opertop->newtype = find_exptype(oplexptr)) == NOTYPE) {
			*aheadptr = '}';
			yyerror("Unknown expanded type operand");
			return(-1);
		}
		*aheadptr = '}';
		oplexptr = aheadptr + 1;
	}
	switch (*oplexptr) {
		case '(':
			if (oplexptr[1] != '%') {
				mode = EXADMD;
				break;
			}
			oplexptr++;
			if (oplex() != REG || *oplexptr++ != ')') return(-1);
			opertop->expspec = NULLSPEC;
			opertop->type = REGDFMD;
			return(*oplexptr);
		case '%':
			switch (oplex()) {
			case REG : opertop->type = REGMD; break;
			case FREG : opertop->type = FREGMD; break;
			case SREG : opertop->type = SREGMD; break;
			case DREG : opertop->type = DREGMD; break;
			case XREG : opertop->type = XREGMD; break;
			default : return(-1);
			}
			if (opertop->type != REGMD && opertop->newtype != NOTYPE)
				return(-1);
			opertop->expspec = NULLSPEC;
			return(*oplexptr);
		case '&':
			mode = IMMD;
			oplexptr++;
			break;
		case '*':
			switch (*++oplexptr) {
			case '%':
				oplexptr++;
				if (oplex() != REG) return(-1);
				opertop->expspec = NULLSPEC;
				opertop->type = REGDFMD;
				return(*oplexptr);
			case '$':
				mode = ABSDFMD;
				oplexptr++;
				break;
			default :
				mode = EXADDFMD;
				break;
			}
			break;
		case '$':
			mode = ABSMD;
			oplexptr++;
			break;
		default:	/* just expr */
			mode = EXADMD;
	}
	/* expr */
	switch (expr1()) {
	case FNDFLOAT:
		if (mode == IMMD && opertop->newtype == NOTYPE) {
			opertop->type = FPIMMD;
			opertop->expspec = NULLSPEC;
			return(*oplexptr);
		}
	case ERROR: return(-1);
	}
	/* (reg) */
	if (*oplexptr == '(') {
		oplexptr++;
		if (oplex() == REG && *oplexptr++ == ')') {
			if (mode == EXADDFMD) {
				opertop->type = DSPDFMD;
			} else if (mode == EXADMD) {
				opertop->type = DSPMD;
			} else return(-1);
			opertop->reg = (tiptop-1)->reg;
			if (*oplexptr == '<') {
				switch (*++oplexptr) {
				case 'B' : opertop->expspec = BYTESPEC; break;
				case 'H' : opertop->expspec = HALFSPEC; break;
				case 'W' : opertop->expspec = WORDSPEC; break;
				case 'L' : opertop->expspec = LITERALSPEC; break;
				case 'S' : opertop->expspec = SHORTSPEC; break;
				default : return(-1);
				}
				if (*++oplexptr != '>') return(-1);
				oplexptr++;
			}
			return(*oplexptr);
		} else return(-1);
	}
	if (mode == IMMD && *oplexptr == '<') {
		switch (*++oplexptr) {
		case 'B' : opertop->expspec = BYTESPEC; break;
		case 'H' : opertop->expspec = HALFSPEC; break;
		case 'W' : opertop->expspec = WORDSPEC; break;
		case 'L' : opertop->expspec = LITERALSPEC; break;
		case 'S' : opertop->expspec = SHORTSPEC; break;
		default : return(-1);
		}
		if (*++oplexptr != '>') return(-1);
		oplexptr++;
	} else opertop->expspec = NULLSPEC;
	opertop->type = mode;
	return(*oplexptr);
}


/*
expr1() parses an expression returning:

	FNDFLT if a floating point constant was found
	FNDEXP for an expression
	ERROR if an error is encountered
*/
static
expr1()
{
	register op = 0, min = 0;

	while (1) {
		while (*oplexptr == '-') {	/* unary minus */
			min ^= 1;
			oplexptr++;
		}
		switch (oplex()) {
		case FPNUM:
			if (tiptop != opertop+1) /* only [-]* <fpnum> allowed*/
				return(ERROR);
			if (min) {
				(tiptop-1)->expval |= (1L << 31);
				*(--(tiptop-1)->fasciip) = '-';
			}
			return(FNDFLOAT);
		case '(':
			if (expr1() != FNDEXPR)
				return(ERROR);
			if (*oplexptr != ')') {
				yyerror("unbalanced parentheses");
				return(ERROR);
			}
			(tiptop-1)->fasciip = NULL;
			oplexptr++;
		case ID:
		case NUM:
			if (min) {
				min = 0;
				if ((tiptop-1)->symptr != NULL)
					yyerror("Illegal unary minus");
				(tiptop-1)->expval = - (tiptop-1)->expval;
				if ((tiptop-1)->fasciip != NULL)
					*(--(tiptop-1)->fasciip) = '-';
			}
			if (op) {	/* perform op */
				tiptop--;
				switch (op) {
				case '+':
					if ((tiptop-1)->symptr == NULL) {
						(tiptop-1)->symptr = tiptop->symptr;
						(tiptop-1)->exptype = tiptop->exptype;
					} else if (tiptop->symptr != NULL) {
						yyerror("Illegal addition");
					}
					(tiptop-1)->expval += tiptop->expval;
					break;
				case '-':
					if ((tiptop-1)->symptr == NULL) {
						if (tiptop->symptr != NULL) {
							yyerror("Illegal subtraction");
						}
						(tiptop-1)->symptr = NULL;
						(tiptop-1)->exptype = ABS;
						(tiptop-1)->expval -= tiptop->expval;
					}
					else if (tiptop->symptr == NULL) {
						(tiptop-1)->expval -= tiptop->expval;
					}
					else if ((tiptop-1)->exptype == tiptop->exptype) {
						(tiptop-1)->expval = 
						((tiptop-1)->symptr->value - 
						tiptop->symptr->value) + 
						((tiptop-1)->expval-tiptop->expval);
						(tiptop-1)->symptr = NULL;
						(tiptop-1)->exptype = ABS;
					}
					else {
						yyerror("Illegal subtraction");
						(tiptop-1)->symptr = NULL;
						(tiptop-1)->exptype = ABS;
						(tiptop-1)->expval = 0;
					}
					break;
				case '*':
					if ((tiptop-1)->symptr != NULL || 
						tiptop->symptr != NULL) {
						yyerror("Illegal multiplication");
					}
					(tiptop-1)->symptr = NULL;
					(tiptop-1)->exptype = ABS;
					(tiptop-1)->expval *= tiptop->expval;
					break;
				case '/':
					if ((tiptop-1)->symptr != NULL || 
						tiptop->symptr != NULL) {
						yyerror("Illegal division");
					}
					(tiptop-1)->symptr = NULL;
					(tiptop-1)->exptype = ABS;
					(tiptop-1)->expval /= tiptop->expval;
					break;
				}
				(tiptop-1)->fasciip = NULL;
			}
			break;
		default:
			return(ERROR);
		}
		switch(*oplexptr) {
		case '+': op = '+'; oplexptr++; break;
		case '-': op = '-'; oplexptr++; break;
		case '*': op = '*'; oplexptr++; break;
		case '/': op = '/'; oplexptr++; break;
		default: return(FNDEXPR);
		}
	}
}


/* lexical analyzer for operand tokens */
static
oplex()
{
	register char	*ahead = oplexptr;
	register char	tmpchar;

	switch (*ahead) {
	case 'A': case 'B': case 'C': case 'D': case 'E':
	case 'F': case 'G': case 'H': case 'I': case 'J':
	case 'K': case 'L': case 'M': case 'N': case 'O':
	case 'P': case 'Q': case 'R': case 'S': case 'T':
	case 'U': case 'V': case 'W': case 'X': case 'Y':
	case 'Z': case 'a': case 'b': case 'c': case 'd':
	case 'e': case 'f': case 'g': case 'h': case 'i':
	case 'j': case 'k': case 'l': case 'm': case 'n':
	case 'o': case 'p': case 'q': case 'r': case 's':
	case 't': case 'u': case 'v': case 'w': case 'x':
	case 'y': case 'z': case '_': case '.':
	{
		register symbol *symptr;

		while (IDFOLLOW(*++ahead)) ;
		if (tmpchar = *ahead) {
			*ahead = '\0';
			symptr = lookup(oplexptr, INSTALL, USRNAME)->stp;
			*ahead = tmpchar;
		} else
			symptr = lookup(oplexptr, INSTALL, USRNAME)->stp;
		oplexptr = ahead;
		if ((tiptop->exptype = symptr->styp & TYPE) == ABS) {
			tiptop->expval = symptr->value;
			tiptop->symptr = NULL;
		} else {
			tiptop->symptr = symptr;
			tiptop->expval = 0;
		}
		tiptop->fasciip = NULL;
		tiptop++;
		return(ID);
	}
	case '0': case '1': case '2': case '3': case '4':
	case '5': case '6': case '7': case '8': case '9':
	{
		register long val;
		register base;
		register c;
		OPERAND fpval;

		if ((val = *ahead - '0') == 0) {
			c = *++ahead;
			if (c == 'x' || c == 'X') {
				base = 16;
				val = 0;
				while (HEXCHAR(c = *++ahead)) {
					val <<= 4;
					if ('a' <= c && c <= 'f')
						val += 10 + c - 'a';
					else if ('A'<= c && c <= 'F')
						val += 10 + c - 'A';
					else	
						val += c - '0';
					}
			} else {
				if (c == '.' || c == 'E' || c == 'e')
					base = 10;	/* fp number */
				else if (OCTCHAR(c)) {
					base = 8;
					val = c - '0';
					while(OCTCHAR(c = *++ahead)) {
						val <<= 3;
						val += c - '0';
					}
				} else {	/* just 0 */
					val = 0;
				}
			}
		} else {
			base = 10;
			while (DECCHAR(c = *++ahead)) {
				val *= 10;
				val += c - '0';
			}
		}
		/* check if its a fp number */
		if ((base == 10) && (c == '.' || c == 'E' || c == 'e')) {
			do {
				if (c == 'E' || c == 'e') {
					/* exp for fp number, next char might
					   be a sign (PLUS/MINUS) so get it here */
					if (((c = *++ahead) != '+') && (c != '-'))
						ahead--;
				}
			} while (isdigit(c = *++ahead) || isalpha(c));

			*ahead = '\0';
			if (validfp(oplexptr)) {
				tiptop->fasciip = oplexptr;
				*(oplexptr = ahead) = c;
				tiptop->symptr = NULL;
				tiptop++;
				return(FPNUM);
			} else {
				yyerror("invalid floating point constant");
				fprintf(stderr, "\t\t... float constant: \"%s\"\n", oplexptr);
				*(oplexptr = ahead) = c;
				return(-1);
			}
		} else {
			*ahead = '\0';
			tiptop->fasciip = oplexptr;
			*(oplexptr = ahead) = c;
			tiptop->exptype = ABS;
			tiptop->expval = val;
			tiptop->symptr = NULL;
			tiptop++;
			return(NUM);
		}
	}
	case '%' : { register int regcode = REG;
		switch (*++oplexptr){
		case 'r':
			switch(*++oplexptr) {
			case '0': tiptop->reg = 0; break;
			case '1': switch (*++oplexptr) {
				  case '0' : tiptop->reg = 10; break;
				  case '1' : tiptop->reg = 11; break;
				  case '2' : tiptop->reg = 12; break;
				  case '3' : tiptop->reg = 13; break;
				  case '4' : tiptop->reg = 14; break;
				  case '5' : tiptop->reg = 15; break;
				  default  : oplexptr--; tiptop->reg = 1; break;
				  }
				  break;
			case '2': tiptop->reg = 2; break;
			case '3': tiptop->reg = 3; break;
			case '4': tiptop->reg = 4; break;
			case '5': tiptop->reg = 5; break;
			case '6': tiptop->reg = 6; break;
			case '7': tiptop->reg = 7; break;
			case '8': tiptop->reg = 8; break;
			case '9': tiptop->reg = 9; break;
			default : goto badreg;
			}
			break;
		case 'a':
			if (*++oplexptr == 'p')
				tiptop->reg = 10;
			else
				goto badreg;
			break;
		case 'f':
			switch (*++oplexptr) {
			case 'p' : tiptop->reg = 9; break;
			case '0' : tiptop->reg = 0; regcode = FREG; break;
			case '1' : tiptop->reg = 1; regcode = FREG; break;
			case '2' : tiptop->reg = 2; regcode = FREG; break;
			case '3' : tiptop->reg = 3; regcode = FREG; break;
			default  : goto badreg;
			}
			break;
		case 's':
			switch (*++oplexptr) {
			case 'p' : tiptop->reg = 12; break;
			case '0' : tiptop->reg = 0; regcode = SREG; break;
			case '1' : tiptop->reg = 1; regcode = SREG; break;
			case '2' : tiptop->reg = 2; regcode = SREG; break;
			case '3' : tiptop->reg = 3; regcode = SREG; break;
			default  : goto badreg;
			}
			break;
		case 'x':
			switch (*++oplexptr) {
			case '0' : tiptop->reg = 0; regcode = XREG; break;
			case '1' : tiptop->reg = 1; regcode = XREG; break;
			case '2' : tiptop->reg = 2; regcode = XREG; break;
			case '3' : tiptop->reg = 3; regcode = XREG; break;
			default  : goto badreg;
			}
			break;
		case 'd':
			switch (*++oplexptr) {
			case '0' : tiptop->reg = 0; regcode = DREG; break;
			case '1' : tiptop->reg = 1; regcode = DREG; break;
			case '2' : tiptop->reg = 2; regcode = DREG; break;
			case '3' : tiptop->reg = 3; regcode = DREG; break;
			default  : goto badreg;
			}
			break;
		case 'i':
			if (*++oplexptr != 's' || *++oplexptr != 'p')
				goto badreg;
			tiptop->reg = 14; break;
		case 'p':
			switch(*++oplexptr) {
			case 'c' : if (oplexptr[1] == 'b' && oplexptr[2] == 'p') {
					oplexptr += 2;
					tiptop->reg = 13;
					break;
				   }
				   tiptop->reg = 15;
				   break;
			case 's' : if (*++oplexptr != 'w') goto badreg;
				   tiptop->reg = 11; break;
			default  : goto badreg;
			}
			break;
		default:
badreg:
			yyerror("invalid register name");
			return(-1);
		}
		tiptop++;
		if (IDFOLLOW(*++oplexptr)) goto badreg;
		return(regcode);
		}

	case '\0': return(*oplexptr);
	default  : return(*oplexptr++);
	}
}


/* print out an operand */
opnd_dmp(opndptr)
OPERAND *opndptr;
{
	switch (opndptr->type) {
	case IMMD 	: printf("IMMD	");
			printf("expval = %d	", opndptr->expval);
			break;
	case FPIMMD	: printf("FPIMMD	");
			printf("expval = %d	", opndptr->expval);
			break;
	case ABSMD	: printf("ABSMD	");
			printf("symptr = 0x%x	", opndptr->symptr);
			printf("expval = %d	", opndptr->expval);
			break;
	case ABSDFMD	: printf("ABSDFMD	");
			printf("symptr = 0x%x	", opndptr->symptr);
			printf("expval = %d	", opndptr->expval);
			break;
	case EXADMD	: printf("EXADMD	");
			printf("symptr = 0x%x	", opndptr->symptr);
			printf("expval = %d	", opndptr->expval);
			break;
	case EXADDFMD	: printf("EXADDFMD	");
			printf("symptr = 0x%x	", opndptr->symptr);
			printf("expval = %d	", opndptr->expval);
			break;
	case DSPMD	: printf("DSPMD	");
			printf("symptr = 0x%x	", opndptr->symptr);
			printf("expval = %d	", opndptr->expval);
			printf("reg = %d	", opndptr->reg);
			break;
	case DSPDFMD	: printf("DSPDFMD	");
			printf("symptr = 0x%x	", opndptr->symptr);
			printf("expval = %d	", opndptr->expval);
			printf("reg = %d	", opndptr->reg);
			break;
	case REGMD	: printf("REGMD	");
			printf("reg = %d	", opndptr->reg);
			break;
	case REGDFMD	: printf("REGDFMD	");
			printf("reg = %d	", opndptr->reg);
			break;
	case FREGMD	: printf("FREGMD	");
			printf("reg = %d	", opndptr->reg);
			break;
	case SREGMD	: printf("SREGMD	");
			printf("reg = %d	", opndptr->reg);
			break;
	case DREGMD	: printf("DREGMD	");
			printf("reg = %d	", opndptr->reg);
			break;
	case XREGMD	: printf("XREGMD	");
			printf("reg = %d	", opndptr->reg);
			break;
	default		: printf("UNKNOWN ");
			break;
	}
	printf("\n");
}
