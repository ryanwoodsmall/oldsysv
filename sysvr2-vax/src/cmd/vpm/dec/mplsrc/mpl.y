/*	mpl.y 1.4 of 2/15/83	*/
/*	@(#)mpl.y	1.4	*/
	/*
	 *	main pass of VPM compiler
	 */
%token		WHILE,FOR,DO,ELSE,SWITCH,CASE,BREAK,CONTINUE,DEFAULT
%token		LOGAND,LOGOR
%token		SC_PRIM SC_NAME SC_LVALUE SC_CONST SC_NULL SC_NP TRACE RCV GETBYTE
%token		BINOP LPAND
%token	ERR NAME CONST FLABEL DREG IF GOTO 
%token	INC DEC COMP NOT
%token	CO SC CM RP LP LC AM RC LB RB 
%token	RETURN CONTINUE
%token	FNAME ARRAY REPOP 
%token  DYN_ARR_ELT TMPNAME WAS_TEMP
%left	LOGOR
%left	LOGAND
%{
#include <stdio.h>
extern char *calloc(),*strcat(),*strcpy();
#define YES	1
#define NO	0
#define ZERO	1
#define OUNA	2
#define BINA	3
/* following constants are for error exits */
#define CALLOC	1
#define POPERR 	2
#define TMPERR	3
#define NAMERR	4
#define TOKLEN	9
#define TMPNUM	10
char *temp_ptr = "_temp1";
char fname[TOKLEN],priex[TOKLEN+TOKLEN],bname[TOKLEN],aname[TOKLEN+TOKLEN],offset[TOKLEN];
char oline[80],ocode[256]; /* ocode holds expr being built */
char labname[TOKLEN]; /* compiler-generated labels - see strlab() routine */
int stmtype,atype;
int intval;
char *repop[] = {
	"MOVM",
	"ORM",
	"XORM",
	"ADDM",
	"SUBM",
	"ANDM",
	"LSHM",
	"RSHM"
	};
char *tmp[TMPNUM] = {
	"_tmp0",
	"_tmp1",
	"_tmp2",
	"_tmp3",
	"_tmp4",
	"_tmp5",
	"_tmp6",
	"_tmp7",
	"_tmp8",
	"_tmp9"
	};
int tmp_ind = -1;
#define EQ(x,y) !strcmp(x,y)
/*
	macros for generating code
*/
#define o0code(F)  	{ if(ocode[0]) printf("%s",ocode); printf(F); ocode[0] = '\0'; }
#define o1code(F,A)  	{ if(ocode[0]) printf("%s",ocode); if(A!=NULL) printf(F,A); ocode[0] = '\0'; }
#define o2code(F,A,B)  	{ if(ocode[0]) printf("%s",ocode); printf(F,A,B); ocode[0] = '\0'; }

#define c0code(F) 	{ sprintf(oline,F); 	strcat(ocode,oline); }
#define c1code(F,A)	{ sprintf(oline,F,A);  	strcat(ocode,oline); }
#define c2code(F,A,B)	{ sprintf(oline,F,A,B);	strcat(ocode,oline); }

/* output a label - label is an int - prefix _ */
#define outlab(N)	{ if(ocode[0]) printf("%s",ocode);  printf("_%d:\n",N); ocode[0] = '\0'; }

/* make sure that a variable has been defined */
#define checkuse(V) if(!search(V,&nlptr)) werror("%s undefined",V);

#define WHILESTM	8
#define FORSTM		9
#define DOSTM		10
#define IFSTM		11
#define IFELSESTM	12
#define SWITCHSTM	13
#define CASESTM		14
#define BREAKSTM	15
#define CONTSTM		16
/*
 * structure stacked for nested control flow statements
 */
	struct lstknode {	/* label stack: used for all nesting */
		int stype;		/* stmt type */
		int lab1;		/* label 1 - usually top */
		int breaklab;		/* where to go on BREAK stmt */
		int contlab;		/* where to go on CONTINUE stmt */
		union {
			int deflab;	/* DEFAULT label in SWITCH */
			int elselab;	/* end of IF-ELSE stmt */
		} l ;
		union {
			char *exptr;	/* expr3 in FOR stmt */
			struct casenode *caselist; /* case list for SWITCH */
		} p ;
		char *swname;	/* name of switch expression */
		struct lstknode *preventry; /* previous lstack entry */
	};
	struct lstknode *toplnode = NULL; /* top of lstack */
	struct lstknode *tptr;

/*
 * structure used to hold info for each CASE statement
 */
	struct casenode {	/* one per CASE statement */
		char *coname;	/* char rep of CONST */
		int caselab;	/* label for stmts in this CASE */
		struct casenode *nextcase; /* ptr to next casenode */
	};

/*
 * structure stacked to evaluate logical expressions (&& and ||)
 */
	struct estknode {	/* expr stack: used only for && and || */
		int truelab;		/* go here if expr is true */
		int falselab;		/* go here if expr is false */
		struct estknode *preventry;  /* previous estack entry */
	} ;
	struct estknode *topenode = NULL; /* top of estack */
	struct estknode *esptr;	/* temp ptr */

/*
 * structure stacked to handle nested binary expressions
 */
	struct bin_estknode {	/* binary expr stack */
		char bin_opcode[16];	/* holds binary opcode for */
					/* level of nesting */
		char bin_priex[TOKLEN+TOKLEN];	/* holds current unary's name */
		struct bin_estknode *bin_preventry;  /* previous estack entry */
	} ;
	struct bin_estknode *bin_topenode = NULL; /* top of estack */
	struct bin_estknode *bin_esptr;	/* temp ptr */
/*
 * structure used to store variable initialization that takes place at
 * time of declaration
 */
	struct init_node {
		char init_code[15];	/* holds one line of init. code */
		struct init_node *nxtentry; /* ptr to next node */
	};
	struct init_node *init_firstptr = NULL;	/* pts. to first init_code node */
	struct init_node *init_lastptr = NULL;  /* pts. to last init_code node */
/*
 * structure inserted into hash table
 */
	struct nlist {		/* basic table entry */
		char *name;	/* string being hashed */
		int type;	/* type of object (e.g., NAME, ARRAY, FNAME) */
		struct nlist *next; /* next node in this bucket */
	} ;
	struct nlist *nlptr;	/* temp hashnode ptr */

#define HASHSIZE 31
struct nlist *hashtab[HASHSIZE];	/* symbol table pointers */

%}
%%
prog:		decl_sect act5 fn_list
		| fn_list
		;
