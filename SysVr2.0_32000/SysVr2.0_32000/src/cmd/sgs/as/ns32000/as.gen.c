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
# include <stdio.h>
# include "as.sym.h"
# include "as.addr.h"
# include "as.cbuf.h"
# define YES 1
# define NO 0

extern short cfgbits, mreg, preg, optbits, fltbits, intbits, ccode, nops,
           reglist;

extern long newdot;
extern int opt, disp;
extern SYM dot;
extern OPR ops[4];
short oprnd;
short mode;

short fbits = 0;
SYM *fsym = (SYM *) 0;
long fvalue = 0;

rev_reglist() {
   int i, c, val = 0;

   for (i = 0; i < 8; i++) {
      c = reglist % 2;
      val = (val << 1) | c;
      reglist >>= 1;
      }
   reglist = val;
}

putinst(format, tsrc, tdest)
   char *format;
   short tsrc, tdest;
{
   char ch, select();
   OPR *op, *gen[2];
   int g = 0, i;
   int imm_size = intbits;

   mode = YES;
   while ( ch = select(&format) )
      switch ( ch ) {
      case 'R':
         imm_size = 0;
         ch = 'r';
      case 'a': case 'r':
         if (oprnd < 0) oprnd = 0;
      case 't': case 'w': case 'x':
         if (oprnd < 0 && ch > 's') oprnd = 1;
         op = &ops[oprnd];
         if (ch=='a' && op->mode==ARREL && op->disp1.xtype==TYPL && op->disp1.lngval == 0)
            op->mode = AREG;
         if (ch != 'r' && op->mode == AIMM)
            Yyerror("illegal immediate operand");
         if (op->mode == AFREG && (ch == 'a' || ch == 't'))
            Yyerror("1: illegal operand type");
         if (op->mode == AFREG && ch == 'r' && tsrc != TFLT)
            Yyerror("2: illegal operand type");
         if (op->mode == AFREG && (ch == 'w' || ch == 'x') && tdest != TFLT)
            Yyerror("3: illegal operand type");
         if (op->mode == AREG && ch == 'r' && tsrc != TINT)
            Yyerror("4: illegal operand type");
         if (op->mode == AREG && (ch == 'w' || ch == 'x') && tdest != TINT)
            Yyerror("5: illegal operand type");
         gengen(op);
         gen[g++] = op;
         break;
      case 'c':
         genbits(4, GENINST, (long) cfgbits);
         break;
      case 'd':
         if (ops[nops-1].mode == AMSP)
            gendisp(&ops[nops-1].disp1);
         else if (ops[nops-1].mode == AEXPR)
            genrel(&ops[nops-1].disp1);
         else
            Yyerror("1: illegal displacement operand");
         break;
      case 'f':
         genbits(1, GENINST, (long) fltbits);
         break;
      case 'i':
         genbits(2, GENINST, (long) intbits);
         break;
      case 'h':
         genbits(4, GENINST, (long) ccode);
         break;
      case 'j':
	 if (ops[nops-1].mode != AEXPR && ops[nops-1].mode !=AIMM)
	    Yyerror("illegal displacement operand");
	 if (ops[nops-1].disp1.xtype != TYPL)
	    Yyerror("illegal length for block move");
	 if ((i = ops[nops-1].disp1.lngval) < 1)
	    Yyerror("illegal length for block move");
	 i--;
	 switch ( intbits ) {
	 case 0:
	    if (i > 15)
	       Yyerror("illegal length for block move");
	    break;
	 case 1:
	    if (i > 7)
	       Yyerror("illegal length for block move");
	    i *= 2;
	    break;
	 case 3:
	    if (i > 3)
	       Yyerror("illegal length for block move");
	    i *= 4;
	    }
	 ops[nops-1].disp1.lngval = i;
	 gendisp(&ops[nops-1].disp1);
	 break;
      case 'k':
         if (ops[nops-1].mode != AEXPR && ops[nops-1].mode !=AIMM)
            Yyerror("2: illegal displacement operand");
         gencons(&ops[nops-1].disp1);
         break;
      case 'l':
         genbits(8, GENVALUE, (long) reglist);
         break;
      case 'm':
         genbits(4, GENINST, (long) mreg);
         break;
      case 'n':
         if (ops[2].mode != AEXPR && ops[2].mode != AIMM)
            Yyerror("illegal operand mode");
         if (ops[3].mode != AEXPR && ops[3].mode != AIMM)
            Yyerror("illegal operand mode");
         genbits(3, GENINST, ops[2].disp1.lngval);
         genbits(5, GENINST, ops[3].disp1.lngval-1);
         genbits(0, FLUSH);
         break;
      case 'o':
         genbits(3, GENINST, (long) optbits);
         break;
      case 'p':
         genbits(4, GENINST, (long) preg);
         break;
      case 'q':
         if ((*ops).mode != AEXPR && (*ops).mode != AIMM)
            Yyerror("illegal operand mode");
         genbits(4, GENINST, (*ops).disp1.lngval);
         break;
      case 's':
         if ((*ops).mode != AREG)
            Yyerror("illegal operand mode");
         genbits(3, GENINST, (*ops).reg1);
         break;
      case ',':
         genbits(0, FLUSH);
         mode = NO;
         for (i = 0; i < g; i++) {
            op = gen[i];
            if (op->mode & AINDX) {
               op->mode &= ~AINDX;
               gengen(op);
               genbits(3, GENINST, (long) op->reg2);
               genbits(0, FLUSH);
               }
            }
         for (i = 0; i < g; i++) {
            op = gen[i];
            if (op->mode == AIMM)
               if (tsrc == TINT)
		  if ( op->disp1.xtype & TFLT )
		     Yyerror("illegal floating point expression");
	          else
                     generate(8*imm_size+8, GENIMVAL, op->disp1.lngval, op->disp1.psym, op->disp1.nsym);
               else {
                   union { double d; float f; long l[2]; } xt;

		   if ( fltbits == 1 ) {
                      xt.f = (op->disp1.xtype & TFLT) ? op->disp1.dblval : (double) op->disp1.lngval;
                      generate(32, GENIMVAL, xt.l[0], 0, 0);
                      }
                   else {
                      xt.d = (op->disp1.xtype & TFLT) ? op->disp1.dblval : (double) op->disp1.lngval;
                      generate(32, GENIMVAL, xt.l[1], 0, 0);
                      generate(32, GENIMVAL, xt.l[0], 0, 0);
                      }
		   }
            else if (op->mode != AFREG && op->mode != AREG && op->mode != ATOS)
               gendisp(&op->disp1);
            if (op->mode == AMREL || op->mode == AEXTR)
               gendisp(&op->disp2);
            }
         break;
         }
}

