%{	/*	VAX 11/780 and 11/750 assembler parse logic */

#include <stdio.h>

#include "symbols.h"
#include "instab.h"
#include "gendefs.h"

#define	TXTCHARS	30

#define defsym(x)	((x.exptype==TSYM)&&((x.symptr->styp&TYPE)!=UNDEF))


extern unsigned short
		cline, /* line number from the .ln pseudo op */
		line; /* current input file line number */

extern short
		readonly, /* put .data stuff into .text segment */
		opt; /* sdi optimization flag */

extern FILE
		*fdin; /* input source */

extern char
		file[], /* assembly input file name */
		cfile[]; /* original C file name */

extern symbol	*dot;	   /* pc pointer symbol */


extern long	newdot,    /* up to date value of pc pointer symbol */
		dotbss;    /* bss pointer symbol */

BYTE	bytesout;	   /* output displacement */
short	bitsout = 0;	   /* nbr of bits accumulated with bit fields */
long	outword = 0;	   /* bit field accumulator */

rexpr	operate(); /* procedure to perform expression operations */

extern upsymins	*lookup(); /* symbol table access routine */

static BYTE defseen = 0; /* flag set if .def expression found on this line */

%}

%union	{
		long	unlong;
		double	unfloat;
		rexpr	unrexpr;
		symbol	*unsymptr;
		instr	*uninsptr;
		addrmode unaddr;
		char	*unstrptr;
	}


%token	<uninsptr>	ZEROP
%token	<uninsptr>	ONEOP
%token	<uninsptr>	TWOOP
%token	<uninsptr>	THROP
%token	<uninsptr>	FOUOP
%token	<uninsptr>	FIVOP
%token	<uninsptr>	SIXOP

%token	<unlong>	ISPACE
%token	<unlong>	IBYTE
%token	<unlong>	IWORD
%token	<unlong>	ILONG
%token	<unlong>	IINT
%token	<unlong>	IDATA
%token	<unlong>	IBSS
%token	<unlong>	IGLOBL
%token	<unlong>	ISET
%token	<unlong>	ITEXT
%token	<unlong>	ICOMM
%token	<unlong>	IALIGN
%token	<unlong>	IFLOAT
%token	<unlong>	IDOUBLE
%token	<unlong>	IORG

/* Symbolic Debugger Support Pseudo Operations */

%token	<unlong>	IFILE ILN IDEF IENDEF
%token	<unlong>	IVAL ISCL ITYPE ILINE ITAG ISIZE IDIM

/* Lexical Tokens recognized by yylex */

%token	<unlong>	SP NL SH LITOP REG REGOP AND SQ LP RP MUL PLUS COMMA 
%token	<unlong>	MINUS ALPH DIV DIG COLON SEMI LSH RSH LB RB XOR
%token	<unlong>	IOR TILDE DQ

/* define operator precedences so that yacc can make a clean parser */

%nonassoc	COLON
%left		PLUS MINUS
%left		IOR XOR AND
%left		MUL DIV LSH RSH TILDE

%token	<unlong>	INTNUM
%token	<unfloat>	FLTNUM
%token	<unsymptr>	IDENT
%token	<unlong>	ERROR
%token	<unstrptr>	STRING

%type	<unrexpr>	iexpr fexpr expr

%type	<unaddr>	operand subop1 subop2

%type	<unlong>	reg

%%

file:		/* empty */ 
					
	| 	file labinst NL =	{ line++;
					  dot->value = newdot;
					  generate(0,NEWSTMT,(long)line,NULLSYM);
					}
	| 	file labinst SEMI =	{ dot->value = newdot;
					  generate(0,NEWSTMT,(long)line,NULLSYM);
					}
	|	file error NL =		{ line++;
					  yyerrok;
					  dot->value = newdot;
					}
		;

labinst:	labels instruction
		;

labels:		/* no label is okay */
	|	labels IDENT COLON =	{ if (($2->styp&TYPE) != UNDEF)
						yyerror("multiply defined label");
					  $2->value = newdot;
					  $2->styp |= dot->styp;
					  if (opt & ((dot->styp&TYPE)==TXT)) /* sdi */

						deflab($2);
					}
		;