decl_sect:	decl_sect decl_line
		| decl_line
		;
decl_line:	ARRAY decl_list decl SC
		| ARRAY decl SC
		;
decl_list:	decl_list decl CM
		| decl CM
		;
decl:		NAME =
		{
			if (search(name,&nlptr)) {
				werror("%s redeclared",name);
			} else { 	/* OK */
				nlptr->type = NAME;
			}
		}
		| NAME act1 REPOP CONST =
		{
			if (search(desreg,&nlptr)){
				werror("%s redeclared",desreg);
			} else {	/* OK */
				nlptr->type = NAME;
			}
			if($3 != 0) {
				yyerror("'=' expected");
			}
			sprintf((get_node())->init_code,"	LDA	%s",desreg);
			sprintf((get_node())->init_code,"	LDBYTE	%s",name);
		}
		| arr_decl
		| arr_decl init_beg RC
		| arr_decl init_beg init_end
		| arr_decl init_beg init_mid init_end
		;
arr_decl:	NAME LB act1 CONST RB =
		{
			if (search(desreg,&nlptr)) {
				werror("%s redeclared",desreg);
			} else { 	/* OK */
				nlptr->type = ARRAY;
			}
			if((intval = atoi(name)) > 200)
				werror("warning: %s declared to have more than 200 elements",desreg);
			o2code("%s\tBSS\t%s\n",desreg,name);
		}
		;
init_beg:	REPOP LC =
		{
			sprintf((get_node())->init_code,"	LDA	%s",desreg);
		}
		;
init_mid:	init_mid init_elt 
		| init_elt
		;
init_elt:	CONST CM =
		{
			sprintf((get_node())->init_code,"	LDBYTE	%s",name);
		}
		;
init_end:	CONST RC =
		{
			sprintf((get_node())->init_code,"	LDBYTE	%s",name);
		}
		;
fn_list:	fn|
		fn_list fn;
fn:		fn_dfn LC stm_list RC =
		{
			if(stmtype != 7){
				o0code("\tRETURNI\t0\n");
			}
			stmtype = 0;
		};
fn_dfn:		NAME LP RP =
		{
			fname[0] = '\0';
			strcpy(fname,name);
			o1code("%s:\n",name);
			if(search(name,&nlptr)) {
				werror("function %s redeclared",name);
			}
		};
stm_list:	stm_list stm
		| stm
		;
stm:		exexpr SC =
		{
			prline();
		}
		| GOTO NAME SC  = 
		{
			/* hash label in name here if nec */
			stmtype = 4;
			o1code("\tJMP\t%s\n",name);
		}|
		RETURN  SC	=
		{
			stmtype = 7;
			o0code("	RETURNI	0\n");
		}|
		RETURN expr SC =
		{
			switch($2){
			case ZERO:
			case CONST:
				o1code("	RETURNI	%s\n",name);
				break;
			case NAME:
				o1code("	RETURN	%s\n",name);
				break;
			case DYN_ARR_ELT:
				o1code("	LDA	%s\n",getind(name));
				o1code("	RETURNX	%s\n",name);
				break;
			default:
				strcat(fname,"__"); /* generate storage */
				o1code("	MOVM	%s\n",fname);
				o1code("	RETURN	%s\n",fname);
			}
			stmtype = 7;
		}
		| SC			/* empty */
		| LC stm_list RC	/* compound stmt */
		| whileprefix stm =
		{
			o1code("\tJMP     %s\n",strlab(toplnode->lab1));
			outlab(toplnode->breaklab);
			poplstk();
		}
		| forprefix stm =
		{
			outlab(toplnode->contlab);
			o1code("%s",toplnode->p.exptr);
			o1code("\tJMP     %s\n",strlab(toplnode->lab1));
			outlab(toplnode->breaklab);
			poplstk();
		}
		| DO dolab stm WHILE docontlab LP expr RP SC =
		{
			if($7==CONST) {
				/* do while(1) - loop forever */
				o1code("\tJMP\t%s\n",strlab(toplnode->lab1));
				ocode[0] = '\0';
			} else {
				move($7,name);
				prline();	/* write code to process expr */
				o1code("\tJMPNEZ  %s\n",strlab(toplnode->lab1));
			}
			outlab(toplnode->breaklab);
			poplstk();
		}
		| ifprefix elsepart =
		{
			outlab(toplnode->l.elselab);
			poplstk();
		}
		| ifprefix =
		{
			outlab(toplnode->lab1);
			poplstk();
		}
		| BREAK SC =
		{
			int label;

			label = getlab(BREAK);
			if(label==0) {
				yyerror("illegal break");
			} else {
				o1code("\tJMP     %s\n",strlab(label));
			}
		}
		| CONTINUE SC = 
		{
			int label;

			label = getlab(CONTINUE);
			if(label==0) {
				yyerror("illegal continue");
			} else {
				o1code("\tJMP     %s\n",strlab(label));
			}
		}
		| switchpart stm = 
		{
			/* end of switch */
			int endlabel;

			if(toplnode->p.caselist == NULL) 
				yyerror("no cases in switch");
			endlabel = (toplnode->lab1);
			o1code("\tJMP\t%s\n",strlab(endlabel+1)); /* skip default */
			outlab(endlabel);
			if(toplnode->l.deflab) /* there was a DEFAULT */
				o1code("\tJMP\t%s\n",strlab(toplnode->l.deflab));
			outlab(endlabel+1);
			outlab(toplnode->breaklab);
			poplstk();
		}
		| label stm
		| error SC
		| error RC
		;
