/*
********************************************************************************
*                         Copyright (c) 1985 AT&T                              *
*                           All Rights Reserved                                *
*                                                                              *
*                                                                              *
*          THIS IS UNPUBLISHED PROPRIETARY SOURCE CODE OF AT&T                 *
*        The copyright notice above does not evidence any actual               *
*        or intended publication of such source code.                          *
********************************************************************************
*/
%{
# include <setjmp.h>
# include <syms.h>
# include "as.sym.h"
# include "as.addr.h"
# include "as.cbuf.h"
# define YES 1
# define NO 0

extern long opt, errflag, newdot, dottxt, dotdat, dotbss;
extern SYM dot;
extern int enter, zflag;

SYM *newsym();
char *strcpy();
INST *find();
int eflag = 0;
int e_addr;

SYM *cursym = (SYM *) 0;
SYM *insure();
AUX *getaux();
char *cfile = (char *) 0;
short ndim;
short nodefseen = 1;
short tagseen = 0;
jmp_buf env;

OPR ops[4];
short nops = 0, cfgbits, mreg, preg, optbits, fltbits,
      intbits, ccode, reglist, instr, outsize;
long line = 1;

%}

%union {
   long unlong;
   double undble;
   INST *uninst;
   SYM  *unsym;
   EXPR unexpr;
   }

%token <unlong> _SPACE _TEXT _DATA _BSS _GLOBAL _SET _COMM _ALIGN
%token <unlong> _ORG _BYTE _WORD _INT _LONG _DOUBLE _FLOAT _FILE _ENTER
%token <unlong> _DEF _ENDEF _DIM _LINE _SCL _SIZE _TAG _TYPE _VAL _LN
%token <unlong> LP RP LB RB NL SEMI COMMA ERRCH COLON DOLLAR EXTR TOS
%token <unlong> PREG MREG OPT SCALE IREG FREG CFG BREG INUM STRING HERE
%token <undble> FNUM
%token <unsym> IDENT
%token <uninst> NINSTR UINSTR BINSTR TINSTR QINSTR FINSTR XINSTR
%token <uninst> MINSTR PINSTR SINSTR CINSTR RINSTR EINSTR

%left <unexpr> OR
%left <unexpr> XOR
%left <unexpr> AND
%left <unexpr> LSH RSH
%left <unexpr> PLUS MINUS
%left <unexpr> STAR DIV MOD
%right <unexpr> NOT

%type <unexpr> expr

%start file

%%

file:		/* empty */
	|	file line {
		   line++;
		   }
	;
line:		lable_list instr_list NL
        |	error NL {
		   yyclearin;
		   yyerrok;
		   dot.value = newdot;
                   nops = 0;
		   instr = YES;
		   }
	;
lable_list:	/* empty */
	|	lable_list IDENT COLON {
		   if ( ($2->segmt & TYPE) )
		      Yyerror("multiply defined label");
                   if ( $2->name[0] == '_' && dot.segmt == TXT )
                      optimize(opt);
		   $2->segmt |= dot.segmt;
		   $2->value = dot.value;
		   }
	;
instr_list:	instruction {
                   generate(0, GENSTMT, 0, 0, 0);
		   instr = YES;
		   dot.value = newdot;
                   nops = 0;
		   }
	|	instr_list SEMI instruction {
                   generate(0, GENSTMT, 0, 0, 0);
		   instr = YES;
		   dot.value = newdot;
                   nops = 0;
		   }
	;