instruction:	/* no instruction is okay */

/* pseudo operations - assembler directives */

/* .space */	|
		ISPACE expr =	{ if (($2.exptype!=TYPL) ||
				      ($2.expval.lngval <= 0))
					yyerror("illegal .space size expression");
				  fill($2.expval.lngval);
				}

/* .byte */	|
		IBYTE setbyte iexprlist = { if (bitsout) {
					       generate((BITSPBY*bytesout),
						GENVALUE,outword,NULLSYM);
					       outword = 0;
					       bitsout = 0;
					       };
					  }

/* .word */	|
		IWORD setword iexprlist = { if (bitsout) {
					       generate((BITSPBY*bytesout),
						GENVALUE,outword,NULLSYM);
					       outword = 0;
					       bitsout = 0;
					       };
					  }

/* .float */	|
		IFLOAT setfloat fexprlist

/* .double */	|
		IDOUBLE setdouble fexprlist

/* .long */	|
		ILONG setlong iexprlist = { if (bitsout) {
					       generate((BITSPBY*bytesout),
						GENVALUE,outword,NULLSYM);
					       outword = 0;
					       bitsout = 0;
					       };
					  }

/* .int */	|
		IINT setint iexprlist = { if (bitsout) {
					       generate((BITSPBY*bytesout),
						GENVALUE,outword,NULLSYM);
					       outword = 0;
					       bitsout = 0;
					       };
					  }

/* .text */	|
		ITEXT = { cgsect(TXT);
			}		 

/* .data */	|
		IDATA = { if (!readonly)
				cgsect(DAT);
			  else
				cgsect(TXT);
			}

/* .bss */	|
		IBSS IDENT COMMA expr COMMA expr = 
			{ long mod;
			  if ($4.exptype != TYPL)
				yyerror(".bss size not absolute");
			  if ($6.exptype != TYPL)
				yyerror(".bss alignment not absolute");
			  if ($4.expval.lngval <= 0)
				yyerror("illegal .bss size");
			  if ($6.expval.lngval <= 0)
				yyerror("illegal .bss alignment");
			  if (($2->styp&TYPE) != UNDEF)
				yyerror("multiply defined .bss label");
			  if (mod = dotbss % $6.expval.lngval)
				dotbss += $6.expval.lngval - mod;
			  $2->value = dotbss;
			  dotbss += $4.expval.lngval;
			  $2->styp |= BSS;
			}
		

/* .globl */	|
		IGLOBL IDENT =	{ $2->styp = $2->styp | EXTERN;
				}

/* .set */	|
		ISET IDENT COMMA expr = { if ($4.exptype != TYPL)
						yyerror("illegal expression in .set");
					   else if ($2==NULL)
						yyerror("unable to define symbol");
					   else {
						$2->value = $4.expval.lngval;
						$2->styp |= ABS;
						};
					 }

/* .comm */	|
		ICOMM IDENT COMMA expr = { if ($4.exptype!=TYPL)
						yyerror("illegal .comm size expression");
					    else if (($2->styp&TYPE)!=UNDEF)
						yyerror("illegal attempt to redefine symbol");
					    else {
						$2->styp = (EXTERN | UNDEF);
						$2->value = $4.expval.lngval;
						};
					  }

/* .align */	|
		IALIGN expr = { if (($2.exptype!=TYPL) ||
				     ($2.expval.lngval < 1) ||
				     ($2.expval.lngval > 1024))
					yyerror("illegal .align expression");
				  else
				     ckalign($2.expval.lngval);
			       }

/* .org */	|
		IORG expr = { int incr;
			       if (($2.exptype!=TYPL)&&(!defsym($2)))
					yyerror("illegal .org expression");
			       else if (defsym($2)) {	
					if ($2.symptr->styp!=dot->styp)
						yyerror("incompatible .org expression type");
				        incr = $2.expval.lngval + $2.symptr->value -
						dot->value;
						}
			       else { /* absolute origins */
				        werror("usage of absolute origins");						incr = $2.expval.lngval - dot->value;
				  	};
			       if (incr < 0)
					yyerror("cannot decrement value of '.'");
			       else
					fill(incr);

			     }

/* Symbolic Debugger (SDB) support pseudo operations */

