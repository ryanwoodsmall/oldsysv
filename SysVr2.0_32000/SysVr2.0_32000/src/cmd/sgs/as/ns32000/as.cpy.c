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
# include "linenum.h"
# include "reloc.h"
# include "as.sym.h"
# include "as.addr.h"
# include "as.cbuf.h"
# define YES 1
# define NO 0


extern SYM dot;
extern long newdot, symbase, txtrel, datrel;
extern long TXTINDX, DATINDX, BSSINDX;
extern long Lnno_base, Lnno_cnt;

RELOC *rbuf, *rptr;

init_rel() {
   rptr = rbuf = (RELOC *) allocate(sizeof(RELOC)*(datrel+txtrel));
   }

translate(seg, null)
   short seg;
   int null;
{
   short pcrel, stype, rtype;
   char *rsym;
   CBUF *code, *cget();
   FILE *fp;
   RELOC r;
   unsigned char zip = ( seg == TXT ? 0xf2 : 0 );

   for ( ; code = cget(seg); newdot += code->cn / 8 ) {
      if ( code->cact == GENSTMT ) {
         dot.value = newdot;
         continue;
         }
      stype = ABS;
      if (code->cpsym != (SYM *) 0)
         while (! code->cpsym->name)
            code->cpsym = code->cpsym->xsym;
      if (code->cnsym != (SYM *) 0) {
         while (! code->cnsym->name)
            code->cnsym = code->cnsym->xsym;
         if ( (code->cnsym->segmt & TYPE) == UNK)
            error("subtrahend of difference undefined");
         if ( code->cpsym != (SYM *) 0)
            if ( (code->cpsym->segmt & TYPE) == UNK)
               error("minuend of difference undefined");
            else if ((code->cpsym->segmt & TYPE) != (code->cnsym->segmt & TYPE))
               error("difference of symbols not in same segment");
         }
      else if (code->cpsym != (SYM *) 0)
         stype = code->cpsym->segmt & TYPE;
      if ( code->cact == GENPCREL ) {
         if (code->cpsym != (SYM *) 0 && (code->cpsym->segmt & TYPE) != TXT)
            warn("illegal pc relative reference");
         if (code->cnsym != (SYM *) 0 && (code->cnsym->segmt & TYPE) != TXT)
            warn("illegal pc relative reference");
         stype = ABS;
         code->cval -= dot.value;
         }
      if (stype == ABS) {
         if ( code->cpsym != 0)
            code->cval += code->cpsym->value;
         if ( code->cnsym != 0)
            code->cval -= code->cnsym->value;
         if ( code->cact == GENRELOC || code->cact == GENPCREL || code->cact == GENNRDSP)
            lenbits(code->cn, &code->cval);
         if ( code->cact != GENVALUE && code->cact != GENRLVAL)
            reverse(code->cn, &code->cval);
         fwrite(&code->cval, 1, code->cn / 8, stdout);
         continue;
         }
      if ( code->cpsym != 0  && code->cpsym->segmt != (EXT | UNK) )
         code->cval += code->cpsym->value;
      rtype = R_ADDRESS | R_ABS;
      switch ( code->cact ) {
      case GENRLVAL:
         rtype |= R_NUMBER;
         break;
      case GENIMVAL:
         rtype |= R_IMMED;
         reverse( code->cn, &code->cval );
         break;
      case GENRELOC:
         rtype |= R_DISPL;
         reverse( code->cn, &code->cval );
         }
      switch ( code->cn ) {
      case 8:
         rtype |= R_S_08;
         break;
      case 16:
         rtype |= R_S_16;
         break;
      case 32:
         rtype |= R_S_32;
         break;
         }
      r.r_vaddr = newdot;
      r.r_type = rtype;
      switch ( stype ) {
      case TXT:
         r.r_symndx = TXTINDX ;
         break;
      case DAT:
         r.r_symndx = DATINDX ;
         break;
      case BSS:
         r.r_symndx = BSSINDX ;
         break;
      case UNK:
         r.r_symndx = code->cpsym->index ;
         }
      fwrite(&code->cval, 1, code->cn / 8, stdout);
      *rptr++ = r;
      }
   dot.value = newdot += null;
   for ( ; null; null--)
      fwrite(&zip, sizeof zip, 1, stdout);
}

reverse(n, val)
   short n;
   long *val;
{
   unsigned short tx;
   union {
      long lng;
      unsigned char sht[4];
      } tmp;

   tmp.lng = *val;
   switch ( n ) {
   case 32:
      tx = tmp.sht[0];
      tmp.sht[0] = tmp.sht[3];
      tmp.sht[3] = tx;
      tx = tmp.sht[1];
      tmp.sht[1] = tmp.sht[2];
      tmp.sht[2] = tx;
      break;
   case 16:
      tx = tmp.sht[0];
      tmp.sht[0] = tmp.sht[1];
      tmp.sht[1] = tx;
      break;
      }
   *val = tmp.lng;
}

lenbits(n, val)
   short n;
   long *val;
{
   switch ( n ) {
   case 8:
      *val = (unsigned long) *val & 0x7f;
      break;
   case 16:
      *val = ((unsigned long) *val & 0x3fff) | 0x8000;
      break;
   case 32:
      *val = (unsigned long) *val | 0xc0000000;
      break;
      }
}

dumprel() {
   int i;
   RELOC *rp;

   for (i = txtrel+datrel, rp = rbuf; i; i--, rp++)
      fwrite(rp, RELSZ, 1, stdout);
}
/* functions having to do with line numbers */
LNNO *lines = (LNNO *) 0;

add_lnno(addr,lnno)
   int addr, lnno;
{
   LNNO *p;

   p = (LNNO *) allocate(LNNOSIZE);
   if ( lines ) {
      p->link = lines->link;
      lines->link = p;
      lines = p;
      }
   else
      lines = p->link = p;
   p->no = lnno;
   p->ad = addr;
   Lnno_cnt++;
}

dumplnno() {
   struct lineno l;
   LNNO *p;
   int i;

   if (! lines)
      return;
   p = lines->link;
   lines->link = (LNNO *) 0;
   for ( i = 0; p; p = p->link, i += LINESZ) {
      l.l_lnno = p->no;
      if ( p->no )
         l.l_addr.l_paddr = p->ad;
      else {
         l.l_addr.l_symndx = ((SYM *) p->ad)->index;
         ((SYM *) p->ad)->aux->a_m.a_fn.loff = Lnno_base + i;
         }
      fwrite(&l,LINESZ,1,stdout);
      }
}

fix_lnno(addr,incr)
   int addr, incr;
{
   register LNNO *p;

   if ( p = lines )
      do {
         if ( p->no && p->ad >= addr )
            p->ad += incr;
         p = p->link;
      } while ( p != lines );
}
STR *strs = (STR *) 0;
Str_size = 4;

int add_str(s)
   char *s;
{
   STR *p;
   int i = Str_size;

   p = (STR *) allocate(STRSIZE);
   p->s_str = s;
   if ( strs ) {
      p->s_lnk = strs->s_lnk;
      strs->s_lnk = p;
      strs = p;
      }
   else
      strs = p->s_lnk = p;
   Str_size += 1 + strlen(s);
   return i;
}

dumpstr() {
   STR *s;

   fwrite(&Str_size,4,1,stdout);
   if ( Str_size == 4 )
      return;
   s = strs->s_lnk;
   strs->s_lnk = (STR *) 0;
   for ( ; s; s = s->s_lnk) {
      fwrite(s->s_str,1,1+strlen(s->s_str),stdout);
      }
}