whileprefix:	WHILE whlab LP expr RP
		= {
			if($4==CONST) {
				/* while(1) - loop forever */
				ocode[0] = '\0';
			} else {
				move($4,name);
				prline();	/* write code to process expr */
				o1code("\tJMPEQZ  %s\n",strlab(toplnode->breaklab));
			}
		  }
		;
whlab:		= {
			tptr = pushlstk();	/* new node for WHILE stm */
			tptr->stype = WHILESTM; 
			(tptr->lab1) = (tptr->contlab) = genlab();
			(tptr->breaklab) = genlab();
			outlab(tptr->lab1);
		  }
		;
forprefix:	for2 exexpr RP =		/* for stmt */
		{
			/* squirrel away reinitialization expr (for-expr3) */
			toplnode->p.exptr = savestring(ocode);
			ocode[0] = '\0';
		}
		| for2 RP =
		{
			/* for-expr3 may be null */
		}
		;
for2:		forlab exexpr SC = 
		{
			if(ocode[0] != '\0') {	/* no write if expr2 is null */
				prline();	/* write code for for-expr2 */
				o1code("\tJMPEQZ  %s\n",strlab(toplnode->breaklab));
			}
		}
		;
forlab:		FOR LP exexpr SC =
		{
			prline();		/* write code for for-expr1 */
			tptr = pushlstk();	/* new node for FOR statment */
			tptr->stype = FORSTM;
			tptr->lab1 = genlab();
			tptr->contlab = genlab();
			tptr->breaklab = genlab();
			outlab(tptr->lab1);
		}
		;
dolab:		=
		{
			tptr = pushlstk();	/* new node for DO stmt */
			tptr->stype = DOSTM;
			tptr->lab1 = genlab();
			tptr->contlab = genlab();
			tptr->breaklab = genlab();
			outlab(tptr->lab1);
		}
		;
docontlab:	= {
			outlab(toplnode->contlab);
		}
		;
ifprefix:	iflab stm
		;
iflab:		IF LP expr RP =
		{
			tptr = pushlstk();	/* new node for IF stmt */
			tptr->stype = IFSTM;
			tptr->lab1 = genlab();
			move($3,name);
			prline();	/* write code to process expr */
			o1code("\tJMPEQZ  %s\n",strlab(tptr->lab1));
		}
		;
elsepart:	ELSE elselab stm
		;
elselab:	=
		{
			if(toplnode->stype == IFSTM) {
				toplnode->stype = IFELSESTM;
				toplnode->l.elselab = genlab();
				o1code("\tJMP     %s\n",strlab(toplnode->l.elselab));
				outlab(toplnode->lab1);
			} else { /* else without if */
				yyerror("illegal else");
			}
		}
		;
switchpart:	SWITCH LP expr RP = 
		{
			/* begin switch */
			tptr = pushlstk();	/* new node for SWITCH */
			tptr->stype = SWITCHSTM;
			tptr->lab1 = genlab();	/* label for 1st case */
			genlab();	/* skip next label */
			tptr->breaklab = genlab();  
			switch($3) {
			case ZERO:
			case CONST:
				yyerror("constant expression in switch");
			case NAME:
				break;
			case DYN_ARR_ELT:
				c1code("	LDA	%s\n",getind(name));
				c1code("	MOVX	%s\n",name);
			default: /* store complex expr */
				sprintf(name,"__%d",tptr->breaklab);
				prline();
				o1code("\tMOVM    %s\n",name);
			}
			tptr->swname = savestring(name); /* save switch expr */
			o1code("\tJMP     %s\n",strlab(toplnode->lab1));
		}
		;