/* .file */	|
		IFILE STRING = { if (cfile[0] != '\0')
					yyerror("only 1 .file allowed");
				 if (strlen($2) > 14)
					yyerror(".file name exceeds 14 chars");
				 else
					strcpy(cfile,$2);
				 generate(0,SETFILE,NULLVAL,NULLSYM);
			       }

/* .ln */	|
		ILN expr = { cline = (short)$2.expval.lngval;
			     generate(0,LINENBR,$2.expval.lngval,$2.symptr);
			   }

		|
		ILN expr COMMA expr = { cline = (short)$2.expval.lngval;
					generate(0,LINENUM,$2.expval.lngval,
								$2.symptr);
					generate(0,LINEVAL,$4.expval.lngval,
								$4.symptr);
				      }

/* .def */	|
		IDEF {defseen = 1;} IDENT = { generate(0,DEFINE,NULLVAL,$3);
					      defseen = 0;
			     }

/* .endef */	|
		IENDEF = { generate(0,ENDEF,NULLVAL,NULLSYM);
		}

/* .val */	|
		IVAL expr = { generate(0,SETVAL,$2.expval.lngval,$2.symptr);
			    }

/* .type */	|
		ITYPE expr = { generate(0,SETTYP,$2.expval.lngval,$2.symptr);
			     }

/* .line */	|
		ILINE expr = { generate(0,SETLNO,$2.expval.lngval,$2.symptr);
			     }

/* .size */	|
		ISIZE expr = { generate(0,SETSIZ,$2.expval.lngval,$2.symptr);
			     }

/* .scl */	|
		ISCL expr = { generate(0,SETSCL,$2.expval.lngval,$2.symptr);
			    }

/* .tag */	|
		ITAG IDENT = { generate(0,SETTAG,NULLVAL,$2);
			     }

/* .dim */	| 
		dotdim /* can be .dim expr [, expr ] */



/* Instructions */

/* INST */	|

		ZEROP 
			= { generate($1->nbits,GENVALUE,$1->opcode,NULLSYM);
			  }

/* INST op1 */	|

		ONEOP operand 
			= { if ($1->tag==EXTIN1 || $1->tag==EXTIN2)
				gensdi($1,$2,NULL,NULL);
			    else {
			    	generate($1->nbits,GENVALUE,$1->opcode,NULLSYM);
			    	genopr($2,1,$1->tag);
				};
			  }

/* INST op1, op2 */ |

		TWOOP operand COMMA operand
			= { if ($1->tag==EXTIN4)
				gensdi($1,$2,$4,NULL);
			    else {
			    	generate($1->nbits,GENVALUE,$1->opcode,NULLSYM);
			    	genopr($2,1,$1->tag);
			    	genopr($4,2,$1->tag);
				};
			  }

/* INST op1, op2, op3 */ |

		THROP operand COMMA operand COMMA operand
			= { if ($1->tag==EXTIN3)
				gensdi($1,$2,$4,$6);
			    else {
			    	generate($1->nbits,GENVALUE,$1->opcode,NULLSYM);
			    	genopr($2,1,$1->tag);
			    	genopr($4,2,$1->tag);
			    	genopr($6,3,$1->tag);
				};
			  }

/* INST op1, op2, op3, op4 */ |

		FOUOP operand COMMA operand COMMA operand COMMA operand
			= { generate($1->nbits,GENVALUE,$1->opcode,NULLSYM);
			    genopr($2,1,$1->tag);
			    genopr($4,2,$1->tag);
			    genopr($6,3,$1->tag);
			    genopr($8,4,$1->tag);
			  }

/* INST op1, op2 op3, op4, op5 */ |

		FIVOP operand COMMA operand COMMA operand COMMA operand COMMA
		      operand
			= { generate($1->nbits,GENVALUE,$1->opcode,NULLSYM);
			    genopr($2,1,$1->tag);
			    genopr($4,2,$1->tag);
			    genopr($6,3,$1->tag);
			    genopr($8,4,$1->tag);
			    genopr($10,5,$1->tag);
			  }