instruction:	/* no instruction is okay */
	|	pseudo
	|	NINSTR {
		   putinst($1->format, TINT, TINT);
		   }
	|	MINSTR mmureg COMMA operand {
		   putinst($1->format, TINT, TINT);
		   }
	|	PINSTR procreg COMMA operand {
		   putinst($1->format, TINT, TINT);
		   }
	|	PINSTR basereg COMMA operand {
		   putinst($1->format, TINT, TINT);
		   }
	|	SINSTR opt_list {
		   putinst($1->format, TINT, TINT);
		   }
	|	SINSTR {
		   optbits = 0x0;
		   putinst($1->format, TINT, TINT);
		   }
	|	CINSTR LB RB {
		   cfgbits = 0;
		   putinst($1->format, TINT, TINT);
		   }
	|	CINSTR LB cfg_list RB {
		   putinst($1->format, TINT, TINT);
		   }
	|	RINSTR LB RB {
		   reglist = 0;
		   putinst($1->format, TINT, TINT);
		   }
	|	RINSTR LB reg_list RB {
                   if (! strcmp($1->mnemon,"save"))
		      rev_reglist();
		   putinst($1->format, TINT, TINT);
		   }
	|	EINSTR LB RB COMMA operand {
		   reglist = 0;
		   if ( eflag ) {
		      if ( ops[nops-1].mode != AEXPR )
		         Yyerror("illegal displacement operand");
		      replenter(e_addr,reglist,&ops[nops-1].disp1);
		      eflag = 0;
		      }
		   else
		      putinst($1->format, TINT, TINT);
		   }
	|	EINSTR LB reg_list RB COMMA operand {
                   rev_reglist();
		   if ( eflag ) {
		      if ( ops[nops-1].mode != AEXPR )
		         Yyerror("illegal displacement operand");
		      replenter(e_addr,reglist,&ops[nops-1].disp1);
		      eflag = 0;
		      }
		   else
		      putinst($1->format, TINT, TINT);
		   }
	|	UINSTR operand {
		   putinst($1->format, TINT, TINT);
		   }
	|	BINSTR operand COMMA operand {
		   putinst($1->format, TINT, TINT);
		   }
	|	FINSTR operand COMMA operand {
		   putinst($1->format, TFLT, TFLT);
		   }
	|	XINSTR operand COMMA operand {
                   if ($1->stype == TFLT)
		      putinst($1->format, TFLT, TINT);
                   else
                      putinst($1->format, TINT, TFLT);
		   }
	|	TINSTR operand COMMA operand COMMA operand {
		   putinst($1->format, TINT, TINT);
		   }
	|	QINSTR operand COMMA operand COMMA operand COMMA operand {
		   putinst($1->format, TINT, TINT);
		   }
	;