exexpr:		expr =
		{
			stmtype = 1;
			move($1,name);
		}|
		lvalue REPOP act1 expr =
		{
			if(debug == 1)
				printf("a  %d %d %d %d\n",$1,$2,$3,$4);
			stmtype = 2;
			if($1 == DYN_ARR_ELT){
			  switch($4){
			  case ZERO:
				if(($2 == 0) || ($2 == 5)) /* MOVM or ANDM */
			  		c1code("	LDA	%s\n",getind(desreg));
					c1code("	CLRX	%s\n",desreg);
				break;
			  case CONST:
				c1code("	MOVI	%s\n",name);
			  	c1code("	LDA	%s\n",getind(desreg));
				c2code("	%sX	%s\n",repop[$2],desreg);
				break;
			  case NAME:
				c1code("	MOV	%s\n",name);
			  	c1code("	LDA	%s\n",getind(desreg));
				c2code("	%sX	%s\n",repop[$2],desreg);
				break;
			  case DYN_ARR_ELT:
				move($4,name);
			  	c1code("	LDA	%s\n",getind(desreg));
				c2code("	%sX	%s\n",repop[$2],desreg);
				break;
			  default:
			  	c1code("	LDA	%s\n",getind(desreg));
				c2code("	%sX	%s\n",repop[$2],desreg);
			  };
			}	  
			else{
			  switch($4){
			  case ZERO:
				if(($2 == 0) || ($2 == 5)) /* MOVM or ANDM */
					c1code("	CLR	%s\n",desreg);
				break;
			  case CONST:
				c1code("	MOVI	%s\n",name);
				c2code("	%s	%s\n",repop[$2],desreg);
				break;
			  case NAME:
				c1code("	MOV	%s\n",name);
				c2code("	%s	%s\n",repop[$2],desreg);
				break;
			  case DYN_ARR_ELT:
				move($4,name);
				c2code("	%s	%s\n",repop[$2],desreg);
				break;
			  default:
				c2code("	%s	%s\n",repop[$2],desreg);
			  };
			};
		}
		|	/* empty */
		;
expr:		una_expr1|
		bin_expr =
		{
			$$ = BINA;
			if(debug == 1)
				printf("d  %d\n",$1);
		}
		| logand expr %prec LOGAND =
		{
			move($2,priex);
			c1code("\tJMPEQZ  %s\n",strlab(topenode->falselab));
			c1code("%s:\n\tMOVI    1\n",strlab(topenode->truelab));
			c1code("%s:\n",strlab(topenode->falselab));
			popestk();
			$$ = BINA;
		}
		| logor expr %prec LOGOR =
		{
			move($2,priex);
			c1code("\tJMPEQZ  %s\n",strlab(topenode->falselab));
			c1code("%s:\n\tMOVI    1\n",strlab(topenode->truelab));	
			c1code("%s:\n",strlab(topenode->falselab));
			popestk();
			$$ = BINA;
		}
		;
logand:		expr LOGAND =
		{
			esptr = pushestk();	/* new expr node */
			esptr->truelab = genlab();
			esptr->falselab = genlab();
			move($1,priex);
			c1code("\tJMPEQZ  %s\n",strlab(esptr->falselab));
		}
		;
logor:		expr LOGOR =
		{
			esptr = pushestk();	/* new expr node */
			esptr->truelab = genlab();
			esptr->falselab = genlab();
			move($1,priex);
			c1code("\tJMPNEZ  %s\n",strlab(esptr->truelab));
		}
		;
label:		NAME CO =
		{
			c1code("%s:\n",name);
			if(search(name,&nlptr)) {
				werror("label %s redeclared",name);
			}
		}
		| CASE CONST CO = 
		{
			if (toplnode->stype != SWITCHSTM) {
				yyerror("case not in switch");
			} else {
				struct casenode *cp, *tp, **tpp;
				int oldlabel;

				cp = ((struct casenode *)calloc(1,sizeof(struct casenode)));
				if(!cp) {
					cerror(CALLOC,"calloc can't get space for case");
					/* cerror exits */
				} else { /* new case node - check for duplicates */
					for( tpp = &(toplnode->p.caselist); tp = *tpp; tpp = &(tp->nextcase)) {
						if(EQ(tp->coname,name)){
							werror("duplicate case in switch (%s)",name);
						}
					}
					/* link new casenode in front of list */
					tp = toplnode->p.caselist; /* old 1st */
					toplnode->p.caselist = cp; /* new 1st */
					cp->nextcase = tp;
					cp->coname = savestring(name);
					cp->caselab = oldlabel = toplnode->lab1;
					toplnode->lab1 = genlab(); /* next case */
					genlab(); /* throw away */
					/* code generation for new case: */
					o1code("\tJMP\t%s\n",strlab(oldlabel+1));
					outlab(oldlabel);
					o1code("\tMOV\t%s\n",toplnode->swname);
					o1code("\tTSTEQLI\t%s\n",cp->coname);
					o1code("\tJMPEQZ\t%s\n",strlab(toplnode->lab1));
					outlab(oldlabel+1);
				}
			}
		}
		| DEFAULT CO =
		{
			if (toplnode->stype != SWITCHSTM) {
				yyerror("default not in switch");
			} else if (toplnode->l.deflab) {
				yyerror("duplicate default in switch");
			} else { /* legal default statement */
				toplnode->l.deflab = genlab();
				outlab(toplnode->l.deflab);
			}
		}
		;