/* INST op1, op2, op3, op4, op5, op6 */ |

		SIXOP operand COMMA operand COMMA operand COMMA operand COMMA
		      operand COMMA operand
			= { generate($1->nbits,GENVALUE,$1->opcode,NULLSYM);
			    genopr($2,1,$1->tag);
			    genopr($4,2,$1->tag);
			    genopr($6,3,$1->tag);
			    genopr($8,4,$1->tag);
			    genopr($10,5,$1->tag);
			    genopr($12,6,$1->tag);
			  }
			;


setbyte:	= { bytesout = 1;
		  };

setlong:	= { bytesout = 4;
		  };

setint:		= { bytesout = 4;
		  };

setword:	= { bytesout = 2;
		  };

setfloat:	= { bytesout = 4;
		  };

setdouble:	= { bytesout = 8;
		  };


/* continuation of .dim pseudo operation definitions */

dotdim:		IDIM expr = { generate(0,SETDIM1,$2.expval.lngval,$2.symptr);
			    }
		|
		dotdim COMMA expr = { generate(0,SETDIM2,$3.expval.lngval,
							$3.symptr);
				    }
		;


	
operand:	/* %r */
		reg = { $$.admode = AREG;
			$$.adreg1 = $1;
		      }

	|	/* ... [%r] */
		subop1 LB reg RB = { $$.admode = $1.admode | AINDX;
				     $$.adreg2 = $3;
				   }

	|	subop1
	;


subop1:		/* -(%r) */
		MINUS LP reg RP = { $$.admode = ADECR;
				    $$.adreg1 = $3;
				  }

	|	/* * ... */
		MUL subop2 = { $$ = $2;
		               if ($2.admode==ABASE) {
				  /* *(%r) is really *0(%r) */
				  $$.admode = (ADISP | ASTAR);
				  $$.adexpr.exptype = TYPL;
				  $$.adexpr.expval.lngval = 0L;
			     	  }
			       else
			       	$$.admode = $2.admode | ASTAR;
		             }

	| subop2
	;

subop2:		/* expression */
		expr = { $$.admode = AEXP;
			 $$.adexpr = $1;
			 $$.adreg1 = $$.adreg2 = 0;
		       }

	|	/* (%r) */
		LP reg RP = { $$.admode = ABASE;
			      $$.adreg1 = $2;
			    }

	|	/* expression (%r)  */
		expr LP reg RP = { $$.admode = ADISP;
				   $$.adreg1 = $3;
				   $$.adexpr = $1;
				 }

	|	/* (%r)+ */
		LP reg RP PLUS = { $$.admode = AINCR;
				   $$.adreg1 = $2;
				 }

	|	/* $ expression */
		LITOP expr = { $$.admode = AIMM;
			       $$.adreg1 = $$.adreg2 = 0;
			       $$.adexpr = $2;
			     }
	;



reg:		REG = { $$ = $1;
		      }
	
	|
		REGOP expr = { if ((($2.exptype&TINT)==0) ||
				   ($2.expval.lngval < 0) ||
				   ($2.expval.lngval > 15))
					yyerror("illegal register specification");
			       else
				 $$ = $2.expval.lngval;
			     }
	;


			    
iexprlist:	iexpr
		|	iexprlist COMMA iexpr
		;


fexprlist:	fexpr
		|	fexprlist COMMA fexpr
		;



fexpr:		expr = { if (($1.exptype&TFLT)==0)
				yyerror("illegal floating point expression");
			  $$=$1;
			  if (bytesout == 4)
			  generate((BITSPBY*bytesout),GENVALUE,$1.expval.dblval,
					NULLSYM);
			  else if (bytesout == 8) { int *ptr;
			  ptr=(int *)&$1.expval.dblval;
			  generate((BITSPBY*4),GENVALUE,*ptr,NULLSYM);
			  ptr++;
			  generate((BITSPBY*4),GENVALUE,*ptr,NULLSYM); }
			  else aerror("illegal floating point size");
		       }
		;