/* select - parse format */
char select(format)
   char **format;
{
   char  c, *fmt = *format;

   oprnd = -1;
   if (c = *fmt++) {
      while (c == '0' || c == '1') {
         fvalue = (fvalue << 1) | (c - '0');
         fbits++;
         c = *fmt++;
         }
      if (c == '%') {
         if (*fmt < 'A')
            oprnd = *fmt++ - '0';
         c = *fmt++;
         }
      *format = fmt;
      }
   if ( (!c) && mode )
      c = ',';
   return c;
}

/* genrel - generate a displacement */
genrel(ex)
   EXPR *ex;
{
   int sz;
   if ( ex->nsym != (SYM *) 0)
      Yyerror("illegal pc relative branch");
   if ( opt )
      sz = 8;
   else
      sz = 8 << disp;
   savrel(sz,dot.value,newdot,ex);
   generate(sz, GENPCREL, ex->lngval, ex->psym, ex->nsym);
}

/* gendisp - generate displacement */
gendisp(ex)
   EXPR *ex;
{
   int sz = 8 << disp;
   generate(sz, GENRELOC, ex->lngval, ex->psym, ex->nsym);
}

/* gencons - generate constant displacement */
gencons(ex)
   EXPR *ex;
{
   generate(32, GENNRDSP, ex->lngval, ex->psym, ex->nsym);
}

/* genbits - generate bit fields */
genbits(n, act, val)
   short n, act;
   unsigned long val;
{
   switch (act) {
   case FLUSH:
      if (fbits)
         generate(fbits, GENVALUE, fvalue, 0, 0);
      fvalue = fbits = 0;
      break;
   case GENINST:
      fvalue = (fvalue << n) | (((1 << n) - 1) & val);
      fbits += n;
      break;
   default:
      if (fbits) {
         generate(fbits, GENVALUE, fvalue, 0, 0);
         fvalue = fbits = 0;
         }
      generate(n, act, val, 0, 0);
      }
}

/* gengen - generate codes for gen operands */
gengen(op)
   OPR *op;
{
   if (op->mode & AINDX) {
      genbits(5, GENINST, (long) op->scale | 0x1c);
      return;
      }
   switch ( op->mode ) {
   case AFREG:
   case AREG:
      genbits(5, GENINST, (long) op->reg1);
      break;
   case ARREL:
      genbits(5, GENINST, (long) op->reg1 | 0x08);
      break;
   case AMREL:
      genbits(5, GENINST, (long) op->reg1 | 0x10);
      break;
   case AEXPR:
      genbits(5, GENINST, (long) 0x15);
      break;
   case AEXTR:
      genbits(5, GENINST, (long) 0x16);
      break;
   case ATOS:
      genbits(5, GENINST, (long) 0x17);
      break;
   case AMSP:
      genbits(5, GENINST, (long) op->reg1 | 0x18);
      break;
   case AIMM:
      genbits(5, GENINST, (long) 0x14);
      break;
   default:
      Yyerror("illegal general operand type");
      }
}