una_expr1:	preprimary =
		{	
			$$ = $1;
			popbin_estk();
		}|
		NOT una_expr1 =
		{
			move($2,name);
			c0code("	TSTEQLI	0\n");
			$$ = OUNA;
		}|
		COMP una_expr1 =
		{
			move($2,name);
			c0code("	XORI	0377\n");
			$$ = OUNA;
		}|
		INC lvalue   = 
		{
			if($2 == DYN_ARR_ELT){
				c1code("	LDA	%s\n",getind(name));
				c1code("	INCX	%s\n",name);
			}
			else c1code("	INC	%s\n",name);
			$$ = OUNA;
		}|
		DEC lvalue   = 
		{
			if($2 == DYN_ARR_ELT){
				c1code("	LDA	%s\n",getind(name));
				c1code("	DECX	%s\n",name);
			}
			else c1code("	DEC	%s\n",name);
			$$ = OUNA;
		}|
		NAME LP RP =
		{
			c1code("	CALL	%s\n",name);
			$$ = OUNA;
		}|
		sys_call =
		{
			$$ = OUNA;
		}|
		LP expr RP =
		{
			$$ = $2;
		};
una_ex2:	una_ex2a =
		{
			if($1 != NAME && $1 != CONST && $1 != ZERO && $1 != DYN_ARR_ELT){
				bin_esptr = pushbin_estk();
				strcpy(bin_esptr->bin_priex,get_tmp());
				c1code("\tMOVM\t%s\n",bin_esptr->bin_priex);
				$$ = TMPNAME;
			};
		};
una_ex2a:	preprimary =
		{
			$$ = $1;
		}|
		NOT una_ex2 =
		{
			move($2,name);
			c0code("	TSTEQLI	0\n");
			$$ = OUNA;
		}|
		COMP una_ex2 =
		{
			move($2,name);
			c0code("	XORI	0377\n");
			$$ = OUNA;
		}|
		INC lvalue   = 
		{
			if($2 == DYN_ARR_ELT){
				c1code("	LDA	%s\n",getind(name));
				c1code("	INCX	%s\n",name);
			}
			else c1code("	INC	%s\n",name);
			$$ = OUNA;
		}|
		DEC lvalue   = 
		{
			if($2 == DYN_ARR_ELT){
				c1code("	LDA	%s\n",getind(name));
				c1code("	DECX	%s\n",name);
			}
			else c1code("	DEC	%s\n",name);
			$$ = OUNA;
		}|
		NAME LP RP =
		{
			c1code("	CALL	%s\n",name);
			$$ = OUNA;
		}|
		sys_call =
		{
			$$ = OUNA;
		}|
		LP expr RP =
		{
			$$ = $2;
		};
preprimary:	primary =
		{
			if(debug == 1)
				printf("b  %d\n",$1);
			$$ = $1;
			strcpy(priex,name);
			bin_esptr = pushbin_estk();
			strcpy(bin_esptr->bin_priex,name);
		};
bin_expr:	una_ex2 act4 BINOP una_expr1 =
		{
			if($4 != NAME && $4 != CONST && $4 != ZERO && $4 != DYN_ARR_ELT) {
				c1code("	MOVM	%s\n",temp_ptr);
			} else {
				/*
				/* if the last instruction sprintf'd to ocode
				/* was "MOVM	_tmp?", then undo that instruc-
				/* tion, since storage of accum. value in
				/* temporary variable was unnecessary.
				*/
				if($1 == TMPNAME){
					ocode[strlen(ocode)-strlen(oline)] = '\0';
					$1 = WAS_TEMP; 
					rel_tmp();
				}
			}
						/* load lside if necessary */
			switch($1){
				case CONST: case ZERO:
					c1code("	MOVI	%s\n",bin_topenode->bin_priex);
					break;
				case NAME:
					c1code("	MOV	%s\n",bin_topenode->bin_priex);
					break;
				case DYN_ARR_ELT:
					c1code("	LDA	%s\n",getind(bin_topenode->bin_priex));
					c1code("	MOVX	%s\n",bin_topenode->bin_priex);
					break;
				case TMPNAME:
					c1code("	MOV	%s\n",bin_topenode->bin_priex);
				default:
					break;
			}
						/* process rside */
			switch($4){
				case NAME:
					c2code("	%s	%s\n",bin_topenode->bin_opcode,name);
					break;
				case DYN_ARR_ELT:
					c1code("	LDA	%s\n",getind(name));
					c2code("	%sX	%s\n",bin_topenode->bin_opcode,name);
					break;
				case CONST: case ZERO:
					c2code("	%sI	%s\n",bin_topenode->bin_opcode,name);
					break;
				default:
					c2code("	%s	%s\n",bin_topenode->bin_opcode,temp_ptr);
			}
			if($1 == CONST || $1 == ZERO || $1 == NAME || $1 == TMPNAME || $1 == DYN_ARR_ELT || $1 == WAS_TEMP) popbin_estk();
		}
		;