iexpr:		expr = { if (($1.exptype&TFLT)!=0)
				yyerror("illegal fixed point expression");
			 if (bitsout) {
				generate((BITSPBY*bytesout),
				 	 GENVALUE,outword,NULLSYM);
				outword = 0;
				bitsout = 0;
				};
			  $$=$1;
			  if ($1.exptype&TDIFF) /* process the special case of
					    (sym - sym) type expression */
			  	generate((BITSPBY*bytesout),GENABSSYM,
					$1.expval.lngval,$1.symptr);
			  else /* generate a relocation entry if you use a
				  symbol in the expression */
			  	generate((BITSPBY*bytesout),GENRELOC,
					$1.expval.lngval,$1.symptr);
		       }

	| /* bit field expression nn:expr */
		expr COLON expr = { if ($1.exptype != TYPL)
					yyerror("illegal bit field expression");
			 	    if (($3.exptype&TINT)==0)
					yyerror("cannot relocate bit field expresion");	
				    if ($1.expval.lngval > (BITSPBY*bytesout))
					yyerror("bit field crosses data boundary");
				    if (($1.expval.lngval+bitsout) > (BITSPBY*bytesout)) {
					  generate(BITSPBY*bytesout,GENVALUE,
						   outword,NULLSYM);
					  bitsout = 0;
					  outword = 0;
					};
				    $3.expval.lngval <<= bitsout;
				    outword |= $3.expval.lngval;
				    bitsout += $1.expval.lngval;
				  }

		;


expr:
		IDENT = { if (($1->styp&TYPE)!=ABS) {
				$$.exptype = TSYM;
			  	$$.symptr = $1;
			  	$$.expval.lngval = 0L;
			  } else { /* an abs symbol is just like an intnum */
				$$.exptype = TYPL;
				$$.symptr = NULL;
				$$.expval.lngval = $1->value;
			  };
			}

	|
		INTNUM = { $$.exptype = TYPL;
			   $$.expval.lngval = $1;
			 }

	|	FLTNUM = { $$.exptype = TYPD;
			   $$.expval.dblval = $1;
			 }

	|	LP expr RP = { $$=$2;
			     }

	|	expr PLUS expr = { $$ = operate(PLUS,$1,$3);
				 }

	|	expr MINUS expr = { $$ = operate(MINUS,$1,$3);
				  }

	|
		expr TILDE expr = { $$ = operate(TILDE,$1,$3);
				  }

	|
		expr IOR expr = { $$ = operate(IOR,$1,$3);
				}

	|
		expr XOR expr = { $$ = operate(XOR,$1,$3);
				}

	| 
		expr AND expr = { $$ = operate(AND,$1,$3);
				}

	|
		expr LSH expr = { $$ = operate(LSH,$1,$3);
				}

	|
		expr RSH expr = { $$ = operate(RSH,$1,$3);
				}

	|
		expr MUL expr = { $$ = operate(MUL,$1,$3);
				}

	|
		expr DIV expr = { $$ = operate(DIV,$1,$3);
				}

	|
		MINUS expr = { $$ = operate(-MINUS,$2,NULL);
			     }

	|
		TILDE expr = { $$ = operate(-TILDE,$2,NULL);
			     }

		;



%%


rexpr operate(op,left,right)