pseudo:		_SPACE expr {
		   if ( $2.xtype != TYPL || $2.lngval <= 0)
		      Yyerror("illegal .space size expression");
		   zeros($2.lngval);
		   }
	|	_ENTER {
		   ops[0].mode = AEXPR;
		   ops[0].disp1.xtype = TYPL;
		   ops[0].disp1.psym = (SYM *) 0;
		   ops[0].disp1.nsym = (SYM *) 0;
		   ops[0].disp1.lngval = 0;
		   nops = 1;
		   reglist = 0;
		   enter = 3;
		   putinst(find("enter")->format, TINT, TINT);
		   eflag = 1;
		   e_addr = newdot;
		   }
	|	_BYTE setbyte iexpr_list
	|	_WORD setword iexpr_list
	|	_INT setint iexpr_list
	|	_LONG setlong iexpr_list
	|	_FLOAT setfloat fexpr_list
	|	_DOUBLE setdouble fexpr_list
	|	_TEXT {
		   chsegmt(TXT);
		   }
	|	_DATA {
		   chsegmt(DAT);
		   }
	|	_BSS IDENT COMMA expr COMMA expr {
		   long inc;

		   if ($4.xtype != TYPL || $6.xtype != TYPL)
		      Yyerror(".bss size/alignment not absolute");
		   if ($4.lngval <= 0 || $6.lngval <= 0)
		      Yyerror("illegal .bss size/alignment");
		   if ( $2->segmt )
		      Yyerror("multiply defined .bss label");
		   if (inc = dotbss % $6.lngval)
		      dotbss += $6.lngval - inc;
		   $2->value = dotbss;
		   dotbss += $4.lngval;
		   $2->segmt |= BSS;
		   }
	|	_GLOBAL IDENT {
		   $2->segmt |= EXT;
		   }
	|	_SET IDENT COMMA expr {
		   if ($4.xtype != TYPL)
		      Yyerror("illegal .set expression");
		   if (($2->segmt & TYPE) > ABS)
		      Yyerror("unable to define symbol");
		   $2->value = $4.lngval;
		   $2->segmt |= ABS;
		   }
	|	_COMM IDENT COMMA expr {
		   if ($4.xtype != TYPL)
		      Yyerror("illegal .comm size expression");
		   if ($2->segmt & TYPE)
		      Yyerror("illegal attempt to redefine symbol");
		   $2->segmt = EXT | UNK;
		   $2->value = $4.lngval;
		   }
	|	_ALIGN expr {
		   long inc;

		   if ($2.xtype != TYPL || $2.lngval < 1 || $2.lngval > 1024)
		      Yyerror("illegal .align expression");
                   if ( dot.segmt == TXT )
                      optimize(opt);
		   if (inc = newdot % $2.lngval)
		      zeros($2.lngval-inc);
		   }
	|	_ORG expr {
		   long inc;

                   if ($2.xtype & TDFF)
                      Yyerror("illegal .org expression");
		   else if (($2.xtype & TSYM) && ($2.psym->segmt & TYPE)) {
		      if (($2.psym->segmt & TYPE) != dot.segmt)
		         Yyerror("incompatible .org expression");
		      inc = $2.lngval + $2.psym->value - dot.value;
		      }
		   else if ($2.xtype == TYPL)
		      inc = $2.lngval - dot.value;
		   else
		      Yyerror("illegal .org expression");
		   if (inc < 0)
		      Yyerror("cannot decrement value of '.'");
		   zeros(inc);
		   }

	/* PSEUDO OPS FOR SYMBOLIC DEBUGGER ( SDB ) */

	|	_FILE STRING {
		   char *s, *t;

		   if ( cfile )
		      Yyerror("multiple C source files");
		   cfile = (char *) allocate(-1+strlen($2));
		   s = (char *) $2;
		   for ( t = cfile, s++; *s != '"'; )
		      *t++ = *s++;
		   *t = '\0';
		   }
	|	_LN expr {
		   add_lnno(dot.value,$2.lngval);
		   }
	|	_LN expr COMMA expr {
		   add_lnno($4.lngval,$2.lngval);
		   }
	|	_DEF { set_sdb(); nodefseen = 0; } IDENT {
                   set_reg();
                   nodefseen = 1;
		   cursym = $3;
		   }
	|	_ENDEF {
		   spec_sym(cursym);
		   }
	|	_LINE expr {
		   if (! cursym->aux)
		      cursym->aux = getaux();
		   cursym->aux->lnno = $2.lngval;
		   }
	|	_SCL expr {
		   cursym->scl = $2.lngval;
		   if ( ISTAG(cursym->scl) )
		      cursym->access |= TAG;
		   }
	|	_SIZE expr {
		   if (! cursym->aux)
		      cursym->aux = getaux();
		   cursym->aux->size = $2.lngval;
		   }
	|	_TYPE expr {
		   cursym->type = $2.lngval;
		   }
	|	_TAG { set_sdb(); tagseen = 2; } IDENT {
                   set_reg();
                   tagseen = 0;
		   if (! cursym->aux)
		      cursym->aux = getaux();
		   cursym->aux->tag = $3;
		   }
	|	_VAL expr {
		   cursym->value = $2.lngval;
		   if ( $2.xtype & TSYM )
		      cursym->xsym = $2.psym;
		   }
	|	_DIM dim_list {
		   ;
		   }
	;
dim_list:	expr {
		   if (! cursym->aux)
		      cursym->aux = getaux();
		   ndim = 0;
		   cursym->aux->a_m.dim[0] = $1.lngval;
		   }
	|	dim_list COMMA expr {
		   if (++ndim > 3)
		      Yyerror("too many dimensions for symbolic debugger");
		   cursym->aux->a_m.dim[ndim] = $3.lngval;
		   }
	;
mmureg:		MREG {
		   mreg = $1;
		   }
	;
procreg:	PREG {
		   preg = $1;
		   }
	;
basereg:	BREG {
		   preg = $1;
		   }
	;
cfg_list:	CFG {
		   cfgbits = $1;
		   }
	|	cfg_list COMMA CFG {
		   cfgbits |= $3;
		   }
	;