sys_call:	SC_PRIM act6 LP expr RP =
		{
			switch($4){
				case NAME:
					c2code("	%s	%s\n",bin_topenode->bin_opcode,name);
					break;
				case DYN_ARR_ELT:
					c1code("	LDA	%s\n",getind(name));
					c2code("	%sX	%s\n",bin_topenode->bin_opcode,name);
					break;
				case ZERO:
				case CONST:
					c2code("	%sI	%s\n",bin_topenode->bin_opcode,name);
					break;
				default:
					c1code("	MOVM	%s\n",temp_ptr);
					c2code("	%s	%s\n",bin_topenode->bin_opcode,temp_ptr);
					break;
			};
			popbin_estk();
		}|
		SC_NAME LP NAME RP =
		{
			checkuse(name);
			c2code("	%s	%s\n",opcode,name);
		}|
		SC_LVALUE LP lvalue RP =
		{
			if($3 == DYN_ARR_ELT){
				c1code("	LDA	%s\n",getind(name));
				c2code("	%sX	%s\n",opcode,name);
			}
			else c2code("	%s	%s\n",opcode,name);
		}|
		RCV LPAND lvalue RP =
		{
			if($3 == DYN_ARR_ELT){
				c1code("\tLDA\t%s\n",getind(name));
				c1code("\tRCVX\t%s\n",name);
			}
			else c1code("\tRCV\t%s\n",name);
		}|
		GETBYTE LPAND lvalue RP =
		{
			if($3 == DYN_ARR_ELT){
				c1code("\tLDA\t%s\n",getind(name));
				c1code("\tGETBYTEX\t%s\n",name);
			}
			else c1code("\tGETBYTE\t%s\n",name);
		}|
		SC_CONST LP CONST RP =
		{
			c2code("	%sI	%s\n",opcode,name);
		}|
		SC_NULL LP RP =
		{
			c1code("\t%s\n",opcode);
		}|
		TRACE LP expr RP =
		{
			switch($3){
				case NAME:
					c1code("\tTRACE1\t%s\n",name);
					break;
				case DYN_ARR_ELT:
					c1code("\tLDA\t%s\n",getind(name));
					c1code("\tTRACE1X\t%s\n",name);
					break;
				case ZERO:
				case CONST:
					c1code("\tTRACE1I\t%s\n",name);
					break;
				default:
					c1code("	MOVM	%s\n",temp_ptr);
					c1code("	TRACE1	%s\n",temp_ptr);
					break;
			};
		}|
		TRACE LP primary act2 CM expr RP =
		{
			switch($6){
				case NAME:
					c1code("\tMOV\t%s\n",name);
					break;
				case DYN_ARR_ELT:
					c1code("\tLDA\t%s\n",getind(name));
					c1code("\tMOVX\t%s\n",name);
					break;
				case ZERO:
				case CONST:
					c1code("\tMOVI\t%s\n",name);
					break;
				default:
					break;
					
			}
			switch($3){
				case NAME:
					c1code("\tTRACE2\t%s\n",aname);
					break;
				case DYN_ARR_ELT:
					c1code("\tLDA\t%s\n",getind(aname));
					c1code("\tTRACE2X\t%s\n",aname);
					break;
				default:
					c1code("\tTRACE2I\t%s\n",aname);
					break;
			};
		}|
		SC_NP act6 LP NAME act2 CM expr RP =
		{
			checkuse(aname);
			switch($7){
				case NAME:
					c1code("\tMOV\t%s\n",name);
						break;
				case DYN_ARR_ELT:
					c1code("\tLDA\t%s\n",getind(name));
					c1code("\tMOVX\t%s\n",name);
					break;
				case ZERO:
				case CONST:
					c1code("\tMOVI\t%s\n",name);
					break;
				default:
					break;
			};		
			switch($4){
				case NAME:
					c2code("\t%s\t%s\n",bin_topenode->bin_opcode,aname);
						break;
				case DYN_ARR_ELT:
					c1code("\tLDA\t%s\n",getind(aname));
					c2code("\t%sX\t%s\n",bin_topenode->bin_opcode,aname);
					break;
				default:
					c2code("\t%s\t%s\n",bin_topenode->bin_opcode,aname);
					break;
			};
			popbin_estk();
		}
		;
primary:	lvalue|
		CONST =
		{
			if(zeros(name))
				$$ = ZERO;
			else
				$$ = CONST;
		}|
		LP primary RP =
		{
			$$ = $2;
		};
lvalue:		NAME act3 LB index RB =
		{
			checkuse(bname);
			if(nlptr->type != ARRAY)
				werror("%s not declared as type array",bname);
			strcpy(&offset[0],name);
			strcpy(&name[0],bname);
			strcat(name,"+");
			strcat(name,offset);
			if( $4 == CONST) $$ = NAME;
			else $$ = DYN_ARR_ELT;
		}|
		NAME =
		{
			checkuse(name);
			$$ = NAME;
			if(debug == 1)
					printf("e  %d\n",$1);
		};
index:		CONST =
		{
			$$ = CONST;
		}|
		NAME =
		{
			checkuse(name);
			$$ = NAME;
		};
act1:		={
			strcpy(desreg,name);
		};
act2:		={
			strcpy(&aname[0],name);
		};
act3:
		={
			strcpy(&bname[0],name);
			atype = NAME;
		};
act4:		={
			strcpy(bin_topenode->bin_opcode,opcode);
		};
act5:		={
			pr_init_code();
		};
act6:		={
			bin_esptr = pushbin_estk();
			strcpy(bin_esptr->bin_opcode,opcode);
		};
%%
 