int op;
rexpr left, right;

	{ rexpr result;
	  
	  if (op < 0) { /* unary operator  - or ~ */
		  op = - op;
		  switch (op) {

			case MINUS: switch (left.exptype) {

					case TSYM: result.exptype = TSYM;
						   /* a negative symbol ptr
						      indicates symbol subtract
						      to code gen routines */
						   result.symptr =
						   (symbol*)(-(int)left.symptr);
						   result.expval.lngval =
							left.expval.lngval;
						   return(result);
						   
					case TYPL: result.exptype = TYPL;
						   result.symptr = NULL;
						   result.expval.lngval =
						   - left.expval.lngval;
						   return(result);

					case TYPD: result.exptype = TYPD;
						   result.symptr = NULL;
						   result.expval.dblval =
						   - left.expval.dblval;
						   return(result);

					default: yyerror("illegal expression operand");
				    };

			case TILDE: switch (left.exptype) {

					case TYPL: result.exptype = TYPL;
						   result.symptr = NULL;
						   result.expval.lngval =
						   ~ left.expval.lngval;
						   return(result);

					default: yyerror("illegal expression operand");
				    };

			default: yyerror("illegal expression");
			
			};
	   	}
	  else  /* binary operator */
		{ switch (op)  {

			case DIV: if (left.exptype==TYPL&&right.exptype==TYPL) {
					result.exptype = TYPL;
					result.symptr = NULL;
					result.expval.lngval =
					left.expval.lngval/right.expval.lngval;
					return(result);

				} else

				if (left.exptype==TYPD&&right.exptype==TYPD) {
					result.exptype = TYPD;
					result.symptr = NULL;
					result.expval.dblval =
					left.expval.dblval/right.expval.dblval;
					return(result);

				} else

					yyerror("illegal expression operands");
				break;

			case MUL: if (left.exptype==TYPL&&right.exptype==TYPL) {
					result.exptype = TYPL;
					result.symptr = NULL;
					result.expval.lngval =
					left.expval.lngval*right.expval.lngval;
					return(result);

				} else

				if (left.exptype==TYPD&&right.exptype==TYPD) {
					result.exptype = TYPD;
					result.symptr = NULL;
					result.expval.dblval =
					left.expval.dblval*right.expval.dblval;
					return(result);

				} else

					yyerror("illegal expression operands");
				break;

			case RSH: if (left.exptype==TYPL&&right.exptype==TYPL) {
					result.exptype = TYPL;
					result.symptr = NULL;
					result.expval.lngval =
					left.expval.lngval>>right.expval.lngval;
					return(result);

				} else

					yyerror("illegal expression operands");
				break;

			case LSH: if (left.exptype==TYPL&&right.exptype==TYPL) {
					result.exptype = TYPL;
					result.symptr = NULL;
					result.expval.lngval =
					left.expval.lngval<<right.expval.lngval;
					return(result);

				} else

					yyerror("illegal expression operands");
				break;

			case AND: if (left.exptype==TYPL&&right.exptype==TYPL) {
					result.exptype = TYPL;
					result.symptr = NULL;
					result.expval.lngval =
					left.expval.lngval&right.expval.lngval;
					return(result);

				} else

					yyerror("illegal expression operands");
				break;

			case XOR: if (left.exptype==TYPL&&right.exptype==TYPL) {
					result.exptype = TYPL;
					result.symptr = NULL;
					result.expval.lngval =
					left.expval.lngval^right.expval.lngval;
					return(result);

				} else

					yyerror("illegal expression operands");
				break;

			case IOR: if (left.exptype==TYPL&&right.exptype==TYPL) {
					result.exptype = TYPL;
					result.symptr = NULL;
					result.expval.lngval =
					left.expval.lngval|right.expval.lngval;
					return(result);

				} else

					yyerror("illegal expression operands");
				break;

			case TILDE: if (left.exptype==TYPL&&right.exptype==TYPL) {
					result.exptype = TYPL;
					result.symptr = NULL;
					result.expval.lngval =
					left.expval.lngval|(~right.expval.lngval);
					return(result);

				} else

					yyerror("illegal expression operands");
				break;

			case MINUS: if (left.exptype==TYPL&&right.exptype==TYPL) {
					result.exptype = TYPL;
					result.symptr = NULL;
					result.expval.lngval =
					left.expval.lngval-right.expval.lngval;
					return(result);

				} else

				if (left.exptype==TYPD&&right.exptype==TYPD) {
					result.exptype = TYPD;
					result.symptr = NULL;
					result.expval.dblval =
					left.expval.dblval-right.expval.dblval;
					return(result);

				} else
				
				if ((left.exptype == TSYM) &&
				    (right.exptype == TSYM)) {
					result.exptype = (TSYM | TDIFF);
					result.symptr = right.symptr;
					result.expval.lngval=(long)left.symptr;
					return(result);

				} else

				if ((left.exptype == TYPL) &&
				    (right.exptype == TSYM)) {
					result.exptype = TSYM;
					/* negative symbol ptr indicates symbol
					   subtract to code gen routines */
					result.symptr = 
						(symbol*)(-(int)right.symptr);
					result.expval.lngval = 
					left.expval.lngval-right.expval.lngval;
					return(result);

				} else

				if ((left.exptype == TSYM) &&
				    (right.exptype == TYPL)) {
					result.exptype = TSYM;
					result.symptr = left.symptr;
					result.expval.lngval = 
					left.expval.lngval-right.expval.lngval;
					return(result);

				} else

					yyerror("illegal expression operands");
				break;

			case PLUS: if (left.exptype==TYPL&&right.exptype==TYPL) {
					result.exptype = TYPL;
					result.symptr = NULL;
					result.expval.lngval =
					left.expval.lngval+right.expval.lngval;
					return(result);

				} else

				if (left.exptype==TYPD&&right.exptype==TYPD) {
					result.exptype = TYPD;
					result.symptr = NULL;
					result.expval.dblval =
					left.expval.dblval+right.expval.dblval;
					return(result);

				} else

				if (((left.exptype==TYPL)&&(right.exptype==TSYM)) ||
				    ((left.exptype==TSYM)&&(right.exptype==TYPL))) {
					result.exptype = TSYM;
					result.expval.lngval = 
				        left.expval.lngval+right.expval.lngval;
					if (left.exptype==TSYM) 
						result.symptr = left.symptr;
					else 
						result.symptr = right.symptr;
					return(result);

				} else

					yyerror("illegal expression operands");
				break;

			      default: yyerror("illegal expression");

			      };
		  
	  	};

	}