opt_list:	OPT {
                   optbits = $1;
		   }
	|	opt_list COMMA OPT {
 		   if ($3 > 1)
                      optbits &= 0x01;
                   optbits |= $3;
		   }
	;
reg_list:	IREG {
		   reglist = 0x01 << (7-$1);
		   }
	|	reg_list COMMA IREG {
		   reglist |= ( 0x01 << (7-$3) );
		   }
	;
operand:	argument {
		   nops++;
		   }
	;
argument:	DOLLAR expr {
		   ops[nops].mode = AIMM;
		   ops[nops].disp1 = $2;
		   }
	|	FREG {
		   ops[nops].mode = AFREG;
		   ops[nops].reg1 = $1;
		   }
	|	subopr LB IREG SCALE RB {
		   ops[nops].mode |= AINDX;
		   ops[nops].reg2 = $3;
		   ops[nops].scale = $4;
		   }
	|	subopr
	;
subopr:		IREG {
		   ops[nops].mode = AREG;
		   ops[nops].reg1 = $1;
		   }
	|	expr LP IREG RP {
		   ops[nops].mode = ARREL;
		   ops[nops].reg1 = $3;
		   ops[nops].disp1 = $1;
		   }
	|	expr LP expr LP BREG RP RP {
		   ops[nops].mode = AMREL;
		   ops[nops].reg1 = ($5 & 0x07);
		   ops[nops].disp1 = $3;
		   ops[nops].disp2 = $1;
		   }	
	|	expr {
		   ops[nops].mode = AEXPR;
		   ops[nops].disp1 = $1;
		   }
	|	EXTR LP expr RP PLUS expr {
		   ops[nops].mode = AEXTR;
		   ops[nops].disp1 = $3;
		   ops[nops].disp2 = $6;
		   }
	|	TOS {
		   ops[nops].mode = ATOS;
		   }
	|	expr LP BREG RP {
		   ops[nops].mode = AMSP;
		   ops[nops].reg1 = ($3 & 0x07);
		   ops[nops].disp1 = $1;
		   }
	|	STAR PLUS expr {
		   ops[nops].mode = AMSP;
		   ops[nops].reg1 = 0x3;
		   if ($3.xtype != TYPL)
		      Yyerror("illegal displacement");
		   ops[nops].disp1 = $3;
		   }
	|	STAR MINUS expr {
		   ops[nops].mode = AMSP;
		   ops[nops].reg1 = 0x3;
		   if ($3.xtype != TYPL)
		      Yyerror("illegal displacement");
		   ops[nops].disp1 = $3;
		   ops[nops].disp1.lngval = - ops[nops].disp1.lngval;
		   }
	;
setbyte:	{ outsize = 8; }
	;
setword:	{ outsize = 16; }
	;
setint:		{ outsize = 32; }
	;
setlong:	{ outsize = 32; }
	;
setfloat:       { outsize = 32; }
        ;
setdouble:      { outsize = 64; }
        ;
iexpr_list:	iexpr
        |       string
	|	iexpr_list COMMA iexpr
	|	iexpr_list COMMA string
	;
iexpr:		expr {
		   if ($1.xtype & TFLT)
		      Yyerror("illegal fixed-point expression");
		   generate(outsize, GENRLVAL, $1.lngval, $1.psym, $1.nsym);
		   }
	;
fexpr_list:	fexpr
	|	fexpr_list COMMA fexpr
	;
fexpr:		expr {
                   union { double d; float f; long l[2]; } xt;

		   if ($1.xtype & TSYM || $1.xtype & TDFF)
		      Yyerror("illegal floating-point expression");
                   if (outsize == 32) {
                      xt.f = ($1.xtype & TFLT) ? $1.dblval : (double) $1.lngval;
                      generate(32, GENRLVAL, xt.l[0], 0, 0);
                      }
                   else {
                      xt.d = ($1.xtype & TFLT) ? $1.dblval : (double) $1.lngval;
                      generate(32, GENRLVAL, xt.l[0], 0, 0);
                      generate(32, GENRLVAL, xt.l[1], 0, 0);
                      }
		   }
	;