int debug = 0;
int yyline = 1;
char infile[128];
#include "mpl.yl"
char	desreg[TOKLEN+TOKLEN];
main(argc,argv)
int argc;
char *argv[];
{
	int parseval;

	yyinit(argc,argv);
	if (search(temp_ptr,&nlptr)) {
		werror("%s redeclared",name);
	} else { 	/* OK */
		nlptr->type = NAME;
	}
	parseval = yyparse();
	if(debug>=1)
		psymtab();
	if(parseval)
		return;
	yyaccpt();
}
yyinit(argc,argv)
int argc;
char *argv[];
{
	while(argc > 1 && argv[1][0] == '-'){
		switch(argv[1][1]){
			case 'd':
				debug = atoi(&argv[1][2]);
				if(debug == 1)
					printf("debug = %d\n",debug);
				break;
		}
		argc--;
		argv++;
	}
}
yyaccpt()
{
}

werror(msg,name)	/* non-fatal error */
char *msg, *name;
{
	extern int yyline;
	extern char infile[];

	fprintf(stderr,"\n***** mpl: ");
	fprintf(stderr,"file %s, line %d: ",infile,yyline);
	fprintf(stderr,msg,name);
	fputc('\n',stderr);
}

yyerror(s)		/* non-fatal -- one argument */
char *s;
{
	extern int yyline;
	extern char infile[];

	fprintf(stderr,"\n***** mpl: ");
	fprintf(stderr,"file %s, line %d: ",infile,yyline);
	fprintf(stderr,"%s",s);
	fputc('\n',stderr);
}

cerror(errno,msg)		/* fatal error - die */
int errno;
char *msg;
{
	extern int yyline;
	extern char infile[];

	fprintf(stderr,"\n***** mpl: ");
	fprintf(stderr,"file %s, line %d: ",infile,yyline);
	fprintf(stderr,"%s.  Goodbye!\n",msg);
	exit(errno);
}

/*
 * general help functions
 */

prline()	/* this should be a macro */
{
	if(debug == 2){
		printf("*%d	%s\n",yyline,sline);
		sline[0]='\0';
	}
	printf("%s",ocode);
	ocode[0]='\0';
}

char *
savestring(string)		/* save string, return ptr to it */
char *string;
{
	char *loc;

	if((loc = ((char *) calloc(1,strlen(string)+1))) == NULL) {
		cerror(CALLOC,"savestring: calloc can't get space");
		/* cerror exits */
	}
	strcpy(loc,string);
	return(loc);
}

zeros(s)	/* does the string s contain only zeros? */
char *s;
{
	while(*s != '\0') {
		if(*s != '0')
			return(NO);
		++s;
	}

	return(YES);
}

char *
getind(arr_elt)
char *arr_elt;
{
	char *s;
	
	if((s = (char *)strchr(arr_elt,'+')) == (char *)0){
		yyerror("dynamic array element not properly specified in string variable");
		exit(NAMERR);
	}
	*s++ = '\0';
	return(s);
}


move(type,source)	/* move value of type type from source into AC */
int type;
char *source;
{
	switch(type) {
		case ZERO:
		case CONST:
			c1code("\tMOVI\t%s\n",source);
			break;
		case NAME:
			c1code("\tMOV \t%s\n",source);
			break;
		case DYN_ARR_ELT:
			c1code("\tLDA \t%s\n",getind(source));
			c1code("\tMOVX\t%s\n",source);
			break;
	}
}


int nexlab = 23000;
#define LABINC 1

genlab()	/* generate a new label */
{
	nexlab += LABINC;
	return(nexlab-LABINC);
}


/* make a char string label out of integer label intlab */
char *
strlab(intlab)
{
	sprintf(labname,"_%d",intlab);
	return(labname);
}


/* 
 * routines for manipulating label stack
 */

struct lstknode *
pushlstk()		/* allocate new lstknode and return ptr to it */
{
	struct lstknode *nptr;

	nptr = ((struct lstknode *) calloc(1,sizeof(struct lstknode)));
	if (nptr) {	/* got space */
		nptr->preventry = toplnode;
		return(toplnode = nptr);
	} else {	/* no space from calloc */
		cerror(CALLOC,"calloc can't get stack space");
		/* cerror exits */
	}
}

poplstk()		/* pop lstk and free popped node */
{
	struct lstknode *nptr;

	if(debug>=1) printlstk();

	nptr = toplnode;
	if(nptr==NULL) {
		cerror(POPERR,"attempt to pop empty stack");
		/* cerror exits */
	}
	else {	/* OK */
		toplnode = nptr->preventry;
		free(nptr);
	}
}

getlab(target)	/* search lstack for dest of a target statement */
		/* target must be BREAK or CONTINUE */
		/* return zero if none found */
int target;
{
	struct lstknode *lp;

	lp = toplnode;
	while(lp != NULL) {
		switch(target) {
			case BREAK:	if(lp->breaklab)
						return(lp->breaklab);
					break;
			case CONTINUE:	if(lp->contlab)
						return(lp->contlab);
					break;
		}
		lp = lp->preventry;
	}
	return(0);	/* none found */
}