#if FLEXNAMES
char	yytext[BUFSIZ];
#else
char	yytext[TXTCHARS+2];
#endif

short type[] = {
	EOF,
	SP,	0,	0,	0,	0,	0,	0,	0,
	0,	SP,	NL,	0,	0,	SP,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	0,	0,	0,	0,	0,	0,	0,	0,
	SP,	0,	DQ,	SH,	LITOP,	REG,	AND,	SQ,
	LP,	RP,	MUL,	PLUS,	COMMA,	MINUS,	ALPH,	DIV,
	DIG,	DIG,	DIG,	DIG,	DIG,	DIG,	DIG,	DIG,
	DIG,	DIG,	COLON,	SEMI,	LSH,	0,	RSH,	0,
	0,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	LB,	0,	RB,	XOR,	ALPH,
	0,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,	ALPH,
	ALPH,	ALPH,	ALPH,	0,	IOR,	0,	TILDE,	0,
};

#define INBUFSIZ	2048

static char	inbuf[INBUFSIZ + 2];
static char	*inptr = &inbuf[INBUFSIZ];

#define mygetc(x)	{ if (++inptr > &inbuf[INBUFSIZ]) { \
			  inbuf[fread(&inbuf[1],1,INBUFSIZ,fdin)+1]=EOF; \
			  inptr = &inbuf[1]; \
			  }; \
			  x = *inptr; }

#define unmygetc(x)	*inptr-- = x


