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

extern int dottxt, newdot;

typedef struct opt_node OPT;

struct opt_node {
   OPT *next;
   long oval;
   SYM *osym;
   long oaddr;
   short size;
   } *list = (OPT *) 0;

static OPT *head;
OPT *o_prev = (OPT *) &head;    /* know that next is offset zero */

static sizes[] = { 8, 16, 32 };

# define OPTSIZE sizeof(OPT)

savrel(size,iadr,addr,expr)
   int size;
   int iadr;
   int addr;
   EXPR *expr;
{
   OPT *p;
   int isize;

   isize = addr - iadr;
   p = (OPT *) allocate(OPTSIZE);
   p->oaddr = addr;
   p->oval = expr->lngval + isize;
   p->osym = expr->psym;
   p->size = size;
   if ( list ) {
      p->next = list->next;
      list = list->next = p;
      }
   else
      o_prev->next = list = p->next = p;
}

optimize(opt)
   int opt;
{
   short stop = 0, new;
   OPT *p;
   int i;

   chsegmt(TXT);
   if (! list)
      return;
   o_prev = list;
   list = list->next;
   if (! opt) {
      list = (OPT *) 0;
      return;
      }
   p = list;
   do {
      p = p->next;
      } while ( p != list );
   for (i = 0; ! stop; i = 1 - i) {
      stop = 1;
      new = 1;
      while ( new ) {
         new = 0;
         p = list;
         do {
            if ( p->size == sizes[i] )
               if (! fit(p,i) ) {
                  new = 1;
                  adjust(i+1,p);
                  }
            p = p->next;
            } while ( p != list ) ;
         if (new)
            stop = 0;
         }
      }
   list = (OPT *) 0;
   chsegmt(TXT);
}

adjust(inc,p)
   int inc;
   OPT *p;
{
   OPT *q;

   fixsymval(p->oaddr+p->size/8, inc, TXT);
   fix_lnno(p->oaddr+p->size/8, inc);
   for ( q = p->next; q != list; q = q->next) {
      q->oaddr += inc;
      }
   p->size <<= 1;
   newdot += inc;
}

opt_done() {
   list = o_prev->next = head;
}

fixopts(addr,inc)
   int addr, inc;
{
   register OPT *p;

   if ( p = list )
      do {
         if ( p->oaddr >= addr )
            p->oaddr += inc;
         } while ( list != (p = p->next));
}

fit(p,i)
   OPT *p;
   int i;
{
   SYM *s = p->osym;

   int val;

   if ( s && (s->segmt & TYPE) != TXT )
      return 0;
   val = (s ? s->value : 0) + p->oval - p->oaddr;
   switch ( i ) {
   case 0:
      if ( -64 <= val && val < 64 )
         return 1;
      return 0;
   case 1:
      if ( -8192 <= val && val < 8192 )
         return 1;
      return 0;
      }
   return 1;
}
getopt(p)
   CBUF *p;
{
   OPT *q;

   q = list;
   list = list->next;
   if ( ! q )
      error("too few entries in pcrel list");
   if ( newdot != q->oaddr )
      error("unordered pcrel list");
   p->cn = q->size;
}
