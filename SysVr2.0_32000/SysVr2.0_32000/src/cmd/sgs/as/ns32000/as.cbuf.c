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
# include "as.cbuf.h"
# include "as.addr.h"
# define YES 1
# define NO 0

extern long newdot, txtrel, datrel;
extern short bitpos;
int enter;
CBUF *e_list[3];

extern SYM dot;

CLIST t_head = { (CLIST *) 0, t_head.cblock },
      d_head = { (CLIST *) 0, d_head.cblock };
CLIST *txtptr = &t_head, *datptr = &d_head;

bufhead() {
   txtptr = &t_head;
   datptr = &d_head;
   txtptr->cptr = txtptr->cblock;
   datptr->cptr = datptr->cblock;
}

CBUF *cget(seg)
   short seg;
{
   CLIST *xc = seg == TXT ? txtptr : datptr;
   CBUF *cb;

   if (! xc->cnt) {
      if (! xc->link)
         return 0;
      xc = xc->link;
      *( seg == TXT ? &txtptr : &datptr ) = xc;
      xc->cptr = xc->cblock;
      }
   xc->cnt--;
   cb = xc->cptr++;
   if ( seg == TXT && cb->cact == GENPCREL )
      getopt(cb);
   return cb;
}

generate (n,act,val,psym,nsym)
   short n, act;
   long val;
   SYM *psym, *nsym;
{
   CLIST *xc;
   CBUF *cp;
   short *cnt;
   long *relcnt;

   switch (dot.segmt) {
   case TXT:
      xc = txtptr;
      relcnt = &txtrel;
      break;
   case BSS:
      if ( n ) {
         Yyerror("Attempt to initialize bss");
         return;
         }
   case DAT:
      xc = datptr;
      relcnt = &datrel;
      break;
      }
   if (xc->cnt == CBLEN) {
      xc->link = (CLIST *) allocate(CLISTSIZE);
      xc = xc->link;
      *( dot.segmt == TXT ? &txtptr : &datptr ) = xc;
      xc->cnt = 0;
      xc->cptr = xc->cblock;
      xc->link = (CLIST *) 0;
      }
   if (act!=GENNRDSP && act!=GENVALUE && act!=GENPCREL && nsym == (SYM *) 0)
      if (psym != (SYM *) 0 && (psym->segmt & TYPE) != ABS)
         (*relcnt)++;
   if (act == GENRELOC || act == GENPCREL || act == GENNRDSP)
      if ((psym == (SYM *) 0 && nsym == (SYM *) 0)) {
         val -= (act == GENPCREL) ? dot.value : 0;
         if (-64 <= val && val <= 63)
            n = 8;
         else if (-8192 <= val && val <= 8191)
            n = 16;
         else
            n = 32;
         val += (act == GENPCREL) ? dot.value : 0;
         }
   cp = xc->cptr++;
   if ( enter )
      e_list[3-enter--] = cp;
   cp->cact = act;
   cp->cn = n;
   cp->cval = val;
   cp->cpsym = psym;
   cp->cnsym = nsym;
   xc->cnt++;
   bitpos += n;
   newdot += bitpos / 8;
   bitpos %= 8;
}

# define FILL (dot.segmt == TXT ? 0xf2f2f2f2 : 0)

zeros(n)
   long n;
{
   for (; n >= 4; n -= 4 )
      generate(32, GENVALUE, FILL, 0, 0);
   if ( n )
      generate(n * 8, GENVALUE, FILL, 0, 0);
}



replenter(a,r,x)
   int a, r;
   EXPR *x;
{
   if ( x->xtype != TYPL )
      Yyerror("invalid operand to enter");
   e_list[1]->cval = r;
   e_list[2]->cval = x->lngval;
   if ( x->lngval < 64 )
      ;
   else if ( x->lngval < 8192 ) {
      e_list[2]->cn = 16;
      fixsymval(a,1,TXT);
      fixopts(a,1);
      newdot++;
      }
   else {
      e_list[2]->cn = 32;
      fixsymval(a,3,TXT);
      fixopts(a,3);
      newdot += 3;
      }
}