printlstk()		/* print label stack */
{
	struct lstknode *tp;

	tp = toplnode;
	while(tp) {
		printf("\t\t\tstatement type: %d*****\n",tp->stype);
		printf("\t\t\tlab1: %d\n",tp->lab1);
		printf("\t\t\tcontlab: %d\n",tp->contlab);
		printf("\t\t\tbreaklab: %d\n",tp->breaklab);
		tp = tp->preventry;
	}
}

/*
 * routines for manipulating expression stack (&& and ||)
 */

struct estknode *
pushestk()		/* allocate new estknode and return ptr to it */
{
	struct estknode *nptr;

	nptr = ((struct estknode *) calloc(1,sizeof(struct estknode)));
	if (nptr) {	/* got space */
		nptr->preventry = topenode;
		return(topenode = nptr);
	} else {	/* no space from calloc */
		cerror(CALLOC,"calloc can't get space for expression stack");
		/* cerror exits */
	}
}

popestk()	/* pop estack and free popped node */
{
	struct estknode *nptr;

	nptr = topenode;
	if(nptr==NULL) {
		cerror(POPERR,"attempt to pop empty expr stack");
		/* cerror exits */
	} else {
		topenode = nptr->preventry;
		free(nptr);
	}
}

/*
 * stack routines for nested expressions
 */

struct bin_estknode *
pushbin_estk()		/* allocate new bin_estknode and return ptr to it */
{
	struct bin_estknode *nptr;

	nptr = ((struct bin_estknode *) calloc(1,sizeof(struct bin_estknode)));
	if (nptr) {	/* got space */
		nptr->bin_preventry = bin_topenode;
		return(bin_topenode = nptr);
	} else {	/* no space from calloc */
		cerror(CALLOC,"calloc can't get space for binary expression stack");
		/* cerror exits */
	}
}

popbin_estk()	/* pop bin_estack and free popped node */
{
	struct bin_estknode *nptr;

	nptr = bin_topenode;
	if(nptr==NULL) {
		cerror(POPERR,"attempt to pop empty expr stack");
		/* cerror exits */
	} else {
		bin_topenode = nptr->bin_preventry;
		free(nptr);
	}
}

struct init_node *
get_node()
{
	struct init_node *nptr;

	nptr = ((struct init_node *) calloc(1,sizeof(struct init_node)));
	if(nptr) {	/* got space */
		if(init_firstptr == NULL) init_firstptr = nptr;
		else init_lastptr->nxtentry = nptr;
		nptr->nxtentry = NULL;
		return(init_lastptr = nptr);
	} else {	/* no space from calloc */
		cerror(CALLOC,"calloc can't get space for initialization linked list");
		/* cerror exits */
	}
}

pr_init_code()
{
	struct init_node *nptr, *curptr;

	nptr = init_firstptr;
	while(nptr != NULL){
		printf("%s\n",nptr->init_code);
		curptr = nptr;
		nptr = nptr->nxtentry;
		free(curptr);
	}
	init_firstptr = NULL;
	init_lastptr = NULL;
}

char *
get_tmp()
{
	if( ++tmp_ind > 9 ) {
		werror("allowable number ( %d ) of tmp variables exceeded",TMPNUM);
		exit(TMPERR);
	}
	if(search(tmp[tmp_ind],&nlptr)); else nlptr->type = NAME;
	return(tmp[tmp_ind]);
}

rel_tmp()
{
	if( --tmp_ind < -1) {
		yyerror("tmp variable scheme fouled");
		exit(TMPERR);
	}
}

/*
 * hash table routines
 */

hash(s)		/* form hash value for string s */
char *s;
{
	int count;
	unsigned int temp, hashval;

	count = 1;
	hashval = temp = 0;
	while(*s != '\0') {
		temp = temp << 8;
		temp |= ((unsigned int) *s);
		if(count++ == 3) {
			hashval += temp;
			temp = 0;
			count = 1;
		}
		++s;
	}

	if(count > 1)
		hashval += temp;
	return(hashval % HASHSIZE);
}

search(name,node)	/* search hash table for name:
			 * 	return YES(1) if found;
			 * 	return NO(0) if not found.
			 * node returns ptr to new node 
			 */
char *name;
struct nlist **node;
{
	int hashval;
	struct nlist *tp,*create();

	hashval = hash(name);

	for(tp = hashtab[hashval]; tp; tp = tp->next)
		if(EQ(name,tp->name)) {
			*node = tp;
			return(YES);		/* found name */
		}

	tp = create(name); 		 	/* not found - insert */
	tp->next = hashtab[hashval];
	hashtab[hashval] = tp;
	*node = tp;
	return(NO);
}

struct nlist *
create(name)		/* allocate new hashtab node, return ptr to it */
char *name;
{
	struct nlist *tp;
	char *savestring();

	tp = ((struct nlist *) calloc(1,sizeof(struct nlist)));
	if (tp)	{	/* got space */
		tp->next = NULL;
		tp->name = savestring(name);
		return(tp);
	} else {
		cerror(CALLOC,"create: no space to expand symbol table");
		/* cerror exits */
	}
}

psymtab()	/* print symbol table */
{
	int i;
	struct nlist *tp;

	for(i=0; i<HASHSIZE; i++) {
		printf("%d:\t----------\n",i);
		if(!hashtab[i])
			printf("\t*NULL*\n");
		else 
			for(tp = hashtab[i]; tp; tp = tp->next)
				printf("\t%s \n",tp->name);
	}
}

