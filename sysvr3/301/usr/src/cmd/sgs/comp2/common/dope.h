/*	Copyright (c) 1984 AT&T	*/
/*	  All Rights Reserved  	*/

/*	THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T	*/
/*	The copyright notice above does not evidence any   	*/
/*	actual or intended publication of such source code.	*/

#ident	"@(#)pcc2:common/dope.h	10.1"
int dope[ DSIZE ];
char *opst[DSIZE];

struct dopest 
{
	int dopeop; 
	char opst[8]; 
#ifdef	STY
	char * opst2;		/* string corresponding to op's #define value */
#endif
	int dopeval; 
} indope[] = 
#ifndef	STY
#define	dopedef(value, printstring, flags) \
	value, printstring, flags
#else
#define	dopedef(value, printstring, flags) \
	value, printstring, "value", flags
#endif
{
	dopedef(NAME, "NAME", LTYPE ),
	dopedef(STRING, "STRING", LTYPE ),
	dopedef(REG, "REG", LTYPE ),
	dopedef(TEMP, "TEMP", LTYPE ),
	dopedef(VAUTO, "AUTO", LTYPE ),
	dopedef(VPARAM, "PARAM", LTYPE ),
	dopedef(ICON, "ICON", LTYPE ),
	dopedef(FCON, "FCON", LTYPE ),
	dopedef(CCODES, "CCODES", LTYPE ),
	dopedef(UNARY MINUS, "U-", UTYPE ),
	dopedef(STAR, "STAR", UTYPE ),
	dopedef(UNARY AND, "U&", UTYPE ),
	dopedef(UNARY CALL, "UCALL", UTYPE|CALLFLG ),
	dopedef(UNARY FORTCALL, "UFCALL", UTYPE|CALLFLG ),
	dopedef(NOT, "!", UTYPE|LOGFLG ),
	dopedef(COMPL, "~", UTYPE ),
	dopedef(INIT, "INIT", UTYPE ),
	dopedef(CONV, "CONV", UTYPE ),
	dopedef(PLUS, "+", BITYPE|FLOFLG|SIMPFLG|COMMFLG ),
	dopedef(ASG PLUS, "+=", BITYPE|ASGFLG|ASGOPFLG|FLOFLG|SIMPFLG|COMMFLG ),
	dopedef(MINUS, "-", BITYPE|FLOFLG|SIMPFLG ),
	dopedef(ASG MINUS, "-=", BITYPE|FLOFLG|SIMPFLG|ASGFLG|ASGOPFLG ),
	dopedef(MUL, "*", BITYPE|FLOFLG|MULFLG ),
	dopedef(ASG MUL, "*=", BITYPE|FLOFLG|MULFLG|ASGFLG|ASGOPFLG ),
	dopedef(AND, "&", BITYPE|SIMPFLG|COMMFLG ),
	dopedef(ASG AND, "&=", BITYPE|SIMPFLG|COMMFLG|ASGFLG|ASGOPFLG ),
	dopedef(QUEST, "?", BITYPE ),
	dopedef(COLON, ":", BITYPE ),
	dopedef(ANDAND, "&&", BITYPE|LOGFLG ),
	dopedef(OROR, "||", BITYPE|LOGFLG ),
	dopedef(CM, ",", BITYPE ),
	dopedef(COMOP, ",OP", BITYPE ),
	dopedef(FREE, "FREE!?!", LTYPE ),
	dopedef(ASSIGN, "=", BITYPE|ASGFLG ),
	dopedef(DIV, "/", BITYPE|FLOFLG|MULFLG|DIVFLG ),
	dopedef(ASG DIV, "/=", BITYPE|FLOFLG|MULFLG|DIVFLG|ASGFLG|ASGOPFLG ),
	dopedef(MOD, "%", BITYPE|DIVFLG ),
	dopedef(ASG MOD, "%=", BITYPE|DIVFLG|ASGFLG|ASGOPFLG ),
	dopedef(LS, "<<", BITYPE|SHFFLG ),
	dopedef(ASG LS, "<<=", BITYPE|SHFFLG|ASGFLG|ASGOPFLG ),
	dopedef(RS, ">>", BITYPE|SHFFLG ),
	dopedef(ASG RS, ">>=", BITYPE|SHFFLG|ASGFLG|ASGOPFLG ),
	dopedef(OR, "|", BITYPE|COMMFLG|SIMPFLG ),
	dopedef(ASG OR, "|=", BITYPE|COMMFLG|SIMPFLG|ASGFLG|ASGOPFLG ),
	dopedef(ER, "^", BITYPE|COMMFLG|SIMPFLG ),
	dopedef(ASG ER, "^=", BITYPE|COMMFLG|SIMPFLG|ASGFLG|ASGOPFLG ),
	dopedef(INCR, "++", BITYPE|ASGFLG ),
	dopedef(DECR, "--", BITYPE|ASGFLG ),
	dopedef(STREF, "->", BITYPE ),
	dopedef(CALL, "CALL", BITYPE|CALLFLG ),
	dopedef(FORTCALL, "FCALL", BITYPE|CALLFLG ),
	dopedef(EQ, "==", BITYPE|LOGFLG ),
	dopedef(NE, "!=", BITYPE|LOGFLG ),
	dopedef(LE, "<=", BITYPE|LOGFLG ),
	dopedef(LT, "<", BITYPE|LOGFLG ),
	dopedef(GE, ">=", BITYPE|LOGFLG ),
	dopedef(GT, ">", BITYPE|LOGFLG ),
	dopedef(UGT, "UGT", BITYPE|LOGFLG ),
	dopedef(UGE, "UGE", BITYPE|LOGFLG ),
	dopedef(ULT, "ULT", BITYPE|LOGFLG ),
	dopedef(ULE, "ULE", BITYPE|LOGFLG ),
	dopedef(ARS, ">>A", BITYPE ),
	dopedef(TYPE, "TYPE", LTYPE ),
	dopedef(LB, "[", BITYPE ),
	dopedef(CBRANCH, "CBRANCH", BITYPE ),
	dopedef(GENLAB, "GENLAB", UTYPE ),
	dopedef(GENUBR, "GENUBR", UTYPE ),
	dopedef(GENBR, "GENBR", UTYPE ),
	dopedef(CMP, "CMP", BITYPE ),
	dopedef(CMPE, "CMPE", BITYPE ),		/* IEEE exception raising fp comparison */
	dopedef(FLD, "FLD", UTYPE ),
	dopedef(PMUL, "P*", BITYPE ),
	dopedef(PDIV, "P/", BITYPE ),
#ifdef	CG
	dopedef(RETURN, "RETURN", LTYPE ),
#else
	dopedef(RETURN, "RETURN", BITYPE|ASGFLG ),
#endif
	dopedef(CAST, "CAST", BITYPE|ASGFLG ),
	dopedef(GOTO, "GOTO", UTYPE ),
	dopedef(STASG, "STASG", BITYPE|STRFLG ),
	dopedef(STARG, "STARG", UTYPE|STRFLG ),
	dopedef(STCALL, "STCALL", BITYPE|CALLFLG|STRFLG ),
	dopedef(UNARY STCALL, "USTCALL", UTYPE|CALLFLG|STRFLG ),
	dopedef(RNODE, "RNODE", LTYPE ),
	dopedef(SNODE, "SNODE", LTYPE ),
	dopedef(QNODE, "QNODE", LTYPE ),
	dopedef(MANY, "MANY", BITYPE ),
	dopedef(FUNARG, "ARG", UTYPE ),
	dopedef(UOP0, "UOP0", UTYPE|SPFLG ),
	dopedef(UOP1, "UOP1", UTYPE|SPFLG ),
	dopedef(UOP2, "UOP2", UTYPE|SPFLG ),
	dopedef(UOP3, "UOP3", UTYPE|SPFLG ),
	dopedef(UOP4, "UOP4", UTYPE|SPFLG ),
	dopedef(UOP5, "UOP5", UTYPE|SPFLG ),
	dopedef(UOP6, "UOP6", UTYPE|SPFLG ),
	dopedef(UOP7, "UOP7", UTYPE|SPFLG ),
	dopedef(UOP8, "UOP8", UTYPE|SPFLG ),
	dopedef(UOP9, "UOP9", UTYPE|SPFLG ),
#ifdef IN_LINE
	dopedef(INCALL, "INCALL", BITYPE|CALLFLG ),
	dopedef(UNARY INCALL, "UINCALL", UTYPE|CALLFLG ),
#endif
#ifdef	CG
	dopedef(COPY, "COPY", LTYPE ),
	dopedef(COPYASM, "COPYASM", LTYPE|CALLFLG ),
	dopedef(CURCAP, "CURCAP", LTYPE ),
	dopedef(NOP, "NOP", LTYPE ),
	dopedef(BEGF, "BEGF", LTYPE ),
	dopedef(ENTRY, "ENTRY", LTYPE),
	dopedef(PROLOG, "PROLOG", LTYPE),
	dopedef(ENDF, "ENDF", LTYPE ),
	dopedef(LOCCTR, "LOCCTR", LTYPE ),
	dopedef(SWBEG, "SWBEG", UTYPE ),
	dopedef(SWCASE, "SWCASE", LTYPE ),
	dopedef(SWEND, "SWEND", LTYPE ),
	dopedef(ALLOC, "ALLOC", UTYPE ),
	dopedef(DEFNAM, "DEFNAM", LTYPE ),
	dopedef(UNINIT, "UNINIT", LTYPE ),
	dopedef(CURFRAME, "CURFRAME", LTYPE ),
	dopedef(FCHAIN, "FCHAIN", UTYPE ),
	dopedef(FSELECT, "FSELECT", BITYPE ),
	dopedef(BMOVE, "BMOVE", BITYPE ),
	dopedef(BMOVEO, "BMOVEO", BITYPE ),
	dopedef(EXSETV, "EXSETV", UTYPE ),
	dopedef(EXGETV, "EXGETV", LTYPE ),
	dopedef(EXCLEAR, "EXCLEAR", LTYPE ),
	dopedef(EXTEST, "EXTEST", LTYPE ),
	dopedef(EXRAISE, "EXRAISE", LTYPE ),
	dopedef(JUMP, "JUMP", LTYPE ),
	dopedef(SINIT, "SINIT", LTYPE ),
	dopedef(LET, "LET", BITYPE ),
	dopedef(CSE, "CSE", LTYPE ),
	dopedef(ALIGN, "ALIGN", LTYPE ),
	dopedef(NPRETURN, "NPRETURN", LTYPE ),
	dopedef(VLRETURN, "VLRETURN", BITYPE ),
	dopedef(FCONV, "FCONV", UTYPE ),
	dopedef(BCMP, "BCMP", BITYPE ),
	dopedef(SEMI, ";", BITYPE ),
	dopedef(UNARY MUL, "*", UTYPE ),
#endif	/* def CG */

	-1,	0,	0
};
#undef	dopedef

mkdope()
{
	register struct dopest *q;

	for( q = indope; q->dopeop >= 0; ++q )
	{
		/* Sorry about this....  But, better to find out things are
		** messed up here, even if it takes funny code.
		*/
		if (q->dopeop >= DSIZE)
#ifdef	STY
		    yyerror
#else
		    cerror
#endif
		    	("bad dope[] OP %s\n", q->opst);
		dope[q->dopeop] = q->dopeval;
		opst[q->dopeop] = q->opst;
	}
}