yylex()
{
	long value;
	short base;
	register short c;
	register short ctype;
	char fltchr[64];
	char *cptr;
	upsymins	id;
	static BYTE instseen = 0; /* instruction seen on line */

	double atof();

	mygetc(c);
	while (type[c+1] == SP) mygetc(c);

	ctype = type[c+1];
	value = c;

	switch (ctype) {

			case REG: /* %n indicating register n */
				mygetc(value);
				if (value >= '0' && value <= '9') {
					yylval.unlong = value - '0';
					if (value=='1') {
						mygetc(value);
						if (value >= '0' &&
						    value <= '5') 
							yylval.unlong =
								10+value-'0';
						else
							unmygetc(value);
						};
					return(REG);
					}
				else	{
					unmygetc(value);
					return(REGOP);
					};
				
			case ALPH: /* some kind of alpha-numeric string */
				cptr = yytext;
				do {
#if FLEXNAMES
					if (cptr < &yytext[BUFSIZ-1])
#else
					if (cptr <= &yytext[TXTCHARS])
#endif
						*cptr++ = value;
					mygetc(value);
					ctype = type[value+1];
				    } while (ctype == DIG || ctype == ALPH);
#if !FLEXNAMES
				yytext[8] =
#endif
				*cptr = '\0';
				/* look to see if it is a label */
				while (value == ' ' || value == '\t')
					mygetc(value);
				unmygetc(value); /* throw it back */
				if (value == ':') { /* it's a label */
					id = *lookup(yytext,INSTALL,USRNAME);
					yylval.unsymptr = id.stp;
					return(IDENT);
					};
				/* look to see if it is a special symbol
				   or an instruction mnemonic */
				if (!instseen) { /* look for an instruction */
					id = *lookup(yytext,N_INSTALL,MNEMON);
					if (id.stp==NULL || id.stp->tag==0) {
						yyerror("invalid instruction mnemonic");
						return(ERROR);
						};
					/* it is some kind of instruction */
					yylval.uninsptr = id.itp;
					instseen = 1;
					return(id.itp->val+256);
					}
				else { /* some kind of operand ?? */
				       /* is it a reserved symbol? */
					if (!defseen) {
					  id = *lookup(yytext,N_INSTALL,MNEMON);
					  if (id.stp!=NULL&&id.stp->tag==SPSYM){
						yylval.unlong = id.itp->opcode;
						return(REG);
						};
					};
					/* it is just a user's symbol */
					id = *lookup(yytext,INSTALL,USRNAME);
					yylval.unsymptr = id.stp;
					return(IDENT);
					};


			case DIG: /* some kind of number */
				yylval.unfloat = 0.0;
				yylval.unlong = value-'0';
				if (value=='0') {
					mygetc(value);
					if (value=='x' || value=='X') {
						base = 16;
					} else if (value=='d' || value=='D' ||
						    value=='f' || value=='F') {
						cptr = &fltchr[0];
						mygetc(value);
						while (cptr<&fltchr[63]&&
						    value =='.'
						 || value=='e' || value=='d' ||
						    value=='E' || value=='D'
						 || value=='-' || value=='+' ||
							 (type+1)[value]==DIG) {
							*cptr++ = value;
							mygetc(value);
						};
						unmygetc(value);
						*cptr++ = '\0';
						yylval.unfloat = atof(fltchr);
						return(FLTNUM);
					} else {
						unmygetc(value);
						base = 8;
					}
				} else
					base = 10;
				mygetc(value);
				while ((type+1)[value]==DIG
				    || (base==16 &&
				    (value>='a' && value<='f'||value>='A' && value<='F'))) {
					if (base==8)
						yylval.unlong *= 8;
					else if (base==10)
						yylval.unlong *= 10;
					else
						yylval.unlong *= 16;
					if (value>='a' && value<='f')
						value -= 'a' - 10 - '0';
					else if (value>='A' && value<='F')
						value -= 'A' - 10 - '0';
					yylval.unlong += value-'0';
					mygetc(value);
				}
				unmygetc(value);
				return(INTNUM);

			case SQ: /* '? --> return the integer value of ? */
				mygetc(yylval.unlong);
				if (yylval.unlong == '\n')
					unmygetc('\n'); /* get the NL next time */
				return(INTNUM);

			case DQ: /* this is the beginning of a string "..." */
				yylval.unstrptr = cptr = yytext;
				mygetc(c);
				while ((c != '"') && (c != '\n')) {
					if (cptr <= &yytext[TXTCHARS])
						*cptr++ = c;
					mygetc(c);
					};
				*cptr = '\0';
				if (c == '\n')
					yyerror("string not terminated");
				return(STRING);

			case EOF: /* no more input, that's all folks */
				return(0);

			case SH: /* # comment to end of line */
				mygetc(c);
				while (c != '\n') 
					mygetc(c);
				instseen = 0;
				return(NL);

			case NL: /* new line */
				instseen = 0;
				return(NL);

			case SEMI: /* logical end of statement */
				instseen = 0;
				return(SEMI);

			case 0: /* ????? */
				yyerror("illegal input character");
				return(ERROR);

			default: /* if all else fails, return token */
				return(ctype);
			};
}

fill(nbytes)
register long nbytes;
{
	register long fillval;

	fillval = (dot->styp == TXT)	? ((TXTFILL<<8)|TXTFILL)
					: ((FILL<<8)|FILL);
	while (nbytes >= 2) {
		generate(2*BITSPBY,0,fillval,NULLSYM);
		nbytes -= 2;
	}
	if (nbytes)
		generate(BITSPBY,0,fillval,NULLSYM);
} /* fill */

ckalign(size)
long size;
{
	long mod;
	if ((mod = newdot % size) != 0) {
		fill(size - mod);
	}
} /* ckalign */