string:         STRING {
                   char *s = (char *) $1;

                   for (s++; *s != '"'; )
                      generate(8, GENVALUE, esc(&s), 0, 0);
                   }
        ;
expr:		IDENT {
		   if (($1->segmt & TYPE) == ABS) {
		      $$.xtype = TYPL;
		      $$.psym = $$.nsym = (SYM *) 0;
		      $$.lngval = $1->value;
		      }
		   else {
		      $$.xtype = TYPL|TSYM;
		      $$.psym = $1;
                      $$.nsym = (SYM *) 0;
		      $$.lngval = 0;
		      }
		   }
	|	HERE {
		   $$.xtype = TSYM;
		   $$.psym = newsym();
		   $$.psym->name = strcpy(allocate(2),".");
		   $$.psym->value = dot.value;
		   $$.psym->segmt = dot.segmt;
		   $$.nsym = (SYM *) 0;
		   $$.lngval = 0;
		   }
 	|	INUM {
		   $$.xtype = TYPL;
		   $$.psym = $$.nsym = (SYM *) 0;
		   $$.lngval = $1;
		   }
	|	FNUM {
		   $$.xtype = TYPD;
		   $$.psym = $$.nsym = (SYM *) 0;
 		   $$.dblval = $1;
		   }
        |       LP expr RP {
                   $$ = $2;
                   }
	|	expr PLUS expr {
                   $$.xtype = maxtype($1.xtype,$3.xtype);
                   if ($$.xtype & TFLT) {
                      $$.dblval = ($1.xtype & TFLT) ? $1.dblval : (double) $1.lngval;
                      $$.dblval += ($3.xtype & TFLT) ? $3.dblval : (double) $3.lngval;
                      $$.psym = $$.nsym = (SYM *) 0;
                      }
                   else {
 	              $$.lngval = $1.lngval + $3.lngval;
                      $$.psym = (SYM *)((int)$1.psym+(int)$3.psym);
		      $$.nsym = (SYM *)((int)$1.nsym+(int)$3.nsym);
                      }
		   }
	|	expr MINUS expr {
                   if (($1.xtype & TSYM) && ($3.xtype & TDFF))
                      Yyerror("illegal operator types");
                   if (($1.xtype & TDFF) && ($3.xtype & TSYM))
                      Yyerror("illegal operator types");
                   if (($1.xtype & TFLT) || ($3.xtype & TFLT)) {
                      $$.xtype = maxtype($1.xtype,$3.xtype);
                      $$.dblval = ($1.xtype & TFLT) ? $1.dblval : (double) $1.lngval;
                      $$.dblval -= ($3.xtype & TFLT) ? $3.dblval : (double) $3.lngval;
                      $$.psym = $$.nsym = (SYM *) 0;
                      }
                   else {
                      $$.xtype = TYPL;
                      if (($1.xtype & TSYM) || ($3.xtype & TDFF))
                         $$.xtype |= TSYM;
                      if (($1.xtype & TDFF) || ($3.xtype & TSYM))
                         $$.xtype |= TDFF;
    	              $$.lngval = $1.lngval - $3.lngval;
                      $$.psym = (SYM *)((int)$1.psym+(int)$3.nsym);
   			   $$.nsym = (SYM *)((int)$1.nsym+(int)$3.psym);
                      }
		   }
	|	expr STAR expr {
                   $$.xtype = maxtype($1.xtype,$3.xtype);
                   if ($$.xtype & TFLT) {
                      $$.dblval = ($1.xtype & TFLT) ? $1.dblval : (double) $1.lngval;
                      $$.dblval *= ($3.xtype & TFLT) ? $3.dblval : (double) $3.lngval;
                      }
                   else
 	              $$.lngval = $1.lngval * $3.lngval;
                   $$.psym = $$.nsym = (SYM *) 0;
		   }
	|	expr DIV expr {
                   $$.xtype = maxtype($1.xtype,$3.xtype);
                   if ($3.xtype & TFLT) {
                      if ($3.dblval == 0.0)
                         Yyerror("division by zero");
                      }
                   else if ($3.lngval == 0)
                      Yyerror("division by zero");
                   if ($$.xtype & TFLT) {
                      $$.dblval = ($1.xtype & TFLT) ? $1.dblval : (double) $1.lngval;
                      $$.dblval /= ($3.xtype & TFLT) ? $3.dblval : (double) $3.lngval;
                      }
                   else
 	              $$.lngval = $1.lngval / $3.lngval;
                   $$.psym = $$.nsym = (SYM *) 0;
		   }
	|	expr MOD expr {
		   if ($1.xtype == TYPL && $3.xtype == TYPL) {
		      $$.xtype = TYPL;
                      if ( $3.lngval == 0 )
                         Yyerror("modulo by zero");
		      $$.lngval = $1.lngval * $3.lngval;
                      $$.psym = $$.nsym = (SYM *) 0;
		      }
		   else
		      Yyerror("bad operator types");
		   }
	|	expr AND expr {
		   if ($1.xtype == TYPL && $3.xtype == TYPL) {
		      $$.xtype = TYPL;
		      $$.lngval = $1.lngval & $3.lngval;
                      $$.psym = $$.nsym = (SYM *) 0;
		      }
		   else
		      Yyerror("bad operator types");
		   }
	|	expr XOR expr {
		   if ($1.xtype == TYPL && $3.xtype == TYPL) {
		      $$.xtype = TYPL;
		      $$.lngval = $1.lngval ^ $3.lngval;
                      $$.psym = $$.nsym = (SYM *) 0;
		      }
		   else
		      Yyerror("bad operator types");
		   }
	|	expr OR expr {
		   if ($1.xtype == TYPL && $3.xtype == TYPL) {
		      $$.xtype = TYPL;
		      $$.lngval = $1.lngval | $3.lngval;
                      $$.psym = $$.nsym = (SYM *) 0;
		      }
		   else
		      Yyerror("bad operator types");
		   }
	|	expr LSH expr {
		   if ($1.xtype == TYPL && $3.xtype == TYPL) {
		      $$.xtype = TYPL;
		      $$.lngval = $1.lngval << $3.lngval;
                      $$.psym = $$.nsym = (SYM *) 0;
		      }
		   else
		      Yyerror("bad operator types");
		   }
	|	expr RSH expr {
		   if ($1.xtype == TYPL && $3.xtype == TYPL) {
		      $$.xtype = TYPL;
		      $$.lngval = $1.lngval >> $3.lngval;
                      $$.psym = $$.nsym = (SYM *) 0;
		      }
		   else
		      Yyerror("bad operator types");
		   }
	|	NOT expr {
		   if ($2.xtype == TYPL) {
		      $$.xtype = TYPL;
		      $$.lngval = ~ $2.lngval;
                      $$.psym = $$.nsym = (SYM *) 0;
		      }
		   else
		      Yyerror("bad operator types");
		   }
	|	MINUS expr %prec NOT {
                   if ($2.xtype & TFLT) {
                      $$.xtype = TYPD;
                      $$.dblval = - $2.dblval;
                      $$.psym = $$.nsym = (SYM *) 0;
                      }
                   else {
                      $$.xtype = TYPL;
                      if ($2.xtype & TSYM)
                         $$.xtype |= TDFF;
                      if ($2.xtype & TDFF)
                         $$.xtype |= TSYM;
 	              $$.lngval = - $2.lngval;
                      $$.psym = $2.nsym;
		      $$.nsym = $2.psym;
                      }
		   }
	;
%%
# include <stdio.h>
double atof();
# include "lex.yy.c"

Yyerror(s)
   char *s;
{
   yyerror(s);
   longjmp(env, 1);
}

yyerror(s)
   char *s;
{
   errflag = 1;
   fprintf(stderr,"as - %d: %s\n", line, s);
}

int maxtype(a,b)
   int a, b;
{
   if (a & TINT)
      return b;
   if (b & TINT)
      return a;
   if ((a & TFLT) && (b & TFLT))
      return TYPD;
   if ( ((a | b) & (TSYM|TDFF)) == (TSYM|TDFF))
      return TSYM|TDFF;
   Yyerror("incompatible expression types");
}
